//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "dmc_items.h"

IMPLEMENT_SERVERCLASS_ST( CDMCItem, DT_DMCItem )
	SendPropBool(SENDINFO( m_bQuake3Bob ) ),
	SendPropVector( SENDINFO( m_vOriginalSpawnOrigin ) ),
	SendPropVector( SENDINFO( m_vOriginalSpawnAngles ) ),
	//SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Constructor 
//-----------------------------------------------------------------------------
CDMCItem::CDMCItem()
{
	m_bQuake3Bob = true;
}

//-----------------------------------
// Purpose:
//-----------------------------------
bool CDMCItem::CreateItemVPhysicsObject( void )
{
	VPhysicsDestroyObject();

	return BaseClass::CreateItemVPhysicsObject();
}

//-----------------------------------
// Purpose:
//-----------------------------------
void CDMCItem::Spawn( void )
{
	if ( g_pGameRules->IsAllowedToSpawn( this ) == false )
	{
		UTIL_Remove( this );
		return;
	}

	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );
	SetBlocksLOS( false );
	AddEFlags( EFL_NO_ROTORWASH_PUSH );

	// This will make them not collide with the player, but will collide
	// against other items + weapons
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( true, /*ITEM_PICKUP_BOX_BLOAT*/ 24 );
	SetTouch( &CItem::ItemTouch );

	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetupPhysics();

	SetThink( &CItem::FallThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------
// Purpose:
//-----------------------------------
void CDMCItem::Activate( void )
{
	BaseClass::Activate();
}

//-----------------------------------
// Purpose: Sets up item physics.
//-----------------------------------
void CDMCItem::SetupPhysics()
{
	if ( !HasSpawnFlags( SF_ITEM_DISABLE_BOB ) )
	{
		//m_bQuake3Bob = true;
		VPhysicsDestroyObject();
		SetMoveType( MOVETYPE_NONE );
		SetSolid( SOLID_NONE );
		AddSolidFlags( GetSolidFlags() | FSOLID_NOT_STANDABLE | FSOLID_TRIGGER );
		return;
	}

	m_bQuake3Bob = false;

	if ( !CreateItemVPhysicsObject() )
		return;

	// Constrained start?
	if ( HasSpawnFlags( SF_ITEM_START_CONSTRAINED ) )
	{
		//Constrain the item in place
		IPhysicsObject *pReferenceObject, *pAttachedObject;

		pReferenceObject = g_PhysWorldObject;
		pAttachedObject = VPhysicsGetObject();

		if ( pReferenceObject && pAttachedObject )
		{
			constraint_fixedparams_t fixed;
			fixed.Defaults();
			fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );

			fixed.constraint.forceLimit = lbs2kg( 10000 );
			fixed.constraint.torqueLimit = lbs2kg( 10000 );

			m_pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

			m_pConstraint->SetGameData( (void *) this );

			PhysSetGameFlags( pAttachedObject, FVPHYSICS_NO_PLAYER_PICKUP );
		}
	}
}

//-----------------------------------
// Purpose:
//-----------------------------------
CBaseEntity* CDMCItem::Respawn( void )
{
	SetTouch( NULL );
	AddEffects( EF_NODRAW );

	VPhysicsDestroyObject();

	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER );

	UTIL_SetOrigin( this, g_pGameRules->VecItemRespawnSpot( this ) );// blip to whereever you should respawn.
	SetAbsAngles( g_pGameRules->VecItemRespawnAngles( this ) );// set the angles.

	if ( !HasSpawnFlags( SF_ITEM_START_CONSTRAINED ) && HasSpawnFlags( SF_ITEM_DISABLE_BOB ) )
		UTIL_DropToFloor( this, MASK_SOLID );

	RemoveAllDecals(); //remove any decals

	SetThink ( &CDMCItem::Materialize );
	SetNextThink( gpGlobals->curtime + g_pGameRules->FlItemRespawnTime( this ) );
	return this;
}

//-----------------------------------
// Purpose:
//-----------------------------------
void CDMCItem::Materialize( void )
{
	SetupPhysics();

	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EmitSound( "AlyxEmp.Charge" );

		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	SetTouch( &CItem::ItemTouch );
}