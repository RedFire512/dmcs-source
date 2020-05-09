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
	#define CWeaponNailGun C_WeaponNailGun
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponNailGun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponNailGun, CBaseHL2MPCombatWeapon );

	CWeaponNailGun();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	virtual void Precache( void );
	virtual void PrimaryAttack();
	
private:
	CWeaponNailGun( const CWeaponNailGun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponNailGun, DT_WeaponNailGun )

BEGIN_NETWORK_TABLE( CWeaponNailGun, DT_WeaponNailGun )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponNailGun )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_nailgun, CWeaponNailGun );
PRECACHE_WEAPON_REGISTER( weapon_nailgun );

acttable_t	CWeaponNailGun::m_acttable[] =
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

IMPLEMENT_ACTTABLE( CWeaponNailGun );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponNailGun::CWeaponNailGun()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNailGun::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "projectile_nail" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponNailGun::PrimaryAttack( void )
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

	// Fire the nail
#ifdef GAME_DLL
	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	CProjectileNail *pNail = CProjectileNail::CreateNail(
		false, 
		vecSrc, 
		pPlayer->EyeAngles(),
		pPlayer,
		this, 
		true );
	pNail=pNail; // avoid compiler warning..
#endif

	//Add our view kick in
	AddViewKick();
}