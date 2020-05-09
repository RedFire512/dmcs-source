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

class CWeaponRocketLauncher : public CWeaponHL2MPBase
{
public:
	DECLARE_CLASS( CWeaponRocketLauncher, CWeaponHL2MPBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_ACTTABLE();

	CWeaponRocketLauncher();

	virtual void PrimaryAttack();
	virtual void WeaponIdle();

	void DoFireEffects();
	void Precache( void );

	virtual	void FireRocket( void );

private:
	CWeaponRocketLauncher( const CWeaponRocketLauncher & );
};

#endif // WEAPON_ROCKETLAUNCHER_H