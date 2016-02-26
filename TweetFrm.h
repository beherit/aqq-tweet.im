//---------------------------------------------------------------------------
// Copyright (C) 2013-2016 Krzysztof Grochocki
//
// This file is part of tweet.IM
//
// tweet.IM is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// tweet.IM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifndef TweetFrmH
#define TweetFrmH
#define WM_ALPHAWINDOWS (WM_USER + 666)
//---------------------------------------------------------------------------
#include "acProgressBar.hpp"
#include "sBevel.hpp"
#include "sButton.hpp"
#include "sCheckBox.hpp"
#include "sColorSelect.hpp"
#include "sComboBox.hpp"
#include "sEdit.hpp"
#include "sGroupBox.hpp"
#include "sLabel.hpp"
#include "sListView.hpp"
#include "sMemo.hpp"
#include "sPageControl.hpp"
#include "sSkinManager.hpp"
#include "sSkinProvider.hpp"
#include "sSpeedButton.hpp"
#include "sSpinEdit.hpp"
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdHTTP.hpp>
#include <IdIOHandler.hpp>
#include <IdIOHandlerSocket.hpp>
#include <IdIOHandlerStack.hpp>
#include <IdSSL.hpp>
#include <IdSSLOpenSSL.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdThreadComponent.hpp>
#include <System.Actions.hpp>
#include <System.Classes.hpp>
#include <System.Win.TaskbarCore.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.FileCtrl.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Taskbar.hpp>
//---------------------------------------------------------------------------
class TSettingsForm : public TForm
{
__published:	// IDE-managed Components
	TsButton *SaveButton;
	TActionList *ActionList;
	TAction *aSaveSettings;
	TAction *aLoadSettings;
	TAction *aExit;
	TsButton *CancelButton;
	TsButton *OKButton;
	TAction *aSettingsChanged;
	TsButton *ManualAvatarsUpdateButton;
	TFileListBox *FileListBox;
	TsProgressBar *ProgressBar;
	TIdThreadComponent *ManualAvatarsUpdateThread;
	TIdThreadComponent *AutoAvatarsUpdateThread;
	TIdHTTP *AUIdHTTP;
	TsButton *AvatarStyleSaveButton;
	TsButton *AvatarStyleDefaultButton;
	TsSkinManager *sSkinManager;
	TsBevel *Bevel;
	TsPageControl *sPageControl;
	TsTabSheet *AvatarsTabSheet;
	TsSkinProvider *sSkinProvider;
	TsGroupBox *AvatarsSizeGroupBox;
	TsGroupBox *AvatarsUpdateGroupBox;
	TsLabel *LastAvatarsUpdateInfoLabel;
	TsComboBox *AutoAvatarsUpdateComboBox;
	TsGroupBox *AvatarsStyleGroupBox;
	TsLabel *AvatarsStyleLabel;
	TsLabel *EditAvatarsStyleLabel;
	TsMemo *AvatarsStyleMemo;
	TsLabel *ProgressLabel;
	TsLabel *PixelInfoLabel;
	TsLabel *AvatarInfoLabel;
	TsLabel *XSizeLabel;
	TsEdit *AvatarHeightEdit;
	TsSpinEdit *AvatarWidthCSpinEdit;
	TAction *aForceDisconnect;
	TsTabSheet *HighlightMsgTabSheet;
	TsCheckBox *HighlightMsgCheckBox;
	TsListView *HighlightMsgListView;
	TsSpeedButton *EraseHighlightMsgSpeedButton;
	TsSpeedButton *AddHighlightMsgsSpeedButton;
	TsSpeedButton *RemoveHighlightMsgSpeedButton;
	TsColorSelect *HighlightMsgColorSelect;
	TsEdit *ColorHighlightMsgEdit;
	TsEdit *ItemHighlightMsgEdit;
	TsComboBox *HighlightMsgModeComboBox;
	TIdHTTP *IdHTTP;
	TIdSSLIOHandlerSocketOpenSSL *IdSSLIOHandlerSocketOpenSSL;
	TIdThreadComponent *GetAvatarsThread;
	TTaskbar *Taskbar;
	TsLabel *AvatarsUpdateLabel;
	TsLabel *HighlightMsgModeLabel;
	TsLabel *LastAvatarsUpdateLabel;
	TsLabel *UsedAvatarsStyleLabel;
	void __fastcall SaveButtonClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall aLoadSettingsExecute(TObject *Sender);
	void __fastcall aSaveSettingsExecute(TObject *Sender);
	void __fastcall aExitExecute(TObject *Sender);
	void __fastcall OKButtonClick(TObject *Sender);
	void __fastcall aSettingsChangedExecute(TObject *Sender);
	void __fastcall AvatarWidthCSpinEditChange(TObject *Sender);
	void __fastcall ManualAvatarsUpdateButtonClick(TObject *Sender);
	void __fastcall ManualAvatarsUpdateThreadRun(TIdThreadComponent *Sender);
	void __fastcall AutoAvatarsUpdateThreadRun(TIdThreadComponent *Sender);
	void __fastcall EditAvatarsStyleLabelClick(TObject *Sender);
	void __fastcall AvatarStyleDefaultButtonClick(TObject *Sender);
	void __fastcall AvatarsStyleMemoChange(TObject *Sender);
	void __fastcall AvatarStyleSaveButtonClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall aForceDisconnectExecute(TObject *Sender);
	void __fastcall HighlightMsgColorSelectChange(TObject *Sender);
	void __fastcall EraseHighlightMsgSpeedButtonClick(TObject *Sender);
	void __fastcall AddHighlightMsgsSpeedButtonClick(TObject *Sender);
	void __fastcall HighlightMsgListViewSelectItem(TObject *Sender, TListItem *Item, bool Selected);
	void __fastcall RemoveHighlightMsgSpeedButtonClick(TObject *Sender);
	void __fastcall HighlightMsgListViewEdited(TObject *Sender, TListItem *Item, UnicodeString &S);
	void __fastcall HighlightMsgCheckBoxClick(TObject *Sender);
	void __fastcall HighlightMsgListViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall HighlightMsgModeComboBoxChange(TObject *Sender);
	void __fastcall GetAvatarsThreadRun(TIdThreadComponent *Sender);
	void __fastcall ColorHighlightMsgEditChange(TObject *Sender);
	void __fastcall sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning);
private:	// User declarations
public:		// User declarations
	__fastcall TSettingsForm(TComponent* Owner);
	void __fastcall WMTransparency(TMessage &Message);
	bool __fastcall IdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL);
	bool __fastcall AUIdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL);
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ALPHAWINDOWS,TMessage,WMTransparency);
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
#endif
