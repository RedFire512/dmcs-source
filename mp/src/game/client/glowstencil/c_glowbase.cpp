#include "cbase.h"
#include "c_glowbase.h"

C_BaseGlowEntity::C_BaseGlowEntity()
{
    m_pGlowEffect = nullptr;
    m_bClientSideGlowEnabled = false;
}

C_BaseGlowEntity::~C_BaseGlowEntity()
{
    DestroyGlowEffect();
}

void C_BaseGlowEntity::UpdateGlowEffect()
{
    // Destroy the existing effect
    if ( m_pGlowEffect )
    {
        DestroyGlowEffect();
    }

    // Create a new effect
    if ( ShouldCreateGlow() )
    {
        float r, g, b;
        GetGlowEffectColor( r, g, b );

        m_pGlowEffect = new CGlowObject( this, Vector( r, g, b ), 1.0f, GlowOccluded(), GlowUnoccluded() );
    }
}

void C_BaseGlowEntity::DestroyGlowEffect()
{
    if ( m_pGlowEffect )
    {
        delete m_pGlowEffect;
        m_pGlowEffect = nullptr;
    }
}


IMPLEMENT_CLIENTCLASS_DT( C_BaseGlowProp, DT_BaseGlowProp, CBaseGlowProp )
    RecvPropBool( RECVINFO( m_bGlowEnabled ) ),
    RecvPropInt( RECVINFO( m_GlowColor ) ),
    RecvPropInt( RECVINFO( m_fGlowType ) ),
END_RECV_TABLE()

void C_BaseGlowProp::OnPreDataChanged( DataUpdateType_t updateType )
{
    C_BaseAnimating::OnPreDataChanged( updateType );

    m_bOldGlowEnabled = m_bGlowEnabled;
    m_GlowOldColor = m_GlowColor.Get();
    m_fOldGlowType = m_fGlowType;
}

void C_BaseGlowProp::OnDataChanged( DataUpdateType_t updateType )
{
    C_BaseAnimating::OnDataChanged( updateType );


    if ( m_bOldGlowEnabled != m_bGlowEnabled || m_GlowOldColor != m_GlowColor || m_fOldGlowType != m_fGlowType )
    {
        UpdateGlowEffect();
    }
}

bool C_BaseGlowProp::ShouldCreateGlow()
{
    return m_bGlowEnabled || m_bClientSideGlowEnabled;
}

void C_BaseGlowProp::GetGlowEffectColor( float& r, float& g, float& b )
{
    r = m_GlowColor.GetR() / 255.0f;
    g = m_GlowColor.GetG() / 255.0f;
    b = m_GlowColor.GetB() / 255.0f;
}

bool C_BaseGlowProp::GlowOccluded()
{
    return (m_fGlowType & GLOWFLAG_OCCLUDED) != 0;
}

bool C_BaseGlowProp::GlowUnoccluded()
{
    return (m_fGlowType & GLOWFLAG_UNOCCLUDED) != 0;
}
