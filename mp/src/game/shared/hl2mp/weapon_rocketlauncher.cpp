#include "cbase.h"
#include "weapon_rocketlauncher.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "explode.h"

	#define RPG_DAMAGE 150
	#define RPG_RADIUS 200
#endif

#ifndef CLIENT_DLL
BEGIN_DATADESC( CProjectileRocket )
	DEFINE_FUNCTION( RocketTouch ),
	DEFINE_THINKFUNC( FlyThink ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CProjectileRocket, DT_ProjectileRocket )
	SendPropVector( SENDINFO( m_vInitialVelocity ), 20,	0, -3000, 3000 )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( rpg_missile, CProjectileRocket );
PRECACHE_WEAPON_REGISTER( rpg_missile );

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CProjectileRocket::CProjectileRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CProjectileRocket::~CProjectileRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectileRocket::Precache( void )
{
	PrecacheScriptSound( "Missile.Ignite" );	
	PrecacheModel( "models/weapons/w_missile_launch.mdl" );
	PrecacheModel( "models/weapons/w_missile.mdl" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectileRocket::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	SetModel( "models/weapons/w_missile_launch.mdl" );

	UTIL_SetSize( this, -Vector( 2,2,2 ), Vector( 2,2,2 ) );

	SetTouch( &CProjectileRocket::RocketTouch );

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );
	
	m_takedamage = DAMAGE_NO;
	SetGravity( 0.1 );
	SetDamage( RPG_DAMAGE );	

	AddFlag( FL_OBJECT );

	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );

	EmitSound( "Missile.Ignite" );

	SetThink( &CProjectileRocket::FlyThink );
	SetNextThink( gpGlobals->curtime );
}

//-----------------------------------------------------------------------------
// Purpose: Stops any kind of tracking and shoots dumb
//-----------------------------------------------------------------------------
void CProjectileRocket::Fire( void )
{
	SetThink( NULL );
	SetMoveType( MOVETYPE_FLY );

	SetModel( "models/weapons/w_missile.mdl" );
	UTIL_SetSize( this, vec3_origin, vec3_origin );

	EmitSound( "Missile.Ignite" );
}

//-----------------------------------------------------------------------------
// The actual explosion 
//-----------------------------------------------------------------------------
void CProjectileRocket::DoExplosion( trace_t *pTrace )
{
	// Pull out of the wall a bit
	if ( pTrace->fraction != 1.0 )
		SetAbsOrigin( pTrace->endpos + (pTrace->plane.normal * 0.6) );

	// Explosion effect on client
	Vector vecOrigin = GetAbsOrigin();
	ExplosionCreate( 
		vecOrigin,
		GetAbsAngles(),
		GetOwnerEntity(),
		RPG_DAMAGE,				//magnitude
		RPG_RADIUS,				//radius
		SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE,
		0.0f,					//explosion force
		this );					//inflictor

	CTakeDamageInfo info( this, GetOwnerEntity(), vec3_origin, GetAbsOrigin(), GetDamage(), DMG_BLAST, 0 );
	RadiusDamage( info, vecOrigin, RPG_RADIUS, CLASS_NONE, NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectileRocket::Explode( void )
{
	// Don't explode against the skybox. Just pretend that 
	// the missile flies off into the distance.
	const trace_t &tr = CBaseEntity::GetTouchTrace();
	const trace_t *p = &tr;
	trace_t *newTrace = const_cast<trace_t*>(p);

	DoExplosion( newTrace );

	if ( newTrace->m_pEnt && !newTrace->m_pEnt->IsPlayer() )
		UTIL_DecalTrace( newTrace, "Scorch" );

	StopSound( "Missile.Ignite" );
	UTIL_Remove( this );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CProjectileRocket::RocketTouch( CBaseEntity *pOther )
{
	Assert( pOther );
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	if ( pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
		return;

	// if we hit the skybox, just disappear
	const trace_t &tr = CBaseEntity::GetTouchTrace();

	const trace_t *p = &tr;
	trace_t *newTrace = const_cast<trace_t*>(p);

	if( tr.surface.flags & SURF_SKY )
	{
		UTIL_Remove( this );
		return;
	}

	if( !pOther->IsPlayer() && pOther->m_takedamage == DAMAGE_YES )
	{
		CTakeDamageInfo info;
		info.SetAttacker( this );
		info.SetInflictor( this );
		info.SetDamage( 50 );
		info.SetDamageForce( vec3_origin );	// don't worry about this not having a damage force.
											// It will explode on touch and impart its own forces
		info.SetDamageType( DMG_CLUB );

		Vector dir;
		AngleVectors( GetAbsAngles(), &dir );

		pOther->DispatchTraceAttack( info, dir, newTrace );
		ApplyMultiDamage();

		if( pOther->IsAlive() )
		{
			Explode();
		}

		// if it's not alive, continue flying
	}
	else
	{
		Explode();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CProjectileRocket::FlyThink( void )
{
	QAngle angles;

	VectorAngles( GetAbsVelocity(), angles );

	SetAbsAngles( angles );
	
	SetNextThink( gpGlobals->curtime + 0.1f );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//
// Input  : &vecOrigin - 
//			&vecAngles - 
//			NULL - 
//
// Output : CProjectileRocket
//-----------------------------------------------------------------------------
CProjectileRocket *CProjectileRocket::Create( const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL )
{
	CProjectileRocket *pMissile = (CProjectileRocket *) CBaseEntity::Create( szClassname, vecOrigin, vecAngles, pOwner );
	pMissile->SetOwnerEntity( pOwner );
	pMissile->Spawn();
	
	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );

	Vector vRocket = vecForward * 1300;

	pMissile->SetAbsVelocity( vRocket );	
	pMissile->SetupInitialTransmittedGrenadeVelocity( vRocket );

	pMissile->SetAbsAngles( vecAngles );

	return pMissile;
}

void CProjectileRocket::SetupInitialTransmittedGrenadeVelocity( const Vector &velocity )
{
	m_vInitialVelocity = velocity;
}

#endif

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRocketLauncher, DT_BaseRocketWeapon )

BEGIN_NETWORK_TABLE( CWeaponRocketLauncher, DT_BaseRocketWeapon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponRocketLauncher )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_rocketlauncher, CWeaponRocketLauncher );
PRECACHE_WEAPON_REGISTER( weapon_rocketlauncher );

acttable_t	CWeaponRocketLauncher::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_RPG,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_RPG,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_RPG,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_RPG,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_RPG,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_RPG,					false },
};

IMPLEMENT_ACTTABLE( CWeaponRocketLauncher );

CWeaponRocketLauncher::CWeaponRocketLauncher()
{
}

void CWeaponRocketLauncher::Precache()
{
	BaseClass::Precache();
}

void CWeaponRocketLauncher::PrimaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer( GetPlayerOwner() );
	
	// Out of ammo?
	if ( m_iClip1 <= 0 )
	{
		if (m_bFireOnEmpty)
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
		}

		return;
	}

	// player "shoot" animation
	ToHL2MPPlayer( pPlayer )->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
		
	FireRocket();
	DoFireEffects();

	m_iClip1--; 

	if ( m_iClip1 <= 0 && pPlayer->GetAmmoCount( GetPrimaryAmmoType() ) <= 0 )
		Lower();

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration() + 0.5;
	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration() + 0.5;	//length of the fire anim!
}

void CWeaponRocketLauncher::FireRocket( void )
{
	Assert( !"Derived classes must implement this." );
}

void CWeaponRocketLauncher::DoFireEffects()
{
	CBasePlayer *pPlayer = GetPlayerOwner();
	
	if ( pPlayer )
		 pPlayer->DoMuzzleFlash();

	//smoke etc
}

void CWeaponRocketLauncher::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > gpGlobals->curtime )
		return;

	SendWeaponAnim( ACT_VM_IDLE );

	m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();
}