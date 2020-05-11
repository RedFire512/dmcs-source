//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Definitions that are shared by the game DLL and the client DLL.
//
//=============================================================================//
#include "cbase.h"
#include "hl2_shareddefs.h"

//-----------------------------------------------------------------------------
// Ammo.
//-----------------------------------------------------------------------------
const char* g_aAmmoNames[] =
{
	"AMMO_NONE",
	"AMMO_SHELLS",
	"AMMO_NAILS",
	"AMMO_ROCKETS",
	"AMMO_NAILS",
	"AMMO_LAST"
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *GetAmmoName( int iAmmoType )
{
	DMCAmmoTypes eAmmoType = (DMCAmmoTypes)iAmmoType;
	return g_aAmmoNames[ eAmmoType ];
}

//-----------------------------------------------------------------------------
// Weapons.
//-----------------------------------------------------------------------------
const char *g_aWeaponNames[] =
{
	"WEAPON_NONE",

	"WEAPON_AXE",
	"WEAPON_CROWBAR",
	"WEAPON_SHOTGUN",
	"WEAPON_SUPERSHOTGUN",
	"WEAPON_NAILGUN",
	"WEAPON_SUPERNAILGUN",
	"WEAPON_ROCKETLAUNCHER",
	"WEAPON_BFG",
	"WEAPON_GRENADELAUNCHER",
	"WEAPON_LIGHTNING",
};

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int GetWeaponId( const char *pszWeaponName )
{
	// if this doesn't match, you need to add missing weapons to the array
	COMPILE_TIME_ASSERT( WEAPON_LAST == ARRAYSIZE( g_aWeaponNames ) );

	for ( int iWeapon = 0; iWeapon < ARRAYSIZE( g_aWeaponNames ); ++iWeapon )
	{
		if ( !Q_stricmp( pszWeaponName, g_aWeaponNames[iWeapon] ) )
			return iWeapon;
	}

	return WEAPON_NONE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
const char *WeaponIdToAlias( int iWeapon )
{
	// if this doesn't match, you need to add missing weapons to the array
	COMPILE_TIME_ASSERT( WEAPON_LAST == ARRAYSIZE( g_aWeaponNames ) );

	if ( ( iWeapon >= ARRAYSIZE( g_aWeaponNames ) ) || ( iWeapon < 0 ) )
		return NULL;

	return g_aWeaponNames[iWeapon];
}