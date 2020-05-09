//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "projectile_nail.h"

#define NAILGUN_MODEL "models/nail.mdl"

LINK_ENTITY_TO_CLASS( projectile_nail, CProjectileNail );
PRECACHE_REGISTER( projectile_nail );

BEGIN_DATADESC( CProjectileNail )
	DEFINE_FUNCTION( NailTouch )
END_DATADESC()

CProjectileNail *CProjectileNail::CreateNail( 
	bool fSendClientNail, 
	Vector vecOrigin, 
	QAngle vecAngles, 
	CBaseEntity *pOwner, 
	CBaseEntity *pLauncher, 
	bool fMakeClientNail )
{
	CProjectileNail *pNail = (CProjectileNail*)CreateEntityByName( "projectile_nail" );
	if ( !pNail )
		return NULL;

	pNail->SetAbsOrigin( vecOrigin );
	pNail->SetAbsAngles( vecAngles );
	pNail->SetOwnerEntity( pOwner );
	pNail->Spawn();
	pNail->SetTouch( &CProjectileNail::NailTouch );
	pNail->m_iDamage = 9;

	return pNail;
}

CProjectileNail *CProjectileNail::CreateSuperNail( Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, CBaseEntity *pLauncher )
{
	// Super nails simply do more damage
	CProjectileNail *pNail = CreateNail( false, vecOrigin, vecAngles, pOwner, pLauncher, true );
	pNail->m_iDamage = 13;
	return pNail;
}

//=========================================================
// CProjectileNail::Spawn
//
// Creates an nail entity on the server that is invisible 
// so that it won't be sent to clients. At the same time sends a
// tiny message that creates a flying nail entity on all 
// clients. (sjb)
//=========================================================
void CProjectileNail::Spawn()
{
	// motor
	SetMoveType( MOVETYPE_FLYGRAVITY );
	SetSolid( SOLID_BBOX );

	SetModel( NAILGUN_MODEL );

	SetSize( Vector( 0, 0, 0), Vector(0, 0, 0) );

	Vector vForward;
	AngleVectors( GetAbsAngles(), &vForward );
	SetAbsVelocity( vForward * 1000 );
	SetGravity( 0.5 );

	SetNextThink( gpGlobals->curtime + 6 );
	SetThink( &CProjectileNail::SUB_Remove );
}

void CProjectileNail::Precache()
{
	PrecacheModel( NAILGUN_MODEL );
	BaseClass::Precache();
}

void CProjectileNail::NailTouch( CBaseEntity *pOther )
{
	// Damage person hit by the nail
	CBaseEntity *pevOwner = GetOwnerEntity();
	if ( !pevOwner || !pOther )
	{
		UTIL_Remove( this );
		return;
	}

	// Nails don't touch each other.
	if ( dynamic_cast< CProjectileNail* >( pOther ) == NULL )
	{
		Vector vVelNorm = GetAbsVelocity();
		VectorNormalize( vVelNorm );

		Vector vDamageForce = vVelNorm;
		Vector vDamagePosition = GetAbsOrigin();

		int iStartHealth = pOther->GetHealth();
		CTakeDamageInfo info( this, pevOwner, vDamageForce, vDamagePosition, m_iDamage, DMG_SLASH );
		pOther->TakeDamage( info );
		
		// Did they take the damage?
		if ( pOther->GetHealth() < iStartHealth )
		{
			SpawnBlood( GetAbsOrigin(), GetAbsVelocity(), pOther->BloodColor(), m_iDamage );
		}

		// Do an impact trace in case we hit the world.
		trace_t tr;
		UTIL_TraceLine( 
			GetAbsOrigin() - vVelNorm * 30,
			GetAbsOrigin() + vVelNorm * 50,
			MASK_SOLID_BRUSHONLY,
			pevOwner,
			COLLISION_GROUP_DEBRIS,
			&tr );

		if ( tr.fraction < 1 )
		{
			UTIL_ImpactTrace( &tr, DMG_SLASH );
		}

		UTIL_Remove( this );
	}	
}
