//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#include "cbase.h"
#include "player.h"
#include "hl2mp_player.h"
#include "in_buttons.h"
#include "gameinterface.h"
#include "bot_main.h"
#include "dmc_bot.h"
#include "hl2mp_gamerules.h"

// support for nav mesh
#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CDMCBot;
void Bot_Think( CDMCBot *pBot );

extern ConVar bot_quota;
extern ConVar bot_reconnect;

// Handler for the "bot" command.
CON_COMMAND_F( bot_add, "Add a bot. Optionally takes a name for the bot, otherwise a random name will be chosen.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if( !TheNavMesh->IsLoaded() )
		Warning( "No navigation mesh loaded! Can't create bot" );

	ConVarRef bot_quota("bot_quota");
	bot_quota.SetValue(bot_quota.GetInt() + 1);

	if (!FStrEq("", args.ArgS())) {
		BotPutInServer(false, args.ArgS());
	}
}

CON_COMMAND_F( bot_kick, "Kick all bots.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	ConVarRef bot_quota("bot_quota");
	bot_quota.SetValue(0);
}

static int g_CurBotNumber = 1;


class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CDMCBot *pPlayer = static_cast<CDMCBot *>( CreateEntityByName( "bot" ) );
		if ( pPlayer )
		{
			pPlayer->SetPlayerName( playername );
		}

		return pPlayer;
	}
};

const char* g_aszBotNames[] =
{
	"Steve",
	"Markus",
	"Nathan",
	"Michael",
	"August",
	"Tony",
	"Henry",
	"Creeper",
	"Enderman",
	"Heavy",
	"Medic",
	"Demoman",
	"Spy",
	"Scout",
	"Sniper",
	"Soldier",
	"Pyro",
	"Engineer",
	"Butter",
	"Gordon",
	"Alex",
	"Alyx",
	"Eli",
	"John",
	"Saxton Hale",
	"Bill",
	"Francis",
	"Louis",
	"Zoey",
	"Coach",
	"Nick",
	"Ellis",
	"Rochelle",
	"Judith",
	"Chell",
	"Jim",
	"Jimmy",
	"Mike",
	"Wallace",
	"Doug",
	"Cave",
	"Helena",
	"Harold",
	"Henry",
	"Ivan",
	"Richard",
	"Isaac",
	"Lauren",
	"Arne",
	"Sam",
	"Samuel",
	"Sheckley",
	"Simmons",
	"Adrian",
	"Barney",
	"Odell",
	"Azian",
	"Wheatley",
	"Greg",
	"Griggs",
	"Grigori",
	"GLaDOS",
	"Miller",
	"Atlas",
	"P-Body",
	"Bob",
	"Halsey",
	"Sarge",
	"Griff",
	"Caboose",
	"Will",
	"William",
	"Church",
	"Leonard",
	"Chris",
	"Christopher",
	"Daniel",
	"Dan",
	"Not Bitl",
	"Booker",
	"Elizabeth",
	"Emmet",
	"Jack",
	"Eleanor",
	"Augustus",
	"Benny",
	"Leonardo",
	"Raphael",
	"Michelangelo",
	"Donatello",
	"Don",
	"Raph",
	"Donny",
	"Donnie",
	"Freddy",
	"Cortana",
	"Pierce",
	"Duke",
	"Tyler",
	"Owen",
	"Rob",
	"Tobor",
	"Jackson",
};

const char *g_charBotChat_Text[] =
{
	"I feel tired running around this arena all day!",
	"Anyone up for something to eat after this?",
	"My legs feel like jelly.",
	"Kill all the things!",
	"I am coming for you!",
	"So, how is everyone doing?",
	"FEAR ME!",
	"FEAR MY PRESENCE!",
	"Come on, I wanna win this!",
	"I wish this gun fired faster.",
	"I wish I ran faster.",
	"I wish I was a human.",
	"Don't mess with me.",
	"KILL ALL THE THINGS!",
	"DON'T MESS WITH ME!",
	"RIP AND TEAR!",
	"I regret nothing.",
	"Guns don't kill people, I DO.",
	"Hope no one is using aimbots or something like that.",
	"I will find you!",
	"I WILL FIND YOU!",
	"I AM COMING FOR YOU!",
	"How is everyone's day so far?",
};

//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool  bFrozen, const char *name )
{
	int iNumBotNames = ARRAYSIZE(g_aszBotNames);

	// This trick lets us create a CDMCBot for this client instead of the CHL2MP_Player
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	if (!name)
	{
		for (int i = 0; i < 5; ++i) {
			name = g_aszBotNames[random->RandomInt(0, iNumBotNames - 1)];
			if (!UTIL_PlayerByName(name))
				break;
		}
	}
	edict_t *pEdict = engine->CreateFakeClient(name);
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Msg( "Failed to create Bot (no edict available)\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CDMCBot *pPlayer = ((CDMCBot*)CBaseEntity::Instance( pEdict ));

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	pPlayer->ChangeTeam( TEAM_UNASSIGNED );
	//pPlayer->RemoveAllItems( true );
	
	// don't want bots to collide
	//pPlayer->SetCollisionGroup( COLLISION_GROUP_PLAYER );

	pPlayer->Initialize();

	g_CurBotNumber++;

	return pPlayer;
}

void Bot_Say( CDMCBot *pBot )
{
	int nBotChatText = ARRAYSIZE( g_charBotChat_Text );
	int randomChoiceBotChatText = rand() % nBotChatText;
	const char *pRandomBotSpawn = g_charBotChat_Text[ randomChoiceBotChatText ];
	char bottalkstring[2048];
	Q_snprintf( bottalkstring, sizeof( bottalkstring ), "%s : %s\n", pBot->GetPlayerName(), pRandomBotSpawn );

	UTIL_SayTextAll( bottalkstring, pBot, true );
}

void Bot_MaintainQuota()
{
	int iIdealNumberOfBots = bot_quota.GetInt();

	static float last_reconnect = 0;
	if (bot_reconnect.GetBool())
	{
		if (gpGlobals->curtime > last_reconnect + bot_reconnect.GetFloat() || gpGlobals->curtime < last_reconnect)
		{
			if (bot_quota.GetInt() > 0)
			{
				RandomSeed(gpGlobals->framecount);
				int initial_player = RandomInt(1, gpGlobals->maxClients);
				int player = initial_player;

				while ((++player) % (gpGlobals->maxClients + 1) != initial_player)
				{
					CHL2MP_Player* pPlayer = ToHL2MPPlayer(UTIL_PlayerByIndex(player % (gpGlobals->maxClients + 1)));
					if (!pPlayer)
						continue;

					if (!pPlayer->IsBot())
						continue;

					engine->ServerCommand(UTIL_VarArgs("kick \"%s\"\n", pPlayer->GetPlayerName()));
					break;
				}
			}

			last_reconnect = gpGlobals->curtime;
		}
	}

	int iCurrentHumans = 0;
	int iCurrentBots = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (pPlayer->IsBot())
			iCurrentBots++;
		else
			iCurrentHumans++;
	}

	if (!engine->IsDedicatedServer() && iCurrentHumans == 0)
		return;

	if ( !HL2MPRules() )
		return;

	if (iCurrentHumans == 0)
		iIdealNumberOfBots = 0;

	if (iCurrentHumans + iIdealNumberOfBots >= gpGlobals->maxClients - 1)
		iIdealNumberOfBots = gpGlobals->maxClients - iCurrentHumans - 1;

	if (engine->IsDedicatedServer())
		iIdealNumberOfBots = min(iIdealNumberOfBots, 4);

	if (iCurrentBots == iIdealNumberOfBots)
		return;

	RandomSeed(gpGlobals->framecount);
	while (iCurrentBots < iIdealNumberOfBots)
	{
		BotPutInServer( false );
		iCurrentBots++;
	}

	while (iCurrentBots > iIdealNumberOfBots)
	{
		CHL2MP_Player* pBotToKick = NULL;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );

			if (!pPlayer)
				continue;

			if (pPlayer->IsBot())
			{
				pBotToKick = pPlayer;
				break;
			}
		}

		Assert(pBotToKick);
		if (pBotToKick)
		{
			engine->ServerCommand(UTIL_VarArgs( "kick \"%s\"\n", pBotToKick->GetPlayerName() ));
			iCurrentBots--;
		}
		else
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{	
	Bot_MaintainQuota();

	random->SetSeed(gpGlobals->curtime*1000);
	RandomSeed(gpGlobals->curtime*1000);

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CHL2MP_Player *pPlayer = ToHL2MPPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (!pPlayer->IsBot())
			continue;

		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CDMCBot *pBot = static_cast< CDMCBot* >( pPlayer );
			pBot->BotThink();
		}
	}
}
