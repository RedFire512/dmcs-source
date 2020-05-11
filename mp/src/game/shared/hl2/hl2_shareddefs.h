//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Definitions that are shared by the game DLL and the client DLL.
//
//=============================================================================//
#ifndef HL2_SHAREDDEFS_H
#define HL2_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "const.h"

//-----------------------------------------------------------------------------
// Gamerules.
//-----------------------------------------------------------------------------
#define DMCS_GAME_DESCRIPTION "Deathmatch Classic: Source"
#define DMCS_MAX_PLAYERS 128

//-----------------------------------------------------------------------------
// Ammo.
//-----------------------------------------------------------------------------
enum DMCAmmoTypes
{
	AMMO_DUMMY = 0,
	AMMO_SHELLS,
	AMMO_NAILS,
	AMMO_ROCKETS,
	AMMO_CELLS,
	AMMO_LAST
};

const char *GetAmmoName( int iAmmoType );

//-----------------------------------------------------------------------------
// Weapon names.
//-----------------------------------------------------------------------------
enum DMCWeaponIDs
{
	WEAPON_NONE = 0,

	WEAPON_AXE,
	WEAPON_CROWBAR,
	WEAPON_SHOTGUN,
	WEAPON_SUPERSHOTGUN,
	WEAPON_NAILGUN,
	WEAPON_SUPERNAILGUN,
	WEAPON_ROCKETLAUNCHER,
	WEAPON_BFG,
	WEAPON_GRENADELAUNCHER,
	WEAPON_LIGHTNING,

	WEAPON_LAST
};

extern const char *g_aWeaponNames[WEAPON_LAST];
int GetWeaponId( const char *pszWeaponName );
const char *WeaponIdToAlias( int iWeapon );

//--------------------------------------------------------------------------
// Collision groups
//--------------------------------------------------------------------------
enum HL2Collisions
{
	HL2COLLISION_GROUP_PLASMANODE = LAST_SHARED_COLLISION_GROUP,
	HL2COLLISION_GROUP_SPIT,
	HL2COLLISION_GROUP_HOMING_MISSILE,
	HL2COLLISION_GROUP_COMBINE_BALL,

	HL2COLLISION_GROUP_FIRST_NPC,
	HL2COLLISION_GROUP_HOUNDEYE,
	HL2COLLISION_GROUP_CROW,
	HL2COLLISION_GROUP_HEADCRAB,
	HL2COLLISION_GROUP_STRIDER,
	HL2COLLISION_GROUP_GUNSHIP,
	HL2COLLISION_GROUP_ANTLION,
	HL2COLLISION_GROUP_LAST_NPC,
	HL2COLLISION_GROUP_COMBINE_BALL_NPC,
};

//--------------------------------------------------------------------------
// The various states the player can be in during the join game process.
//--------------------------------------------------------------------------
enum HL2MPPlayerState
{
	// Happily running around in the game.
	STATE_ACTIVE = 0,
	STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.
	NUM_PLAYER_STATES
};

//--------------
// HL2 SPECIFIC
//--------------
#define DMG_SNIPER			(DMG_LASTGENERICFLAG<<1)	// This is sniper damage
#define DMG_MISSILEDEFENSE	(DMG_LASTGENERICFLAG<<2)	// The only kind of damage missiles take. (special missile defense)

#endif // HL2_SHAREDDEFS_H
