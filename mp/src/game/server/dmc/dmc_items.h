//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef DMC_ITEMS_H
#define DMC_ITEMS_H
#ifdef _WIN32
#pragma once
#endif

#include "items.h"

#define SF_ITEM_DISABLE_BOB			0x00000002	// Disable Quake Like Item Bob

class CDMCItem : public CItem
{
public:
	DECLARE_CLASS( CDMCItem, CItem );
	DECLARE_SERVERCLASS();
	
	CDMCItem();

	void Spawn( void );
	void Activate( void );
	
	virtual CBaseEntity* Respawn( void ) OVERRIDE;
	virtual void Materialize( void ) OVERRIDE;
	virtual void SetupPhysics();
	bool CreateItemVPhysicsObject( void );

	CNetworkVar( bool, m_bQuake3Bob );	// Whether or not the item should bob like Quake 3 Arena.

	CNetworkVar( Vector, m_vOriginalSpawnOrigin );
	CNetworkVar( QAngle, m_vOriginalSpawnAngles );
};
#endif // DMC_ITEMS_H