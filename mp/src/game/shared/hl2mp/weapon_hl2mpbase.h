//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_HL2MPBASE_H
#define WEAPON_HL2MPBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "hl2mp_player_shared.h"
#include "basecombatweapon_shared.h"
#include "hl2mp_weapon_parse.h"

#if defined( CLIENT_DLL )
	#define CWeaponHL2MPBase C_WeaponHL2MPBase
	void UTIL_ClipPunchAngleOffset( QAngle &in, const QAngle &punch, const QAngle &clip );
#endif

class CHL2MP_Player;

// These are the names of the ammo types that go in the CAmmoDefs and that the 
// weapon script files reference.

// Given an ammo type (like from a weapon's GetPrimaryAmmoType()), this compares it
// against the ammo name you specify.
// MIKETODO: this should use indexing instead of searching and strcmp()'ing all the time.
bool IsAmmoType( int iAmmoType, const char *pAmmoName );

class CWeaponHL2MPBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponHL2MPBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CWeaponHL2MPBase();

	virtual void Spawn();

	// All predicted weapons need to implement and return true
	virtual bool IsPredicted() const { return true; }

	CBasePlayer* GetPlayerOwner() const;
	CHL2MP_Player* GetHL2MPPlayerOwner() const;

	void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );
	
	// Weapon Data.
	CHL2MPSWeaponInfo const	&GetHL2MPWpnData() const;
	virtual int GetWeaponID( void ) const;
	virtual bool IsWeapon( int iWeapon ) const;

	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual void FallInit( void );

	// Reloading
	// Reloading is gone from DMC:S
	virtual	void CheckReload( void ) OVERRIDE {}
	virtual void FinishReload( void ) OVERRIDE {}
	virtual void AbortReload( void ) OVERRIDE {}
	virtual bool DefaultReload( int iClipSize1, int iClipSize2, int iActivity ) { return false; }
	virtual bool ReloadsSingly( void ) const  { return false; }
	virtual bool Reload() OVERRIDE { return false; }
	virtual bool CanReload( void ) OVERRIDE { return false; }
	virtual bool ReloadOrSwitchWeapons( void ) OVERRIDE;
	virtual bool ShouldDisplayReloadHUDHint() OVERRIDE{ return false; }
	virtual void DisplayReloadHudHint() OVERRIDE{}
	virtual void RescindReloadHudHint() OVERRIDE {}
	
public:
#if defined( CLIENT_DLL )
	virtual bool ShouldPredict();
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float CalcViewmodelBob( void );
#else
	virtual void Materialize( void );
	virtual	int	ObjectCaps( void );
	virtual void FallThink( void );						// make the weapon fall to the ground after spawning
#endif

	float m_flPrevAnimTime;
	float m_flNextResetCheckTime;

	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin;	}
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles;	}

private:

	CWeaponHL2MPBase( const CWeaponHL2MPBase & );

	Vector m_vOriginalSpawnOrigin;
	QAngle m_vOriginalSpawnAngles;
};

#endif // WEAPON_HL2MPBASE_H