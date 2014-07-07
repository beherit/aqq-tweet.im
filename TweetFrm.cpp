//---------------------------------------------------------------------------
// Copyright (C) 2013-2014 Krzysztof Grochocki
//
// This file is part of tweet.IM
//
// tweet.IM is free software; you can redistribute it and/or modify
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
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "TweetFrm.h"
#include <inifiles.hpp>
#include <utilcls.h>
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
TTweetForm *TweetForm;
//---------------------------------------------------------------------------
__declspec(dllimport)UnicodeString GetPluginUserDir();
__declspec(dllimport)UnicodeString GetPluginUserDirW();
__declspec(dllimport)UnicodeString GetThemeSkinDir();
__declspec(dllimport)UnicodeString GetAvatarsDir();
__declspec(dllimport)UnicodeString GetAvatarStyle();
__declspec(dllimport)TColor GetWarningColor();
__declspec(dllimport)bool ChkSkinEnabled();
__declspec(dllimport)bool ChkThemeAnimateWindows();
__declspec(dllimport)bool ChkThemeGlowing();
__declspec(dllimport)int GetSaturation();
__declspec(dllimport)int GetHUE();
__declspec(dllimport)UnicodeString MD5File(UnicodeString FileName);
__declspec(dllimport)void SetAvatarStyle(UnicodeString Style);
__declspec(dllimport)bool ChkAvatarsListItem();
__declspec(dllimport)UnicodeString GetAvatarsListItem();
__declspec(dllimport)void LoadSettings();
__declspec(dllimport)UnicodeString EncodeBase64(UnicodeString Str);
//---------------------------------------------------------------------------
bool AnimateMode;
bool ForceDisconnect = false;
//---------------------------------------------------------------------------
__fastcall TTweetForm::TTweetForm(TComponent* Owner)
		: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::WMTransparency(TMessage &Message)
{
  Application->ProcessMessages();
  if(sSkinManager->Active) sSkinProvider->BorderForm->UpdateExBordersPos(true,(int)Message.LParam);
}
//---------------------------------------------------------------------------

//Pobieranie pliku z danego URL
bool __fastcall TTweetForm::IdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL)
{
  //Ustawianie pozycji pliku na poczatek
  File->Position = 0;
  //Proba pobrania danych
  try
  {
	//Wywolanie polaczenia
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
  //Polaczenie bylo zerwane
  if(IdHTTPManualDisconnected)
  {
	IdHTTPManualDisconnected = false;
	return false;
  }
  //Pobranie kodu odpowiedzi
  int Response = IdHTTP->ResponseCode;
  //Wszystko ok
  if(Response==200)
   return true;
  //Inne bledy
  else
   return false;
}
//---------------------------------------------------------------------------

//Pobieranie pliku z danego URL (dla auto/manual update awatarow)
bool __fastcall TTweetForm::AUIdHTTPGetFileToMem(TMemoryStream* File, UnicodeString URL)
{
  //Ustawianie pozycji pliku na poczatek
  File->Position = 0;
  //Proba pobrania danych
  try
  {
	//Wywolanie polaczenia
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
  //Polaczenie bylo zerwane
  if(AUIdHTTPManualDisconnected)
  {
	AUIdHTTPManualDisconnected = false;
	return false;
  }
  //Pobranie kodu odpowiedzi
  int Response = AUIdHTTP->ResponseCode;
  //Wszystko ok
  if(Response==200)
   return true;
  //Inne bledy
  else
   return false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::FormCreate(TObject *Sender)
{
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

void __fastcall TTweetForm::FormShow(TObject *Sender)
{
  //Wlaczona zaawansowana stylizacja okien
  if(sSkinManager->Active)
  {
	//Kolor labelow
	UsedAvatarsStyleLabel->Kind->Color = GetWarningColor();
	LastAvatarsUpdateLabel->Kind->Color = UsedAvatarsStyleLabel->Kind->Color;
  }
  //Zaawansowana stylizacja okien wylaczona
  else
  {
	//Kolor labelow
	UsedAvatarsStyleLabel->Kind->Color = clGreen;
	LastAvatarsUpdateLabel->Kind->Color = clGreen;
  }
  //Odczyt ustawien wtyczki
  aLoadSettings->Execute();
  //Wlaczanie przyciskow
  SaveButton->Enabled = false;
  //Ustawienie domyslnie wlaczonej karty ustawien
  sPageControl->ActivePage = AvatarsTabSheet;
  //Ustawienie kontrolek
  AvatarsStyleGroupBox->Height = 42;
  EditAvatarsStyleLabel->Caption = "(edytuj)";
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::aLoadSettingsExecute(TObject *Sender)
{
  TIniFile *Ini = new TIniFile( GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
  //Awatary
  AvatarWidthCSpinEdit->Value = Ini->ReadInteger("Avatars","Size",25);
  UnicodeString tLastUpdate = Ini->ReadString("Avatars","LastUpdate","");
  if(!tLastUpdate.IsEmpty())
   LastAvatarsUpdateLabel->Caption = tLastUpdate;
  else
   LastAvatarsUpdateLabel->Caption = "brak danych";
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
   if(HighlightMsgModeComboBox->ItemIndex==2)
	HighlightMsgModeLabel->Visible = true;
   else
	HighlightMsgModeLabel->Visible = false;
  delete Ini;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::aSaveSettingsExecute(TObject *Sender)
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

void __fastcall TTweetForm::aSettingsChangedExecute(TObject *Sender)
{
  //Aktywowanie przycisku zapisywania ustawien
  SaveButton->Enabled = true;
}
//---------------------------------------------------------------------------

//Wymuszanie rozlaczenia sie protokolow HTTP
void __fastcall TTweetForm::aForceDisconnectExecute(TObject *Sender)
{
  //Wymuszenie zakonczenie petli watkow
  ForceDisconnect = true;
  //Rozlaczenie IdHTTP
  IdHTTP->Disconnect();
  AUIdHTTP->Disconnect();
}
//---------------------------------------------------------------------------

//Zamkniecie formy ustawien
void __fastcall TTweetForm::aExitExecute(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::SaveButtonClick(TObject *Sender)
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

void __fastcall TTweetForm::OKButtonClick(TObject *Sender)
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

void __fastcall TTweetForm::AvatarWidthCSpinEditChange(TObject *Sender)
{
  AvatarHeightEdit->Text = AvatarWidthCSpinEdit->Value;
  aSettingsChanged->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AvatarsStyleMemoChange(TObject *Sender)
{
  //Zezwalanie/blokowanie zapisania stylu awatarow
  if(AvatarsStyleMemo->Text.Pos("CC_AVATAR"))
   AvatarStyleSaveButton->Enabled = true;
  else
   AvatarStyleSaveButton->Enabled = false;
  //Zezwalanie/blokowanie przywracanie domyslnego stylu awatarow
  if(AvatarsStyleMemo->Text != "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
   AvatarStyleDefaultButton->Enabled = true;
  else
   AvatarStyleDefaultButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AvatarStyleDefaultButtonClick(TObject *Sender)
{
  //Przywracanie domyslnego stylu awatarow
  if(AvatarsStyleMemo->Text != "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
   AvatarsStyleMemo->Text = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AvatarStyleSaveButtonClick(TObject *Sender)
{
  //Zapisanie stylu awatarow do pliku
  TIniFile *Ini = new TIniFile(GetPluginUserDir() + "\\\\tweetIM\\\\Settings.ini");
  Ini->WriteString("Avatars64", "Style", EncodeBase64(AvatarsStyleMemo->Text));
  delete Ini;
  //Ustawienie stylu w rdzeniu wtyczki
  SetAvatarStyle(AvatarsStyleMemo->Text);
  //Info o rodzaju stylu
  if(AvatarsStyleMemo->Text == "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
  {
	UsedAvatarsStyleLabel->Caption = "domyœlny";
	EditAvatarsStyleLabel->Left = UsedAvatarsStyleLabel->Left + UsedAvatarsStyleLabel->Width + 6;
  }
  else
  {
	UsedAvatarsStyleLabel->Caption = "w³asny";
	EditAvatarsStyleLabel->Left = UsedAvatarsStyleLabel->Left + UsedAvatarsStyleLabel->Width + 6;
  }
  //Zamkniecie edycji stylu awatarow
  EditAvatarsStyleLabel->Caption = "(edytuj)";
  AnimateMode = false;
  AnimateTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::EditAvatarsStyleLabelClick(TObject *Sender)
{
  //Wylaczanie timera animacji
  if(AnimateTimer->Enabled)
   AnimateTimer->Enabled = false;
  //Chowanie edycji stylu awatarow
  if(AvatarsStyleGroupBox->Height==42)
  {
	EditAvatarsStyleLabel->Caption = "(anuluj edycjê)";
	AvatarsStyleMemo->Text = GetAvatarStyle();
	AvatarStyleSaveButton->Enabled = false;
	AnimateMode = true;
	AnimateTimer->Enabled = true;
  }
  //Pokazywanie edycji stylu awatarow
  else
  {
	EditAvatarsStyleLabel->Caption = "(edytuj)";
	AnimateMode = false;
	AnimateTimer->Enabled = true;
  }
  //Zezwalanie/blokowanie przywracanie domyslnego stylu awatarow
  if(AvatarsStyleMemo->Text != "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
   AvatarStyleDefaultButton->Enabled = true;
  else
   AvatarStyleDefaultButton->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::ManualAvatarsUpdateButtonClick(TObject *Sender)
{
  if(ManualAvatarsUpdateButton->Caption=="SprawdŸ aktualizacje")
  {
	//Zmiana caption na buttonie
	ManualAvatarsUpdateButton->Caption = "Przerwij aktualizacje";
	//Sprawdzanie czy folder awatar istnieje
	if(!DirectoryExists(GetPluginUserDirW() + "\\tweetIM\\Avatars"))
	 CreateDir(GetPluginUserDirW() + "\\tweetIM\\Avatars");
	//Wlaczenie paska postepu
	ProgressBar->Position = 0;
	ProgressBar->Visible = true;
	ProgressLabel->Caption = "Pobieranie danych...";
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

void __fastcall TTweetForm::HighlightMsgCheckBoxClick(TObject *Sender)
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

void __fastcall TTweetForm::HighlightMsgListViewEdited(TObject *Sender, TListItem *Item,
		  UnicodeString &S)
{
  aSettingsChanged->Execute();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::HighlightMsgListViewKeyDown(TObject *Sender, WORD &Key,
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

void __fastcall TTweetForm::HighlightMsgListViewSelectItem(TObject *Sender, TListItem *Item,
		  bool Selected)
{
  //Wczytyawanie elementow szybkiej edycji
  if(HighlightMsgListView->ItemIndex!=-1)
  {
	ItemHighlightMsgEdit->Text = HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Caption;
	ColorHighlightMsgEdit->Text = HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->SubItems->Strings[0];
	RemoveHighlightMsgSpeedButton->Enabled = true;
  }
  else
   RemoveHighlightMsgSpeedButton->Enabled = false;
}
//---------------------------------------------------------------------------

//Przywrocenie domylsnych zawartosci kontrolek
void __fastcall TTweetForm::EraseHighlightMsgSpeedButtonClick(TObject *Sender)
{
  ItemHighlightMsgEdit->Text = "";
  ColorHighlightMsgEdit->Text = "#FF0000";
  HighlightMsgListView->ItemIndex = -1;
}
//---------------------------------------------------------------------------

//Pobieranie koloru w HEX z komponentu
void __fastcall TTweetForm::HighlightMsgColorSelectChange(TObject *Sender)
{
  TColor Color = HighlightMsgColorSelect->ColorValue;
  int R,G,B;
  R = GetRValue(Color);
  G = GetGValue(Color);
  B = GetBValue(Color);
  ColorHighlightMsgEdit->Text = "#" + IntToHex(R,2) + IntToHex(G,2) + IntToHex(B,2);
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AddHighlightMsgsSpeedButtonClick(TObject *Sender)
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

void __fastcall TTweetForm::RemoveHighlightMsgSpeedButtonClick(TObject *Sender)
{
  //Usuwanie elementu
  if(HighlightMsgListView->ItemIndex!=-1)
  {
	HighlightMsgListView->Items->Item[HighlightMsgListView->ItemIndex]->Delete();
	SaveButton->Enabled = true;
  }
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::HighlightMsgModeComboBoxChange(TObject *Sender)
{
  aSettingsChanged->Execute();
  if(HighlightMsgModeComboBox->ItemIndex==2)
   HighlightMsgModeLabel->Visible = true;
  else
   HighlightMsgModeLabel->Visible = false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AnimateTimerTimer(TObject *Sender)
{
  //Pokazywanie ukrytego panelu
  if(AnimateMode)
  {
	if(AvatarsStyleGroupBox->Height < 162)
	 AvatarsStyleGroupBox->Height = AvatarsStyleGroupBox->Height + 5;
	else
	 AnimateTimer->Enabled = false;
  }
  //Chowanie panelu
  else
  {
	if(AvatarsStyleGroupBox->Height > 42)
	 AvatarsStyleGroupBox->Height = AvatarsStyleGroupBox->Height - 5;
	else
	 AnimateTimer->Enabled = false;
  }
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::ManualAvatarsUpdateThreadRun(TIdThreadComponent *Sender)
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
	UpdateURL = "http://twitter.com/api/users/profile_image/" + ExtractFileName(FileListBox->FileName);
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
		else
		 DeleteFile(FileListBox->FileName + ".tmp");
	  }
	  else
	   delete MemFile;
	}
	else
	 delete MemFile;
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
  ManualAvatarsUpdateButton->Caption ="SprawdŸ aktualizacje";
  if(ForceDisconnect)
  {
	ManualAvatarsUpdateButton->Enabled = true;
	ForceDisconnect = false;
  }
  //Wylaczenie watku
  ManualAvatarsUpdateThread->Stop();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AutoAvatarsUpdateThreadRun(TIdThreadComponent *Sender)
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
	UpdateURL = "http://twitter.com/api/users/profile_image/" + ExtractFileName(FileListBox->FileName);
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
		else
		 DeleteFile(FileListBox->FileName + ".tmp");
	  }
	  else
	   delete MemFile;
	}
	else
	 delete MemFile;
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
  ManualAvatarsUpdateButton->Caption ="SprawdŸ aktualizacje";
  if(ForceDisconnect)
  {
	ManualAvatarsUpdateButton->Enabled = true;
	ForceDisconnect = false;
  }
  //Wylaczenie watku
  AutoAvatarsUpdateThread->Stop();
}
//---------------------------------------------------------------------------*/

void __fastcall TTweetForm::AUIdHTTPWorkBegin(TObject *ASender, TWorkMode AWorkMode,
		  __int64 AWorkCountMax)
{
  //Wlaczenie timera pilnujacego zawieszenie polaczenia
  AUIdHTTPTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AUIdHTTPWork(TObject *ASender, TWorkMode AWorkMode, __int64 AWorkCount)

{
  //Ponowne wlaczenie timera pilnujacego zawieszenie polaczenia
  AUIdHTTPTimer->Enabled = false;
  AUIdHTTPTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AUIdHTTPWorkEnd(TObject *ASender, TWorkMode AWorkMode)

{
  //Wylaczenie timera pilnujacego zawieszenie polaczenia
  AUIdHTTPTimer->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AUIdHTTPTimerTimer(TObject *Sender)
{
  //Wylaczenie timera pilnujacego zawieszenie polaczenia
  AUIdHTTPTimer->Enabled = false;
  //Odznaczenie wymuszenia przerwania polaczenia
  AUIdHTTPManualDisconnected = true;
  //Przerwanie polaczenia
  AUIdHTTP->Disconnect();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::IdHTTPWorkBegin(TObject *ASender, TWorkMode AWorkMode,
		  __int64 AWorkCountMax)
{
  //Wlaczenie timera pilnujacego zawieszenie polaczenia
  AvatarsIdHTTPTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::IdHTTPWork(TObject *ASender, TWorkMode AWorkMode,
		  __int64 AWorkCount)
{
  //Ponowne wlaczenie timera pilnujacego zawieszenie polaczenia
  AvatarsIdHTTPTimer->Enabled = false;
  AvatarsIdHTTPTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::IdHTTPWorkEnd(TObject *ASender, TWorkMode AWorkMode)
{
  //Wylaczenie timera pilnujacego zawieszenie polaczenia
  AvatarsIdHTTPTimer->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::AvatarsIdHTTPTimerTimer(TObject *Sender)
{
  //Wylaczenie timera pilnujacego zawieszenie polaczenia
  AvatarsIdHTTPTimer->Enabled = false;
  //Odznaczenie wymuszenia przerwania polaczenia
  IdHTTPManualDisconnected = true;
  //Przerwanie polaczenia
  IdHTTP->Disconnect();
}
//---------------------------------------------------------------------------

void __fastcall TTweetForm::GetAvatarsThreadRun(TIdThreadComponent *Sender)
{
  //Pobranie itemu z listy awatarow do pobrania
  UnicodeString TweetSender = GetAvatarsListItem();
  //Jest jakis awatar do pobrania
  if(!TweetSender.IsEmpty())
  {
	//Tworzenie nowego pliku w pamieci
	TMemoryStream* MemFile = new TMemoryStream;
	MemFile->Position = 0;
	//Pobieranie awatara
	if(IdHTTPGetFileToMem(MemFile,"http://twitter.com/api/users/profile_image/" + TweetSender))
	{
	  MemFile->Position = 0;
	  if(MemFile->Size!=0)
	  {
		MemFile->SaveToFile(GetAvatarsDir() + "\\\\" + TweetSender);
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

void __fastcall TTweetForm::ColorHighlightMsgEditChange(TObject *Sender)
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

void __fastcall TTweetForm::sSkinManagerSysDlgInit(TacSysDlgData DlgData, bool &AllowSkinning)
{
  AllowSkinning = false;
}
//---------------------------------------------------------------------------

