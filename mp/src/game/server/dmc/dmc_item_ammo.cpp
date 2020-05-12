//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The various ammo types for DMC:S
//
//=============================================================================//
#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
extern int ITEM_GiveAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false );

//---------------------------------------------------------
// Purpose:	Base class for all ammo items.
//---------------------------------------------------------
class CItem_Ammo : public CItem 
{
public:
    DECLARE_CLASS( CItem_Ammo, CItem );

private:
    const char *m_szModelName;  //Model name of the ammo box
    const char *m_szAmmoName;   //Ammo definition of class
    float m_flAmmoCount;        //Ammo count which will be given to picker

public:
    //Constructor
    CItem_Ammo( const char *name, const float count, const char *model ) : m_szModelName( model ), m_szAmmoName( name ), m_flAmmoCount( count ) { }

    void Spawn()
    {
        Precache();

        SetModel( m_szModelName );

        BaseClass::Spawn();
    }

    void Precache()
    {
        PrecacheModel( m_szModelName );
    }

    bool MyTouch( CBasePlayer *plr )
    {
        if( ITEM_GiveAmmo( plr, m_flAmmoCount, m_szAmmoName ) ) 
		{
            if( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO ) 
                UTIL_Remove( this );

			return true;
        }

        return false;
    }
};

//========================================================================
//  >> Item_Spikes
//========================================================================
class CItem_Spikes : public CItem_Ammo 
{
public:
    DECLARE_CLASS( CItem_Spikes, CItem_Ammo )

    CItem_Spikes() : BaseClass( "AMMO_NAILS", 25, "models/items/boxsrounds.mdl" ) { }
};

class CItem_SpikesLarge : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_SpikesLarge, CItem_Ammo )

    CItem_SpikesLarge() : BaseClass( "AMMO_NAILS", 50, "models/items/boxsrounds.mdl" ) { }
};

LINK_ENTITY_TO_CLASS( item_spikes, CItem_Spikes );
LINK_ENTITY_TO_CLASS( item_spikes_large, CItem_SpikesLarge );

// ========================================================================
//  >> Item_Shells
// ========================================================================
class CItem_Shells : public CItem_Ammo 
{
public:
    DECLARE_CLASS( CItem_Shells, CItem_Ammo );
    CItem_Shells() : BaseClass( "AMMO_SHELLS", 20, "models/items/boxbuckshot.mdl" ) { }
};

class CItem_ShellsLarge : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_ShellsLarge, CItem_Ammo );
    CItem_ShellsLarge() : BaseClass( "AMMO_SHELLS", 40, "models/items/boxbuckshot.mdl" ) { }
};

LINK_ENTITY_TO_CLASS( item_shells, CItem_Shells );
LINK_ENTITY_TO_CLASS( item_shells_large, CItem_ShellsLarge );

// ========================================================================
// >> Item_Rockets
// ========================================================================
class CItem_Rockets : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_Rockets, CItem_Ammo );
    CItem_Rockets() : BaseClass( "AMMO_ROCKETS", 5, "models/weapons/w_missile_closed.mdl" ) { }
};

class CItem_RocketsLarge : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_RocketsLarge, CItem_Ammo );
    CItem_RocketsLarge() : BaseClass( "AMMO_ROCKETS", 10, "models/weapons/w_missile_closed.mdl" ) { }
};

LINK_ENTITY_TO_CLASS( item_rockets, CItem_Rockets );
LINK_ENTITY_TO_CLASS( item_rockets_large, CItem_RocketsLarge );

// ========================================================================
// >> Item_Cells
// ========================================================================
class CItem_Cells : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_Cells, CItem_Ammo );
    CItem_Cells() : BaseClass( "AMMO_CELLS", 6, "models/items/battery.mdl" ) { }
};

class CItem_CellsLarge : public CItem_Ammo
{
public:
    DECLARE_CLASS( CItem_CellsLarge, CItem_Ammo );
    CItem_CellsLarge() : BaseClass( "AMMO_CELLS", 12, "models/items/battery.mdl" ) { }
};

LINK_ENTITY_TO_CLASS( item_cells, CItem_Cells );
LINK_ENTITY_TO_CLASS( item_cells_large, CItem_CellsLarge );
