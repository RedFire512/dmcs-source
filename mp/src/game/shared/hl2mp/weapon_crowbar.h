//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#ifndef HL2MP_WEAPON_CROWBAR_H
#define HL2MP_WEAPON_CROWBAR_H
#pragma once

#include "weapon_hl2mpbasebasebludgeon.h"

#ifdef CLIENT_DLL
#define CWeaponCrowbar C_WeaponCrowbar
#endif

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponCrowbar : public CBaseHL2MPBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponCrowbar, CBaseHL2MPBludgeonWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponCrowbar();

	virtual int GetWeaponID( void ) const { return WEAPON_CROWBAR; }

	float GetRange( void );
	float GetFireRate( void );

	void AddViewKick( void );
	float GetDamageForActivity( Activity hitActivity );
	void SecondaryAttack( void )	{ return; }

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }
	virtual Activity	GetSecondaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }
	virtual Activity	GetMissPrimaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }
	virtual Activity	GetMissSecondaryAttackActivity( void ) { return ACT_VM_PRIMARYATTACK; }

	// Animation event
#ifndef CLIENT_DLL
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	int WeaponMeleeAttack1Condition( float flDot, float flDist );
#endif

	CWeaponCrowbar( const CWeaponCrowbar & );	
};

#endif // HL2MP_WEAPON_CROWBAR_H