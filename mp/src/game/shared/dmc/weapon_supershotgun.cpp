//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_shotgun.h"

#ifdef CLIENT_DLL
#define CWeaponSuperShotgun C_WeaponSuperShotgun
#endif

class CWeaponSuperShotgun : public CWeaponShotgun
{
public:
	DECLARE_CLASS( CWeaponSuperShotgun, CWeaponShotgun );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

public:

	CWeaponSuperShotgun( void );

	virtual int GetWeaponID( void ) const { return WEAPON_SUPERSHOTGUN; }

	virtual int AmmoPerShot() { return 2; } // Super Shotgun uses 2 bullets per shot

private:
	CWeaponSuperShotgun( const CWeaponSuperShotgun & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSuperShotgun, DT_WeaponSuperShotgun )

BEGIN_NETWORK_TABLE( CWeaponSuperShotgun, DT_WeaponSuperShotgun )
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponSuperShotgun )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_supershotgun, CWeaponSuperShotgun );
PRECACHE_WEAPON_REGISTER( weapon_supershotgun );

acttable_t	CWeaponSuperShotgun::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SHOTGUN,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SHOTGUN,			false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SHOTGUN,					false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SHOTGUN,			false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,		false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SHOTGUN,					false },
};

IMPLEMENT_ACTTABLE( CWeaponSuperShotgun );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponSuperShotgun::CWeaponSuperShotgun( void )
{
	m_fMinRange1		= 0.0;
	m_fMaxRange1		= 500;
}