//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "NPCEvent.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"
#include "Sprite.h"
#include "Beam_Shared.h"
#include "Takedamageinfo.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
	#include "c_te_effect_dispatch.h"

	#define CWeaponLightning C_WeaponLightning
#else
	#include "player.h"
	#include "hl2mp_player.h"
	#include "soundent.h"
	#include "game.h"
	#include "te_effect_dispatch.h"
#endif

enum EFireState 
{ 
	FIRE_OFF,
	FIRE_STARTUP, 
	FIRE_CHARGE 
};

#define LIGHTGUN_PULSE_INTERVAL			0.1
#define LIGHTGUN_DISCHARGE_INTERVAL		0.1
#define LIGHTGUN_BEAM_SPRITE		"sprites/xbeam1.vmt"
#define LIGHTGUN_FLARE_SPRITE		"sprites/XSpark1.vmt"

//-----------------------------------------------------------------------------
// CWeaponLightning
//-----------------------------------------------------------------------------
class CWeaponLightning : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponLightning, CBaseHL2MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

    CWeaponLightning( void );

	virtual int GetWeaponID( void ) const { return WEAPON_LIGHTNING; }

	virtual bool Deploy( void );
	virtual void PrimaryAttack( void );
    virtual void Precache( void );
	virtual void WeaponIdle( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	virtual bool HasAmmo( void );
	virtual void UseAmmo( int count );
	
	void Attack( void );
	void EndAttack( void );
	void Fire( const Vector &vecOrigSrc, const Vector &vecDir );
	void UpdateEffect( const Vector &startPoint, const Vector &endPoint );
	void CreateEffect( void );
	void DestroyEffect( void );

	EFireState m_fireState;

	float m_flAmmoUseTime;	// since we use < 1 point of ammo per update, we subtract ammo on a timer.
	float m_flShakeTime;
	float m_flStartFireTime;
	float m_flDmgTime;

	CHandle<CSprite> m_hSprite;
	CHandle<CBeam> m_hBeam;
	CHandle<CBeam> m_hNoise;
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponLightning, DT_WeaponLightning );

BEGIN_NETWORK_TABLE( CWeaponLightning, DT_WeaponLightning )
END_NETWORK_TABLE()    

BEGIN_PREDICTION_DATA( CWeaponLightning )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_lightning, CWeaponLightning );
PRECACHE_WEAPON_REGISTER( weapon_lightning );

acttable_t	CWeaponLightning::m_acttable[] =
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_PHYSGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_PHYSGUN,			false },
	{ ACT_MP_RUN,						ACT_HL2MP_RUN_PHYSGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_PHYSGUN,			false },
	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,	false },
	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,		false },
	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_PHYSGUN,					false },
};

IMPLEMENT_ACTTABLE( CWeaponLightning );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponLightning::CWeaponLightning( void )
{
	m_bReloadsSingly = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponLightning::Precache( void )
{
    //PrecacheScriptSound( "Weapon_Lightning.Loop" );

	PrecacheModel( LIGHTGUN_BEAM_SPRITE );
	PrecacheModel( LIGHTGUN_FLARE_SPRITE );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CWeaponLightning::Deploy( void )
{
	m_fireState = FIRE_OFF;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CWeaponLightning::HasAmmo( void )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	if ( !pPlayer )
		return false;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponLightning::UseAmmo( int count )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	if ( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) >= count )
		pPlayer->RemoveAmmo( count, m_iPrimaryAmmoType );
	else
		pPlayer->RemoveAmmo( pPlayer->GetAmmoCount( m_iPrimaryAmmoType ), m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponLightning::PrimaryAttack( void )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	Vector vecAiming	= pPlayer->GetAutoaimVector( 0 );
	Vector vecSrc		= pPlayer->Weapon_ShootPosition( );

	switch( m_fireState )
	{
		case FIRE_OFF:
		{
			if ( !HasAmmo() )
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.25;
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.25;
				WeaponSound( EMPTY );
				return;
			}

			m_flAmmoUseTime = gpGlobals->curtime;// start using ammo ASAP.      

			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
						
			m_flShakeTime = 0;
			m_flStartFireTime = gpGlobals->curtime;

			SetWeaponIdleTime( gpGlobals->curtime + 0.1 );

			m_flDmgTime = gpGlobals->curtime + LIGHTGUN_PULSE_INTERVAL;
			m_fireState = FIRE_STARTUP;
		}
		break;

		case FIRE_STARTUP:
		{
			Fire( vecSrc, vecAiming );
		
			if ( gpGlobals->curtime >= ( m_flStartFireTime + 2.0 ) )
			{
                //EmitSound( "Weapon_Lightning.Loop" );

				m_fireState = FIRE_CHARGE;
			}

			if ( !HasAmmo() )
			{
				EndAttack();
				m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
				m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
			}
		}
		case FIRE_CHARGE:
		{
			Fire( vecSrc, vecAiming );
		
			if ( !HasAmmo() )
			{
				EndAttack();
				m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
				m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
			}
		}
		break;
	}
}

void CWeaponLightning::Fire( const Vector &vecOrigSrc, const Vector &vecDir )
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	if ( !pPlayer )
		return;

    WeaponSound( SINGLE );

	Vector vecDest	= vecOrigSrc + (vecDir * MAX_TRACE_LENGTH);

	trace_t	tr;
	UTIL_TraceLine( vecOrigSrc, vecDest, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.allsolid )
		return;

	CBaseEntity *pEntity = tr.m_pEnt;
	if ( pEntity == NULL )
		return;

	if ( m_hSprite )
	{
		if ( pEntity->m_takedamage != DAMAGE_NO )
			m_hSprite->TurnOn();
		else
			m_hSprite->TurnOff();
	}

	if ( m_flDmgTime < gpGlobals->curtime )
	{
		// wide mode does damage to the ent, and radius damage
		if ( pEntity->m_takedamage != DAMAGE_NO )
		{
			ClearMultiDamage();
			CTakeDamageInfo info( this, pPlayer, 7.0f * g_pGameRules->GetDamageMultiplier(), DMG_SHOCK | DMG_DISSOLVE );
			CalculateMeleeDamageForce( &info, vecDir, tr.endpos );
			pEntity->DispatchTraceAttack( info, vecDir, &tr );
			ApplyMultiDamage();
		}

		// do the water damage.
		// todo: figure how to do it
#ifndef CLIENT_DLL
		if ( pPlayer->IsPlayerUnderwater() )
			RadiusDamage( CTakeDamageInfo( this, pPlayer, 18.0f * g_pGameRules->GetDamageMultiplier() / 4, DMG_ENERGYBEAM | DMG_DISSOLVE ), tr.endpos, 128, CLASS_NONE, NULL );
#endif

		if ( !pPlayer->IsAlive() )
			return;

		if ( gpGlobals->curtime >= m_flAmmoUseTime )
		{
			if ( !pPlayer->IsPlayerUnderwater() )
			{ 
				// use 1 ammo/second
				UseAmmo( 1 );
				m_flAmmoUseTime = gpGlobals->curtime + 0.1;	
			}
			else
			{
				// use 19 ammo/second
				UseAmmo( 19 );
				m_flAmmoUseTime = gpGlobals->curtime /*+ 1.9*/;	
			}
		}

		m_flDmgTime = gpGlobals->curtime + LIGHTGUN_DISCHARGE_INTERVAL;

		if ( m_flShakeTime < gpGlobals->curtime )
		{
#ifndef CLIENT_DLL
			UTIL_ScreenShake( tr.endpos, 5.0, 150.0, 0.75, 250.0, SHAKE_START );
#endif
			m_flShakeTime = gpGlobals->curtime + 1.5;
		}
	}

	Vector vecUp, vecRight;
	QAngle angDir;

	VectorAngles( vecDir, angDir );
	AngleVectors( angDir, NULL, &vecRight, &vecUp );

	Vector tmpSrc = vecOrigSrc + (vecUp * -8) + (vecRight * 3);
	UpdateEffect( tmpSrc, tr.endpos );
}

void CWeaponLightning::UpdateEffect( const Vector &startPoint, const Vector &endPoint )
{
	if ( !m_hBeam )
		CreateEffect();

	if ( m_hBeam )
		m_hBeam->SetStartPos( endPoint );

	if ( m_hSprite )
	{
		m_hSprite->SetAbsOrigin( endPoint );

		m_hSprite->m_flFrame += 8 * gpGlobals->frametime;
		if ( m_hSprite->m_flFrame > m_hSprite->Frames() )
			m_hSprite->m_flFrame = 0;
	}

	if ( m_hNoise )
		m_hNoise->SetStartPos( endPoint );
}

void CWeaponLightning::CreateEffect( void )
{
#ifndef CLIENT_DLL    
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	if ( !pPlayer )
		return;


	DestroyEffect();

	m_hBeam = CBeam::BeamCreate( LIGHTGUN_BEAM_SPRITE, 3.5 );
	m_hBeam->PointEntInit( GetAbsOrigin(), this );
	m_hBeam->SetBeamFlags( FBEAM_SINENOISE );
	m_hBeam->SetEndAttachment( 1 );
	m_hBeam->AddSpawnFlags( SF_BEAM_TEMPORARY );	// Flag these to be destroyed on save/restore or level transition
	m_hBeam->SetOwnerEntity( pPlayer );
	m_hBeam->SetScrollRate( 10 );
	m_hBeam->SetBrightness( 200 );
	m_hBeam->SetColor( 50, 50, 255 );
	m_hBeam->SetNoise( 0.2 );

	m_hNoise = CBeam::BeamCreate( LIGHTGUN_BEAM_SPRITE, 5.0 );
	m_hNoise->PointEntInit( GetAbsOrigin(), this );
	m_hNoise->SetEndAttachment( 1 );
	m_hNoise->AddSpawnFlags( SF_BEAM_TEMPORARY );
	m_hNoise->SetOwnerEntity( pPlayer );
	m_hNoise->SetScrollRate( 25 );
	m_hNoise->SetBrightness( 200 );
	m_hNoise->SetColor( 50, 50, 255 );
	m_hNoise->SetNoise( 0.8 );

	m_hSprite = CSprite::SpriteCreate( LIGHTGUN_FLARE_SPRITE, GetAbsOrigin(), false );
	m_hSprite->SetScale( 1.0 );
	m_hSprite->SetTransparency( kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	m_hSprite->AddSpawnFlags( SF_SPRITE_TEMPORARY );
	m_hSprite->SetOwnerEntity( pPlayer );
#endif    
}

void CWeaponLightning::DestroyEffect( void )
{
#ifndef CLIENT_DLL    
	if ( m_hBeam )
	{
		UTIL_Remove( m_hBeam );
		m_hBeam = NULL;
	}

	if ( m_hNoise )
	{
		UTIL_Remove( m_hNoise );
		m_hNoise = NULL;
	}

	if ( m_hSprite )
	{
		m_hSprite->Expand( 10, 500 );
		m_hSprite = NULL;
	}
#endif
}

void CWeaponLightning::EndAttack( void )
{
    //StopSound( "Weapon_Lightning.Loop" );

	SetWeaponIdleTime( gpGlobals->curtime + 2.0 );
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5;

	m_fireState = FIRE_OFF;

	DestroyEffect();
}

bool CWeaponLightning::Holster( CBaseCombatWeapon *pSwitchingTo )
{
    EndAttack();

	return BaseClass::Holster( pSwitchingTo );
}

void CWeaponLightning::WeaponIdle( void )
{
	if ( !HasWeaponIdleTimeElapsed() )
		return;

	if ( m_fireState != FIRE_OFF )
		 EndAttack();
	
	int iAnim;

	float flRand = random->RandomFloat( 0,1 );
	float flIdleTime;
	if ( flRand <= 0.5 )
	{
		iAnim = ACT_VM_IDLE;
		flIdleTime = gpGlobals->curtime + random->RandomFloat( 10, 15 );
	}
	else 
	{
		iAnim = ACT_VM_FIDGET;
		flIdleTime = gpGlobals->curtime + 3.0;
	}

	SendWeaponAnim( iAnim );

	SetWeaponIdleTime( flIdleTime );
}