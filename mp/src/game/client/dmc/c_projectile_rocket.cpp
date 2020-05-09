//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_baseanimating.h"
#include "iefx.h"
#include "dlight.h"
#include "c_te_legacytempents.h"

class C_ProjectileRocket : public C_BaseAnimating
{
public:
	DECLARE_CLASS( C_ProjectileRocket, C_BaseAnimating );
	DECLARE_CLIENTCLASS();

					C_ProjectileRocket();
	virtual			~C_ProjectileRocket();

	virtual void Spawn();
	virtual int DrawModel( int flags );
	virtual void PostDataUpdate( DataUpdateType_t type );
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void CreateLightEffects( void );

private:
	CNetworkVector( m_vInitialVelocity );
	float m_flSpawnTime;
};


IMPLEMENT_CLIENTCLASS_DT( C_ProjectileRocket, DT_ProjectileRocket, CProjectileRocket )
	RecvPropVector( RECVINFO( m_vInitialVelocity ) )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ProjectileRocket::C_ProjectileRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ProjectileRocket::~C_ProjectileRocket()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ProjectileRocket::Spawn()
{
	m_flSpawnTime = gpGlobals->curtime;
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ProjectileRocket::PostDataUpdate( DataUpdateType_t type )
{
	BaseClass::PostDataUpdate( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		// Now stick our initial velocity into the interpolation history 
		CInterpolatedVar< Vector > &interpolator = GetOriginInterpolator();
		
		interpolator.ClearHistory();
		float changeTime = GetLastChangeTime( LATCH_SIMULATION_VAR );

		// Add a sample 1 second back.
		Vector vCurOrigin = GetLocalOrigin() - m_vInitialVelocity;
		interpolator.AddToHead( changeTime - 1.0, &vCurOrigin, false );

		// Add the current sample.
		vCurOrigin = GetLocalOrigin();
		interpolator.AddToHead( changeTime, &vCurOrigin, false );

		// do the same for angles
		CInterpolatedVar< QAngle > &rotInterpolator = GetRotationInterpolator();

		rotInterpolator.ClearHistory();

		// Add a rotation sample 1 second back
		QAngle vCurAngles = GetLocalAngles();
		rotInterpolator.AddToHead( changeTime - 1.0, &vCurAngles, false );

		// Add the current rotation
		rotInterpolator.AddToHead( changeTime - 1.0, &vCurAngles, false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ProjectileRocket::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		CreateLightEffects();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_ProjectileRocket::DrawModel( int flags )
{
	// During the first half-second of our life, don't draw ourselves
	if ( gpGlobals->curtime - m_flSpawnTime < 0.2 )
		return 0;

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ProjectileRocket::CreateLightEffects( void )
{
	// Handle the dynamic light
	AddEffects( EF_DIMLIGHT );

	dlight_t *dl;

	if ( IsEffectActive( EF_DIMLIGHT ) )
	{	
		dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC + index );
		dl->origin = GetAbsOrigin();
		dl->color.r = 252;
		dl->color.g = 238;
		dl->color.b = 128;
		dl->radius = 256.0f;
		dl->die = gpGlobals->curtime + 0.1;
		tempents->RocketFlare( GetAbsOrigin() );
	}
}