//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
//=============================================================================//
#include "cbase.h"
#include "hl2mp_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Purpose:	Base class for all Armor items.
//---------------------------------------------------------
class CItem_Armor : public CItem 
{
public:
    DECLARE_CLASS( CItem_Armor, CItem );

private:
    int m_iAmmount;        // Armor to give

public:
    //Constructor
    CItem_Armor( int count ) : m_iAmmount( count ) { }

    void Spawn()
    {
        Precache();

        SetModel( "models/items/hevsuit.mdl" );

        BaseClass::Spawn();
    }

    void Precache()
    {
        PrecacheModel( "models/items/hevsuit.mdl" );
		PrecacheScriptSound( "Armor.Touch" );
    }

    bool MyTouch(  CBasePlayer *pPlayer  )
    {
		if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_YES )
			Respawn();
		else
			UTIL_Remove( this );	

		CHL2MP_Player *pHL2MPPlayer = ToHL2MPPlayer( pPlayer );
		return ( pHL2MPPlayer && pHL2MPPlayer->ApplyArmor( m_iAmmount ) );
    }
};

//========================================================================
//  >> Item_Armor1
//========================================================================
class CItem_Armor1 : public CItem_Armor
{
public:
	DECLARE_CLASS( CItem_Armor1, CItem_Armor );

    CItem_Armor1() : BaseClass( 100 ) { }

};

LINK_ENTITY_TO_CLASS( item_armor1, CItem_Armor1 );

//========================================================================
//  >> Item_Armor2
//========================================================================
class CItem_Armor2 : public CItem_Armor
{
public:
	DECLARE_CLASS( CItem_Armor2, CItem_Armor );

    CItem_Armor2() : BaseClass( 150 ) { }

};

LINK_ENTITY_TO_CLASS( item_armor2, CItem_Armor2 );

//========================================================================
//  >> Item_ArmorInv
//========================================================================
class CItem_ArmorInv : public CItem_Armor
{
public:
	DECLARE_CLASS( CItem_ArmorInv, CItem_Armor );

	CItem_ArmorInv() : BaseClass( 200 ) { }

};

LINK_ENTITY_TO_CLASS( item_armorInv, CItem_ArmorInv );

//========================================================================
//  >> Item_ArmorUltra
//========================================================================
class CItem_ArmorUltra : public CItem_Armor
{
public:
	DECLARE_CLASS( CItem_ArmorUltra, CItem_Armor );

	CItem_ArmorUltra() : BaseClass( 500 ) { }

};

LINK_ENTITY_TO_CLASS( item_armorUltra, CItem_ArmorUltra );