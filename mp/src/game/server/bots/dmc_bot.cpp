#include "cbase.h"
#include "dmc_bot.h"
#include "movehelper_server.h"
#include "datacache/imdlcache.h"
#include "BasePropDoor.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar bot_mimic( "bot_mimic", "0", FCVAR_CHEAT );
ConVar bot_freeze( "bot_freeze", "0", FCVAR_CHEAT );
ConVar bot_crouch( "bot_crouch", "0", FCVAR_CHEAT );
ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "180", FCVAR_CHEAT );
ConVar bot_attack( "bot_attack", "0", FCVAR_CHEAT );
ConVar bot_mimic_flip_pitch( "bot_mimic_flip_pitch", "0", FCVAR_CHEAT );
ConVar bot_quota("bot_quota", "0", FCVAR_ARCHIVE|FCVAR_GAMEDLL, "Try to keep this many bots in the server");
ConVar bot_reconnect("bot_reconnect", "0", FCVAR_CHEAT);

LINK_ENTITY_TO_CLASS( bot, CDMCBot );

void CDMCBot::Initialize()
{
	// set bot skills
	m_flSkill[BOT_SKILL_YAW_RATE] = 10;
	m_flSkill[BOT_SKILL_SPEED] = 400; 
	m_flSkill[BOT_SKILL_STRAFE] = 5;

	//State_Transition( STATE_ACTIVE );

	CCommand args;
	args.Tokenize( "joingame" );
	this->ClientCommand( args );
}

void CDMCBot::Spawn()
{
	BaseClass::Spawn();

	hEnemy.Set( NULL );
	ResetNavigationParams();
	m_AlreadyCheckedHideSpots.RemoveAll();
	m_flNextDealObstacles = 0;
	m_flCreateRandomPathCoolDown = 0;
	m_flNextProximityCheck = 0;
	m_flDistTraveled = 0; // distance this bot has traveled recently, since last stuck check
	m_flMinRangeAttack = 60.0f;
	m_bInRangeToAttack = false;
	m_flNextStrafeTime = 0;
	m_flStrafeSkillRelatedTimer = 0;
	m_flNextBotMeleeAttack = 0;
}

void CDMCBot::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	m_flLastDeathTime = gpGlobals->curtime;
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input : *fakeclient -
// *viewangles -
// forwardmove -
// m_flSideMove -
// upmove -
// buttons -
// impulse -
// msec -
// Output : virtual void
//-----------------------------------------------------------------------------
void CDMCBot::RunPlayerMove( CUserCmd &cmd, float frametime )
{
	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( this );

	MDLCACHE_CRITICAL_SECTION();

	PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}

void CDMCBot::HandleRespawn( CUserCmd &cmd )
{
	if ( gpGlobals->curtime < m_flLastDeathTime + 5 )
		return;

	//State_Transition( STATE_ACTIVE );

	// Try hitting my buttons occasionally
	if ( random->RandomInt( 0, 100 ) > 80 )
	{
		// Respawn the bot
		if ( random->RandomInt( 0, 1 ) == 0 )
			cmd.buttons |= IN_JUMP;
		else
			cmd.buttons = 0;
	}
}

// here bot updates important info that is used multiple times along the thinking process
void CDMCBot::InfoGathering()
{
	if ( !GetEnemy() )
	{
		m_flBotToEnemyDist = 9999;
		m_flHeightDifToEnemy = 0;
		m_bEnemyOnSights = false;

		m_flDistTraveled += fabs( GetLocalVelocity().Length() ); // this is used for stuck checking,
		return;
	}

	m_flBotToEnemyDist = ( GetLocalOrigin() - GetEnemy()->GetLocalOrigin() ).Length();

	trace_t tr;
	UTIL_TraceHull( EyePosition(), GetEnemy()->EyePosition() - Vector(0,0,20), -BotTestHull, BotTestHull, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );

	if( tr.m_pEnt == GetEnemy() ) // vision line between both
		m_bEnemyOnSights = true;
	else
		m_bEnemyOnSights = false;

	m_bInRangeToAttack = (m_flBotToEnemyDist < m_flMinRangeAttack) && FInViewCone( GetEnemy() );

	m_flDistTraveled += fabs(GetLocalVelocity().Length()); // this is used for stuck checking,

	m_flHeightDifToEnemy = GetLocalOrigin().z - GetEnemy()->GetLocalOrigin().z;
}

bool CHL2MP_Player::RunMimicCommand( CUserCmd& cmd )
{
	if ( !IsBot() )
		return false;

	if ( bot_freeze.GetBool() )
		return true;

	int iMimic = abs( bot_mimic.GetInt() );
	if ( iMimic > gpGlobals->maxClients )
		return false;

	CBasePlayer *pPlayer = UTIL_PlayerByIndex( iMimic );
	if ( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand() )
		return false;

	cmd = *pPlayer->GetLastUserCommand();
	cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

	if ( bot_mimic_flip_pitch.GetBool() )
		cmd.viewangles[PITCH] = -cmd.viewangles[PITCH];

	pl.fixangle = FIXANGLE_NONE;

	return true;
}

//-----------------------------------------------------------------------------
// Run this Bot's AI for one tick.
//-----------------------------------------------------------------------------
void CDMCBot::BotThink()
{
	// Make sure we stay being a bot
	AddFlag( FL_FAKECLIENT );

	if ( IsEFlagSet( EFL_BOT_FROZEN ) )
		return;

	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );

	if ( !IsAlive() )
	{
		HandleRespawn( cmd );
	}
	else if ( bot_mimic.GetBool() )
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt()  );
		if ( pPlayer && pPlayer->GetLastUserCommand() )
		{
			cmd = *pPlayer->GetLastUserCommand();

			ConVarRef bot_mimic_yaw_offset( "bot_mimic_yaw_offset" );
			cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

			if ( bot_mimic_flip_pitch.GetBool() )
				cmd.viewangles[PITCH] = -cmd.viewangles[PITCH];

			if( bot_crouch.GetInt() )
				cmd.buttons |= IN_DUCK;
		}
	}
	else if ( bot_freeze.GetBool() )
	{
		if ( bot_attack.GetBool() )
		{
			if ( GetActiveWeapon()->Clip1() == 0 )
				cmd.buttons |= IN_RELOAD;
			else if ( RandomFloat( 0.0,1.0 ) > 0.5 && gpGlobals->curtime >= GetNextAttack() )
				cmd.buttons |= IN_ATTACK;
		}
	}
	else
	{
		trace_t tr_front;
		Vector Forward;
		AngleVectors(GetLocalAngles(), &Forward);
		UTIL_TraceHull( GetLocalOrigin()+Vector(0,0,5), GetLocalOrigin() + Vector(0,0,5) + (Forward * 50), GetPlayerMins(), GetPlayerMaxs(), MASK_PLAYERSOLID, this, COLLISION_GROUP_NONE, &tr_front );

		// enemy acquisition
		if( !GetEnemy() || RecheckEnemy() || !GetEnemy()->IsAlive() )
		{
			if( GetEnemy() && !GetEnemy()->IsAlive() )
				ResetNavigationParams();

			AcquireEnemy();

			m_flTimeToRecheckEnemy = gpGlobals->curtime + 1.0f;
		}

		// assume we have an enemy from now on

		InfoGathering();

		Attack(cmd);

		if( m_flTimeToRecheckStuck < gpGlobals->curtime )
			CheckStuck( cmd );

		if( m_flNextDealObstacles < gpGlobals->curtime )
			DealWithObstacles( tr_front.m_pEnt, cmd );

		Navigation(cmd);

		CheckNavMeshAttrib( &tr_front, cmd );
	}

	RunPlayerMove( cmd, gpGlobals->frametime );

	int m_iCanWeChat = random->RandomInt( 1, 90 );
	if ( m_iCanWeChat == 30 )
	{
		void Bot_Say( CDMCBot *pBot );
		Bot_Say( this );
	}
}

CON_COMMAND_F( bot_teleport, "Teleports the first bot to the player.", FCVAR_CHEAT )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 

	if (!pPlayer)
		return;

	Vector vecEye = pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset();

	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward, NULL, NULL);

	trace_t tr;
	UTIL_TraceHull(vecEye, vecEye + vecForward * 100, VEC_HULL_MIN, VEC_HULL_MAX, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER_MOVEMENT, &tr);

	NDebugOverlay::Line(vecEye, tr.endpos, 0, 0, 255, false, 10);

	CHL2MP_Player* pBot = NULL;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player* pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		if (pPlayer->IsBot())
		{
			pBot = pPlayer;
			break;
		}
	}

	if (!pBot)
		return;

	pBot->SetAbsOrigin( tr.endpos );
}