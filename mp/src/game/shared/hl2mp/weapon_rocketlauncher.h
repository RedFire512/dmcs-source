//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#ifndef WEAPON_ROCKETLAUNCHER_H
#define WEAPON_ROCKETLAUNCHER_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_hl2mpbasehlmpcombatweapon.h"

#ifndef CLIENT_DLL
	#include "baseanimating.h"
	#include "smoke_trail.h"
#endif

#ifndef CLIENT_DLL
class RocketTrail;
class CWeaponRocketLauncher;

//###########################################################################
//	>> CProjectileRocket ( missile launcher class is below this one! )
//###########################################################################
class CProjectileRocket : public CBaseAnimating
{
	DECLARE_CLASS( CProjectileRocket, CBaseAnimating );

public:
	CProjectileRocket();
	~CProjectileRocket();
	
	void	Spawn( void );
	void	Precache( void );
	void	RocketTouch( CBaseEntity *pOther );
	void	Explode( void );
	void	Fire( void );

	static CProjectileRocket *Create( const char *szClassname, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner );	

	void SetupInitialTransmittedGrenadeVelocity( const Vector &velocity );

	CHandle<CWeaponRocketLauncher> m_hOwner;

protected:
	virtual void DoExplosion( trace_t *pTrace );

	void FlyThink( void );

	CNetworkVector( m_vInitialVelocity );

private:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------
#if defined( CLIENT_DLL )
	#define CWeaponRocketLauncher C_WeaponRocketLauncher
#endif

class CWeaponRocketLauncher : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponRocketLauncher, CBaseHL2MPCombatWeapon );
public:
	DECLARE_DATADESC();
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponRocketLauncher( void );
	~CWeaponRocketLauncher();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	NotifyRocketDied( void );

private:
	CNetworkHandle( CBaseEntity, m_hMissile );
};

#endif // WEAPON_ROCKETLAUNCHER_H