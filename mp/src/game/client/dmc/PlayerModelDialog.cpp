//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "PlayerModelDialog.h"
#include <stdio.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "filesystem.h"
#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include "tier1/convar.h"
#include "ienginevgui.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;


// IDK why, idk why, barnnet don't let me alone
// i cannot take anymore please barnnet shu the fuck up
// this piece of shit gave me us so fucking problems to get pluged up
// fuck it - kris
static vgui::DHANDLE<CPlayerModelDialog> g_hPlayerMenu;

CON_COMMAND( OpenPlayerModelSelection, "Opens The PlayerModel Picker" )
{
    if ( !g_hPlayerMenu.Get() )
    {
        vgui::VPANEL parent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
        if ( parent == NULL )
        {
            Assert( 0 );
            return;
        }

        auto* pPanel = new CPlayerModelDialog( parent );

        g_hPlayerMenu.Set( pPanel );
    }

    auto* pPanel = g_hPlayerMenu.Get();

    // Center
    int x, y, w, h;
    vgui::surface()->GetWorkspaceBounds( x, y, w, h );
    
    int mw = pPanel->GetWide();
    int mh = pPanel->GetTall();
    pPanel->SetPos( x + w / 2 - mw / 2, y + h / 2 - mh / 2 );

    pPanel->Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPlayerModelDialog::CPlayerModelDialog( vgui::VPANEL parent ) : BaseClass( NULL, "PlayerModelDialog" )
{
	SetParent( parent );
	SetSize( 348, 460 );
	SetSizeable( false );
	SetDeleteSelfOnClose( true );
	
	m_pModelList = new ComboBox( this, "ModelList", 12, false );
	m_lcurrentModel = new Label( this, "CurrentModelPanel", "" );

	LoadControlSettings("Resource/PlayerModelDialog.res");

	// create KeyValues object to load/save config options
	LoadModelList();
	DialogInit();

	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CPlayerModelDialog::~CPlayerModelDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModelDialog::DialogInit()
{
	static ConVarRef cl_playermodel("cl_playermodel");
	const char* playermodel = cl_playermodel.GetString();
	char szModelName[1024];
	Q_snprintf(szModelName, sizeof(szModelName), "Current: %s\n", playermodel);
	m_lcurrentModel->SetText(szModelName);
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModelDialog::OnClose()
{
	BaseClass::OnClose();
	MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModelDialog::OnOK()
{
	char szModelName[64];
	Q_strncpy(szModelName, GetModelName(), sizeof(szModelName));

	char szModelCommand[1024];

	// create the command to execute
	Q_snprintf(szModelCommand, sizeof(szModelCommand), "cl_playermodel models/player/%s\n", szModelName);

	// exec
	engine->ClientCmd_Unrestricted(szModelCommand);

	OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CPlayerModelDialog::OnCommand(const char *command)
{
	if ( !stricmp( command, "Ok" ) )
	{
		OnOK();
		return;
	}

	BaseClass::OnCommand( command );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModelDialog::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CPlayerModelDialog::LoadModels()
{
	FileFindHandle_t findHandle = NULL;

	const char *pszFilename = g_pFullFileSystem->FindFirst("models/player/*.mdl", &findHandle);
	while (pszFilename)
	{
		m_pModelList->AddItem(pszFilename, new KeyValues("data", "modelname", pszFilename));
		pszFilename = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CPlayerModelDialog::LoadModelList()
{
	// clear the current list (if any)
	m_pModelList->DeleteAllItems();

	// Load the GameDir maps
	LoadModels();

	// set the first item to be selected
	m_pModelList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CPlayerModelDialog::GetModelName()
{
	int count = m_pModelList->GetItemCount();

	// if there is only one entry it's the special "select random map" entry
	if( count <= 1 )
		return NULL;

	const char *modelname = NULL;
	modelname = m_pModelList->GetActiveItemUserData()->GetString("modelname");

	return modelname;
}

//-----------------------------------------------------------------------------
// Purpose: Sets currently selected map in the map combobox
//-----------------------------------------------------------------------------
void CPlayerModelDialog::SetModel(const char *name)
{
	for (int i = 0; i < m_pModelList->GetItemCount(); i++)
	{
		if (!m_pModelList->IsItemIDValid(i))
			continue;

		if (!stricmp(m_pModelList->GetItemUserData(i)->GetString("modelname"), name))
		{
			m_pModelList->ActivateItem(i);
			break;
		}
	}
}