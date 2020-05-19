//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef WEAPON_SHOTGUN_H
#define WEAPON_SHOTGUN_H
#pragma once

#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifdef CLIENT_DLL
#define CWeaponShotgun C_WeaponShotgun
#endif

//-----------------------------------------------------------------------------
// CWeaponShotgun
//-----------------------------------------------------------------------------
class CWeaponShotgun : public CBaseHL2MPCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponShotgun, CBaseHL2MPCombatWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

public:
	CWeaponShotgun( void );

	virtual int GetWeaponID( void ) const { return WEAPON_SHOTGUN; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_10DEGREES;
		return cone;
	}

	virtual int AmmoPerShot() { return 1; }
	virtual int GetMinBurst() { return 1; }
	virtual int GetMaxBurst() { return 3; }
	virtual float GetFireRate( void ) { return 0.7; };

	void ItemHolsterFrame( void );
	void ItemPostFrame( void );
	void PrimaryAttack( void );
	void DryFire( void );

private:
	CWeaponShotgun( const CWeaponShotgun & );
};

#endif // WEAPON_SHOTGUN_H
