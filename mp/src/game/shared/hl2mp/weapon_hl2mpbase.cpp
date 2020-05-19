//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "takedamageinfo.h"
#include "ammodef.h"
#include "hl2mp_gamerules.h"
#include "weapon_hl2mpbase.h"

#if defined( CLIENT_DLL )
	#include "c_hl2mp_player.h"
	#include "dmc/quake3bob.h"

	ConVar glow_outline_color_red( "glow_outline_color_red", "255", FCVAR_ARCHIVE );
	ConVar glow_outline_color_green( "glow_outline_color_green", "255", FCVAR_ARCHIVE );
	ConVar glow_outline_color_blue( "glow_outline_color_blue", "255", FCVAR_ARCHIVE );
	extern ConVar glow_outline_effect_enable;
#else
	#include "hl2mp_player.h"
	#include "vphysics/constraints.h"
#endif

// ----------------------------------------------------------------------------- //
// Global functions.
// ----------------------------------------------------------------------------- //
bool IsAmmoType( int iAmmoType, const char *pAmmoName )
{
	return GetAmmoDef()->Index( pAmmoName ) == iAmmoType;
}

// ----------------------------------------------------------------------------- //
// CWeaponHL2MPBase tables.
// ----------------------------------------------------------------------------- //
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponHL2MPBase, DT_WeaponHL2MPBase )

BEGIN_NETWORK_TABLE( CWeaponHL2MPBase, DT_WeaponHL2MPBase )
#if !defined( CLIENT_DLL )
	SendPropBool( SENDINFO( m_bQuake3Bob ) ),
	SendPropVector( SENDINFO( m_vOriginalSpawnOrigin ) ),
	SendPropVector( SENDINFO( m_vOriginalSpawnAngles ) ),
#else
	RecvPropBool( RECVINFO( m_bQuake3Bob ) ),
	RecvPropVector( RECVINFO( m_vOriginalSpawnOrigin ) ),
	RecvPropVector( RECVINFO( m_vOriginalSpawnAngles ) ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponHL2MPBase ) 
END_PREDICTION_DATA()

#ifdef GAME_DLL
BEGIN_DATADESC( CWeaponHL2MPBase )
END_DATADESC()
#endif

// ----------------------------------------------------------------------------- //
// CWeaponHL2MPBase implementation. 
// ----------------------------------------------------------------------------- //
CWeaponHL2MPBase::CWeaponHL2MPBase()
{
	SetPredictionEligible( true );
	AddSolidFlags( FSOLID_TRIGGER ); // Nothing collides with these but it gets touches.

	m_bQuake3Bob = true;

	m_flNextResetCheckTime = 0.0f;
	m_bFiresUnderwater = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::Spawn()
{
	BaseClass::Spawn();

	// Set this here to allow players to shoot dropped weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	
	m_vOriginalSpawnOrigin = GetAbsOrigin();
	m_vOriginalSpawnAngles = GetAbsAngles();

#ifdef CLIENT_DLL
	// For Q3A Item bobbing, make starting angle random.
	ClientRotAng = { 0.0, FRand(0.0, 359.0), 0.0 };
	ClientThink();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Become a child of the owner (MOVETYPE_FOLLOW)
//			disables collisions, touch functions, thinking
// Input  : *pOwner - new owner/operator
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

#if !defined( CLIENT_DLL )
	RemoveSpawnFlags( SF_WEAPON_DISABLE_BOB );
	m_bQuake3Bob = false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: If the current weapon has more ammo, reload it. Otherwise, switch 
//			to the next best weapon we've got. Returns true if it took any action.
//-----------------------------------------------------------------------------
bool CWeaponHL2MPBase::ReloadOrSwitchWeapons( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	Assert( pOwner );

	m_bFireOnEmpty = false;

	// If we don't have any ammo, switch to the next best weapon
	if ( !HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime && m_flNextSecondaryAttack < gpGlobals->curtime )
	{
		// weapon isn't useable, switch.
		if ( ( (GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) == false ) && ( g_pGameRules->SwitchToNextBestWeapon( pOwner, this ) ) )
		{
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
			return true;
		}
	}

	return false;
}

void CWeaponHL2MPBase::WeaponSound( WeaponSound_t sound_type, float soundtime /* = 0.0f */ )
{
#ifdef CLIENT_DLL

	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char *shootsound = GetWpnData().aShootSounds[ sound_type ]; 
	if ( !shootsound || !shootsound[0] )
		return;

	CBroadcastRecipientFilter filter; // this is client side only
	if ( !te->CanPredict() )
		return;
				
	CBaseEntity::EmitSound( filter, GetPlayerOwner()->entindex(), shootsound, &GetPlayerOwner()->GetAbsOrigin() ); 
#else
	BaseClass::WeaponSound( sound_type, soundtime );
#endif
}

CBasePlayer* CWeaponHL2MPBase::GetPlayerOwner() const
{
	return dynamic_cast< CBasePlayer* >( GetOwner() );
}

CHL2MP_Player* CWeaponHL2MPBase::GetHL2MPPlayerOwner() const
{
	return dynamic_cast< CHL2MP_Player* >( GetOwner() );
}

#ifdef CLIENT_DLL
	
void CWeaponHL2MPBase::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( GetPredictable() && !ShouldPredict() )
		ShutdownPredictable();
}


bool CWeaponHL2MPBase::ShouldPredict()
{
	if ( GetOwner() && GetOwner() == C_BasePlayer::GetLocalPlayer() )
		return true;

	return BaseClass::ShouldPredict();
}

#else
//-----------------------------------------------------------------------------
// Purpose: Make the weapon visible and tangible
//-----------------------------------------------------------------------------
CBaseEntity* CWeaponHL2MPBase::Respawn( void )
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CWeaponHL2MPBase *pNewWeapon = (CWeaponHL2MPBase *)CBaseEntity::Create( GetClassname(), g_pGameRules->VecWeaponRespawnSpot( this ), GetLocalAngles(), GetOwnerEntity() );

	if ( pNewWeapon )
	{
		pNewWeapon->AddEffects( EF_NODRAW );// invisible for now
		pNewWeapon->SetTouch( NULL );// no touch
		pNewWeapon->SetThink( &CBaseCombatWeapon::AttemptToMaterialize );
		pNewWeapon->AddSpawnFlags( GetSpawnFlags() );
		pNewWeapon->SetupPhysics();

		if ( !HasSpawnFlags( SF_WEAPON_START_CONSTRAINED ) && HasSpawnFlags( SF_WEAPON_DISABLE_BOB ) )
			UTIL_DropToFloor( this, MASK_SOLID );

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->SetNextThink( gpGlobals->curtime + g_pGameRules->FlWeaponRespawnTime( this ) );
	}
	else
	{
		Warning( "Respawn failed to create %s!\n", GetClassname() );
	}

	return (CBaseEntity *)pNewWeapon;
}

void CWeaponHL2MPBase::Materialize( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound( "AlyxEmp.Charge" );
		
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		SetupPhysics();
		HL2MPRules()->AddLevelDesignerPlacedObject( this );
	}

	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		if ( GetOriginalSpawnOrigin() == vec3_origin )
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}

	SetPickupTouch();

	SetThink (NULL);
}

int CWeaponHL2MPBase::ObjectCaps()
{
	return BaseClass::ObjectCaps() & ~FCAP_IMPULSE_USE;
}

#endif

void CWeaponHL2MPBase::FallInit( void )
{
#ifndef CLIENT_DLL
	SetModel( GetWorldModel() );

	SetupPhysics();

	SetPickupTouch();
	
	SetThink( &CWeaponHL2MPBase::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Set up physics for weapon
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::SetupPhysics( void )
{
	VPhysicsDestroyObject();

	if ( !HasSpawnFlags( SF_WEAPON_DISABLE_BOB ) )
	{
		//m_bQuake3Bob = true;

		VPhysicsDestroyObject();
		SetMoveType( MOVETYPE_NONE );
		SetSolid( SOLID_NONE );
		return;
	}

	m_bQuake3Bob = false;

	if ( !VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false ) )
	{
		SetMoveType( MOVETYPE_FLYGRAVITY );
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_TRIGGER );
	}
	else
	{
		SetMoveType( MOVETYPE_VPHYSICS );

		// Constrained start?
		if ( HasSpawnFlags( SF_WEAPON_START_CONSTRAINED ) )
		{
			//Constrain the weapon in place
			IPhysicsObject *pReferenceObject, *pAttachedObject;

			pReferenceObject = g_PhysWorldObject;
			pAttachedObject = VPhysicsGetObject();

			if ( pReferenceObject && pAttachedObject )
			{
				constraint_fixedparams_t fixed;
				fixed.Defaults();
				fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );

				fixed.constraint.forceLimit = lbs2kg( 10000 );
				fixed.constraint.torqueLimit = lbs2kg( 10000 );

				m_pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

				m_pConstraint->SetGameData( (void *) this );
			}
		}
	}
}

void CWeaponHL2MPBase::FallThink( void )
{
	// Prevent the common HL2DM weapon respawn bug from happening
	// When a weapon is spawned, the following chain of events occurs:
	// - Spawn() is called (duh), which then calls FallInit()
	// - FallInit() is called, and prepares the weapon's 'Think' function (CBaseCombatWeapon::FallThink())
	// - FallThink() is called, and performs several checks before deciding whether the weapon should Materialize()
	// - Materialize() is called (the HL2DM version above), which sets the weapon's respawn location.
	// The problem occurs when a weapon isn't placed properly by a level designer.
	// If the weapon is unable to move from its location (e.g. if its bounding box is halfway inside a wall), Materialize() never gets called.
	// Since Materialize() never gets called, the weapon's respawn location is never set, so if a person picks it up, it respawns forever at
	// 0 0 0 on the map (infinite loop of fall, wait, respawn, not nice at all for performance and bandwidth!)
	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		if ( GetOriginalSpawnOrigin() == vec3_origin )
		{
			m_vOriginalSpawnOrigin = GetAbsOrigin();
			m_vOriginalSpawnAngles = GetAbsAngles();
		}
	}

	return BaseClass::FallThink();
}
#endif // GAME_DLL

const CHL2MPSWeaponInfo &CWeaponHL2MPBase::GetHL2MPWpnData() const
{
	const FileWeaponInfo_t *pWeaponInfo = &GetWpnData();
	const CHL2MPSWeaponInfo *pHL2MPInfo;

#ifdef _DEBUG
	pHL2MPInfo = dynamic_cast< const CHL2MPSWeaponInfo* >( pWeaponInfo );
	Assert( pHL2MPInfo );
#else
	pHL2MPInfo = static_cast< const CHL2MPSWeaponInfo* >( pWeaponInfo );
#endif

	return *pHL2MPInfo;
}

int CWeaponHL2MPBase::GetWeaponID( void ) const
{
	Assert( !"Derived classes must implement this." );
	return WEAPON_NONE;
}

bool CWeaponHL2MPBase::IsWeapon( int iWeapon ) const
{
	return GetWeaponID() == iWeapon;
}

void CWeaponHL2MPBase::FireBullets( const FireBulletsInfo_t &info )
{
	FireBulletsInfo_t modinfo = info;

	modinfo.m_iPlayerDamage = GetHL2MPWpnData().m_iPlayerDamage;

	BaseClass::FireBullets( modinfo );
}

#if defined( CLIENT_DLL )
extern float g_lateralBob;
extern float g_verticalBob;

static ConVar	cl_bobcycle( "cl_bobcycle","0.8", FCVAR_CHEAT );
static ConVar	cl_bob( "cl_bob","0.002", FCVAR_CHEAT );
static ConVar	cl_bobup( "cl_bobup","0.5", FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose:
// Output : float
//-----------------------------------------------------------------------------
float CWeaponHL2MPBase::CalcViewmodelBob( void )
{
	static	float bobtime;
	static	float lastbobtime;
	static  float lastspeed;
	float	cycle;

	CBasePlayer *player = ToBasePlayer( GetOwner() );
	//Assert( player );

	//NOTENOTE: For now, let this cycle continue when in the air, because it snaps badly without it

	if ( ( !gpGlobals->frametime ) ||
			( player == NULL ) ||
			( cl_bobcycle.GetFloat() <= 0.0f ) ||
			( cl_bobup.GetFloat() <= 0.0f ) ||
			( cl_bobup.GetFloat() >= 1.0f ) )
	{
		//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
		return 0.0f;// just use old value
	}

	//Find the speed of the player
	float speed = player->GetLocalVelocity().Length2D();
	float flmaxSpeedDelta = max( 0.f, (gpGlobals->curtime - lastbobtime) * 320.0f );

	// don't allow too big speed changes
	speed = clamp( speed, lastspeed-flmaxSpeedDelta, lastspeed+flmaxSpeedDelta );
	speed = clamp( speed, -320, 320 );

	lastspeed = speed;

	//FIXME: This maximum speed value must come from the server.
	//		 MaxSpeed() is not sufficient for dealing with sprinting - jdw

	float bob_offset = RemapVal( speed, 0, 320, 0.0f, 1.0f );

	bobtime += ( gpGlobals->curtime - lastbobtime ) * bob_offset;
	lastbobtime = gpGlobals->curtime;

	//Calculate the vertical bob
	cycle = bobtime - (int)(bobtime/cl_bobcycle.GetFloat())*cl_bobcycle.GetFloat();
	cycle /= cl_bobcycle.GetFloat();

	if ( cycle < cl_bobup.GetFloat() )
		cycle = M_PI * cycle / cl_bobup.GetFloat();
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup.GetFloat())/(1.0 - cl_bobup.GetFloat());

	g_verticalBob = speed*0.005f;
	g_verticalBob = g_verticalBob*0.3 + g_verticalBob*0.7*sin(cycle);

	g_verticalBob = clamp( g_verticalBob, -7.0f, 4.0f );

	//Calculate the lateral bob
	cycle = bobtime - (int)(bobtime/cl_bobcycle.GetFloat()*2)*cl_bobcycle.GetFloat()*2;
	cycle /= cl_bobcycle.GetFloat()*2;

	if ( cycle < cl_bobup.GetFloat() )
		cycle = M_PI * cycle / cl_bobup.GetFloat();
	else
		cycle = M_PI + M_PI*(cycle-cl_bobup.GetFloat())/(1.0 - cl_bobup.GetFloat());

	g_lateralBob = speed*0.005f;
	g_lateralBob = g_lateralBob*0.3 + g_lateralBob*0.7*sin(cycle);
	g_lateralBob = clamp( g_lateralBob, -7.0f, 4.0f );

	//NOTENOTE: We don't use this return value in our case (need to restructure the calculation function setup!)
	return 0.0f;

}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : &origin -
//			&angles -
//			viewmodelindex -
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles )
{
	Vector	forward, right;
	AngleVectors( angles, &forward, &right, NULL );

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA( origin, g_verticalBob * 0.4f, forward, origin );

	// Z bob a bit more
	origin[2] += g_verticalBob * 0.1f;

	// bob the angles
	angles[ ROLL ]	+= g_verticalBob * 0.5f;
	angles[ PITCH ]	-= g_verticalBob * 0.4f;

	angles[ YAW ]	-= g_lateralBob  * 0.3f;

	//VectorMA( origin, g_lateralBob * 0.2f, right, origin );
}

//-----------------------------------------------------------------------------
// Purpose: Quake 3 bobbing for weapons
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::ClientThink()
{
	if ( IsAbsQueriesValid() && m_bQuake3Bob )
	{
		// Rotate
		Quake3Rotate( this, ClientRotAng );
		Quake3Bob( this, m_vOriginalSpawnOrigin );
	}

	UpdateGlow();

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::UpdateGlow()
{
    if ( !glow_outline_effect_enable.GetBool() )
    {
        if ( IsClientSideGlowEnabled() )
            SetClientSideGlowEnabled( false );

        return;
    }

	C_HL2MP_Player* pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
    if ( pPlayer && GetOwner() == nullptr && pPlayer->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) < ITEM_GLOW_DIST_SQR )
    {
        if ( !IsClientSideGlowEnabled() )
            SetClientSideGlowEnabled( true );
    }
    else
    {
        SetClientSideGlowEnabled( false );
    }
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponHL2MPBase::GetGlowEffectColor( float& r, float& g, float& b )
{
	r = glow_outline_color_red.GetFloat();
	g = glow_outline_color_green.GetFloat();
	b = glow_outline_color_blue.GetFloat();
}

void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip )
{
	QAngle	final = in + punch;

	//Clip each component
	for ( int i = 0; i < 3; i++ )
	{
		if ( final[i] > clip[i] )
			final[i] = clip[i];
		else if ( final[i] < -clip[i] )
			final[i] = -clip[i];

		//Return the result
		in[i] = final[i] - punch[i];
	}
}

#endif

