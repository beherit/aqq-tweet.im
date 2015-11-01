//---------------------------------------------------------------------------
// Copyright (C) 2013-2015 Krzysztof Grochocki
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
#include <vcl.h>
#include <LangAPI.hpp>
#pragma hdrstop
#include "TweetFrm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "acProgressBar"
#pragma link "sBevel"
#pragma link "sButton"
#pragma link "sCheckBox"
#pragma link "sColorSelect"
#pragma link "sComboBox"
#pragma link "sEdit"
#pragma link "sGroupBox"
#pragma link "sLabel"
#pragma link "sListView"
#pragma link "sMemo"
#pragma link "sPageControl"
#pragma link "sSkinManager"
#pragma link "sSkinProvider"
#pragma link "sSpeedButton"
#pragma link "sSpinEdit"
#pragma resource "*.dfm"
TSettingsForm *SettingsForm;
//---------------------------------------------------------------------------
__declspec(dllimport)UnicodeString GetPluginUserDir();
__declspec(dllimport)UnicodeString GetPluginUserDirW();
__declspec(dllimport)UnicodeString GetThemeSkinDir();
__declspec(dllimport)UnicodeString GetAvatarsDir();
__declspec(dllimport)UnicodeString GetAvatarStyle();
__declspec(dllimport)void SetAvatarStyle(UnicodeString Style);
__declspec(dllimport)int GetAvatarType();
__declspec(dllimport)void SetAvatarType(int Type);
__declspec(dllimport)TColor GetWarningColor();
__declspec(dllimport)bool ChkSkinEnabled();
__declspec(dllimport)bool ChkThemeAnimateWindows();
__declspec(dllimport)bool ChkThemeGlowing();
__declspec(dllimport)int GetHUE();
__declspec(dllimport)int GetSaturation();
__declspec(dllimport)int GetBrightness();
__declspec(dllimport)UnicodeString MD5File(UnicodeString FileName);
__declspec(dllimport)bool ChkAvatarsListItem();
__declspec(dllimport)UnicodeString GetAvatarsListItem();
__declspec(dllimport)void LoadSettings();
__declspec(dllimport)UnicodeString EncodeBase64(UnicodeString Str);
//---------------------------------------------------------------------------
bool ForceDisconnect = false;
//---------------------------------------------------------------------------
__fastcall TSettingsForm::TSettingsForm(TComponent* Owner)
		: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::WMTransparency(TMessage &Message)
{
	Application->ProcessMessages();
	if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

//Pobieranie pliku z danego URL
bool __fastcall TSettingsForm::IdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL)
{
	//Ustawianie pozycji pliku na poczatek
	File->Position = 0;
	//Proba pobrania danych
	try
	{
		//Wywolanie polaczenia
		IdHTTP->ConnectTimeout = 10000;
		IdHTTP->Get(URL, File);
	}
	//Blad
	catch(const Exception& e)
	{
		//Hack na wywalanie sie IdHTTP
		if(e.Message=="Connection Closed Gracefully.")
		{
			//Hack
			IdHTTP->CheckForGracefulDisconnect(false);
			//Rozlaczenie polaczenia
			IdHTTP->Disconnect();
			//Ustawianie pozycji pliku na poczatek
			File->Position = 0;
			return false;
		}
		//Inne bledy
		else
		{
			//Rozlaczenie polaczenia
			IdHTTP->Disconnect();
			//Ustawianie pozycji pliku na poczatek
			File->Position = 0;
			return false;
		}
	}
	//Ustawianie pozycji pliku na poczatek
	File->Position = 0;
	//Pobranie kodu odpowiedzi
	int Response = IdHTTP->ResponseCode;
	//Wszystko ok
	if(Response==200) return true;
	//Inne bledy
	else return false;
}
//---------------------------------------------------------------------------

//Pobieranie pliku z danego URL (dla auto/manual update awatarow)
bool __fastcall TSettingsForm::AUIdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL)
{
	//Ustawianie pozycji pliku na poczatek
	File->Position = 0;
	//Proba pobrania danych
	try
	{
		//Wywolanie polaczenia
		AUIdHTTP->ConnectTimeout = 10000;
		AUIdHTTP->Get(URL, File);
	}
	//Blad
	catch(const Exception& e)
	{
		//Hack na wywalanie sie IdHTTP
		if(e.Message=="Connection Closed Gracefully.")
		{
			//Hack
			AUIdHTTP->CheckForGracefulDisconnect(false);
			//Rozlaczenie polaczenia
			AUIdHTTP->Disconnect();
			//Ustawianie pozycji pliku na poczatek
			File->Position = 0;
			return false;
		}
		//Inne bledy
		else
		{
			//Rozlaczenie polaczenia
			AUIdHTTP->Disconnect();
			//Ustawianie pozycji pliku na poczatek
			File->Position = 0;
			return false;
		}
	}
	//Ustawianie pozycji pliku na poczatek
	File->Position = 0;
	//Pobranie kodu odpowiedzi
	int Response = AUIdHTTP->ResponseCode;
	//Wszystko ok
	if(Response==200) return true;
	//Inne bledy
	else return false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormCreate(TObject *Sender)
{
	//Lokalizowanie formy
	LangForm(this);
	//Poprawka pozycji komponentow
	UsedAvatarsStyleLabel->Left = AvatarsStyleLabel->Left + Canvas->TextWidth(AvatarsStyleLabel->Caption) + 6;
	EditAvatarsStyleLabel->Left = UsedAvatarsStyleLabel->Left + Canvas->TextWidth(UsedAvatarsStyleLabel->Caption) + 6;
	AutoAvatarsUpdateComboBox->Left = AvatarsUpdateLabel->Left + Canvas->TextWidth(AvatarsUpdateLabel->Caption) + 6;
	LastAvatarsUpdateLabel->Left = LastAvatarsUpdateInfoLabel->Left + Canvas->TextWidth(LastAvatarsUpdateInfoLabel->Caption) + 6;
	//Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
		UnicodeString ThemeSkinDir = GetThemeSkinDir();
		//Plik zaawansowanej stylizacji okien istnieje
		if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
		{
			//Dane pliku zaawansowanej stylizacji okien
			ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
			sSkinManager->SkinDirectory = ThemeSkinDir;
			sSkinManager->SkinName = "Skin.asz";
			//Ustawianie animacji AlphaControls
			if(ChkThemeAnimateWindows()) sSkinManager->AnimEffects->FormShow->Time = 200;
			else sSkinManager->AnimEffects->FormShow->Time = 0;
			sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
			//Zmiana kolorystyki AlphaControls
			sSkinManager->HueOffset = GetHUE();
			sSkinManager->Saturation = GetSaturation();
			sSkinManager->Brightness = GetBrightness();
			//Aktywacja skorkowania AlphaControls
			sSkinManager->Active = true;
		}
		//Brak pliku zaawansowanej stylizacji okien
		else sSkinManager->Active = false;
	}
	//Zaawansowana stylizacja okien wylaczona
	else sSkinManager->Active = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormShow(TObject *Sender)
{
	//Wlaczona zaawansowana stylizacja okien
	if(sSkinManager->Active)
	{
		//Kolor labelow
		UsedAvatarsStyleLabel->Font->Color = GetWarningColor();
		LastAvatarsUpdateLabel->Font->Color = UsedAvatarsStyleLabel->Font->Color;
	}
	//Zaawansowana stylizacja okien wylaczona
	else
	{
		//Kolor labelow
		UsedAvatarsStyleLabel->Font->Color = clGreen;
		LastAvatarsUpdateLabel->Font->Color = clGreen;
	}
	//Odczyt ustawien wtyczki
	aLoadSettings->Execute();
	//Odczyt typu stylu awatarow
	int AvatarType = GetAvatarType();
	if(AvatarType)
	{
		if(AvatarType==1) UsedAvatarsStyleLabel->Caption = GetLangStr("Own");
		else UsedAvatarsStyleLabel->Caption = GetLangStr("Default");
		EditAvatarsStyleLabel->Visible = true;
	}
	else
	{
		UsedAvatarsStyleLabel->Caption = GetLangStr("FromTheme");
		EditAvatarsStyleLabel->Visible = false;
	}
	EditAvatarsStyleLabel->Left = UsedAvatarsStyleLabel->Left + Canvas->TextWidth(UsedAvatarsStyleLabel->Caption) + 6;
	//Wlaczanie przyciskow
	SaveButton->Enabled = false;
	//Ustawienie domyslnie wlaczonej karty ustawien
	sPageControl->ActivePage = AvatarsTabSheet;
	//Ustawienie kontrolek
	AvatarsStyleGroupBox->Height = 42;
	EditAvatarsStyleLabel->Caption = GetLangStr("Edit");
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aLoadSettingsExecute(TObject *Sender)
{
	TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
	//Awatary
	AvatarWidthCSpinEdit->Value = Ini->ReadInteger("Avatars","Size",25);
	UnicodeString tLastUpdate = Ini->ReadString("Avatars","LastUpdate","");
	if(!tLastUpdate.IsEmpty()) LastAvatarsUpdateLabel->Caption = tLastUpdate;
	else LastAvatarsUpdateLabel->Caption = GetLangStr("NoData");
	int tLastUpdateCount = Ini->ReadInteger("Avatars","LastUpdateCount",0);
	if(tLastUpdateCount) LastAvatarsUpdateLabel->Caption = LastAvatarsUpdateLabel->Caption + " (" + IntToStr(tLastUpdateCount) + ")";
	AutoAvatarsUpdateComboBox->ItemIndex = Ini->ReadInteger("Avatars","UpdateMode",0);
	//Wyroznianie
	HighlightMsgCheckBox->Checked = Ini->ReadBool("HighlightMsg","Enabled",false);
	HighlightMsgModeComboBox->ItemIndex = Ini->ReadInteger("HighlightMsg","Mode",0);
	HighlightMsgListView->Clear();
	TStringList *HighlightItems = new TStringList;
	Ini->ReadSection("HighlightItems",HighlightItems);
	if(HighlightItems->Count)
	{
		for(int Count=0;Count<HighlightItems->Count;Count++)
		{
			UnicodeString ItemName = HighlightItems->Strings[Count];
			UnicodeString HighlightItem = Ini->ReadString("HighlightItems",ItemName,"");
			if(!HighlightItem.IsEmpty())
			{
				UnicodeString Item = HighlightItem;
				Item = Item.Delete(Item.Pos(";"),Item.Length());
				UnicodeString Color = HighlightItem;
				Color = Color.Delete(1,Color.Pos(";"));
				HighlightMsgListView->Items->Add();
				HighlightMsgListView->Items->Item[HighlightMsgListView->Items->Count-1]->Caption = Item;
				HighlightMsgListView->Items->Item[HighlightMsgListView->Items->Count-1]->SubItems->Add(Color);
			}
		}
	}
	delete HighlightItems;
	HighlightMsgListView->Enabled = HighlightMsgCheckBox->Checked;
	EraseHighlightMsgSpeedButton->Enabled = HighlightMsgCheckBox->Checked;
	AddHighlightMsgsSpeedButton->Enabled = HighlightMsgCheckBox->Checked;
	if(HighlightMsgCheckBox->Checked==false) RemoveHighlightMsgSpeedButton->Enabled = false;
	HighlightMsgColorSelect->Enabled = HighlightMsgCheckBox->Checked;
	ColorHighlightMsgEdit->Enabled = HighlightMsgCheckBox->Checked;
	ItemHighlightMsgEdit->Enabled = HighlightMsgCheckBox->Checked;
	HighlightMsgModeComboBox->Enabled = HighlightMsgCheckBox->Checked;
	if(HighlightMsgModeComboBox->ItemIndex==2) HighlightMsgModeInfoLabel->Visible = true;
	else HighlightMsgModeInfoLabel->Visible = false;
	delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aSaveSettingsExecute(TObject *Sender)
{
	TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
	//Awatary
	Ini->WriteInteger("Avatars","Size",AvatarWidthCSpinEdit->Value);
	Ini->WriteString("Avatars","UpdateMode",AutoAvatarsUpdateComboBox->ItemIndex);
	//Wyroznianie
	Ini->WriteBool("HighlightMsg","Enabled",HighlightMsgCheckBox->Checked);
	Ini->WriteInteger("HighlightMsg","Mode",HighlightMsgModeComboBox->ItemIndex);
	Ini->EraseSection("HighlightItems");
	if(HighlightMsgListView->Items!=0)
	{
		for(int Count=0;Count<HighlightMsgListView->Items->Count;Count++)
			Ini->WriteString("HighlightItems",("Item"+IntToStr(Count+1)),HighlightMsgListView->Items->Item[Count]->Caption + ";" + HighlightMsgListView->Items->Item[Count]->SubItems->Strings[0]);
	}
	delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::aSettingsChangedExecute(TObject *Sender)
{
	//Aktywowanie przycisku zapisywania ustawien
	SaveButton->Enabled = true;
}
//---------------------------------------------------------------------------

//Wymuszanie rozlaczenia sie protokolow HTTP
void __fastcall TSettingsForm::aForceDisconnectExecute(TObject *Sender)
{
	//Wymuszenie zakonczenie petli watkow
	ForceDisconnect = true;
	//Rozlaczenie IdHTTP
	IdHTTP->Disconnect();
	AUIdHTTP->Disconnect();
}
//---------------------------------------------------------------------------

//Zamkniecie formy ustawien
void __fastcall TSettingsForm::aExitExecute(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::SaveButtonClick(TObject *Sender)
{
	//Wylaczanie przyciskow
	SaveButton->Enabled = false;
	CancelButton->Enabled = false;
	OKButton->Enabled = false;
	//Zapisanie ustawien
	aSaveSettings->Execute();
	//Odczyt ustawien w rdzeniu wtyczki
	LoadSettings();
	//Wlaczanie przyciskow
	CancelButton->Enabled = true;
	OKButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::OKButtonClick(TObject *Sender)
{
	//Wylaczanie przyciskow
	SaveButton->Enabled = false;
	CancelButton->Enabled = false;
	OKButton->Enabled = false;
	//Zapisanie ustawien
	aSaveSettings->Execute();
	//Odczyt ustawien w rdzeniu wtyczki
	LoadSettings();
	//Zamkniecie formy
	Close();
	//Wlaczanie przyciskow
	CancelButton->Enabled = true;
	OKButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AvatarWidthCSpinEditChange(TObject *Sender)
{
	AvatarHeightEdit->Text = AvatarWidthCSpinEdit->Value;
	aSettingsChanged->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AvatarsStyleMemoChange(TObject *Sender)
{
	//Zezwalanie/blokowanie zapisania stylu awatarow
	if(AvatarsStyleMemo->Text.Pos("CC_AVATAR"))
		AvatarStyleSaveButton->Enabled = true;
	else AvatarStyleSaveButton->Enabled = false;
	//Zezwalanie/blokowanie przywracanie domyslnego stylu awatarow
	if(AvatarsStyleMemo->Text != "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
		AvatarStyleDefaultButton->Enabled = true;
	else AvatarStyleDefaultButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AvatarStyleDefaultButtonClick(TObject *Sender)
{
	//Przywracanie domyslnego stylu awatarow
	AvatarsStyleMemo->Text = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AvatarStyleSaveButtonClick(TObject *Sender)
{
	//Zapisanie stylu awatarow do pliku
	TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
	if(AvatarsStyleMemo->Text == "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
	{
		Ini->DeleteKey("Avatars64", "Style");
        SetAvatarType(2);
		UsedAvatarsStyleLabel->Caption = GetLangStr("Default");
	}
	else
	{
		Ini->WriteString("Avatars64", "Style", EncodeBase64(AvatarsStyleMemo->Text));
		SetAvatarType(1);
		UsedAvatarsStyleLabel->Caption = GetLangStr("Own");
	}
	delete Ini;
	//Ustawienie stylu w rdzeniu wtyczki
	SetAvatarStyle(AvatarsStyleMemo->Text);
	//Zamkniecie edycji stylu awatarow
	AvatarsStyleGroupBox->Height = 42;
	EditAvatarsStyleLabel->Caption = GetLangStr("Edit");
	UsedAvatarsStyleLabel->Left = AvatarsStyleLabel->Left + Canvas->TextWidth(AvatarsStyleLabel->Caption) + 6;
	EditAvatarsStyleLabel->Left = UsedAvatarsStyleLabel->Left + Canvas->TextWidth(UsedAvatarsStyleLabel->Caption) + 6;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::EditAvatarsStyleLabelClick(TObject *Sender)
{
	//Pokazywanie edycji stylu awatarow
	if(AvatarsStyleGroupBox->Height==42)
	{
		EditAvatarsStyleLabel->Caption = GetLangStr("CancelEditing");
		AvatarsStyleMemo->Text = GetAvatarStyle();
		AvatarStyleSaveButton->Enabled = false;
		AvatarsStyleGroupBox->Height = 162;
	}
	//Chowanie edycji stylu awatarow
	else
	{
		EditAvatarsStyleLabel->Caption = GetLangStr("Edit");
		AvatarsStyleGroupBox->Height = 42;
	}
	//Zezwalanie/blokowanie przywracanie domyslnego stylu awatarow
	if(AvatarsStyleMemo->Text != "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
		AvatarStyleDefaultButton->Enabled = true;
	else AvatarStyleDefaultButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ManualAvatarsUpdateButtonClick(TObject *Sender)
{
	if(ManualAvatarsUpdateButton->Caption==GetLangStr("CheckForUpdates"))
	{
		//Zmiana caption na buttonie
		ManualAvatarsUpdateButton->Caption = GetLangStr("AbortUpdates");
		//Sprawdzanie czy folder awatar istnieje
		if(!DirectoryExists(GetPluginUserDirW() + "\\tweetIM\\Avatars"))
			CreateDir(GetPluginUserDirW() + "\\tweetIM\\Avatars");
		//Wlaczenie paska postepu
		ProgressBar->Position = 0;
		ProgressBar->Visible = true;
		ProgressLabel->Caption = GetLangStr("RetrievingData");
		ProgressLabel->Visible = true;
		//Wlaczenie paska postepu na taskbarze
		Taskbar->ProgressValue = 0;
		Taskbar->ProgressState = TTaskBarProgressState::Normal;
		//Pobieranie listy plikow
		FileListBox->Directory = "";
		FileListBox->Directory = GetPluginUserDirW() + "\\tweetIM\\Avatars";
		//Ignorowanie plikow *.tmp i plikow ze spacja (np. konflikty stworzone przez Dropbox'a)
		for(int Count=0;Count<FileListBox->Items->Count;Count++)
		{
			if(ExtractFileName(FileListBox->Items->Strings[Count]).Pos(".tmp")>0)
			{
				DeleteFile(FileListBox->Items->Strings[Count]);
				FileListBox->Items->Strings[Count] = "TMP_DELETE";
			}
			else if(ExtractFileName(FileListBox->Items->Strings[Count]).Pos(" ")>0)
			{
				DeleteFile(FileListBox->Items->Strings[Count]);
				FileListBox->Items->Strings[Count] = "TMP_DELETE";
			}
		}
		while(FileListBox->Items->IndexOf("TMP_DELETE")!=-1)
			FileListBox->Items->Delete(FileListBox->Items->IndexOf("TMP_DELETE"));
		//Ustawianie maksymalnego paska postepu
		ProgressBar->Max = FileListBox->Items->Count;
		//Ustawianie maksymalnego paska postepu na taskbarze
		Taskbar->ProgressMaxValue = FileListBox->Items->Count;
		//Wlacznie aktualizacji
		ManualAvatarsUpdateThread->Start();
	}
	else
	{
		//Wylaczanie buttona
		ManualAvatarsUpdateButton->Enabled = false;
		//Wymuszenie zakonczenie petli
		ForceDisconnect = true;
		//Rozlaczenie IdHTTP
		AUIdHTTP->Disconnect();
	}
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::HighlightMsgCheckBoxClick(TObject *Sender)
{
	aSettingsChanged->Execute();
	HighlightMsgListView->Enabled = HighlightMsgCheckBox->Checked;
	EraseHighlightMsgSpeedButton->Enabled = HighlightMsgCheckBox->Checked;
	AddHighlightMsgsSpeedButton->Enabled = HighlightMsgCheckBox->Checked;
	if(HighlightMsgCheckBox->Checked==false) RemoveHighlightMsgSpeedButton->Enabled = false;
	HighlightMsgColorSelect->Enabled = HighlightMsgCheckBox->Checked;
	ColorHighlightMsgEdit->Enabled = HighlightMsgCheckBox->Checked;
	ItemHighlightMsgEdit->Enabled = HighlightMsgCheckBox->Checked;
	HighlightMsgModeComboBox->Enabled = HighlightMsgCheckBox->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::HighlightMsgListViewEdited(TObject *Sender, TListItem *Item,
			UnicodeString &S)
{
	aSettingsChanged->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::HighlightMsgListViewKeyDown(TObject *Sender, WORD &Key,
			TShiftState Shift)
{
	//Wcisniecie przycisku Delete
	if(Key==46)
	{
		//Usuwanie elementu
		if(HighlightMsgListView->ItemIndex!=-1)
		{
			HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Delete();
			SaveButton->Enabled = true;
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::HighlightMsgListViewSelectItem(TObject *Sender, TListItem *Item,
			bool Selected)
{
	//Wczytyawanie elementow szybkiej edycji
	if(HighlightMsgListView->ItemIndex!=-1)
	{
		ItemHighlightMsgEdit->Text = HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Caption;
		ColorHighlightMsgEdit->Text = HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->SubItems->Strings[0];
		RemoveHighlightMsgSpeedButton->Enabled = true;
	}
	else RemoveHighlightMsgSpeedButton->Enabled = false;
}
//---------------------------------------------------------------------------

//Przywrocenie domylsnych zawartosci kontrolek
void __fastcall TSettingsForm::EraseHighlightMsgSpeedButtonClick(TObject *Sender)
{
	ItemHighlightMsgEdit->Text = "";
	ColorHighlightMsgEdit->Text = "#FF0000";
	HighlightMsgListView->ItemIndex = -1;
}
//---------------------------------------------------------------------------

//Pobieranie koloru w HEX z komponentu
void __fastcall TSettingsForm::HighlightMsgColorSelectChange(TObject *Sender)
{
	TColor Color = HighlightMsgColorSelect->ColorValue;
	int R,G,B;
	R = GetRValue(Color);
	G = GetGValue(Color);
	B = GetBValue(Color);
	ColorHighlightMsgEdit->Text = "#" + IntToHex(R,2) + IntToHex(G,2) + IntToHex(B,2);
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AddHighlightMsgsSpeedButtonClick(TObject *Sender)
{
	//Edycja juz dodanych elementow
	if(HighlightMsgListView->ItemIndex!=-1)
	{
		//Sprawdzanie czy element jest juz dodany
		bool NewItem = true;
		for(int Count=0;Count<HighlightMsgListView->Items->Count;Count++)
		{
			if(HighlightMsgListView->Items->Item[Count]->Caption.LowerCase()==ItemHighlightMsgEdit->Text.LowerCase())
			{
				if(HighlightMsgListView->Items->Item[Count]->SubItems->Strings[0].LowerCase()==ColorHighlightMsgEdit->Text.LowerCase())
					NewItem = false;
				Count = HighlightMsgListView->Items->Count;
			}
		}
		//Element ulegl zmianie
		if(NewItem)
		{
			HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Caption = ItemHighlightMsgEdit->Text;
			HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->SubItems->Strings[0] = ColorHighlightMsgEdit->Text;
			SaveButton->Enabled = true;
		}
	}
	//Dowananie nowych elementow
	else
	{
		if((!ItemHighlightMsgEdit->Text.IsEmpty())&&(!ColorHighlightMsgEdit->Text.IsEmpty()))
		{
			ItemHighlightMsgEdit->Text = StringReplace(ItemHighlightMsgEdit->Text, ";", "", TReplaceFlags() << rfReplaceAll);
			ColorHighlightMsgEdit->Text = StringReplace(ColorHighlightMsgEdit->Text, ";", "", TReplaceFlags() << rfReplaceAll);
			//Sprawdzanie czy element jest juz dodany
			bool NewItem = true;
			for(int Count=0;Count<HighlightMsgListView->Items->Count;Count++)
			{
				if(HighlightMsgListView->Items->Item[Count]->Caption.LowerCase()==ItemHighlightMsgEdit->Text.LowerCase())
				{
					NewItem = false;
					Count = HighlightMsgListView->Items->Count;
				}
			}
			//Element nie jest jeszcze dodany
			if(NewItem)
			{
				HighlightMsgListView->Items->Add();
				HighlightMsgListView->Items->Item[HighlightMsgListView->Items->Count-1]->Caption = ItemHighlightMsgEdit->Text;
				HighlightMsgListView->Items->Item[HighlightMsgListView->Items->Count-1]->SubItems->Add(ColorHighlightMsgEdit->Text);
				SaveButton->Enabled = true;
				ItemHighlightMsgEdit->Text = "";
			}
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::RemoveHighlightMsgSpeedButtonClick(TObject *Sender)
{
	//Usuwanie elementu
	if(HighlightMsgListView->ItemIndex!=-1)
	{
		HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Delete();
		SaveButton->Enabled = true;
	}
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::HighlightMsgModeComboBoxChange(TObject *Sender)
{
	aSettingsChanged->Execute();
	if(HighlightMsgModeComboBox->ItemIndex==2) HighlightMsgModeInfoLabel->Visible = true;
	else HighlightMsgModeInfoLabel->Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ManualAvatarsUpdateThreadRun(TIdThreadComponent *Sender)
{
	//Petla aktualizacji
	int NewAvatars=0;
	for(int Count=0;Count<FileListBox->Items->Count;Count++)
	{
		//Ustawianie paska postepu
		FileListBox->ItemIndex = Count;
		ProgressLabel->Caption = "(" + IntToStr(Count+1) + "/" + IntToStr(FileListBox->Items->Count) + ") " + ExtractFileName(FileListBox->FileName) + "...";
		//Tworzenie nowego pliku w pamieci
		TMemoryStream* MemFile = new TMemoryStream;
		MemFile->Position = 0;
		//Ustalanie adresu pobierania awataru
		UnicodeString UpdateURL;
		UpdateURL = "https://beherit.pl/tweetIM/?user=" + ExtractFileName(FileListBox->FileName);
		//Pobieranie awatara
		if(AUIdHTTPGetFileToMem(MemFile,UpdateURL))
		{
			MemFile->Position = 0;
			if(MemFile->Size!=0)
			{
				MemFile->SaveToFile(FileListBox->FileName + ".tmp");
				delete MemFile;
				if(MD5File(FileListBox->FileName + ".tmp")!=MD5File(FileListBox->FileName))
				{
					DeleteFile(FileListBox->FileName);
					MoveFile((FileListBox->FileName + ".tmp").w_str(),FileListBox->FileName.w_str());
					NewAvatars++;
				}
				else DeleteFile(FileListBox->FileName + ".tmp");
			}
			else delete MemFile;
		}
		else delete MemFile;
		//Kolejny plik
		ProgressBar->Position++;
		Taskbar->ProgressValue++;
		//Wymuszenie wylaczenia
		if(ForceDisconnect) Count = FileListBox->Items->Count;
	}
	//Ustawianie daty ostatniej aktualizacji
	TDateTime pLogTime = TDateTime::CurrentDateTime();
	UnicodeString pLogTimeStr = pLogTime.FormatString("yyyy-mm-dd hh:nn:ss");
	if(NewAvatars) LastAvatarsUpdateLabel->Caption = pLogTimeStr + " (" + IntToStr(NewAvatars) + ")";
	else LastAvatarsUpdateLabel->Caption = pLogTimeStr;
	TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
	Ini->WriteString("Avatars", "LastUpdate", pLogTimeStr);
	Ini->WriteInteger("Avatars", "LastUpdateCount", NewAvatars);
	delete Ini;
	//Wylaczenie paska postepu
	ProgressBar->Visible = false;
	ProgressLabel->Visible = false;
	//Wylaczenie paska postepuna taskbarze
	Taskbar->ProgressState = TTaskBarProgressState::None;
	//Default caption
	ManualAvatarsUpdateButton->Caption = GetLangStr("CheckForUpdates");
	if(ForceDisconnect)
	{
		ManualAvatarsUpdateButton->Enabled = true;
		ForceDisconnect = false;
	}
	//Wylaczenie watku
	ManualAvatarsUpdateThread->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::AutoAvatarsUpdateThreadRun(TIdThreadComponent *Sender)
{
	//Petla aktualizacji
	int NewAvatars=0;
	for(int Count=0;Count<FileListBox->Items->Count;Count++)
	{
		//Ustawianie paska postepu
		FileListBox->ItemIndex = Count;
		ProgressLabel->Caption = "(" + IntToStr(Count+1) + "/" + IntToStr(FileListBox->Items->Count) + ") " + ExtractFileName(FileListBox->FileName) + "...";
		//Tworzenie nowego pliku w pamieci
		TMemoryStream* MemFile = new TMemoryStream;
		MemFile->Position = 0;
		//Ustalanie adresu pobierania awataru
		UnicodeString UpdateURL;
		UpdateURL = "https://beherit.pl/tweetIM/?user=" + ExtractFileName(FileListBox->FileName);
		//Pobieranie awatara
		if(AUIdHTTPGetFileToMem(MemFile,UpdateURL))
		{
			MemFile->Position = 0;
			if(MemFile->Size!=0)
			{
				MemFile->SaveToFile(FileListBox->FileName + ".tmp");
				delete MemFile;
				if(MD5File(FileListBox->FileName + ".tmp")!=MD5File(FileListBox->FileName))
				{
					DeleteFile(FileListBox->FileName);
					MoveFile((FileListBox->FileName + ".tmp").w_str(),FileListBox->FileName.w_str());
					NewAvatars++;
				}
				else DeleteFile(FileListBox->FileName + ".tmp");
			}
			else delete MemFile;
		}
		else delete MemFile;
		//Kolejny plik
		ProgressBar->Position++;
		Taskbar->ProgressValue++;
		//Wymuszenie wylaczenia
		if(ForceDisconnect) Count = FileListBox->Items->Count;
	}
	//Ustawianie daty ostatniej aktualizacji
	TDateTime pLogTime = TDateTime::CurrentDateTime();
	UnicodeString pLogTimeStr = pLogTime.FormatString("yyyy-mm-dd hh:nn:ss");
	if(NewAvatars) LastAvatarsUpdateLabel->Caption = pLogTimeStr + " (" + IntToStr(NewAvatars) + ")";
	else LastAvatarsUpdateLabel->Caption = pLogTimeStr;
	TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
	Ini->WriteString("Avatars", "LastUpdate", pLogTimeStr);
	Ini->WriteInteger("Avatars", "LastUpdateCount", NewAvatars);
	delete Ini;
	//Wylaczenie paska postepu
	ProgressBar->Visible = false;
	ProgressLabel->Visible = false;
	//Wylaczenie paska postepuna taskbarze
	Taskbar->ProgressState = TTaskBarProgressState::None;
	//Default caption
	ManualAvatarsUpdateButton->Caption = GetLangStr("CheckForUpdates");
	if(ForceDisconnect)
	{
		ManualAvatarsUpdateButton->Enabled = true;
		ForceDisconnect = false;
	}
	//Wylaczenie watku
	AutoAvatarsUpdateThread->Stop();
}
//---------------------------------------------------------------------------*/

void __fastcall TSettingsForm::GetAvatarsThreadRun(TIdThreadComponent *Sender)
{
	//Pobranie itemu z listy awatarow do pobrania
	UnicodeString Data = GetAvatarsListItem();
	//Jest jakis awatar do pobrania
	if(!Data.IsEmpty())
	{
		//Parsowanie danycg
		UnicodeString Nick = Data;
		Nick = Nick.Delete(Nick.Pos(";"),Nick.Length());
		UnicodeString URL = Data;
		URL = URL.Delete(1,URL.Pos(";"));
		//Tworzenie nowego pliku w pamieci
		TMemoryStream* MemFile = new TMemoryStream;
		MemFile->Position = 0;
		//Pobieranie awatara
		if(IdHTTPGetFileToMem(MemFile,URL))
		{
			MemFile->Position = 0;
			if(MemFile->Size!=0)
			{
				MemFile->SaveToFile(GetAvatarsDir() + "\\\\" + Nick);
				delete MemFile;
			}
			else delete MemFile;
		}
		else delete MemFile;
	}
	//Zatrzymanie watku
	if(!ChkAvatarsListItem()) GetAvatarsThread->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ColorHighlightMsgEditChange(TObject *Sender)
{
	if((ColorHighlightMsgEdit->Text.Pos("#")==1)&&(ColorHighlightMsgEdit->Text.Length()==7))
	{
		//Parsowanie koloru
		UnicodeString Color = ColorHighlightMsgEdit->Text;
		Color.Delete(1,1);
		UnicodeString Red = Color;
		Red.Delete(3,Red.Length());
		UnicodeString Green = Color;
		Green.Delete(1,2);
		Green.Delete(3,Green.Length());
		UnicodeString Blue = Color;
		Blue.Delete(1,4);
		//Konwersja HEX na RGB
		HighlightMsgColorSelect->ColorValue = (TColor)RGB(HexToInt(Red),HexToInt(Green),HexToInt(Blue));
	}
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning)
{
	AllowSkinning = false;
}
//---------------------------------------------------------------------------

