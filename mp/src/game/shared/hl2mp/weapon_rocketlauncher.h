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

#ifdef CLIENT_DLL
	#include "iviewrender_beams.h"
#else
	#include "Sprite.h"
	#include "npcevent.h"
	#include "beam_shared.h"
#endif

#ifndef CLIENT_DLL
class CWeaponRocketLauncher;
class RocketTrail;
 
//###########################################################################
//	>> CProjectileRocket		(missile launcher class is below this one!)
//###########################################################################
class CProjectileRocket : public CBaseCombatCharacter
{
	DECLARE_CLASS( CProjectileRocket, CBaseCombatCharacter );

public:
	CProjectileRocket();
	~CProjectileRocket();

	Class_T Classify( void ) { return CLASS_MISSILE; }
	
	void	Spawn( void );
	void	Precache( void );
	void	MissileTouch( CBaseEntity *pOther );
	void	Explode( void );
	void	ShotDown( void );
	void	AccelerateThink( void );
	void	AugerThink( void );
	void	IgniteThink( void );
	void	SeekThink( void );
	void	DumbFire( void );
	void	SetGracePeriod( float flGracePeriod );

	int		OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void	Event_Killed( const CTakeDamageInfo &info );
	
	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	unsigned int PhysicsSolidMaskForEntity( void ) const;

	CHandle<CWeaponRocketLauncher>		m_hOwner;

	static CProjectileRocket *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner );

protected:
	virtual void DoExplosion();	
	virtual int AugerHealth() { return m_iMaxHealth - 20; }

	// Creates the smoke trail
	void CreateSmokeTrail( void );

	CHandle<RocketTrail>	m_hRocketTrail;
	float					m_flAugerTime;		// Amount of time to auger before blowing up anyway
	float					m_flMarkDeadTime;
	float					m_flDamage;

private:
	float					m_flGracePeriodEndsAt;

	DECLARE_DATADESC();
};
#endif

//-----------------------------------------------------------------------------
// RPG
//-----------------------------------------------------------------------------

#ifdef CLIENT_DLL
#define CWeaponRocketLauncher C_WeaponRocketLauncher
#endif

class CWeaponRocketLauncher : public CBaseHL2MPCombatWeapon
{
	DECLARE_CLASS( CWeaponRocketLauncher, CBaseHL2MPCombatWeapon );
public:

	CWeaponRocketLauncher();
	~CWeaponRocketLauncher();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	Precache( void );

	void	PrimaryAttack( void );
	virtual float GetFireRate( void ) { return 1; };

	void	DecrementAmmo( CBaseCombatCharacter *pOwner );

	bool	WeaponShouldBeLowered( void );
	bool	Lower( void );

	bool	CanHolster( void );

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 1; }
	float	GetMinRestTime() { return 4.0; }
	float	GetMaxRestTime() { return 4.0; }

	void	NotifyRocketDied( void );

	bool	HasAnyAmmo( void );

	CBaseEntity *GetMissile( void ) { return m_hMissile; }

	DECLARE_ACTTABLE();
	
protected:

	CNetworkHandle( CBaseEntity,	m_hMissile );

private:
	
	CWeaponRocketLauncher( const CWeaponRocketLauncher & );
};
#endif // WEAPON_ROCKETLAUNCHER_H