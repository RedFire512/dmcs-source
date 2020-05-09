//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL2.
//
//=============================================================================//

#include "cbase.h"
#include "vcollide_parse.h"
#include "c_hl2mp_player.h"
#include "view.h"
#include "takedamageinfo.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"
#include "iviewrender_beams.h"			// flashlight beam
#include "r_efx.h"
#include "dlight.h"
#include "iclientvehicle.h"
#include "c_basetempentity.h"
#include "prediction.h"
#include "bone_setup.h"
#include "input.h"
#include "collisionutils.h"
#include "c_team.h"
#include "obstacle_pushaway.h"

// Don't alias here
#if defined( CHL2MP_Player )
#undef CHL2MP_Player	
#endif

#define FLASHLIGHT_DISTANCE 1000
#define CYCLELATCH_TOLERANCE		0.15f

LINK_ENTITY_TO_CLASS( player, C_HL2MP_Player );

BEGIN_RECV_TABLE_NOBASE( C_HL2MP_Player, DT_HL2MPLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
//	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),
END_RECV_TABLE()

BEGIN_RECV_TABLE_NOBASE( C_HL2MP_Player, DT_HL2MPNonLocalPlayerExclusive )
	RecvPropVector( RECVINFO_NAME( m_vecNetworkOrigin, m_vecOrigin ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[0] ) ),
	RecvPropFloat( RECVINFO( m_angEyeAngles[1] ) ),

	RecvPropInt( RECVINFO( m_cycleLatch ), 0, &C_HL2MP_Player::RecvProxy_CycleLatch ),
END_RECV_TABLE()

IMPLEMENT_CLIENTCLASS_DT(C_HL2MP_Player, DT_HL2MP_Player, CHL2MP_Player)

	RecvPropDataTable( "hl2mplocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_HL2MPLocalPlayerExclusive) ),
	RecvPropDataTable( "hl2mpnonlocaldata", 0, 0, &REFERENCE_RECV_TABLE(DT_HL2MPNonLocalPlayerExclusive) ),

	RecvPropEHandle( RECVINFO( m_hRagdoll ) ),
	RecvPropInt( RECVINFO( m_iSpawnInterpCounter ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_HL2MP_Player )
	DEFINE_PRED_FIELD( m_flCycle, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_ARRAY_TOL( m_flEncodedController, FIELD_FLOAT, MAXSTUDIOBONECTRLS, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE, 0.02f ),
	DEFINE_PRED_FIELD( m_nNewSequenceParity, FIELD_INTEGER, FTYPEDESC_OVERRIDE | FTYPEDESC_PRIVATE | FTYPEDESC_NOERRORCHECK ),
END_PREDICTION_DATA()

static ConVar cl_playermodel( "cl_playermodel", "none", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_SERVER_CAN_EXECUTE, "Default Player Model");
ConVar cl_rollspeed( "cl_rollspeed", "300.0", FCVAR_USERINFO | FCVAR_ARCHIVE ); // 300.0
ConVar cl_rollangle( "cl_rollangle", "0.65", FCVAR_USERINFO | FCVAR_ARCHIVE ); // 0.65
ConVar cl_bobcycle( "cl_bobcycle", "0.8", FCVAR_USERINFO | FCVAR_ARCHIVE );
ConVar cl_bob( "cl_bob", "0.01", FCVAR_USERINFO | FCVAR_ARCHIVE );
ConVar cl_bobup( "cl_bobup", "0.5", FCVAR_USERINFO | FCVAR_ARCHIVE );
ConVar cl_iyaw_cycle( "cl_iyaw_cycle", "2.0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_iroll_cycle( "cl_iroll_cycle", "0.5", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_ipitch_cycle( "cl_ipitch_cycle", "1.0", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_iyaw_level( "cl_iyaw_level", "0.3", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_iroll_level( "cl_iroll_level", "0.1", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );
ConVar cl_ipitch_level( "cl_ipitch_level", "0.3", FCVAR_USERINFO | FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY );

void SpawnBlood (Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage);

C_HL2MP_Player::C_HL2MP_Player() : m_iv_angEyeAngles( "C_HL2MP_Player::m_iv_angEyeAngles" )
{
	m_iIDEntIndex = 0;
	m_iSpawnInterpCounterCache = 0;

	AddVar( &m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR );

//	m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
	m_PlayerAnimState = CreateHL2MPPlayerAnimState( this );

	m_blinkTimer.Invalidate();

	m_pFlashlightBeam = NULL;

	m_flServerCycle = -1.0f;
}

C_HL2MP_Player::~C_HL2MP_Player( void )
{
	ReleaseFlashlight();
	m_PlayerAnimState->Release();
}

int C_HL2MP_Player::GetIDTarget() const
{
	return m_iIDEntIndex;
}

//-----------------------------------------------------------------------------
// Purpose: Update this client's target entity
//-----------------------------------------------------------------------------
void C_HL2MP_Player::UpdateIDTarget()
{
	if ( !IsLocalPlayer() )
		return;

	// Clear old target and find a new one
	m_iIDEntIndex = 0;

	// don't show IDs in chase spec mode
	if ( GetObserverMode() == OBS_MODE_CHASE || 
		 GetObserverMode() == OBS_MODE_DEATHCAM )
		 return;

	trace_t tr;
	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin(), 1500, MainViewForward(), vecEnd );
	VectorMA( MainViewOrigin(), 10,   MainViewForward(), vecStart );
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

	if ( !tr.startsolid && tr.DidHitNonWorldEntity() )
	{
		C_BaseEntity *pEntity = tr.m_pEnt;

		if ( pEntity && (pEntity != this) )
		{
			m_iIDEntIndex = pEntity->entindex();
		}
	}
}

void C_HL2MP_Player::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator )
{
	Vector vecOrigin = ptr->endpos - vecDir * 4;

	float flDistance = 0.0f;
	
	if ( info.GetAttacker() )
	{
		flDistance = (ptr->endpos - info.GetAttacker()->GetAbsOrigin()).Length();
	}

	if ( m_takedamage )
	{
		AddMultiDamage( info, this );

		int blood = BloodColor();

		if ( blood != DONT_BLEED )
		{
			SpawnBlood( vecOrigin, vecDir, blood, flDistance );// a little surface blood.
			TraceBleed( flDistance, vecDir, ptr, info.GetDamageType() );
		}
	}
}


C_HL2MP_Player* C_HL2MP_Player::GetLocalHL2MPPlayer()
{
	return (C_HL2MP_Player*)C_BasePlayer::GetLocalPlayer();
}

//-----------------------------------------------------------------------------
/**
 * Orient head and eyes towards m_lookAt.
 */
void C_HL2MP_Player::UpdateLookAt( void )
{
	// head yaw
	if (m_headYawPoseParam < 0 || m_headPitchPoseParam < 0)
		return;

	// orient eyes
	m_viewtarget = m_vLookAtTarget;

	// blinking
	if (m_blinkTimer.IsElapsed())
	{
		m_blinktoggle = !m_blinktoggle;
		m_blinkTimer.Start( RandomFloat( 1.5f, 4.0f ) );
	}

	// Figure out where we want to look in world space.
	QAngle desiredAngles;
	Vector to = m_vLookAtTarget - EyePosition();
	VectorAngles( to, desiredAngles );

	// Figure out where our body is facing in world space.
	QAngle bodyAngles( 0, 0, 0 );
	bodyAngles[YAW] = GetLocalAngles()[YAW];


	float flBodyYawDiff = bodyAngles[YAW] - m_flLastBodyYaw;
	m_flLastBodyYaw = bodyAngles[YAW];
	

	// Set the head's yaw.
	float desired = AngleNormalize( desiredAngles[YAW] - bodyAngles[YAW] );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	m_flCurrentHeadYaw = ApproachAngle( desired, m_flCurrentHeadYaw, 130 * gpGlobals->frametime );

	// Counterrotate the head from the body rotation so it doesn't rotate past its target.
	m_flCurrentHeadYaw = AngleNormalize( m_flCurrentHeadYaw - flBodyYawDiff );
	desired = clamp( desired, m_headYawMin, m_headYawMax );
	
	SetPoseParameter( m_headYawPoseParam, m_flCurrentHeadYaw );

	
	// Set the head's yaw.
	desired = AngleNormalize( desiredAngles[PITCH] );
	desired = clamp( desired, m_headPitchMin, m_headPitchMax );
	
	m_flCurrentHeadPitch = ApproachAngle( desired, m_flCurrentHeadPitch, 130 * gpGlobals->frametime );
	m_flCurrentHeadPitch = AngleNormalize( m_flCurrentHeadPitch );
	SetPoseParameter( m_headPitchPoseParam, m_flCurrentHeadPitch );
}

void C_HL2MP_Player::ClientThink( void )
{
	bool bFoundViewTarget = false;
	
	Vector vForward;
	AngleVectors( GetLocalAngles(), &vForward );

	for( int iClient = 1; iClient <= gpGlobals->maxClients; ++iClient )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( iClient );
		if(!pEnt || !pEnt->IsPlayer())
			continue;

		if ( pEnt->entindex() == entindex() )
			continue;

		Vector vTargetOrigin = pEnt->GetAbsOrigin();
		Vector vMyOrigin =  GetAbsOrigin();

		Vector vDir = vTargetOrigin - vMyOrigin;
		
		if ( vDir.Length() > 128 ) 
			continue;

		VectorNormalize( vDir );

		if ( DotProduct( vForward, vDir ) < 0.0f )
			 continue;

		m_vLookAtTarget = pEnt->EyePosition();
		bFoundViewTarget = true;
		break;
	}

	if ( bFoundViewTarget == false )
	{
		m_vLookAtTarget = GetAbsOrigin() + vForward * 512;
	}

	UpdateIDTarget();

	// Avoidance
	if ( gpGlobals->curtime >= m_fNextThinkPushAway )
	{
		PerformObstaclePushaway( this );
		m_fNextThinkPushAway =  gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_HL2MP_Player::DrawModel( int flags )
{
	if ( !m_bReadyToDraw )
		return 0;

	return BaseClass::DrawModel(flags);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HL2MP_Player::ProcessMuzzleFlashEvent()
{
	//FIXME: We should really use a named attachment for this
	if ( m_Attachments.Count() > 0 )
	{
		Vector vAttachment, vAng;
		QAngle angles;

		GetAttachment( 1, vAttachment, angles ); // set 1 instead "attachment"
		AngleVectors( angles, &vAng );
		vAttachment += vAng * 2;
		
		dlight_t *dl = effects->CL_AllocDlight ( index );
		dl->origin = vAttachment;
		dl->color.r = 252;
		dl->color.g = 238;
		dl->color.b = 128;
		dl->die = gpGlobals->curtime + 0.05f;
		dl->radius = random->RandomFloat( 245.0f, 256.0f );
		dl->decay = 512.0f;
	}
}

//-----------------------------------------------------------------------------
// Should this object receive shadows?
//-----------------------------------------------------------------------------
bool C_HL2MP_Player::ShouldReceiveProjectedTextures( int flags )
{
	Assert( flags & SHADOW_FLAGS_PROJECTED_TEXTURE_TYPE_MASK );

	if ( IsEffectActive( EF_NODRAW ) )
		 return false;

	if( flags & SHADOW_FLAGS_FLASHLIGHT )
	{
		return true;
	}

	return BaseClass::ShouldReceiveProjectedTextures( flags );
}

void C_HL2MP_Player::DoImpactEffect( trace_t &tr, int nDamageType )
{
	if ( GetActiveWeapon() )
	{
		GetActiveWeapon()->DoImpactEffect( tr, nDamageType );
		return;
	}

	BaseClass::DoImpactEffect( tr, nDamageType );
}

void C_HL2MP_Player::PreThink( void )
{
	BaseClass::PreThink();
}

const QAngle &C_HL2MP_Player::EyeAngles()
{
	if( IsLocalPlayer() )
	{
		return BaseClass::EyeAngles();
	}
	else
	{
		return m_angEyeAngles;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HL2MP_Player::AddEntity( void )
{
	BaseClass::AddEntity();

	//Tony; modified so third person can do the beam too.
	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		//Tony; if local player, not in third person, and there's a beam, destroy it. It will get re-created if they go third person again.
		if ( this == C_BasePlayer::GetLocalPlayer() && !::input->CAM_IsThirdPerson() && m_pFlashlightBeam != NULL )
		{
			ReleaseFlashlight();
			return;
		}
		else if( this != C_BasePlayer::GetLocalPlayer() || ::input->CAM_IsThirdPerson() )
		{
			int iAttachment = LookupAttachment( "anim_attachment_RH" );

			if ( iAttachment < 0 )
				return;

			Vector vecOrigin;
			//Tony; EyeAngles will return proper whether it's local player or not.
			QAngle eyeAngles = EyeAngles();

			GetAttachment( iAttachment, vecOrigin, eyeAngles );

			Vector vForward;
			AngleVectors( eyeAngles, &vForward );
				
			trace_t tr;
			UTIL_TraceLine( vecOrigin, vecOrigin + (vForward * 200), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

			if( !m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_nType = TE_BEAMPOINTS;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_pszModelName = "sprites/glow01.vmt";
				beamInfo.m_pszHaloName = "sprites/glow01.vmt";
				beamInfo.m_flHaloScale = 3.0;
				beamInfo.m_flWidth = 8.0f;
				beamInfo.m_flEndWidth = 35.0f;
				beamInfo.m_flFadeLength = 300.0f;
				beamInfo.m_flAmplitude = 0;
				beamInfo.m_flBrightness = 60.0;
				beamInfo.m_flSpeed = 0.0f;
				beamInfo.m_nStartFrame = 0.0;
				beamInfo.m_flFrameRate = 0.0;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;
				beamInfo.m_nSegments = 8;
				beamInfo.m_bRenderable = true;
				beamInfo.m_flLife = 0.5;
				beamInfo.m_nFlags = FBEAM_FOREVER | FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
				
				m_pFlashlightBeam = beams->CreateBeamPoints( beamInfo );
			}

			if( m_pFlashlightBeam )
			{
				BeamInfo_t beamInfo;
				beamInfo.m_vecStart = tr.startpos;
				beamInfo.m_vecEnd = tr.endpos;
				beamInfo.m_flRed = 255.0;
				beamInfo.m_flGreen = 255.0;
				beamInfo.m_flBlue = 255.0;

				beams->UpdateBeamInfo( m_pFlashlightBeam, beamInfo );

				//Tony; local players don't make the dlight.
				if( this != C_BasePlayer::GetLocalPlayer() )
				{
					dlight_t *el = effects->CL_AllocDlight( 0 );
					el->origin = tr.endpos;
					el->radius = 50; 
					el->color.r = 200;
					el->color.g = 200;
					el->color.b = 200;
					el->die = gpGlobals->curtime + 0.1;
				}
			}
		}
	}
	else if ( m_pFlashlightBeam )
	{
		ReleaseFlashlight();
	}
}
//-----------------------------------------------------------------------------
// Purpose: Creates, destroys, and updates the flashlight effect as needed.
//-----------------------------------------------------------------------------
void C_HL2MP_Player::UpdateFlashlight()
{
	// The dim light is the flashlight.
	if ( IsEffectActive( EF_DIMLIGHT ) )
	{
		if (!m_pHL2MPFlashLightEffect)
		{
			// Turned on the headlight; create it.
			m_pHL2MPFlashLightEffect = new CHL2MPFlashlightEffect(index);

			if (!m_pHL2MPFlashLightEffect)
				return;

			m_pHL2MPFlashLightEffect->TurnOn();
		}

		Vector vecForward, vecRight, vecUp;
		Vector position = EyePosition();

		if ( ::input->CAM_IsThirdPerson() )
		{
			int iAttachment = LookupAttachment( "anim_attachment_RH" );

			if ( iAttachment >= 0 )
			{
				Vector vecOrigin;
				//Tony; EyeAngles will return proper whether it's local player or not.
				QAngle eyeAngles = EyeAngles();

				GetAttachment( iAttachment, vecOrigin, eyeAngles );

				Vector vForward;
				AngleVectors( eyeAngles, &vecForward, &vecRight, &vecUp );
				position = vecOrigin;
			}
			else
				EyeVectors( &vecForward, &vecRight, &vecUp );
		}
		else
			EyeVectors( &vecForward, &vecRight, &vecUp );

		// Update the light with the new position and direction.		
		m_pHL2MPFlashLightEffect->UpdateLight( position, vecForward, vecRight, vecUp, FLASHLIGHT_DISTANCE );
	}
	else if (m_pHL2MPFlashLightEffect)
	{
		// Turned off the flashlight; delete it.
		delete m_pHL2MPFlashLightEffect;
		m_pHL2MPFlashLightEffect = NULL;
	}
}

void CHL2MPFlashlightEffect::UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance )
{
	CFlashlightEffect::UpdateLight( vecPos, vecDir, vecRight, vecUp, nDistance );
}

ShadowType_t C_HL2MP_Player::ShadowCastType( void ) 
{
	if ( !IsVisible() )
		 return SHADOWS_NONE;

	return SHADOWS_RENDER_TO_TEXTURE_DYNAMIC;
}


const QAngle& C_HL2MP_Player::GetRenderAngles()
{
	if ( IsRagdoll() )
	{
		return vec3_angle;
	}
	else
	{
		return m_PlayerAnimState->GetRenderAngles();
	}
}

bool C_HL2MP_Player::ShouldDraw( void )
{
	// If we're dead, our ragdoll will be drawn for us instead.
	if ( !IsAlive() )
		return false;

	if( GetTeamNumber() == TEAM_SPECTATOR )
		return false;

	if( IsLocalPlayer() && IsRagdoll() )
		return true;
	
	if ( IsRagdoll() )
		return false;

	return BaseClass::ShouldDraw();
}

void C_HL2MP_Player::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if ( state == SHOULDTRANSMIT_END )
	{
		if( m_pFlashlightBeam != NULL )
		{
			ReleaseFlashlight();
		}
	}

	BaseClass::NotifyShouldTransmit( state );
}

void C_HL2MP_Player::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	UpdateVisibility();
}

void C_HL2MP_Player::PostDataUpdate( DataUpdateType_t updateType )
{
	if ( m_iSpawnInterpCounter != m_iSpawnInterpCounterCache )
	{
		MoveToLastReceivedPosition( true );
		ResetLatched();
		m_iSpawnInterpCounterCache = m_iSpawnInterpCounter;
	}

	BaseClass::PostDataUpdate( updateType );
}

void C_HL2MP_Player::RecvProxy_CycleLatch( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	C_HL2MP_Player* pPlayer = static_cast<C_HL2MP_Player*>( pStruct );

	float flServerCycle = (float)pData->m_Value.m_Int / 16.0f;
	float flCurCycle = pPlayer->GetCycle();
	// The cycle is way out of sync.
	if ( fabs( flCurCycle - flServerCycle ) > CYCLELATCH_TOLERANCE )
	{
		pPlayer->SetServerIntendedCycle( flServerCycle );
	}
}

void C_HL2MP_Player::ReleaseFlashlight( void )
{
	if( m_pFlashlightBeam )
	{
		m_pFlashlightBeam->flags = 0;
		m_pFlashlightBeam->die = gpGlobals->curtime - 1;

		m_pFlashlightBeam = NULL;
	}
}

float C_HL2MP_Player::GetFOV( void )
{
	//Find our FOV with offset zoom value
	float flFOVOffset = C_BasePlayer::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = GetMinFOV();
	
	// Don't let it go too low
	flFOVOffset = MAX( min_fov, flFOVOffset );

	return flFOVOffset;
}

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector C_HL2MP_Player::GetAutoaimVector( float flDelta )
{
	// Never autoaim a predicted weapon (for now)
	Vector	forward;
	AngleVectors( EyeAngles() + m_Local.m_vecPunchAngle, &forward );
	return	forward;
}

void C_HL2MP_Player::ItemPreFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	// Disallow shooting while zooming
	if ( m_nButtons & IN_ZOOM )
	{
		//FIXME: Held weapons like the grenade get sad when this happens
		m_nButtons &= ~(IN_ATTACK|IN_ATTACK2);
	}

	BaseClass::ItemPreFrame();

}
	
void C_HL2MP_Player::ItemPostFrame( void )
{
	if ( GetFlags() & FL_FROZEN )
		 return;

	BaseClass::ItemPostFrame();
}

C_BaseAnimating *C_HL2MP_Player::BecomeRagdollOnClient()
{
	// Let the C_CSRagdoll entity do this.
	// m_builtRagdoll = true;
	return NULL;
}


//-----------------------------------------------------------------------------
// Purpose: HL1's view bob, roll and idle effects.
//-----------------------------------------------------------------------------
void C_HL2MP_Player::CalcVehicleView( IClientVehicle* pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov )
{
	BaseClass::CalcVehicleView( pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov );

	if ( pVehicle != nullptr )
	{
		if ( pVehicle->GetVehicleEnt() != nullptr )
		{
			Vector Velocity;
			pVehicle->GetVehicleEnt()->EstimateAbsVelocity( Velocity );

			if ( Velocity.Length() == 0 )
			{
				IdleScale += gpGlobals->frametime * 0.05;
				if ( IdleScale > 1.0 )
					IdleScale = 1.0;
			}
			else
			{
				IdleScale -= gpGlobals->frametime;
				if ( IdleScale < 0.0 )
					IdleScale = 0.0;
			}

			CalcViewIdle( eyeAngles );
		}
	}
}

void C_HL2MP_Player::CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov )
{
	BaseClass::CalcPlayerView( eyeOrigin, eyeAngles, fov );

	Vector Velocity;
	EstimateAbsVelocity( Velocity );

	if ( Velocity.Length() == 0 )
	{
		IdleScale += gpGlobals->frametime * 0.05;
		if ( IdleScale > 1.0 )
			IdleScale = 1.0;
	}
	else
	{
		IdleScale -= gpGlobals->frametime;
		if ( IdleScale < 0.0 )
			IdleScale = 0.0;
	}

	CalcViewBob( eyeOrigin );
	CalcViewIdle( eyeAngles );
}

void C_HL2MP_Player::CalcViewRoll( QAngle& eyeAngles )
{
	if ( GetMoveType() == MOVETYPE_NOCLIP )
		return;

	float Side = CalcRoll( GetAbsAngles(), GetAbsVelocity(), cl_rollangle.GetFloat(), cl_rollspeed.GetFloat() ) * 4.0;
	eyeAngles[ROLL] += Side;

	if ( GetHealth() <= 0 )
	{
		eyeAngles[ROLL] = 80;
		return;
	}
}

void C_HL2MP_Player::CalcViewBob( Vector& eyeOrigin )
{
	if ( !IsAlive() )
		return;

	float Cycle;
	Vector Velocity;

	if ( GetGroundEntity() == nullptr || gpGlobals->curtime == BobLastTime )
	{
		eyeOrigin.z += ViewBob;
		return;
	}

	BobLastTime = gpGlobals->curtime;
	BobTime += gpGlobals->frametime;

	Cycle = BobTime - (int)( BobTime / cl_bobcycle.GetFloat() ) * cl_bobcycle.GetFloat();
	Cycle /= cl_bobcycle.GetFloat();

	if ( Cycle < cl_bobup.GetFloat() )
		Cycle = M_PI * Cycle / cl_bobup.GetFloat();
	else
		Cycle = M_PI + M_PI * ( Cycle - cl_bobup.GetFloat()) / (1.0 - cl_bobup.GetFloat() );

	EstimateAbsVelocity( Velocity );
	Velocity.z = 0;

	ViewBob = sqrt( Velocity.x * Velocity.x + Velocity.y * Velocity.y ) * cl_bob.GetFloat();
	ViewBob = ViewBob * 0.3 + ViewBob * 0.7 * sin(Cycle);
	ViewBob = min( ViewBob, 4 );
	ViewBob = max( ViewBob, -7 );

	eyeOrigin.z += ViewBob;
}

void C_HL2MP_Player::CalcViewIdle( QAngle& eyeAngles )
{
	if ( !IsAlive() )
		return;

	eyeAngles[ROLL] += IdleScale * sin( gpGlobals->curtime * cl_iroll_cycle.GetFloat() ) * cl_iroll_level.GetFloat();
	eyeAngles[PITCH] += IdleScale * sin( gpGlobals->curtime * cl_ipitch_cycle.GetFloat() ) * cl_ipitch_level.GetFloat();
	eyeAngles[YAW] += IdleScale * sin( gpGlobals->curtime * cl_iyaw_cycle.GetFloat() ) * cl_iyaw_level.GetFloat();
}

IRagdoll* C_HL2MP_Player::GetRepresentativeRagdoll() const
{
	if ( m_hRagdoll.Get() )
	{
		C_HL2MPRagdoll *pRagdoll = (C_HL2MPRagdoll*)m_hRagdoll.Get();

		return pRagdoll->GetIRagdoll();
	}
	else
	{
		return NULL;
	}
}

//HL2MPRAGDOLL


IMPLEMENT_CLIENTCLASS_DT_NOBASE( C_HL2MPRagdoll, DT_HL2MPRagdoll, CHL2MPRagdoll )
	RecvPropVector( RECVINFO(m_vecRagdollOrigin) ),
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO(m_nForceBone) ),
	RecvPropVector( RECVINFO(m_vecForce) ),
	RecvPropVector( RECVINFO( m_vecRagdollVelocity ) )
END_RECV_TABLE()



C_HL2MPRagdoll::C_HL2MPRagdoll()
{

}

C_HL2MPRagdoll::~C_HL2MPRagdoll()
{
	PhysCleanupFrictionSounds( this );

	if ( m_hPlayer )
	{
		m_hPlayer->CreateModelInstance();
	}
}

void C_HL2MPRagdoll::Interp_Copy( C_BaseAnimatingOverlay *pSourceEntity )
{
	if ( !pSourceEntity )
		return;
	
	VarMapping_t *pSrc = pSourceEntity->GetVarMapping();
	VarMapping_t *pDest = GetVarMapping();
		
	// Find all the VarMapEntry_t's that represent the same variable.
	for ( int i = 0; i < pDest->m_Entries.Count(); i++ )
	{
		VarMapEntry_t *pDestEntry = &pDest->m_Entries[i];
		const char *pszName = pDestEntry->watcher->GetDebugName();
		for ( int j=0; j < pSrc->m_Entries.Count(); j++ )
		{
			VarMapEntry_t *pSrcEntry = &pSrc->m_Entries[j];
			if ( !Q_strcmp( pSrcEntry->watcher->GetDebugName(), pszName ) )
			{
				pDestEntry->watcher->Copy( pSrcEntry->watcher );
				break;
			}
		}
	}
}

void C_HL2MPRagdoll::ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if( !pPhysicsObject )
		return;

	Vector dir = pTrace->endpos - pTrace->startpos;

	if ( iDamageType == DMG_BLAST )
	{
		dir *= 4000;  // adjust impact strenght
				
		// apply force at object mass center
		pPhysicsObject->ApplyForceCenter( dir );
	}
	else
	{
		Vector hitpos;  
	
		VectorMA( pTrace->startpos, pTrace->fraction, dir, hitpos );
		VectorNormalize( dir );

		dir *= 4000;  // adjust impact strenght

		// apply force where we hit it
		pPhysicsObject->ApplyForceOffset( dir, hitpos );	

		// Blood spray!
//		FX_CS_BloodSpray( hitpos, dir, 10 );
	}

	m_pRagdoll->ResetRagdollSleepAfterTime();
}


void C_HL2MPRagdoll::CreateHL2MPRagdoll( void )
{
	// First, initialize all our data. If we have the player's entity on our client,
	// then we can make ourselves start out exactly where the player is.
	C_HL2MP_Player *pPlayer = dynamic_cast< C_HL2MP_Player* >( m_hPlayer.Get() );
	
	if ( pPlayer && !pPlayer->IsDormant() )
	{
		// move my current model instance to the ragdoll's so decals are preserved.
		pPlayer->SnatchModelInstance( this );

		VarMapping_t *varMap = GetVarMapping();

		// Copy all the interpolated vars from the player entity.
		// The entity uses the interpolated history to get bone velocity.
		bool bRemotePlayer = (pPlayer != C_BasePlayer::GetLocalPlayer());			
		if ( bRemotePlayer )
		{
			Interp_Copy( pPlayer );

			SetAbsAngles( pPlayer->GetRenderAngles() );
			GetRotationInterpolator().Reset();

			m_flAnimTime = pPlayer->m_flAnimTime;
			SetSequence( pPlayer->GetSequence() );
			m_flPlaybackRate = pPlayer->GetPlaybackRate();
		}
		else
		{
			// This is the local player, so set them in a default
			// pose and slam their velocity, angles and origin
			SetAbsOrigin( m_vecRagdollOrigin );
			
			SetAbsAngles( pPlayer->GetRenderAngles() );

			SetAbsVelocity( m_vecRagdollVelocity );

			int iSeq = pPlayer->GetSequence();
			if ( iSeq == -1 )
			{
				Assert( false );	// missing walk_lower?
				iSeq = 0;
			}
			
			SetSequence( iSeq );	// walk_lower, basic pose
			SetCycle( 0.0 );

			Interp_Reset( varMap );
		}		
	}
	else
	{
		// overwrite network origin so later interpolation will
		// use this position
		SetNetworkOrigin( m_vecRagdollOrigin );

		SetAbsOrigin( m_vecRagdollOrigin );
		SetAbsVelocity( m_vecRagdollVelocity );

		Interp_Reset( GetVarMapping() );
		
	}

	SetModelIndex( m_nModelIndex );

	// Make us a ragdoll..
	m_nRenderFX = kRenderFxRagdoll;

	matrix3x4_t boneDelta0[MAXSTUDIOBONES];
	matrix3x4_t boneDelta1[MAXSTUDIOBONES];
	matrix3x4_t currentBones[MAXSTUDIOBONES];
	const float boneDt = 0.05f;

	if ( pPlayer && !pPlayer->IsDormant() )
	{
		pPlayer->GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
	}
	else
	{
		GetRagdollInitBoneArrays( boneDelta0, boneDelta1, currentBones, boneDt );
	}

	InitAsClientRagdoll( boneDelta0, boneDelta1, currentBones, boneDt );
}


void C_HL2MPRagdoll::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if ( type == DATA_UPDATE_CREATED )
	{
		CreateHL2MPRagdoll();
	}
}

IRagdoll* C_HL2MPRagdoll::GetIRagdoll() const
{
	return m_pRagdoll;
}

void C_HL2MPRagdoll::UpdateOnRemove( void )
{
	VPhysicsSetObject( NULL );

	BaseClass::UpdateOnRemove();
}

//-----------------------------------------------------------------------------
// Purpose: clear out any face/eye values stored in the material system
//-----------------------------------------------------------------------------
void C_HL2MPRagdoll::SetupWeights( const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights )
{
	BaseClass::SetupWeights( pBoneToWorld, nFlexWeightCount, pFlexWeights, pFlexDelayedWeights );

	static float destweight[128];
	static bool bIsInited = false;

	CStudioHdr *hdr = GetModelPtr();
	if ( !hdr )
		return;

	int nFlexDescCount = hdr->numflexdesc();
	if ( nFlexDescCount )
	{
		Assert( !pFlexDelayedWeights );
		memset( pFlexWeights, 0, nFlexWeightCount * sizeof(float) );
	}

	if ( m_iEyeAttachment > 0 )
	{
		matrix3x4_t attToWorld;
		if (GetAttachment( m_iEyeAttachment, attToWorld ))
		{
			Vector local, tmp;
			local.Init( 1000.0f, 0.0f, 0.0f );
			VectorTransform( local, attToWorld, tmp );
			modelrender->SetViewTarget( GetModelPtr(), GetBody(), tmp );
		}
	}
}

void C_HL2MP_Player::UpdateClientSideAnimation()
{
	m_PlayerAnimState->Update( EyeAngles()[YAW], EyeAngles()[PITCH] );

	BaseClass::UpdateClientSideAnimation();
}

CStudioHdr *C_HL2MP_Player::OnNewModel( void )
{
	CStudioHdr *hdr = BaseClass::OnNewModel();

	InitializePoseParams();

	// Reset the players animation states, gestures
	if ( m_PlayerAnimState )
	{
		m_PlayerAnimState->OnNewModel();
	}

	return hdr;
}
//-----------------------------------------------------------------------------
// Purpose: Clear all pose parameters
//-----------------------------------------------------------------------------
void C_HL2MP_Player::InitializePoseParams( void )
{
	m_headYawPoseParam = LookupPoseParameter( "head_yaw" );
	GetPoseParameterRange( m_headYawPoseParam, m_headYawMin, m_headYawMax );

	m_headPitchPoseParam = LookupPoseParameter( "head_pitch" );
	GetPoseParameterRange( m_headPitchPoseParam, m_headPitchMin, m_headPitchMax );

	CStudioHdr *hdr = GetModelPtr();
	for ( int i = 0; i < hdr->GetNumPoseParameters() ; i++ )
	{
		SetPoseParameter( hdr, i, 0.0 );
	}
}
// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class C_TEPlayerAnimEvent : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEPlayerAnimEvent, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

	virtual void PostDataUpdate( DataUpdateType_t updateType )
	{
		// Create the effect.
		C_HL2MP_Player *pPlayer = dynamic_cast< C_HL2MP_Player* >( m_hPlayer.Get() );
		if ( pPlayer && !pPlayer->IsDormant() )
		{
			pPlayer->DoAnimationEvent( (PlayerAnimEvent_t)m_iEvent.Get(), m_nData );
		}	
	}

public:
	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
	CNetworkVar( int, m_nData );
};

IMPLEMENT_CLIENTCLASS_EVENT( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent, CTEPlayerAnimEvent );

BEGIN_RECV_TABLE_NOBASE( C_TEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	RecvPropEHandle( RECVINFO( m_hPlayer ) ),
	RecvPropInt( RECVINFO( m_iEvent ) ),
	RecvPropInt( RECVINFO( m_nData ) )
END_RECV_TABLE()

void C_HL2MP_Player::DoAnimationEvent( PlayerAnimEvent_t event, int nData )
{
	if ( IsLocalPlayer() )
	{
		if ( ( prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
			return;
	}

	MDLCACHE_CRITICAL_SECTION();
	m_PlayerAnimState->DoAnimationEvent( event, nData );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_HL2MP_Player::CalculateIKLocks( float currentTime )
{
	if (!m_pIk) 
		return;

	int targetCount = m_pIk->m_target.Count();
	if ( targetCount == 0 )
		return;

	// In TF, we might be attaching a player's view to a walking model that's using IK. If we are, it can
	// get in here during the view setup code, and it's not normally supposed to be able to access the spatial
	// partition that early in the rendering loop. So we allow access right here for that special case.
	SpatialPartitionListMask_t curSuppressed = partition->GetSuppressedLists();
	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );
	CBaseEntity::PushEnableAbsRecomputations( false );

	for (int i = 0; i < targetCount; i++)
	{
		trace_t trace;
		CIKTarget *pTarget = &m_pIk->m_target[i];

		if (!pTarget->IsActive())
			continue;

		switch( pTarget->type)
		{
		case IK_GROUND:
			{
				pTarget->SetPos( Vector( pTarget->est.pos.x, pTarget->est.pos.y, GetRenderOrigin().z ));
				pTarget->SetAngles( GetRenderAngles() );
			}
			break;

		case IK_ATTACHMENT:
			{
				C_BaseEntity *pEntity = NULL;
				float flDist = pTarget->est.radius;

				// FIXME: make entity finding sticky!
				// FIXME: what should the radius check be?
				for ( CEntitySphereQuery sphere( pTarget->est.pos, 64 ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
				{
					C_BaseAnimating *pAnim = pEntity->GetBaseAnimating( );
					if (!pAnim)
						continue;

					int iAttachment = pAnim->LookupAttachment( pTarget->offset.pAttachmentName );
					if (iAttachment <= 0)
						continue;

					Vector origin;
					QAngle angles;
					pAnim->GetAttachment( iAttachment, origin, angles );

					// debugoverlay->AddBoxOverlay( origin, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 255, 0, 0, 0, 0 );

					float d = (pTarget->est.pos - origin).Length();

					if ( d >= flDist)
						continue;

					flDist = d;
					pTarget->SetPos( origin );
					pTarget->SetAngles( angles );
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 255, 0, 0, 0 );
				}

				if (flDist >= pTarget->est.radius)
				{
					// debugoverlay->AddBoxOverlay( pTarget->est.pos, Vector( -pTarget->est.radius, -pTarget->est.radius, -pTarget->est.radius ), Vector( pTarget->est.radius, pTarget->est.radius, pTarget->est.radius), QAngle( 0, 0, 0 ), 0, 0, 255, 0, 0 );
					// no solution, disable ik rule
					pTarget->IKFailed( );
				}
			}
			break;
		}
	}

	CBaseEntity::PopEnableAbsRecomputations();
	partition->SuppressLists( curSuppressed, true );
}

//-----------------------------------------------------------------------------
// Purpose: Try to steer away from any players and objects we might interpenetrate
//-----------------------------------------------------------------------------
#define HL2MP_AVOID_MAX_RADIUS_SQR		5184.0f			// Based on player extents and max buildable extents.
#define HL2MP_OO_AVOID_MAX_RADIUS_SQR		0.00029f

ConVar hl2mp_max_separation_force ( "hl2mp_max_separation_force", "256", FCVAR_CHEAT|FCVAR_HIDDEN );

extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

void C_HL2MP_Player::AvoidPlayers( CUserCmd *pCmd )
{
	// This is only used in team play.
	if ( !HL2MPRules()->IsTeamplay() )
		return;

	// Don't test if the player doesn't exist or is dead.
	if ( IsAlive() == false )
		return;

	C_Team *pTeam = ( C_Team * )GetTeam();
	if ( !pTeam )
		return;

	// Up vector.
	static Vector vecUp( 0.0f, 0.0f, 1.0f );

	Vector vecHL2MPPlayerCenter = GetAbsOrigin();
	Vector vecHL2MPPlayerMin = GetPlayerMins();
	Vector vecHL2MPPlayerMax = GetPlayerMaxs();
	float flZHeight = vecHL2MPPlayerMax.z - vecHL2MPPlayerMin.z;
	vecHL2MPPlayerCenter.z += 0.5f * flZHeight;
	VectorAdd( vecHL2MPPlayerMin, vecHL2MPPlayerCenter, vecHL2MPPlayerMin );
	VectorAdd( vecHL2MPPlayerMax, vecHL2MPPlayerCenter, vecHL2MPPlayerMax );

	// Find an intersecting player or object.
	int nAvoidPlayerCount = 0;
	C_HL2MP_Player *pAvoidPlayerList[MAX_PLAYERS];

	C_HL2MP_Player *pIntersectPlayer = NULL;
	float flAvoidRadius = 0.0f;

	Vector vecAvoidCenter, vecAvoidMin, vecAvoidMax;
	for ( int i = 0; i < pTeam->GetNumPlayers(); ++i )
	{
		C_HL2MP_Player *pAvoidPlayer = static_cast< C_HL2MP_Player * >( pTeam->GetPlayer( i ) );
		if ( pAvoidPlayer == NULL )
			continue;
		// Is the avoid player me?
		if ( pAvoidPlayer == this )
			continue;

		// Save as list to check against for objects.
		pAvoidPlayerList[nAvoidPlayerCount] = pAvoidPlayer;
		++nAvoidPlayerCount;

		// Check to see if the avoid player is dormant.
		if ( pAvoidPlayer->IsDormant() )
			continue;

		// Is the avoid player solid?
		if ( pAvoidPlayer->IsSolidFlagSet( FSOLID_NOT_SOLID ) )
			continue;

		Vector t1, t2;

		vecAvoidCenter = pAvoidPlayer->GetAbsOrigin();
		vecAvoidMin = pAvoidPlayer->GetPlayerMins();
		vecAvoidMax = pAvoidPlayer->GetPlayerMaxs();
		flZHeight = vecAvoidMax.z - vecAvoidMin.z;
		vecAvoidCenter.z += 0.5f * flZHeight;
		VectorAdd( vecAvoidMin, vecAvoidCenter, vecAvoidMin );
		VectorAdd( vecAvoidMax, vecAvoidCenter, vecAvoidMax );

		if ( IsBoxIntersectingBox( vecHL2MPPlayerMin, vecHL2MPPlayerMax, vecAvoidMin, vecAvoidMax ) )
		{
			// Need to avoid this player.
			if ( !pIntersectPlayer )
			{
				pIntersectPlayer = pAvoidPlayer;
				break;
			}
		}
	}

	// Anything to avoid?
	if ( !pIntersectPlayer )
		return;

	// Calculate the push strength and direction.
	Vector vecDelta;

	// Avoid a player - they have precedence.
	if ( pIntersectPlayer )
	{
		VectorSubtract( pIntersectPlayer->WorldSpaceCenter(), vecHL2MPPlayerCenter, vecDelta );

		Vector vRad = pIntersectPlayer->WorldAlignMaxs() - pIntersectPlayer->WorldAlignMins();
		vRad.z = 0;

		flAvoidRadius = vRad.Length();
	}

	float flPushStrength = RemapValClamped( vecDelta.Length(), flAvoidRadius, 0, 0, hl2mp_max_separation_force.GetInt() ); //flPushScale;

	//Msg( "PushScale = %f\n", flPushStrength );

	// Check to see if we have enough push strength to make a difference.
	if ( flPushStrength < 0.01f )
		return;

	Vector vecPush;
	if ( GetAbsVelocity().Length2DSqr() > 0.1f )
	{
		Vector vecVelocity = GetAbsVelocity();
		vecVelocity.z = 0.0f;
		CrossProduct( vecUp, vecVelocity, vecPush );
		VectorNormalize( vecPush );
	}
	else
	{
		// We are not moving, but we're still intersecting.
		QAngle angView = pCmd->viewangles;
		angView.x = 0.0f;
		AngleVectors( angView, NULL, &vecPush, NULL );
	}

	// Move away from the other player/object.
	Vector vecSeparationVelocity;
	if ( vecDelta.Dot( vecPush ) < 0 )
	{
		vecSeparationVelocity = vecPush * flPushStrength;
	}
	else
	{
		vecSeparationVelocity = vecPush * -flPushStrength;
	}

	// Don't allow the max push speed to be greater than the max player speed.
	float flMaxPlayerSpeed = MaxSpeed();
	float flCropFraction = 1.33333333f;

	if ( ( GetFlags() & FL_DUCKING ) && ( GetGroundEntity() != NULL ) )
	{	
		flMaxPlayerSpeed *= flCropFraction;
	}	

	float flMaxPlayerSpeedSqr = flMaxPlayerSpeed * flMaxPlayerSpeed;

	if ( vecSeparationVelocity.LengthSqr() > flMaxPlayerSpeedSqr )
	{
		vecSeparationVelocity.NormalizeInPlace();
		VectorScale( vecSeparationVelocity, flMaxPlayerSpeed, vecSeparationVelocity );
	}

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;
	Vector currentdir;
	Vector rightdir;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );

	Vector vDirection = vecSeparationVelocity;

	VectorNormalize( vDirection );

	float fwd = currentdir.Dot( vDirection );
	float rt = rightdir.Dot( vDirection );

	float forward = fwd * flPushStrength;
	float side = rt * flPushStrength;

	//Msg( "fwd: %f - rt: %f - forward: %f - side: %f\n", fwd, rt, forward, side );

	pCmd->forwardmove	+= forward;
	pCmd->sidemove		+= side;

	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}

	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}

	float flScale = min( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "Pforwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
}

bool C_HL2MP_Player::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{	
	static QAngle angMoveAngle( 0.0f, 0.0f, 0.0f );

	VectorCopy( pCmd->viewangles, angMoveAngle );

	BaseClass::CreateMove( flInputSampleTime, pCmd );

	AvoidPlayers( pCmd );

	return true;
}
