//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2MPCLIENTSCOREBOARDDIALOG_H
#define HL2MPCLIENTSCOREBOARDDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <clientscoreboarddialog.h>

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CHL2MPClientScoreBoardDialog : public CClientScoreBoardDialog
{
private:
	DECLARE_CLASS_SIMPLE( CHL2MPClientScoreBoardDialog, CClientScoreBoardDialog );

public:
	CHL2MPClientScoreBoardDialog( IViewPort *pViewPort );
	~CHL2MPClientScoreBoardDialog();

	virtual void Reset();
	virtual void Update();

	// vgui overrides for rounded corner background
	virtual void PaintBackground();
	virtual void PaintBorder();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	void UpdateItemVisibiity();
	void InitPlayerList( vgui::SectionedListPanel *pPlayerList, int teamNumber );
	void UpdateTeamInfo();
	void UpdatePlayerList();
	void UpdateSpectatorList();
	bool GetPlayerScoreInfo( int playerIndex, KeyValues *outPlayerInfo );

	bool ShouldShowAsSpectator( int iPlayerIndex );
	void FireGameEvent( IGameEvent *event );

	static bool HL2MPPlayerSortFunc( vgui::SectionedListPanel *list, int itemID1, int itemID2 );

	// rounded corners
	Color m_bgColor;
	Color m_borderColor;

	int m_iStoredScoreboardWidth; // Store the full scoreboard width.

	// player lists
	vgui::SectionedListPanel *m_pPlayerListDM;

	vgui::Label	*m_pPlayerCountLabel_DM;
	vgui::Label *m_pScoreHeader_DM;
	vgui::Label	*m_pDeathsHeader_DM;
	vgui::Label	*m_pPingHeader_DM;
	vgui::Label	*m_pPingLabel_DM;
};

#endif // HL2MPCLIENTSCOREBOARDDIALOG_H
