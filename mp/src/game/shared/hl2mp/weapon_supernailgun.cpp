//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "projectile_nail.h"
#endif

#ifdef CLIENT_DLL
	#define CWeaponSuperNailGun C_WeaponSuperNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponSuperNailGun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponSuperNailGun, CBaseHL2MPCombatWeapon );

	CWeaponSuperNailGun();

	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	virtual void Precache( void );
	virtual void PrimaryAttack();

private:
	CWeaponSuperNailGun( const CWeaponSuperNailGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSuperNailGun, DT_WeaponSuperNailGun )

BEGIN_NETWORK_TABLE( CWeaponSuperNailGun, DT_WeaponSuperNailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponSuperNailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_supernailgun, CWeaponSuperNailGun );
PRECACHE_WEAPON_REGISTER( weapon_supernailgun );

acttable_t	CWeaponSuperNailGun::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponSuperNailGun );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponSuperNailGun::CWeaponSuperNailGun()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::Precache (void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "projectile_nail" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSuperNailGun::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( !pPlayer )
		return;

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// player "shoot" animation
	ToHL2MPPlayer( pPlayer )->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	float fireRate = GetFireRate();

	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound( SINGLE, m_flNextPrimaryAttack );
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		if ( !fireRate )
			break;
	}

	// Create the nail.
	int iCurrentAmmoCount = pPlayer->GetAmmoCount( GetPrimaryAmmoType() );

#ifdef GAME_DLL
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	CProjectileNail *pNail = NULL;
	if ( iCurrentAmmoCount < 4 )
		 pNail = CProjectileNail::CreateNail( false, vecSrc, pPlayer->EyeAngles(), pPlayer, this, true );
	else
		pNail = CProjectileNail::CreateSuperNail( vecSrc, pPlayer->EyeAngles(), pPlayer, this );
#endif

	// Uses 2 nails if it can
	pPlayer->RemoveAmmo( MIN( 2, iCurrentAmmoCount ), GetPrimaryAmmoType() );

	//Add our view kick in
	AddViewKick();
}