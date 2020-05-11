//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements health kits and wall mounted health chargers.
//
//=============================================================================//
#include "cbase.h"
#include "gamerules.h"
#include "hl2mp_player.h"
#include "items.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SF_HEALTH_ROTTEN		0x0001
#define SF_HEALTH_MEGAHEALTH		0x0002

//-----------------------------------------------------------------------------
// >> Item_Health
//-----------------------------------------------------------------------------
class CItem_Health : public CItem
{
public:
	DECLARE_CLASS( CItem_Health, CItem );

	void Spawn( void );
	void Precache( void );
	bool MyTouch( CBasePlayer *pPlayer );
};

LINK_ENTITY_TO_CLASS( item_health, CItem_Health );
PRECACHE_REGISTER( item_health );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_Health::Spawn( void )
{
	Precache();

	if ( m_spawnflags & SF_HEALTH_ROTTEN )
		SetModel( "models/items/healthvial.mdl" );
	else if ( m_spawnflags & SF_HEALTH_MEGAHEALTH )
		SetModel( "models/items/healthkit.mdl" );
	else
		SetModel( "models/items/healthkit.mdl" );

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_Health::Precache( void )
{
	PrecacheModel("models/items/healthkit.mdl");
	PrecacheModel( "models/items/healthvial.mdl" );

	PrecacheScriptSound( "Health.Touch" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
// Output : 
//-----------------------------------------------------------------------------
bool CItem_Health::MyTouch( CBasePlayer *pPlayer )
{
	int healthtogive;

	if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
		Respawn();
	else
		UTIL_Remove( this );

	if ( m_spawnflags & SF_HEALTH_ROTTEN )
		healthtogive = 10;
	else if ( m_spawnflags & SF_HEALTH_MEGAHEALTH )
		healthtogive = 100;
	else
		healthtogive = 25;

	CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer( pPlayer );
	return ( pHL2MPPlayer && pHL2MPPlayer->ApplyHealth( healthtogive ) );
}