//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef HL2MP_PLAYER_SHARED_H
#define HL2MP_PLAYER_SHARED_H
#pragma once

#define HL2MP_PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)
#include "studio.h"

enum HL2MPPlayerState
{
	// Happily running around in the game.
	STATE_ACTIVE=0,
	STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.
	NUM_PLAYER_STATES
};


#if defined( CLIENT_DLL )
#define CHL2MP_Player C_HL2MP_Player
#endif

// Player avoidance
#define PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)

#endif //HL2MP_PLAYER_SHARED_h
