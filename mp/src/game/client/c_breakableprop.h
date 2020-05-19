//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_BREAKABLEPROP_H
#define C_BREAKABLEPROP_H
#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef DMCS_DLL
#include "glowstencil/c_glowbase.h"

class C_BreakableProp : public C_BaseGlowProp
{
	typedef C_BaseGlowProp BaseClass;
#else
class C_BreakableProp : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
#endif // DMCS_DLL
public:
	DECLARE_CLIENTCLASS();

	C_BreakableProp();
	
	virtual void SetFadeMinMax( float fademin, float fademax );

	// Copy fade from another breakable prop
	void CopyFadeFrom( C_BreakableProp *pSource );
};

#endif // C_BREAKABLEPROP_H
