#include "cbase.h"
#include "dmc_bot.h"
#include "hl2mp_gamerules.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CDMCBot::AcquireEnemy()
{
	float minDist = FLT_MAX;
	bool Success = false;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (pPlayer == this)
			continue;

		if (!pPlayer->IsAlive())
			continue;

		//if (HL2MPRules()->PlayerRelationship(this, pPlayer) == GR_TEAMMATE)
		//	continue;

		float dist = (GetLocalOrigin() - pPlayer->GetLocalOrigin()).Length();

		if( dist < minDist )
		{
			minDist = dist;
			hEnemy.Set(pPlayer);
			Success = true;
		}
	}

	return Success;
}

void CDMCBot::Attack( CUserCmd &cmd )
{
	if (!m_bEnemyOnSights)
		return;

	if (!GetActiveWeapon())
		return;

	if (m_bInRangeToAttack && gpGlobals->curtime > m_flNextBotMeleeAttack )
	{
		if (GetActiveWeapon()->IsMeleeWeapon())
		{
			cmd.buttons |= (random->RandomInt(0, 1) == 0)?IN_ATTACK:IN_ATTACK2;
			m_flNextBotMeleeAttack = gpGlobals->curtime + 0.75f;
		}
		else
		{
			cmd.buttons |= IN_ATTACK2;
			m_flNextBotMeleeAttack = gpGlobals->curtime + 0.75f;
		}
	}
	else
	{
		if ( GetActiveWeapon()->Clip1() )
			cmd.buttons |= IN_ATTACK;
	}

	// Try to throw a grenade.
	if ( random->RandomInt( 0, 1 ) )
		cmd.buttons |= IN_ALT2;
}
