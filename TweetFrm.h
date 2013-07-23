//---------------------------------------------------------------------------
#ifndef TweetFrmH
#define TweetFrmH
#define WM_ALPHAWINDOWS (WM_USER + 666)
//---------------------------------------------------------------------------
#include "sBevel.hpp"
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
#include <Vcl.ActnList.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.FileCtrl.hpp>
#include <Vcl.StdCtrls.hpp>
#include "sButton.hpp"
#include "acProgressBar.hpp"
#include "acImage.hpp"
#include "acPNG.hpp"
//---------------------------------------------------------------------------
class TTweetForm : public TForm
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
	TTimer *AnimateTimer;
	TsButton *AvatarStyleSaveButton;
	TsButton *AvatarStyleDefaultButton;
	TsSkinManager *sSkinManager;
	TsBevel *Bevel;
	TsPageControl *sPageControl;
	TsTabSheet *AvatarsTabSheet;
	TsSkinProvider *sSkinProvider;
	TsGroupBox *AvatarsSizeGroupBox;
	TsGroupBox *AvatarsUpdateGroupBox;
	TsLabelFX *LastAvatarsUpdateLabel;
	TsLabel *LastAvatarsUpdateInfoLabel;
	TsComboBox *AutoAvatarsUpdateComboBox;
	TsGroupBox *AvatarsStyleGroupBox;
	TsLabel *AvatarsStyleLabel;
	TsLabelFX *UsedAvatarsStyleLabel;
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
	TsLabel *HighlightMsgModeLabel;
	TIdHTTP *IdHTTP;
	TMemo *FileMemo;
	TTimer *AUIdHTTPTimer;
	TTimer *AvatarsIdHTTPTimer;
	TIdSSLIOHandlerSocketOpenSSL *IdSSLIOHandlerSocketOpenSSL;
	TIdThreadComponent *GetAvatarsThread;
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
	void __fastcall AnimateTimerTimer(TObject *Sender);
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
	void __fastcall AUIdHTTPWork(TObject *ASender, TWorkMode AWorkMode, __int64 AWorkCount);
	void __fastcall AUIdHTTPWorkBegin(TObject *ASender, TWorkMode AWorkMode, __int64 AWorkCountMax);
	void __fastcall AUIdHTTPWorkEnd(TObject *ASender, TWorkMode AWorkMode);
	void __fastcall AUIdHTTPTimerTimer(TObject *Sender);
	void __fastcall IdHTTPWorkBegin(TObject *ASender, TWorkMode AWorkMode, __int64 AWorkCountMax);
	void __fastcall IdHTTPWork(TObject *ASender, TWorkMode AWorkMode, __int64 AWorkCount);
	void __fastcall IdHTTPWorkEnd(TObject *ASender, TWorkMode AWorkMode);
	void __fastcall AvatarsIdHTTPTimerTimer(TObject *Sender);
	void __fastcall GetAvatarsThreadRun(TIdThreadComponent *Sender);
	void __fastcall ColorHighlightMsgEditChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
	bool IdHTTPManualDisconnected;
	bool AUIdHTTPManualDisconnected;
	__fastcall TTweetForm(TComponent* Owner);
	void __fastcall WMTransparency(TMessage &Message);
	bool __fastcall IdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL);
	bool __fastcall AUIdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL);
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ALPHAWINDOWS,TMessage,WMTransparency);
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TTweetForm *TweetForm;
//---------------------------------------------------------------------------
#endif