//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PROJECTILE_NAIL_H
#define PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

class CProjectileNail : public CBaseAnimating
{
public:
	DECLARE_CLASS( CProjectileNail, CBaseAnimating );
	DECLARE_DATADESC();
	
	void	Spawn();
	void	Precache();
	
	// Functions to create all the various types of nails.
	static  CProjectileNail *CreateNail( bool fSendClientNail, Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, CBaseEntity *pLauncher, bool fCreateClientNail );
	static  CProjectileNail *CreateSuperNail( Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, CBaseEntity *pLauncher );

private:

	void	NailTouch( CBaseEntity *pOther );

private:
	Vector m_vecPreviousVelocity;
	int m_iDamage;		// How much damage this nail does when it hits an enemy.
};

#endif // PROJECTILE_NAIL_H