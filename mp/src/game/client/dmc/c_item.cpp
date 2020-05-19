#include "cbase.h"
#include "quake3bob.h"
#include "glowstencil/c_glowbase.h"
#include "c_hl2mp_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar glow_outline_color_red;
extern ConVar glow_outline_color_green;
extern ConVar glow_outline_color_blue;
extern ConVar glow_outline_effect_enable;

//-----------------------------------------------------------------------------
// Client class for pickups, determines if an item should rotate in place.
//-----------------------------------------------------------------------------
class C_Item : public C_BaseGlowEntity
{
	DECLARE_CLASS( C_Item, C_BaseGlowEntity );
	DECLARE_CLIENTCLASS();

public:
	C_Item() 
	{
		ClientRotAng = { 0.0, FRand( 0.0, 359.0 ), 0.0 };
		m_bQuake3Bob = true;
	}

	void Spawn() { ClientThink(); }

	void ClientThink();
	void PostDataUpdate( DataUpdateType_t updateType );
	bool ShouldDraw();

    void UpdateGlow();
    virtual void GetGlowEffectColor( float& r, float& g, float& b ) OVERRIDE;  
    virtual bool GlowOccluded() OVERRIDE { return false; };
    virtual bool GlowUnoccluded() OVERRIDE { return true; };

	bool	m_bQuake3Bob;

	Vector	m_vOriginalSpawnOrigin;
	Vector	m_vOriginalSpawnAngles;

private:
	QAngle ClientRotAng; // m_angRotation is stomped sometimes (CItem returning the ent to spawn position?)
};

IMPLEMENT_CLIENTCLASS_DT( C_Item, DT_Item, CItem )
	RecvPropBool( RECVINFO( m_bQuake3Bob ) ),
	RecvPropVector( RECVINFO( m_vOriginalSpawnOrigin ) ),
	RecvPropVector( RECVINFO( m_vOriginalSpawnAngles ) ),
END_RECV_TABLE()

void C_Item::ClientThink()
{
	if ( IsAbsQueriesValid() && m_bQuake3Bob )
	{
		// Rotate
		Quake3Rotate( this, ClientRotAng );
		Quake3Bob( this, m_vOriginalSpawnOrigin );
	}

	UpdateGlow();

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_Item::PostDataUpdate( DataUpdateType_t updateType )
{
	return BaseClass::PostDataUpdate( updateType );
}

bool C_Item::ShouldDraw()
{
	return BaseClass::ShouldDraw();
}

void C_Item::UpdateGlow()
{
    if ( !glow_outline_effect_enable.GetBool() )
    {
        if ( IsClientSideGlowEnabled() )
            SetClientSideGlowEnabled( false );

        return;
    }

	C_HL2MP_Player* pPlayer = C_HL2MP_Player::GetLocalHL2MPPlayer();
    if ( pPlayer && pPlayer->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) < ITEM_GLOW_DIST_SQR )
    {
        if ( !IsClientSideGlowEnabled() )
            SetClientSideGlowEnabled( true );
    }
    else
    {
        SetClientSideGlowEnabled( false );
    }
}

void C_Item::GetGlowEffectColor( float& r, float& g, float& b )
{
	r = glow_outline_color_red.GetFloat();
	g = glow_outline_color_green.GetFloat();
	b = glow_outline_color_blue.GetFloat();
}