//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "basegrenade_shared.h"
#include "weapon_hl2mpbase_machinegun.h"

#ifdef CLIENT_DLL
	#include "c_hl2mp_player.h"
#else
	#include "hl2mp_player.h"
	#include "grenade_frag.h"
#endif

#ifdef CLIENT_DLL
#define CWeaponGrenadeLauncher C_WeaponGrenadeLauncher
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_DAMAGE 100.0f
#define GRENADE_RADIUS 250.0f
#define GRENADE_TIMER	2.5f //Seconds

class CWeaponGrenadeLauncher : public CHL2MPMachineGun
{
public:
	DECLARE_CLASS( CWeaponGrenadeLauncher, CHL2MPMachineGun );

	CWeaponGrenadeLauncher();

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	
	void	Precache( void );
	void	AddViewKick( void );
	void	PrimaryAttack( void );

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	
private:
	CWeaponGrenadeLauncher( const CWeaponGrenadeLauncher & );
};

IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeLauncher, DT_WeaponGrenadeLauncher )

BEGIN_NETWORK_TABLE( CWeaponGrenadeLauncher, DT_WeaponGrenadeLauncher )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponGrenadeLauncher )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( weapon_grenadelauncher, CWeaponGrenadeLauncher );
PRECACHE_WEAPON_REGISTER( weapon_grenadelauncher );

acttable_t	CWeaponGrenadeLauncher::m_acttable[] = 
{
	{ ACT_MP_STAND_IDLE,				ACT_HL2MP_IDLE_SMG1,					false },
	{ ACT_MP_CROUCH_IDLE,				ACT_HL2MP_IDLE_CROUCH_SMG1,				false },

	{ ACT_MP_RUN,						ACT_HL2MP_RUN_SMG1,						false },
	{ ACT_MP_CROUCHWALK,				ACT_HL2MP_WALK_CROUCH_SMG1,				false },

	{ ACT_MP_ATTACK_STAND_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },
	{ ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,	false },

	{ ACT_MP_RELOAD_STAND,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },
	{ ACT_MP_RELOAD_CROUCH,				ACT_HL2MP_GESTURE_RELOAD_SMG1,			false },

	{ ACT_MP_JUMP,						ACT_HL2MP_JUMP_SMG1,					false },
};

IMPLEMENT_ACTTABLE( CWeaponGrenadeLauncher );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponGrenadeLauncher::CWeaponGrenadeLauncher()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenadeLauncher::Precache( void )
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_frag" );
#endif

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenadeLauncher::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGrenadeLauncher::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CHL2MP_Player *pPlayer = ToHL2MPPlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	//Must have ammo
	if ( ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 ) )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if( m_bInReload )
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound( SINGLE );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;

	// Don't autoaim on grenade tosses
	AngleVectors( pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow );
	VectorScale( vecThrow, 1000.0f, vecThrow );
	
#ifndef CLIENT_DLL
	//Create the grenade
	CBaseGrenade *pGrenade = Fraggrenade_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse( 600, random->RandomInt( -1200,1200 ),0 ), pPlayer, GRENADE_TIMER, false );
	if ( pGrenade )
	{
		if ( pPlayer && pPlayer->m_lifeState != LIFE_ALIVE )
		{
			pPlayer->GetVelocity( &vecThrow, NULL );

			IPhysicsObject *pPhysicsObject = pGrenade->VPhysicsGetObject();
			if ( pPhysicsObject )
			{
				pPhysicsObject->SetVelocity( &vecThrow, NULL );
			}
		}
		
		pGrenade->SetDamage( GRENADE_DAMAGE );
		pGrenade->SetDamageRadius( GRENADE_RADIUS );
	}
#endif

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	// player "shoot" animation
	pPlayer->DoAnimationEvent( PLAYERANIMEVENT_ATTACK_PRIMARY );

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
}