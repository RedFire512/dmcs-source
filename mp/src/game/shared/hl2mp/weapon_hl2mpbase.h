//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
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

#define SF_WEAPON_DISABLE_BOB		(1<<3)	// Disable Quake Like Weapon Bob

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

	virtual void Equip( CBaseCombatCharacter *pOwner );

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
	void ClientThink( void );
	virtual bool ShouldPredict();
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual void AddViewmodelBob( CBaseViewModel *viewmodel, Vector &origin, QAngle &angles );
	virtual	float CalcViewmodelBob( void );

    virtual void GetGlowEffectColor( float& r, float& g, float& b ) OVERRIDE;
    void UpdateGlow();
    virtual bool GlowOccluded() OVERRIDE { return false; }
    virtual bool GlowUnoccluded() OVERRIDE { return true; }
#else
	CBaseEntity *Respawn( void );						// copy a weapon
	virtual void SetupPhysics( void );
	virtual void Materialize( void );
	virtual	int	ObjectCaps( void );
	virtual void FallThink( void );						// make the weapon fall to the ground after spawning
#endif

	float m_flPrevAnimTime;
	float m_flNextResetCheckTime;

	CNetworkVar( bool, m_bQuake3Bob);
	CNetworkVar( Vector, m_vOriginalSpawnOrigin );
	CNetworkVar( QAngle, m_vOriginalSpawnAngles );
	Vector	GetOriginalSpawnOrigin( void ) { return m_vOriginalSpawnOrigin; }
	QAngle	GetOriginalSpawnAngles( void ) { return m_vOriginalSpawnAngles; }
	
private:

	QAngle ClientRotAng; // m_angRotation is stomped sometimes (CDMCItem returning the ent to spawn position?)

private:

	CWeaponHL2MPBase( const CWeaponHL2MPBase & );
};

#endif // WEAPON_HL2MPBASE_H