//---------------------------------------------------------------------------
// Copyright (C) 2013 Krzysztof Grochocki
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

#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma argsused
#include "TweetFrm.h"
#include <PluginAPI.h>
#include <inifiles.hpp>
#include <IdHashMessageDigest.hpp>
#include <fstream.h>
#include <XMLDoc.hpp>

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------

//Uchwyt-do-formy-ustawien---------------------------------------------------
TTweetForm* hTweetForm;
//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//---------------------------------------------------------------------------
//Informacje o aktywnej zakladce
UnicodeString ActiveTabJID;
UnicodeString ActiveTabRes;
int ActiveTabUsrIdx;
//Styl zalacznika
UnicodeString AttachmentStyle;
//Awatary
TStringList *GetAvatarsList = new TStringList;
UnicodeString AvatarStyle;
UnicodeString AvatarsDir;
UnicodeString AvatarsDirW;
//Blokowanie/zezwalanie na sprawdzanie danych w OnPerformCopyData
bool BlockPerformCopyData;
//Dane z notyfikacji OnPerformCopyData
UnicodeString ItemCopyData;
 //Uchwyt do okna rozmowy
HWND hFrmSend;
//Uchwyt do pola RichEdit
HWND hRichEdit;
//Ostatnio wyswietlona wiadomosci od bota
TCustomIniFile* LastAddLineBody = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
TCustomIniFile* LastRecvMsgBody = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
//SETTINGS-------------------------------------------------------------------
int AvatarSize;
UnicodeString StaticAvatarStyle;
bool HighlightMsgChk;
int HighlightMsgModeChk;
TStringList *HighlightMsgItemsList = new TStringList;
TCustomIniFile* HighlightMsgColorsList = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
//FORWARD-AQQ-HOOKS----------------------------------------------------------
int __stdcall OnActiveTab(WPARAM wParam, LPARAM lParam);
int __stdcall OnAddLine(WPARAM wParam, LPARAM lParam);
int __stdcall OnColorChange(WPARAM wParam, LPARAM lParam);
int __stdcall OnModulesLoaded(WPARAM wParam, LPARAM lParam);
int __stdcall OnPerformCopyData(WPARAM wParam, LPARAM lParam);
int __stdcall OnPrimaryTab(WPARAM wParam, LPARAM lParam);
int __stdcall OnRecvMsg(WPARAM wParam, LPARAM lParam);
int __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam);
int __stdcall ServicetweetIMFastSettingsItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceInsertTagItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceInsertNickItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceSendPrivMsgItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceLikeLastTweetItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceShowTimelineItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceShowUserProfileItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceUpdateCommandItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceIngCommandItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceErsCommandItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceUndoTweetCommandItem(WPARAM wParam, LPARAM lParam);
int __stdcall ServiceSavedSearchesCommandItem(WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Szukanie uchwytu do kontrolki TsRichEdit
bool CALLBACK FindRichEdit(HWND hWnd, LPARAM)
{
  //Pobranie klasy okna
  wchar_t WClassName[128];
  GetClassNameW(hWnd, WClassName, sizeof(WClassName));
  //Sprawdzenie klasy okna
  if((UnicodeString)WClassName=="TsRichEdit")
  {
	//Przypisanie uchwytu
	hRichEdit = hWnd;
	return false;
  }
  return true;
}
//---------------------------------------------------------------------------

//Konwersja ciagu znakow na potrzeby INI
UnicodeString StrToIniStr(UnicodeString Str)
{
  //Definicja zmiennych
  wchar_t Buffer[50010];
  wchar_t* B;
  wchar_t* S;
  //Przekazywanie ciagu znakow
  S = Str.w_str();
  //Ustalanie wskaznika
  B = Buffer;
  //Konwersja znakow
  while(*S!='\0')
  {
	switch(*S)
	{
	  case 13:
	  case 10:
		if((*S==13)&&(S[1]==10)) S++;
		else if((*S==10)&&(S[1] == 13)) S++;
		*B = '\\';
		B++;
		*B = 'n';
		B++;
		S++;
	  break;
	  default:
		*B = *S;
		B++;
		S++;
	  break;
	}
  }
  *B = '\0';
  //Zwracanie zkonwertowanego ciagu znakow
  return (wchar_t*)Buffer;
}
//---------------------------------------------------------------------------
UnicodeString IniStrToStr(UnicodeString Str)
{
  //Definicja zmiennych
  wchar_t Buffer[50010];
  wchar_t* B;
  wchar_t* S;
  //Przekazywanie ciagu znakow
  S = Str.w_str();
  //Ustalanie wskaznika
  B = Buffer;
  //Konwersja znakow
  while(*S!='\0')
  {
	if((S[0]=='\\')&&(S[1]=='n'))
	{
	  *B = 13;
	  B++;
	  *B = 10;
	  B++;
	  S++;
	  S++;
	}
	else
	{
	  *B = *S;
	  B++;
	  S++;
	}
  }
  *B = '\0';
  //Zwracanie zkonwertowanego ciagu znakow
  return (wchar_t*)Buffer;
}
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------
UnicodeString GetPluginUserDirW()
{
  return (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do kompozycji
UnicodeString GetThemeDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do skorki kompozycji
UnicodeString GetThemeSkinDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do pliku DLL wtyczki
UnicodeString GetPluginDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINDIR,(WPARAM)(HInstance),0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki do folderu cache z awatarami
UnicodeString GetAvatarsDir()
{
  return AvatarsDir;
}
//---------------------------------------------------------------------------

//Sprawdzanie czy  wlaczona jest zaawansowana stylizacja okien
bool ChkSkinEnabled()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString SkinsEnabled = Settings->ReadString("Settings","UseSkin","1");
  delete Settings;
  return StrToBool(SkinsEnabled);
}
//---------------------------------------------------------------------------

//Sprawdzanie ustawien animacji AlphaControls
bool ChkThemeAnimateWindows()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString AnimateWindowsEnabled = Settings->ReadString("Theme","ThemeAnimateWindows","1");
  delete Settings;
  return StrToBool(AnimateWindowsEnabled);
}
//---------------------------------------------------------------------------
bool ChkThemeGlowing()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString GlowingEnabled = Settings->ReadString("Theme","ThemeGlowing","1");
  delete Settings;
  return StrToBool(GlowingEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien koloru AlphaControls
int GetHUE()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETHUE,0,0);
}
//---------------------------------------------------------------------------
int GetSaturation()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETSATURATION,0,0);
}
//---------------------------------------------------------------------------

//Pobranie stylu labela
TColor GetWarningColor()
{
  //Odczyt pliku
  hTweetForm->FileMemo->Lines->LoadFromFile(GetThemeDir()+"\\\\elements.xml");
  hTweetForm->FileMemo->Text = "<content>" + hTweetForm->FileMemo->Text + " </content>";
  _di_IXMLDocument XMLDoc = LoadXMLData(hTweetForm->FileMemo->Text);
  _di_IXMLNode MainNode = XMLDoc->DocumentElement;
  int MainNodesCount = MainNode->ChildNodes->GetCount();
  //Parsowanie pliku XML
  for(int Count=0;Count<MainNodesCount;Count++)
  {
	_di_IXMLNode MainNodes = MainNode->ChildNodes->GetNode(Count);
	if(MainNodes->NodeName=="colors")
	{
	  int ChildNodesCount = MainNodes->ChildNodes->GetCount();
	  for(int ChildCount=0;ChildCount<ChildNodesCount;ChildCount++)
	  {
		_di_IXMLNode ChildNode = MainNodes->ChildNodes->GetNode(ChildCount);
		if(ChildNode->NodeName=="label-warning")
		{
          //Parsowanie koloru
		  UnicodeString Color = ChildNode->Attributes["color"];
		  Color.Delete(1,1);
		  UnicodeString Red = Color;
		  Red.Delete(3,Red.Length());
		  UnicodeString Green = Color;
		  Green.Delete(1,2);
		  Green.Delete(3,Green.Length());
		  UnicodeString Blue = Color;
		  Blue.Delete(1,4);
		  //Konwersja HEX na RGB
		  return (TColor)RGB(HexToInt(Red),HexToInt(Green),HexToInt(Blue));
        }
	  }
	}
  }
  return (TColor)RGB(0,0,0);
}
//---------------------------------------------------------------------------

//Pobieranie informacji o pliku (wersja itp)
UnicodeString GetFileInfo(wchar_t *ModulePath, String KeyName)
{
  LPVOID lpStr1 = NULL, lpStr2 = NULL;
  WORD* wTmp;
  DWORD dwHandlev = NULL;
  UINT dwLength;
  wchar_t sFileName[1024] = {0};
  wchar_t sTmp[1024] = {0};
  UnicodeString sInfo;
  LPBYTE *pVersionInfo;

  if(ModulePath == NULL) GetModuleFileName( NULL, sFileName, 1024);
  else wcscpy(sFileName, ModulePath);

  DWORD dwInfoSize = GetFileVersionInfoSize(sFileName, &dwHandlev);

  if(dwInfoSize)
  {
	pVersionInfo = new LPBYTE[dwInfoSize];
	if(GetFileVersionInfo(sFileName, dwHandlev, dwInfoSize, pVersionInfo))
	{
	  if(VerQueryValue(pVersionInfo, L"\\VarFileInfo\\Translation", &lpStr1, &dwLength))
	  {
		wTmp = (WORD*)lpStr1;
		swprintf(sTmp, ("\\StringFileInfo\\%04x%04x\\" + KeyName).w_str(), *wTmp, *(wTmp + 1));
		if(VerQueryValue(pVersionInfo, sTmp, &lpStr2, &dwLength)) sInfo = (LPCTSTR)lpStr2;
	  }
	}
	delete[] pVersionInfo;
  }
  return sInfo;
}
//---------------------------------------------------------------------------

//Sprawdzanie czy przekazany znak jest dozwolony
bool AllowedTagsCharacters(UnicodeString Text)
{
  UnicodeString Characters[] = {" ","!","@","#","$","%","^","&","*","(",")","-","_","=","+","[","{","]","}",":",";","'",'"',"<",",",">",".","?","/","\\","|","`","~","’"};
  for(int Char=0;Char<34;Char++)
  {
	if(Text.Pos(Characters[Char])>0) return false;
  }
  return true;
}
//---------------------------------------------------------------------------

//Sprawdzanie czy przekazany znak jest dozwolony
bool AllowedUsersCharacters(UnicodeString Text)
{
  UnicodeString Characters[] = {" ","!","@","#","$","%","^","&","*","(",")","-","=","+","[","{","]","}",":",";","'",'"',"<",",",">",".","?","/","\\","|","`","~","’"};
  for(int Char=0;Char<33;Char++)
  {
	if(Text.Pos(Characters[Char])>0) return false;
  }
  return true;
}
//---------------------------------------------------------------------------

//Zwracanie pozycji uciecia tagu
int TagsCutPosition(UnicodeString Text)
{
  UnicodeString Characters[] = {" ","!","@","#","$","%","^","&","*","(",")","=","+","[","{","]","}",":",";","'",'"',"<",",",">",".","?","/","\\","|","`","~","’"};
  int xPos = Text.Length()+1;
  int yPos;
  for(int Char=0;Char<32;Char++)
  {
	yPos = Text.Pos(Characters[Char]);
	if((yPos<xPos)&&(yPos!=0)) xPos = yPos;
  }
  return xPos;
}
//---------------------------------------------------------------------------

//Zwracanie pozycji uciecia uzytkownika Twittera
int UsersCutPosition(UnicodeString Text)
{
  UnicodeString Characters[] = {" ","!","@","#","$","%","^","&","*","(",")","-","=","+","[","{","]","}",":",";","'",'"',"<",",",">",".","?","/","\\","|","`","~","’"};
  int xPos = Text.Length()+1;
  int yPos;
  for(int Char=0;Char<33;Char++)
  {
	yPos = Text.Pos(Characters[Char]);
	if((yPos<xPos)&&(yPos!=0)) xPos = yPos;
  }
  return xPos;
}
//---------------------------------------------------------------------------

//Pobranie stylu Attachment & Avatars
void GetThemeStyle()
{
  //Bufer odczytu plikow
  char FileBuffer[1024];
  int FileLength;
  int CurrentLength;
  //Reset stylow
  AttachmentStyle = "";
  AvatarStyle = "";
  //URL do aktuanie uzywanej kompozycji
  UnicodeString ThemeURL = GetThemeDir();
  //URL do domyslnej kompozycji
  UnicodeString ThemeURLW = (wchar_t*)(PluginLink.CallService(AQQ_FUNCTION_GETAPPPATH,0,0));
  ThemeURLW = StringReplace(ThemeURLW, "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
  ThemeURLW = ThemeURLW + "\\\\System\\\\Shared\\\\Themes\\\\Standard";
  //Pobieranie stylu wiadomosci
  if(FileExists(ThemeURL + "\\\\Message\\\\In.htm"))
   hTweetForm->FileMemo->Lines->LoadFromFile(ThemeURL + "\\\\Message\\\\In.htm");
  else
   hTweetForm->FileMemo->Lines->LoadFromFile(ThemeURLW + "\\\\Message\\\\In.htm");
  AttachmentStyle = hTweetForm->FileMemo->Text;
  //Formatowanie
  AttachmentStyle.Delete(1,AttachmentStyle.Pos("CC_TEXT")+6);
  //Styl zalacznika
  if(FileExists(ThemeURL + "\\\\Message\\\\Attachment.htm"))
   hTweetForm->FileMemo->Lines->LoadFromFile(ThemeURL + "\\\\Message\\\\Attachment.htm");
  else
   hTweetForm->FileMemo->Lines->LoadFromFile(ThemeURLW + "\\\\Message\\\\Attachment.htm");
  AttachmentStyle = AttachmentStyle + hTweetForm->FileMemo->Text;
  //Formatowanie
  AttachmentStyle = StringReplace(AttachmentStyle, "CC_DAYNAME :: CC_TIME", "", TReplaceFlags() << rfReplaceAll);
  AttachmentStyle = StringReplace(AttachmentStyle, "CC_DAYNAME", "", TReplaceFlags() << rfReplaceAll);
  AttachmentStyle = StringReplace(AttachmentStyle, "CC_TIME", "", TReplaceFlags() << rfReplaceAll);
  AttachmentStyle = StringReplace(AttachmentStyle, "CC_SMARTDATE", "", TReplaceFlags() << rfReplaceAll);
  AttachmentStyle = AttachmentStyle.Trim();
  //Pobieranie stylu awatarow
  if(FileExists(ThemeURL + "\\\\Message\\\\TweetAvatar.htm"))
  {
    //Pobieranie danych z pliku
	hTweetForm->FileMemo->Lines->LoadFromFile(ThemeURL + "\\\\Message\\\\TweetAvatar.htm");
	AvatarStyle = hTweetForm->FileMemo->Text;
	AvatarStyle = AvatarStyle.Trim();
	//Sprawdzanie zawartosci pliku
	if(AvatarStyle.Pos("CC_AVATAR"))
	{
	  hTweetForm->UsedAvatarsStyleLabel->Caption = "z kompozycji";
	  hTweetForm->EditAvatarsStyleLabel->Left = hTweetForm->UsedAvatarsStyleLabel->Left + hTweetForm->UsedAvatarsStyleLabel->Width + 6;
	  hTweetForm->EditAvatarsStyleLabel->Caption = "(edytuj)";
	  hTweetForm->AvatarsStyleGroupBox->Height = 42;
	  hTweetForm->EditAvatarsStyleLabel->Enabled = false;
	}
	else if(!StaticAvatarStyle.IsEmpty())
	{
	  AvatarStyle = StaticAvatarStyle;
	  hTweetForm->UsedAvatarsStyleLabel->Caption = "w³asny";
	  hTweetForm->EditAvatarsStyleLabel->Left = hTweetForm->UsedAvatarsStyleLabel->Left + hTweetForm->UsedAvatarsStyleLabel->Width + 6;
	  hTweetForm->EditAvatarsStyleLabel->Caption = "(edytuj)";
	  hTweetForm->AvatarsStyleGroupBox->Height = 42;
	  hTweetForm->EditAvatarsStyleLabel->Enabled = true;
	}
	else
	{
	  AvatarStyle = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
	  hTweetForm->UsedAvatarsStyleLabel->Caption = "domyœlny";
	  hTweetForm->EditAvatarsStyleLabel->Left = hTweetForm->UsedAvatarsStyleLabel->Left + hTweetForm->UsedAvatarsStyleLabel->Width + 6;
	  hTweetForm->EditAvatarsStyleLabel->Caption = "(edytuj)";
	  hTweetForm->AvatarsStyleGroupBox->Height = 42;
	  hTweetForm->EditAvatarsStyleLabel->Enabled = true;
	}
  }
  else if(!StaticAvatarStyle.IsEmpty())
  {
	AvatarStyle = StaticAvatarStyle;
	hTweetForm->UsedAvatarsStyleLabel->Caption = "w³asny";
	hTweetForm->EditAvatarsStyleLabel->Left = hTweetForm->UsedAvatarsStyleLabel->Left + hTweetForm->UsedAvatarsStyleLabel->Width + 6;
	hTweetForm->EditAvatarsStyleLabel->Caption = "(edytuj)";
	hTweetForm->AvatarsStyleGroupBox->Height = 42;
	hTweetForm->EditAvatarsStyleLabel->Enabled = true;
  }
  else
  {
	AvatarStyle = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
	hTweetForm->UsedAvatarsStyleLabel->Caption = "domyœlny";
	hTweetForm->EditAvatarsStyleLabel->Left = hTweetForm->UsedAvatarsStyleLabel->Left + hTweetForm->UsedAvatarsStyleLabel->Width + 6;
	hTweetForm->EditAvatarsStyleLabel->Caption = "(edytuj)";
	hTweetForm->AvatarsStyleGroupBox->Height = 42;
	hTweetForm->EditAvatarsStyleLabel->Enabled = true;
  }
}
//---------------------------------------------------------------------------

//Pobieranie stylu awatarow z rdzenia wtyczki
UnicodeString GetAvatarStyle()
{
  return AvatarStyle;
}
//---------------------------------------------------------------------------

//Ustawianie stylu awatarow w rdzeniu wtyczki
void SetAvatarStyle(UnicodeString Style)
{
  AvatarStyle = Style;
}
//---------------------------------------------------------------------------

//Automatyczna aktualizacja awatarow
void AutoAvatarsUpdate()
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\tweetIM\\\\Settings.ini");
  //Pobieranie sposobu aktualizacji awatarow
  int AutoAvatarsUpdateMode = Ini->ReadInteger("Avatars","UpdateMode",0);
  //Jezeli awatary maja byc w ogole aktualizowane automatycznie
  if(AutoAvatarsUpdateMode)
  {
	//Pobieranie daty ostatniej aktualizacji
	UnicodeString sLast = Ini->ReadString("Avatars", "LastUpdate", "");
	if(!sLast.IsEmpty())
	 sLast.Delete(sLast.Pos(" "),sLast.Length());
	else
	 sLast = "1899-12-30";
	//Zmienne dekodowania daty
	double Last;
	double Current;
	int DiffTime;
	//Dekodowanie daty ostatniej aktualizacji
	UnicodeString LastYear = sLast;
	LastYear = LastYear.Delete(LastYear.Pos("-"),LastYear.Length());
	UnicodeString LastMonth= sLast;
	LastMonth = LastMonth.Delete(1,LastMonth.Pos("-"));
	LastMonth = LastMonth.Delete(LastMonth.Pos("-"),LastMonth.Length());
	UnicodeString LastDay = sLast;
	LastDay = LastDay.Delete(1,LastDay.Pos("-"));
	LastDay = LastDay.Delete(1,LastDay.Pos("-"));
	Last = EncodeDate(StrToInt(LastYear),StrToInt(LastMonth),StrToInt(LastDay));
	//Pobieranie obecnej daty
	TDateTime Time = TDateTime::CurrentDateTime();
	Current = EncodeDate(StrToInt(Time.FormatString("yyyy")), StrToInt(Time.FormatString("mm")), StrToInt(Time.FormatString("dd")));
	//Porownanie dat
	DiffTime = difftime(Current,Last);
    //Sprawdzanie czy aktualizacja jest wymagana
	if(((AutoAvatarsUpdateMode==1)&&(DiffTime>=1))||
	((AutoAvatarsUpdateMode==2)&&(DiffTime>=7))||
	((AutoAvatarsUpdateMode==3)&&(DiffTime>=30)))
	{
	  //Zmiana caption na buttonie
	  hTweetForm->ManualAvatarsUpdateButton->Caption = "Przerwij aktualizacje";
	  //Tworzenie katalogu z awatarami
	  if(!DirectoryExists(AvatarsDir)) CreateDir(AvatarsDir);
	  //Wlaczenie paska postepu
	  hTweetForm->ProgressBar->Position = 0;
	  hTweetForm->ProgressBar->Visible = true;
	  hTweetForm->ProgressLabel->Caption = "Pobieranie danych...";
	  hTweetForm->ProgressLabel->Visible = true;
	  //Pobieranie listy plikow
	  hTweetForm->FileListBox->Directory = "";
	  hTweetForm->FileListBox->Directory = GetPluginUserDirW() + "\\tweetIM\\Avatars";
	  //Ignoowanie plikow *.tmp
	  for(int Count=0;Count<hTweetForm->FileListBox->Items->Count;Count++)
	  {
		if(ExtractFileName(hTweetForm->FileListBox->Items->Strings[Count]).Pos(".tmp")>0)
		{
		  DeleteFile(hTweetForm->FileListBox->Items->Strings[Count]);
		  hTweetForm->FileListBox->Items->Strings[Count] ="TMP_DELETE";
		}
	  }
	  while(hTweetForm->FileListBox->Items->IndexOf("TMP_DELETE")!=-1)
	   hTweetForm->FileListBox->Items->Delete(hTweetForm->FileListBox->Items->IndexOf("TMP_DELETE"));
	  //Ustawianie maksymalnego paska postepu
	  hTweetForm->ProgressBar->Max = hTweetForm->FileListBox->Items->Count;
	  //Wlacznie aktualizacji
	  hTweetForm->AutoAvatarsUpdateThread->Start();
	}
  }
  delete Ini;
}
//---------------------------------------------------------------------------

bool ChkAvatarsListItem()
{
  if(GetAvatarsList->Count) return true;
  else return false;
}
//---------------------------------------------------------------------------

UnicodeString GetAvatarsListItem()
{
  if(GetAvatarsList->Count)
  {
	UnicodeString Item = GetAvatarsList->Strings[0];
	GetAvatarsList->Delete(0);
	return Item;
  }
  else return "";
}
//---------------------------------------------------------------------------

//Tworzenie elementu szybkiego dostepu do ustawien wtyczki
void BuildtweetIMFastSettings()
{
  TPluginAction FastSettingsItem;
  ZeroMemory(&FastSettingsItem,sizeof(TPluginAction));
  FastSettingsItem.cbSize = sizeof(TPluginAction);
  FastSettingsItem.pszName = L"tweetIMFastSettingsItemButton";
  FastSettingsItem.pszCaption = L"tweet.IM";
  FastSettingsItem.Position = 0;
  FastSettingsItem.IconIndex = 131;
  FastSettingsItem.pszService = L"stweetIMFastSettingsItem";
  FastSettingsItem.pszPopupName = L"popPlugins";
  FastSettingsItem.PopupPosition = 0;
  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&FastSettingsItem));
}
//---------------------------------------------------------------------------

//Serwis szybkiego dostepu do ustawien wtyczki
int __stdcall ServicetweetIMFastSettingsItem(WPARAM wParam, LPARAM lParam)
{
  //Przypisanie uchwytu do formy ustawien
  if(!hTweetForm)
  {
	Application->Handle = (HWND)TweetForm;
	hTweetForm = new TTweetForm(Application);
  }
  //Pokaznie okna ustawien
  hTweetForm->Show();

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do wstawiania tagu
int __stdcall ServiceInsertTagItem(WPARAM wParam, LPARAM lParam)
{
  //Szukanie pola wiadomosci
  if(!hRichEdit) EnumChildWindows(hFrmSend,(WNDENUMPROC)FindRichEdit,0);
  //Pobieranie tekstu z RichEdit
  int iLength = GetWindowTextLengthW(hRichEdit)+1;
  wchar_t* pBuff = new wchar_t[iLength];
  GetWindowTextW(hRichEdit, pBuff, iLength);
  UnicodeString Text = pBuff;
  delete pBuff;
  //Wiadomosc nie zawiera wskazanego tagu
  if(!Text.Pos(ItemCopyData))
  {
	//Usuwanie znakow spacji z prawej strony
	Text = Text.TrimRight();
	//Ustawianie tekstu
	if(!Text.IsEmpty())
	 SetWindowTextW(hRichEdit, (Text + " " + ItemCopyData + " ").w_str());
	else
	 SetWindowTextW(hRichEdit, (ItemCopyData + " ").w_str());
  }
  //Kasowanie tagu
  ItemCopyData = "";

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do wstawiania nicku
int __stdcall ServiceInsertNickItem(WPARAM wParam, LPARAM lParam)
{
  //Szukanie pola wiadomosci
  if(!hRichEdit) EnumChildWindows(hFrmSend,(WNDENUMPROC)FindRichEdit,0);
  //Pobieranie tekstu z RichEdit
  int iLength = GetWindowTextLengthW(hRichEdit)+1;
  wchar_t* pBuff = new wchar_t[iLength];
  GetWindowTextW(hRichEdit, pBuff, iLength);
  UnicodeString Text = pBuff;
  delete pBuff;
  //Wiadomosc nie zawiera wskazanego nicka
  if(!Text.Pos("@"+ItemCopyData))
  {
	//Usuwanie znakow spacji z prawej strony
	Text = Text.TrimRight();
	//Ustawianie tekstu
	if(!Text.IsEmpty())
	 SetWindowTextW(hRichEdit, (Text + " @" + ItemCopyData + " ").w_str());
	else
	 SetWindowTextW(hRichEdit, ("@" + ItemCopyData + " ").w_str());
  }
  //Kasowanie nicku
  ItemCopyData = "";

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do wysylania prywarnej wiadomosci
int __stdcall ServiceSendPrivMsgItem(WPARAM wParam, LPARAM lParam)
{
  //Szukanie pola wiadomosci
  if(!hRichEdit) EnumChildWindows(hFrmSend,(WNDENUMPROC)FindRichEdit,0);
  //Pobieranie tekstu z RichEdit
  int iLength = GetWindowTextLengthW(hRichEdit)+1;
  wchar_t* pBuff = new wchar_t[iLength];
  GetWindowTextW(hRichEdit, pBuff, iLength);
  UnicodeString Text = pBuff;
  delete pBuff;
  //Wiadomosc zawiera jakis tekst
  if(!Text.IsEmpty())
  {
	if(Text.Pos("d ")==1)
	{
	  if(!Text.Pos("d " + ItemCopyData))
	  {
		Text = StringReplace(Text, "d ", "d " + ItemCopyData + " ", TReplaceFlags() << rfReplaceAll);
		SetWindowTextW(hRichEdit, Text.w_str());
	  }
	}
	else SetWindowTextW(hRichEdit, ("d " + ItemCopyData + " " + Text.TrimLeft()).w_str());
  }
  //Wiadomosc jest pusta
  else SetWindowTextW(hRichEdit, ("d " + ItemCopyData + " ").w_str());
  //Kasowanie nicku
  ItemCopyData = "";

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do lajkowania ostatniego tweetniecia
int __stdcall ServiceLikeLastTweetItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = ("fav " + ItemCopyData).w_str();
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do pokazywania najnowszych tweetniec uzytkownika
int __stdcall ServiceShowTimelineItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = ("tl " + ItemCopyData).w_str();
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do pokazywania informacji o uzytkowniku
int __stdcall ServiceShowUserProfileItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = ("p " + ItemCopyData).w_str();
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do "Pobierz nieprzeczytane tweet"
int __stdcall ServiceUpdateCommandItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = L"update";
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do "Obserwuj¹cy"
int __stdcall ServiceIngCommandItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = L"ing";
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do "Obserwuj¹cy"
int __stdcall ServiceErsCommandItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = L"ers";
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do "Usun poprzedne tweetniecie"
int __stdcall ServiceUndoTweetCommandItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = L"undo";
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Serwis do "Zapisane wyszukiwania"
int __stdcall ServiceSavedSearchesCommandItem(WPARAM wParam, LPARAM lParam)
{
  //Struktura kontaktu
  TPluginContact PluginContact;
  PluginContact.JID = ActiveTabJID.w_str();
  PluginContact.Resource = ActiveTabRes.w_str();
  PluginContact.FromPlugin = false;
  PluginContact.UserIdx = ActiveTabUsrIdx;
  //Struktura wiadomosci
  TPluginMessage PluginMessage;
  PluginMessage.cbSize = sizeof(TPluginMessage);
  PluginMessage.JID = ActiveTabJID.w_str();
  PluginMessage.Body = L"ss";
  PluginMessage.Store = false;
  PluginMessage.ShowAsOutgoing = true;
  //Wysylanie wiadomosci
  PluginLink.CallService(AQQ_CONTACTS_SENDMSG ,(WPARAM)(&PluginContact),(LPARAM)(&PluginMessage));

  return 0;
}
//---------------------------------------------------------------------------

//Usuwanie buttona z komendami bota
void DestroyCommandItems()
{
  //Usuwanie "Pobierz nieprzeczytane tweety"
  TPluginAction UpdateCommandItem;
  ZeroMemory(&UpdateCommandItem,sizeof(TPluginAction));
  UpdateCommandItem.cbSize = sizeof(TPluginAction);
  UpdateCommandItem.pszName = L"tweetIMUpdateCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&UpdateCommandItem));
  //Usuwanie "Obserwowani"
  TPluginAction IngCommandItem;
  ZeroMemory(&IngCommandItem,sizeof(TPluginAction));
  IngCommandItem.cbSize = sizeof(TPluginAction);
  IngCommandItem.pszName = L"tweetIMIngCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&IngCommandItem));
  //Usuwanie "Obserwuj¹cy"
  TPluginAction ErsCommandItem;
  ZeroMemory(&ErsCommandItem,sizeof(TPluginAction));
  ErsCommandItem.cbSize = sizeof(TPluginAction);
  ErsCommandItem.pszName = L"tweetIMErsCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ErsCommandItem));
  //Usuwanie "Usun poprzednie tweetniecie"
  TPluginAction UndoTweetCommandItem;
  ZeroMemory(&UndoTweetCommandItem,sizeof(TPluginAction));
  UndoTweetCommandItem.cbSize = sizeof(TPluginAction);
  UndoTweetCommandItem.pszName = L"tweetIMUndoTweetCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&UndoTweetCommandItem));
  //Usuwanie "Zapisane wyszukiwania"
  TPluginAction SavedSearchesCommandItem;
  ZeroMemory(&SavedSearchesCommandItem,sizeof(TPluginAction));
  SavedSearchesCommandItem.cbSize = sizeof(TPluginAction);
  SavedSearchesCommandItem.pszName = L"tweetIMSavedSearcheCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SavedSearchesCommandItem));
  //Usuwanie buttona w oknie rozmowy
  TPluginAction CommandButton;
  ZeroMemory(&CommandButton,sizeof(TPluginAction));
  CommandButton.cbSize = sizeof(TPluginAction);
  CommandButton.pszName = L"tweetIMCommandButton";
  CommandButton.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_TOOLBAR "tbMain" AQQ_CONTROLS_DESTROYBUTTON ,0,(LPARAM)(&CommandButton));
}
//---------------------------------------------------------------------------

//Tworzenie buttona z komendami bota
void BuildCommandItems()
{
  //Usuwanie buttona
  DestroyCommandItems();
  //Tworzenie buttona
  if(hFrmSend)
  {
	//Tworzenie buttona w oknie rozmowy
	TPluginAction CommandButton;
	ZeroMemory(&CommandButton,sizeof(TPluginAction));
	CommandButton.cbSize = sizeof(TPluginAction);
	CommandButton.pszName = L"tweetIMCommandButton";
	CommandButton.Hint = L"Komendy bota tweet.IM";
	CommandButton.Position = 0;
	CommandButton.IconIndex = 131;
	CommandButton.pszPopupName = L"tweetIMCommandPopUp";
	CommandButton.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_TOOLBAR "tbMain" AQQ_CONTROLS_CREATEBUTTON,0,(LPARAM)(&CommandButton));
	//Tworzenie "Zapisane wyszukiwania"
	TPluginAction SavedSearchesCommandItem;
	ZeroMemory(&SavedSearchesCommandItem,sizeof(TPluginAction));
	SavedSearchesCommandItem.cbSize = sizeof(TPluginAction);
	SavedSearchesCommandItem.pszName = L"tweetIMSavedSearcheCommandItem";
	SavedSearchesCommandItem.pszCaption = L"Zapisane wyszukiwania";
	SavedSearchesCommandItem.Position = 0;
	SavedSearchesCommandItem.IconIndex = 16;
	SavedSearchesCommandItem.pszService = L"stweetIMSavedSearcheCommandItem";
	SavedSearchesCommandItem.pszPopupName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SavedSearchesCommandItem));
	//Tworzenie "Usun poprzednie tweetniecie"
	TPluginAction UndoTweetCommandItem;
	ZeroMemory(&UndoTweetCommandItem,sizeof(TPluginAction));
	UndoTweetCommandItem.cbSize = sizeof(TPluginAction);
	UndoTweetCommandItem.pszName = L"tweetIMUndoTweetCommandItem";
	UndoTweetCommandItem.pszCaption = L"Usuñ poprzednie tweetniêcie";
	UndoTweetCommandItem.Position = 0;
	UndoTweetCommandItem.IconIndex = 10;
	UndoTweetCommandItem.pszService = L"stweetIMUndoTweetCommandItem";
	UndoTweetCommandItem.pszPopupName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&UndoTweetCommandItem));
	//Tworzenie "Obserwuj¹cy"
	TPluginAction ErsCommandItem;
	ZeroMemory(&ErsCommandItem,sizeof(TPluginAction));
	ErsCommandItem.cbSize = sizeof(TPluginAction);
	ErsCommandItem.pszName = L"tweetIMErsCommandItem";
	ErsCommandItem.pszCaption = L"Obserwuj¹cy";
	ErsCommandItem.Position = 0;
	ErsCommandItem.IconIndex = 21;
	ErsCommandItem.pszService = L"stweetIMErsCommandItem";
	ErsCommandItem.pszPopupName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&ErsCommandItem));
	//Tworzenie "Obserwowani"
	TPluginAction IngCommandItem;
	ZeroMemory(&IngCommandItem,sizeof(TPluginAction));
	IngCommandItem.cbSize = sizeof(TPluginAction);
	IngCommandItem.pszName = L"tweetIMIngCommandItem";
	IngCommandItem.pszCaption = L"Obserwowani";
	IngCommandItem.Position = 0;
	IngCommandItem.IconIndex = 21;
	IngCommandItem.pszService = L"stweetIMIngCommandItem";
	IngCommandItem.pszPopupName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&IngCommandItem));
	//Tworzenie "Pobierz nieprzeczytane tweety"
	TPluginAction UpdateCommandItem;
	ZeroMemory(&UpdateCommandItem,sizeof(TPluginAction));
	UpdateCommandItem.cbSize = sizeof(TPluginAction);
	UpdateCommandItem.pszName = L"tweetIMUpdateCommandItem";
	UpdateCommandItem.pszCaption = L"Pobierz nieprzeczytane tweety";
	UpdateCommandItem.Position = 0;
	UpdateCommandItem.IconIndex = 19;
	UpdateCommandItem.pszService = L"stweetIMUpdateCommandItem";
	UpdateCommandItem.pszPopupName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&UpdateCommandItem));
  }
}
//---------------------------------------------------------------------------

//Hook na aktwyna zakladke lub okno rozmowy (pokazywanie menu do cytowania, tworzenie buttonow)
int __stdcall OnActiveTab(WPARAM wParam, LPARAM lParam)
{
  //Pobranie uchwytu do okna rozmowy
  if(!hFrmSend) hFrmSend = (HWND)wParam;
  //Szukanie pola wiadomosci
  if(!hRichEdit) EnumChildWindows(hFrmSend,(WNDENUMPROC)FindRichEdit,0);
  //Pobieranie danych kontatku
  TPluginContact ActiveTabContact = *(PPluginContact)lParam;
  ActiveTabJID = (wchar_t*)ActiveTabContact.JID;
  ActiveTabRes = (wchar_t*)ActiveTabContact.Resource;
  ActiveTabUsrIdx = ActiveTabContact.UserIdx;
  //Kontakt jest botem tweet.IM
  if(ActiveTabJID.Pos("@twitter.tweet.im"))
  {
	//"Wlaczenie" procki zbierajacej adresu URL
	BlockPerformCopyData = false;
	//Tworzenie buttona z komendami bota
	BuildCommandItems();
  }
  else
  {
	//"Wylaczenie" procki zbierajacej adresu URL
	BlockPerformCopyData = true;
	//Usuwanie buttona z komendami bota
	DestroyCommandItems();
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na pokazywane wiadomosci (formatowanie tweetow)
int __stdcall OnAddLine(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie danych kontatku
  TPluginContact AddLineContact = *(PPluginContact)wParam;
  //Pobieranie identyfikatora kontatku
  UnicodeString ContactJID = (wchar_t*)AddLineContact.JID;
  //Kontakt jest botem tweet.IM
  if(ContactJID.Pos("@twitter.tweet.im"))
  {
	//Nadawca wiadomosci
	UnicodeString TweetSender = "";
	//Pobieranie informacji  wiadomosci
	TPluginMessage AddLineMessage = *(PPluginMessage)lParam;
	UnicodeString MessageDate = (double)AddLineMessage.Date;
	UnicodeString MessageJID = (wchar_t*)AddLineMessage.JID;
	if(MessageJID.Pos("/")) MessageJID.Delete(MessageJID.Pos("/"),MessageJID.Length());
	UnicodeString Body = (wchar_t*)AddLineMessage.Body;
	Body = Body.Trim();
	//Zabezpieczenie przed bledem bota - dublowanie wiadomosci
	if(Body!=LastAddLineBody->ReadString("Body",ContactJID,""))
	{
	  //Zapamietanie wiadomosci
	  LastAddLineBody->WriteString("Body",ContactJID,Body);
	  //Non-breaking space
  	  if(Body.Pos("&#12288;")) Body = StringReplace(Body, "&#12288;", " ", TReplaceFlags() << rfReplaceAll);
	  //Formatowanie tagow
	  while(Body.Pos("#"))
	  {
	    //Zmienne tymczasowe
	    UnicodeString TempStr = Body;
	    UnicodeString TempStr2 = Body;
	    //Sprawdzanie poprzedniego znaku/frazy
	    TempStr.Delete(TempStr.Pos("#"),TempStr.Length());
	    while(TempStr.Pos(" ")) TempStr.Delete(1,TempStr.Pos(" "));
	    //Sprawdzanie kolejnego znaku
	    TempStr2.Delete(1,TempStr2.Pos("#"));
	    TempStr2.Delete(2,TempStr2.Length());
	    //Jezeli fraza jest tagiem
	    if((AllowedTagsCharacters(TempStr2))
	     &&(TempStr.LowerCase().Pos("href=\"link:")==0)
	     &&(TempStr.LowerCase().Pos("title=\"http://")==0)
	     &&(TempStr.LowerCase().Pos("title=\"https://")==0)
	     &&(TempStr.LowerCase().Pos("title=\"www")==0))
	    {
		  //Usuwanie hasha z tagu
		  UnicodeString TagWithOutHash = Body;
		  TagWithOutHash.Delete(1,TagWithOutHash.Pos("#"));
		  TagWithOutHash.Delete(TagsCutPosition(TagWithOutHash),TagWithOutHash.Length());
		  //Usuwanie polskich znakow
		  UnicodeString TagWithOutHashW = TagWithOutHash;
		  TagWithOutHashW = TagWithOutHashW.LowerCase();
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "ê", "e", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "ó", "o", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "¹", "a", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "œ", "s", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "³", "l", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "¿", "z", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "Ÿ", "z", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "æ", "c", TReplaceFlags() << rfReplaceAll);
		  TagWithOutHashW = StringReplace(TagWithOutHashW, "ñ", "n", TReplaceFlags() << rfReplaceAll);
		  //Tworzenie tagu
		  UnicodeString TagWithHash = "#" + TagWithOutHash;
		  //Formatowanie tagu
		  if((TagWithHash.Pos("-")==0)&&(TagWithHash.Pos("_")==0))
		  {
		    TagWithOutHashW = "[CC_TAGS_LINK]" + TagWithOutHashW + "[CC_TAGS_LINK2][CC_TAGS]" + TagWithOutHash + "[CC_TAGS_LINK3]";
		    Body = StringReplace(Body, TagWithHash, TagWithOutHashW, TReplaceFlags());
		  }
		  else
		  {
		    TagWithOutHashW = StringReplace(TagWithOutHashW, "-", "", TReplaceFlags() << rfReplaceAll);
		    TagWithOutHashW = StringReplace(TagWithOutHashW, "_", "", TReplaceFlags() << rfReplaceAll);
		    TagWithOutHashW = "[CC_TAGS_LINK]" + TagWithOutHashW + "[CC_TAGS_LINK2][CC_TAGS]" + TagWithOutHash + "[CC_TAGS_LINK3]";
		    Body = StringReplace(Body, TagWithHash, TagWithOutHashW, TReplaceFlags());
		  }
	    }
	    else Body = StringReplace(Body, "#", "[CC_TAGS]", TReplaceFlags());
	  }
	  Body = StringReplace(Body, "[CC_TAGS_LINK]", "<A HREF=\"link:https://twitter.com/search?q=%23", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_TAGS_LINK2]", "&src=hash\">", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_TAGS_LINK3]", "</A>", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_TAGS]", "#", TReplaceFlags() << rfReplaceAll);

	  //Formatowanie u¿ytkownikow
	  while(Body.Pos("@"))
	  {
	    //Zmienne tymczasowe
	    UnicodeString TempStr = Body;
	    UnicodeString TempStr2 = Body;
	    //Sprawdzanie poprzedniego znaku/frazy
	    TempStr.Delete(TempStr.Pos("@"),TempStr.Length());
	    while(TempStr.Pos(" ")) TempStr.Delete(1,TempStr.Pos(" "));
	    //Sprawdzanie kolejnego znaku
	    TempStr2.Delete(1,TempStr2.Pos("@"));
	    TempStr2.Delete(2,TempStr2.Length());
	    //Jezeli fraza jest uzytkownikiem
	    if((AllowedUsersCharacters(TempStr2))
	     &&(TempStr.LowerCase().Pos("href=\"link:")==0)
	     &&(TempStr.LowerCase().Pos("title=\"http://")==0)
	     &&(TempStr.LowerCase().Pos("title=\"https://")==0)
	     &&(TempStr.LowerCase().Pos("title=\"www")==0))
	    {
		  //Usuwanie malpy z frazy
		  UnicodeString UserWithOutCaret = Body;
		  UserWithOutCaret.Delete(1,UserWithOutCaret.Pos("@"));
		  UserWithOutCaret.Delete(UsersCutPosition(UserWithOutCaret),UserWithOutCaret.Length());
		  //Fraza z malpa
		  UnicodeString UserWithCaret = "@" + UserWithOutCaret;
		  //Formatowanie linku
		  UserWithOutCaret = "[CC_USERS_LINK]" + UserWithOutCaret + "[CC_USERS_LINK2][CC_USERS]" + UserWithOutCaret + "[CC_USERS_LINK3]";
		  Body = StringReplace(Body, UserWithCaret, UserWithOutCaret, TReplaceFlags());
	    }
	    else Body = StringReplace(Body, "@", "[CC_USERS]", TReplaceFlags());
	  }
	  Body = StringReplace(Body, "[CC_USERS_LINK]", "<A HREF=\"link:https://twitter.com/", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_USERS_LINK2]", "\">", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_USERS_LINK3]", "</A>", TReplaceFlags() << rfReplaceAll);
	  Body = StringReplace(Body, "[CC_USERS]", "@", TReplaceFlags() << rfReplaceAll);

	  //Tworzenie odnosnika dla nadawcy wiadomosci
	  if(ContactJID==MessageJID)
	  {
	    //Wyciagnie nadawcy wiadomosci
	    UnicodeString TempStr = Body;
	    TempStr.Delete(TempStr.Pos("):")+1,TempStr.Length());
	    //Wyciagniecie loginu
	    UnicodeString UserLogin = TempStr;
	    UserLogin.Delete(1,UserLogin.Pos("("));
	    UserLogin.Delete(UserLogin.Pos(")"),UserLogin.Length());
	    TweetSender = UserLogin;
	    //Wyciagniecie wyswietlanej nazwy
	    UnicodeString DisplayName = TempStr;
	    DisplayName.Delete(DisplayName.Pos("("),DisplayName.Length());
	    DisplayName = DisplayName.Trim();
	    //Tworzenie odnosnika
	    Body = StringReplace(Body, TempStr + ":", "<B><A HREF=\"link:https://twitter.com/" + UserLogin + "\" title=\"@" + UserLogin + "\">" + DisplayName + "</A></B>:", TReplaceFlags());
	  }

	  //Dodawanie awatarow
	  if((ContactJID==MessageJID)&&(!TweetSender.IsEmpty()))
	  {
	    //Tworzenie katalogu z awatarami
	    if(!DirectoryExists(AvatarsDir))
	     CreateDir(AvatarsDir);
        //Zmienna awataru
	    UnicodeString Avatars;
	    //Awatara nie ma w folderze cache
	    if(!FileExists(AvatarsDir + "\\\\" + TweetSender))
	    {
		  //Wstawienie online'owego awatara
		  Avatars = StringReplace(AvatarStyle, "CC_AVATAR", "<a href=\"link:https://twitter.com/" + TweetSender + "\" title=\"@" + TweetSender + "\"><img class=\"twitter-avatar\" border=\"0px\" src=\"https://twitter.com/api/users/profile_image/" + TweetSender + "\" width=\"" + IntToStr(AvatarSize) + "px\" height=\"" + IntToStr(AvatarSize) + "px\"></a>", TReplaceFlags() << rfReplaceAll);
		  //Dodanie awatara do pobrania
		  GetAvatarsList->Add(TweetSender);
		  //Wlaczenie watku
		  if(!hTweetForm->GetAvatarsThread->Active) hTweetForm->GetAvatarsThread->Start();
	    }
	    //Awatar znajduje sie w folderze cache
	    else Avatars = StringReplace(AvatarStyle, "CC_AVATAR", "<a href=\"link:https://twitter.com/" + TweetSender + "\" title=\"@" + TweetSender + "\"><img class=\"twitter-avatar\" border=\"0px\" src=\"file:///" + AvatarsDirW + "/" + TweetSender + "\" width=\"" + IntToStr(AvatarSize) + "px\" height=\"" + IntToStr(AvatarSize) + "px\"></a>", TReplaceFlags() << rfReplaceAll);
	    //Dodanie awatar(a/ow) do tresci wiadomosci
	    Body = Avatars + Body;
	  }

	  //Wyroznianie wiadomosci
	  if(ContactJID==MessageJID)
	  {
	    //Jezeli sa jakies elemnty do wyrozniania
	    if(HighlightMsgItemsList->Count)
	    {
		  //Petla wyrozniania wszystkich dodanych fraz
		  for(int Count=0;Count<HighlightMsgItemsList->Count;Count++)
		  {
		    //Pobieranie danych odnosnie wyroznienia
		    UnicodeString Item = HighlightMsgItemsList->Strings[Count];
		    UnicodeString Color = HighlightMsgColorsList->ReadString("Color",Item,"");
		    //Zmiana koloru tekstu (tryb I)
		    if(HighlightMsgModeChk==0)
		    {
			  //Wyrozanie tagow
			  if((Item.Pos("#")==1)&&(Body.LowerCase().Pos(Item.LowerCase())))
			  {
			    UnicodeString ItemBody = Body;
			    ItemBody.Delete(1,ItemBody.LowerCase().Pos(Item.LowerCase())+Item.Length()-1);
			    ItemBody.SetLength(1);
			    if((ItemBody.IsEmpty())||(ItemBody==" ")||(ItemBody=="<"))
			    {
				  Body = "<div style=\"color: CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanianie uzytkownikow
			  else if(Item.Pos("@")==1)
			  {
			    UnicodeString ItemWithOutCaret = StringReplace(Item, "@", "", TReplaceFlags());
			    if((Body.LowerCase().Pos(Item.LowerCase()))
			     ||(ItemWithOutCaret.LowerCase()==TweetSender.LowerCase()))
			    {
				  Body = "<div style=\"color: CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanie dowolnych fraz
			  else if(Body.LowerCase().Pos(Item.LowerCase()))
			  {
			    Body = "<div style=\"color: CC_COLOR;\">" + Body + "</div>";
			    Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }
		    }
		    //Zmiana koloru tekstu (tryb II)
		    else if(HighlightMsgModeChk==1)
		    {
			  //Wyrozanie tagow
			  if((Item.Pos("#")==1)&&(Body.LowerCase().Pos(Item.LowerCase())))
			  {
			    UnicodeString ItemBody = Body;
			    ItemBody.Delete(1,ItemBody.LowerCase().Pos(Item.LowerCase())+Item.Length()-1);
			    ItemBody.SetLength(1);
			    if((ItemBody.IsEmpty())||(ItemBody==" ")||(ItemBody=="<"))
			    {
				  UnicodeString SelectorID = "MSG" + MessageDate;
				  SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
				  Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style><div id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanianie uzytkownikow
			  else if(Item.Pos("@")==1)
			  {
			    UnicodeString ItemWithOutCaret = StringReplace(Item, "@", "", TReplaceFlags());
			    if((Body.LowerCase().Pos(Item.LowerCase()))
			     ||(ItemWithOutCaret.LowerCase()==TweetSender.LowerCase()))
			    {
				  UnicodeString SelectorID = "MSG" + MessageDate;
				  SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
				  Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style><div id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanie dowolnych fraz
			  else if(Body.LowerCase().Pos(Item.LowerCase()))
			  {
			    UnicodeString SelectorID = "MSG" + MessageDate;
			    SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
			    Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style><div id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">" + Body + "</div>";
			    Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }
		    }
		    //Zmiana koloru frazy
		    else if(HighlightMsgModeChk==2)
		    {
			  //Wyrozanie tagow
			  if((Item.Pos("#")==1)&&(Body.LowerCase().Pos(Item.LowerCase())))
			  {
			    UnicodeString ItemBody = Body;
			    ItemBody.Delete(1,ItemBody.LowerCase().Pos(Item.LowerCase())+Item.Length()-1);
			    ItemBody.SetLength(1);
			    if((ItemBody.IsEmpty())||(ItemBody==" ")||(ItemBody=="<"))
			    {
				  UnicodeString SelectorID = "MSG" + MessageDate;
				  SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
				  Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style>" + Body;
				  int Pos = Body.LowerCase().Pos(Item.LowerCase());
				  Body = Body.Insert("</span>",Pos+Item.Length());
				  Body = Body.Insert("<span id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">",Pos);
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanie nadawcow wiadomosci
			  UnicodeString ItemWithOutCaret = StringReplace(Item, "@", "", TReplaceFlags());
			  if(ItemWithOutCaret.LowerCase()==TweetSender.LowerCase())
			  {
			    UnicodeString SelectorID = "MSG" + MessageDate;
			    SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
			    Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style>" + Body;
			    int Pos = Body.LowerCase().Pos((ItemWithOutCaret.LowerCase()+"</a>"));
			    Body = Body.Insert("</span>",Pos+ItemWithOutCaret.Length());
			    Body = Body.Insert("<span id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">",Pos);
			    Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }
			  //Wyroznianie zacytowanych uzytkownikow
			  if(Body.LowerCase().Pos(Item.LowerCase()+"</a>"))
			  {
			    UnicodeString SelectorID = "MSG" + MessageDate;
			    SelectorID = StringReplace(SelectorID, ",", "", TReplaceFlags() << rfReplaceAll);
			    Body = "<style>#" + SelectorID + " a { color: CC_COLOR; }</style>" + Body;
			    int Pos = Body.LowerCase().Pos(Item.LowerCase()+"</a>");
			    Body = Body.Insert("</span>",Pos+Item.Length());
			    Body = Body.Insert("<span id=\"" + SelectorID + "\" style=\"color: CC_COLOR;\">",Pos);
			    Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }
		    }
		    //Zmiana koloru pola wiadomosci
		    else if(HighlightMsgModeChk==3)
		    {
			  //Wyrozanie tagow
			  if((Item.Pos("#")==1)&&(Body.LowerCase().Pos(Item.LowerCase())))
			  {
			    UnicodeString ItemBody = Body;
			    ItemBody.Delete(1,ItemBody.LowerCase().Pos(Item.LowerCase())+Item.Length()-1);
			    ItemBody.SetLength(1);
			    if((ItemBody.IsEmpty())||(ItemBody==" ")||(ItemBody=="<"))
			    {
				  Body = "<div style=\"padding-bottom: 4px; margin: -4px; border: 4px solid CC_COLOR; border-bottom: 0px; background: none repeat scroll 0 0 CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			    }
			  }
			  //Wyrozanianie uzytkownikow
			  else if(Item.Pos("@")==1)
			  {
			    UnicodeString ItemWithOutCaret = StringReplace(Item, "@", "", TReplaceFlags());
			    if((Body.LowerCase().Pos(Item.LowerCase()))
			     ||(ItemWithOutCaret.LowerCase()==TweetSender.LowerCase()))
			    {
				  Body = "<div style=\"padding-bottom: 4px; margin: -4px; border: 4px solid CC_COLOR; border-bottom: 0px; background: none repeat scroll 0 0 CC_COLOR;\">" + Body + "</div>";
				  Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }  
			  }
			  //Wyrozanie dowolnych fraz
			  else if(Body.LowerCase().Pos(Item.LowerCase()))
			  {
			    Body = "<div style=\"padding-bottom: 4px; margin: -4px; border: 4px solid CC_COLOR; border-bottom: 0px; background: none repeat scroll 0 0 CC_COLOR;\">" + Body + "</div>";
			    Body = StringReplace(Body, "CC_COLOR", Color, TReplaceFlags() << rfReplaceAll);
			  }
		    }
		  }
	    }
	  }

	  //Zwrocenie zmodyfikowanej wiadomosci;
	  AddLineMessage.Body = Body.w_str();
	  memcpy((PPluginMessage)lParam,&AddLineMessage, sizeof(TPluginMessage));
	  return 2;
	}
	//Zablokowanie zdublowanej wiadomosci
	else return 1;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kolorystyki AlphaControls
int __stdcall OnColorChange(WPARAM wParam, LPARAM lParam)
{
  //Okno ustawien zostalo juz stworzone
  if(hTweetForm)
  {
	//Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
	  hTweetForm->sSkinManager->HueOffset = wParam;
	  hTweetForm->sSkinManager->Saturation = lParam;
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zaladowanie wszystkich modulow w AQQ (autoupdate awatarow)
int __stdcall OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
  //Automatyczna aktualizacja awatarow
  AutoAvatarsUpdate();

  return 0;
}
//---------------------------------------------------------------------------

//Hook na pobieranie adresow URL z roznych popup (tworzenie itemow w popup menu do akcji z tweetami)
int __stdcall OnPerformCopyData(WPARAM wParam, LPARAM lParam)
{
  //Domyslne usuwanie elementow
  TPluginAction InsertTagItem;
  ZeroMemory(&InsertTagItem,sizeof(TPluginAction));
  InsertTagItem.cbSize = sizeof(TPluginAction);
  InsertTagItem.pszName = L"tweetIMInsertTagItem";
  InsertTagItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertTagItem));
  TPluginAction InsertNickItem;
  ZeroMemory(&InsertNickItem,sizeof(TPluginAction));
  InsertNickItem.cbSize = sizeof(TPluginAction);
  InsertNickItem.pszName = L"tweetIMInsertNickItem";
  InsertNickItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertNickItem));
  TPluginAction SendPrivMsgItem;
  ZeroMemory(&SendPrivMsgItem,sizeof(TPluginAction));
  SendPrivMsgItem.cbSize = sizeof(TPluginAction);
  SendPrivMsgItem.pszName = L"tweetIMSendPrivMsgItem";
  SendPrivMsgItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SendPrivMsgItem));
  TPluginAction LikeLastTweetItem;
  ZeroMemory(&LikeLastTweetItem,sizeof(TPluginAction));
  LikeLastTweetItem.cbSize = sizeof(TPluginAction);
  LikeLastTweetItem.pszName = L"tweetIMLikeLastTweetItem";
  LikeLastTweetItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&LikeLastTweetItem));
  TPluginAction ShowTimelineItem;
  ZeroMemory(&ShowTimelineItem,sizeof(TPluginAction));
  ShowTimelineItem.cbSize = sizeof(TPluginAction);
  ShowTimelineItem.pszName = L"tweetIMShowTimelineItem";
  ShowTimelineItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowTimelineItem));
  TPluginAction ShowUserProfileItem;
  ZeroMemory(&ShowUserProfileItem,sizeof(TPluginAction));
  ShowUserProfileItem.cbSize = sizeof(TPluginAction);
  ShowUserProfileItem.pszName = L"tweetIMShowUserProfileItem";
  ShowUserProfileItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowUserProfileItem));
  TPluginAction SeparatorItem;
  ZeroMemory(&SeparatorItem,sizeof(TPluginAction));
  SeparatorItem.cbSize = sizeof(TPluginAction);
  SeparatorItem.pszName = L"tweetIMSeparatorItem";
  SeparatorItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SeparatorItem));
  //Kasowanie zapamietanych danych
  ItemCopyData = "";
  //Jezeli zezwolono na sprawdzanie danych
  if(!BlockPerformCopyData)
  {
	//Pobranie danych
	UnicodeString CopyData = (wchar_t*)lParam;
	//Tagi
	if(CopyData.Pos("https://twitter.com/search")==1)
	{
	  if(CopyData.Pos("https://twitter.com/search?q=%23")==1)
	  {
		//Wyciaganie tag'u
		CopyData.Delete(1,CopyData.Pos("search?q=%23")+11);
		CopyData.Delete(CopyData.Pos("&src=hash"),CopyData.Length());
		if(!CopyData.IsEmpty())
		{
		  //Kopiowanie tag'u
		  ItemCopyData = "#" + CopyData;
		  //Tworzenie separatora
		  SeparatorItem.cbSize = sizeof(TPluginAction);
		  SeparatorItem.pszName = L"tweetIMSeparatorItem";
		  SeparatorItem.pszCaption = L"-";
		  SeparatorItem.Position = 0;
		  SeparatorItem.IconIndex = 0;
		  SeparatorItem.pszPopupName = L"popURL";
		  SeparatorItem.Handle = (int)hFrmSend;
		  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SeparatorItem));
		  //Tworzenie elementu wstawiania tagu
		  InsertTagItem.cbSize = sizeof(TPluginAction);
		  InsertTagItem.pszName = L"tweetIMInsertTagItem";
		  InsertTagItem.pszCaption = ("Wstaw " + ItemCopyData).w_str();
		  InsertTagItem.Position = 0;
		  InsertTagItem.IconIndex = 11;
		  InsertTagItem.pszService = L"stweetIMInsertTagItem";
		  InsertTagItem.pszPopupName = L"popURL";
		  InsertTagItem.Handle = (int)hFrmSend;
		  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&InsertTagItem));
		}
	  }
	}
	//Uzytkownicy
	else if(CopyData.Pos("https://twitter.com/")==1)
	{
      //Wyciaganie nick'a
	  CopyData.Delete(1,20);
	  if(!CopyData.IsEmpty())
	  {
		//Kopiowanie nick'a
		ItemCopyData = CopyData;
		//Tworzenie separatora
		SeparatorItem.cbSize = sizeof(TPluginAction);
		SeparatorItem.pszName = L"tweetIMSeparatorItem";
		SeparatorItem.pszCaption = L"-";
		SeparatorItem.Position = 0;
		SeparatorItem.IconIndex = 0;
		SeparatorItem.pszPopupName = L"popURL";
		SeparatorItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SeparatorItem));
		//Tworzenie elementu do pokazywania informacji o uzytkowniku
		TPluginAction ShowUserProfileItem;
		ZeroMemory(&ShowUserProfileItem,sizeof(TPluginAction));
		ShowUserProfileItem.cbSize = sizeof(TPluginAction);
		ShowUserProfileItem.pszName = L"tweetIMShowUserProfileItem";
		ShowUserProfileItem.pszCaption = L"Poka¿ informacje o u¿ytkowniku";
		ShowUserProfileItem.Position = 0;
		ShowUserProfileItem.IconIndex = 21;
		ShowUserProfileItem.pszService = L"stweetIMShowUserProfileItem";
		ShowUserProfileItem.pszPopupName = L"popURL";
		ShowUserProfileItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&ShowUserProfileItem));
		//Tworzenie elementu do pokazywania najnowszych tweetniec uzytkownika
		TPluginAction ShowTimelineItem;
		ZeroMemory(&ShowTimelineItem,sizeof(TPluginAction));
		ShowTimelineItem.cbSize = sizeof(TPluginAction);
		ShowTimelineItem.pszName = L"tweetIMShowTimelineItem";
		ShowTimelineItem.pszCaption = L"Poka¿ najnowsze tweetniêcia";
		ShowTimelineItem.Position = 0;
		ShowTimelineItem.IconIndex = 21;
		ShowTimelineItem.pszService = L"stweetIMShowTimelineItem";
		ShowTimelineItem.pszPopupName = L"popURL";
		ShowTimelineItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&ShowTimelineItem));
		//Tworzenie elementu lajkowania ostatniego tweetniecia
		TPluginAction LikeLastTweetItem;
		ZeroMemory(&LikeLastTweetItem,sizeof(TPluginAction));
		LikeLastTweetItem.cbSize = sizeof(TPluginAction);
		LikeLastTweetItem.pszName = L"tweetIMLikeLastTweetItem";
		LikeLastTweetItem.pszCaption = L"Polub ostatnie tweetniêcie";
		LikeLastTweetItem.Position = 0;
		LikeLastTweetItem.IconIndex = 157;
		LikeLastTweetItem.pszService = L"stweetIMLikeLastTweetItem";
		LikeLastTweetItem.pszPopupName = L"popURL";
		LikeLastTweetItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&LikeLastTweetItem));
		//Tworzenie elementu wysylania prywatnej wiadomosci
		TPluginAction SendPrivMsgItem;
  		ZeroMemory(&SendPrivMsgItem,sizeof(TPluginAction));
		SendPrivMsgItem.cbSize = sizeof(TPluginAction);
		SendPrivMsgItem.pszName = L"tweetIMSendPrivMsgItem";
		SendPrivMsgItem.pszCaption = L"Wiadomoœæ prywatna";
		SendPrivMsgItem.Position = 0;
		SendPrivMsgItem.IconIndex = 8;
		SendPrivMsgItem.pszService = L"stweetIMSendPrivMsgItem";
		SendPrivMsgItem.pszPopupName = L"popURL";
		SendPrivMsgItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SendPrivMsgItem));
		//Tworzenie elementu wstawiania nicku
		TPluginAction InsertNickItem;
  		ZeroMemory(&InsertNickItem,sizeof(TPluginAction));
		InsertNickItem.cbSize = sizeof(TPluginAction);
		InsertNickItem.pszName = L"tweetIMInsertNickItem";
		InsertNickItem.pszCaption = ("Wstaw @" + ItemCopyData).w_str();
		InsertNickItem.Position = 0;
		InsertNickItem.IconIndex = 11;
		InsertNickItem.pszService = L"stweetIMInsertNickItem";
		InsertNickItem.pszPopupName = L"popURL";
		InsertNickItem.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&InsertNickItem));
	  }
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

int __stdcall OnPrimaryTab (WPARAM wParam, LPARAM lParam)
{
  //Pobranie uchwytu do okna rozmowy
  if(!hFrmSend) hFrmSend = (HWND)wParam;
  //Szukanie pola wiadomosci
  if(!hRichEdit) EnumChildWindows(hFrmSend,(WNDENUMPROC)FindRichEdit,0);
  //Pobieranie danych kontatku
  TPluginContact PrimaryTabContact = *(PPluginContact)lParam;
  ActiveTabJID = (wchar_t*)PrimaryTabContact.JID;
  ActiveTabRes = (wchar_t*)PrimaryTabContact.Resource;
  ActiveTabUsrIdx = PrimaryTabContact.UserIdx;
  //Kontakt jest botem tweet.IM
  if(ActiveTabJID.Pos("@twitter.tweet.im"))
  {
	//"Wlaczenie" procki zbierajacej adresu URL
	BlockPerformCopyData = false;
	//Tworzenie buttona z komendami bota
	BuildCommandItems();
  }
  else
  {
	//"Wylaczenie" procki zbierajacej adresu URL
	BlockPerformCopyData = true;
	//Usuwanie buttona z komendami bota
	DestroyCommandItems();
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na odbieranie wiadomosci
int __stdcall OnRecvMsg(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie danych kontatku
  TPluginContact RecvMsgContact = *(PPluginContact)wParam;
  //Pobieranie identyfikatora kontatku
  UnicodeString ContactJID = (wchar_t*)RecvMsgContact.JID;
  //Kontakt jest botem tweet.IM
  if(ContactJID.Pos("@twitter.tweet.im"))
  {
	//Pobieranie danych wiadomosci
	TPluginMessage RecvMsgMessage = *(PPluginMessage)lParam;
	//Pobranie tresci wiadomosci
	UnicodeString Body = (wchar_t*)RecvMsgMessage.Body;
	//Zabezpieczenie przed bledem bota - dublowanie wiadomosci
	if(Body!=LastRecvMsgBody->ReadString("Body",ContactJID,""))
	 //Zapamietanie wiadomosci
	 LastRecvMsgBody->WriteString("Body",ContactJID,Body);
	//Blokada wiadomosci
	else return 1;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kompozycji (pobranie stylu zalacznikow oraz zmiana skorkowania wtyczki)
int __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
  //Okno ustawien zostalo juz stworzone
  if(hTweetForm)
  {
	//Wlaczona zaawansowana stylizacja okien
	if(ChkSkinEnabled())
	{
	  //Pobieranie sciezki nowej aktywnej kompozycji
	  UnicodeString ThemeSkinDir = StringReplace((wchar_t*)lParam, "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
	  //Plik zaawansowanej stylizacji okien istnieje
	  if(FileExists(ThemeSkinDir + "\\\\Skin.asz"))
	  {
		//Dane pliku zaawansowanej stylizacji okien
		ThemeSkinDir = StringReplace(ThemeSkinDir, "\\\\", "\\", TReplaceFlags() << rfReplaceAll);
		hTweetForm->sSkinManager->SkinDirectory = ThemeSkinDir;
		hTweetForm->sSkinManager->SkinName = "Skin.asz";
		//Ustawianie animacji AlphaControls
		if(ChkThemeAnimateWindows()) hTweetForm->sSkinManager->AnimEffects->FormShow->Time = 200;
		else hTweetForm->sSkinManager->AnimEffects->FormShow->Time = 0;
		hTweetForm->sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
		//Zmiana kolorystyki AlphaControls
		hTweetForm->sSkinManager->HueOffset = GetHUE();
	    hTweetForm->sSkinManager->Saturation = GetSaturation();
		//Aktywacja skorkowania AlphaControls
		hTweetForm->sSkinManager->Active = true;
	  }
	  //Brak pliku zaawansowanej stylizacji okien
	  else hTweetForm->sSkinManager->Active = false;
	}
	//Zaawansowana stylizacja okien wylaczona
	else hTweetForm->sSkinManager->Active = false;
	//Kolor labelow
	if(hTweetForm->sSkinManager->Active)
	{
	  hTweetForm->UsedAvatarsStyleLabel->Kind->Color = GetWarningColor();
	  hTweetForm->LastAvatarsUpdateLabel->Kind->Color = hTweetForm->UsedAvatarsStyleLabel->Kind->Color;
	}
	else
	{
	  hTweetForm->UsedAvatarsStyleLabel->Kind->Color = clGreen;
	  hTweetForm->LastAvatarsUpdateLabel->Kind->Color = clGreen;
	}
  }
  //Pobranie stylu Attachment & Avatars
  GetThemeStyle();

  return 0;
}
//---------------------------------------------------------------------------

//Zapisywanie zasobów
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
  TPluginTwoFlagParams PluginTwoFlagParams;
  PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
  PluginTwoFlagParams.Param1 = ResName;
  PluginTwoFlagParams.Param2 = ResType;
  PluginTwoFlagParams.Flag1 = (int)HInstance;
  PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
  if(FileExists(FileName))
  {
	UnicodeString Result;
    TFileStream *fs;

	fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
	try
	{
	  TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
	  try
	  {
	    Result = idmd5->HashStreamAsHex(fs);
	  }
	  __finally
	  {
	    delete idmd5;
	  }
    }
	__finally
    {
	  delete fs;
    }

    return Result;
  }
  else
   return 0;
}
//---------------------------------------------------------------------------

//Odczyt ustawien
void LoadSettings()
{
  TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\tweetIM\\\\Settings.ini");
  //Awatary
  AvatarSize = Ini->ReadInteger("Avatars","Size",25);
  StaticAvatarStyle = UTF8ToUnicodeString(IniStrToStr(Ini->ReadString("Avatars","Style","").Trim().w_str()));
  if(StaticAvatarStyle=="<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
   StaticAvatarStyle = "";
  //Wyroznianie
  HighlightMsgChk = Ini->ReadBool("HighlightMsg","Enabled",false);
  HighlightMsgModeChk= Ini->ReadInteger("HighlightMsg","Mode",0);
  HighlightMsgItemsList->Clear();
  HighlightMsgColorsList->EraseSection("Color");
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
		HighlightMsgItemsList->Add(Item);
		HighlightMsgColorsList->WriteString("Color",Item,Color);
	  }
	}
  }
  delete HighlightItems;
  delete Ini;
}
//---------------------------------------------------------------------------

extern "C" int __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
  //Linkowanie wtyczki z komunikatorem
  PluginLink = *Link;
  //Przypisanie uchwytu do formy ustawien
  if(!hTweetForm)
  {
	Application->Handle = (HWND)TweetForm;
	hTweetForm = new TTweetForm(Application);
  }
  //Sciezka folderu prywatnego wtyczek
  UnicodeString PluginUserDir = GetPluginUserDir();
  //Wypakiwanie ikonki tweetIM.dll.png
  //984BE6E674B8DD8C48B4C20EC0913586
  if(!DirectoryExists(PluginUserDir+"\\\\Shared"))
   CreateDir(PluginUserDir+"\\\\Shared");
  if(!FileExists(PluginUserDir+"\\\\Shared\\\\tweetIM.dll.png"))
   ExtractRes((PluginUserDir+"\\\\Shared\\\\tweetIM.dll.png").w_str(),L"SHARED",L"DATA");
  else if(MD5File(PluginUserDir+"\\\\Shared\\\\tweetIM.dll.png")!="984BE6E674B8DD8C48B4C20EC0913586")
   ExtractRes((PluginUserDir+"\\\\Shared\\\\tweetIM.dll.png").w_str(),L"SHARED",L"DATA");
  //Tworzeniu katalogu z ustawieniami wtyczki
  if(!DirectoryExists(PluginUserDir+"\\\\tweetIM"))
   CreateDir(PluginUserDir+"\\\\tweetIM");
  //Sciezka katalogu z awatarami
  AvatarsDir = PluginUserDir+"\\\\tweetIM\\\\Avatars";
  AvatarsDirW = StringReplace(AvatarsDir, "\\\\", "/", TReplaceFlags() << rfReplaceAll);
  //Tworzenie katalogu z awatarami
  if(!DirectoryExists(AvatarsDir))
   CreateDir(AvatarsDir);
  //Tworzenie PopUpMenu
  TPluginAction CommandPopUp;
  ZeroMemory(&CommandPopUp,sizeof(TPluginAction));
  CommandPopUp.cbSize = sizeof(TPluginAction);
  CommandPopUp.pszName = L"tweetIMCommandPopUp";
  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENU,0,(LPARAM)(&CommandPopUp));
  //Tworzenie serwisow
  //Szybki dostep do ustawien wtyczki
  PluginLink.CreateServiceFunction(L"stweetIMFastSettingsItem",ServicetweetIMFastSettingsItem);
  //Serwis do wstawiania tagu
  PluginLink.CreateServiceFunction(L"stweetIMInsertTagItem",ServiceInsertTagItem);
  //Serwis do wstawiania nicku
  PluginLink.CreateServiceFunction(L"stweetIMInsertNickItem",ServiceInsertNickItem);
  //Serwis do wysylania wiadomosci prywatnej
  PluginLink.CreateServiceFunction(L"stweetIMSendPrivMsgItem",ServiceSendPrivMsgItem);
  //Serwis do lajkowania ostatniego tweetniecia
  PluginLink.CreateServiceFunction(L"stweetIMLikeLastTweetItem",ServiceLikeLastTweetItem);
  //Serwis do pokazywania najnowszych tweetniec uzytkownika
  PluginLink.CreateServiceFunction(L"stweetIMShowTimelineItem",ServiceShowTimelineItem);
  //Serwis do pokazywania informacji o uzytkowniku
  PluginLink.CreateServiceFunction(L"stweetIMShowUserProfileItem",ServiceShowUserProfileItem);
  //Serwis do "Pobierz nieprzeczytane tweety"
  PluginLink.CreateServiceFunction(L"stweetIMUpdateCommandItem",ServiceUpdateCommandItem);
  //Serwis do "Obserwowani"
  PluginLink.CreateServiceFunction(L"stweetIMIngCommandItem",ServiceIngCommandItem);
  //Serwis do "Obserwuj¹cy"
  PluginLink.CreateServiceFunction(L"stweetIMErsCommandItem",ServiceErsCommandItem);
  //Seris do "Usun poprzednie tweeetniecie"
  PluginLink.CreateServiceFunction(L"stweetIMUndoTweetCommandItem",ServiceUndoTweetCommandItem);
  //Serwis do "Zapisane wyszukiwania"
  PluginLink.CreateServiceFunction(L"stweetIMSavedSearcheCommandItem",ServiceSavedSearchesCommandItem);
  //Tworzenie interfejsu szybkiego dostepu do ustawien wtyczki
  BuildtweetIMFastSettings();
  //Definiowanie User-Agent dla polaczen HTTP
  hTweetForm->IdHTTP->Request->UserAgent = "AQQ IM Plugin: tweet.IM/" + GetFileInfo(GetPluginDir().w_str(), L"FileVersion") + " (+http://beherit.pl)";
  hTweetForm->AUIdHTTP->Request->UserAgent = hTweetForm->IdHTTP->Request->UserAgent;
  //Hook na aktwyna zakladke lub okno rozmowy (pokazywanie menu do cytowania, tworzenie buttonow)
  PluginLink.HookEvent(AQQ_CONTACTS_BUDDY_ACTIVETAB,OnActiveTab);
  //Hook na pokazywane wiadomosci (formatowanie tweetow)
  PluginLink.HookEvent(AQQ_CONTACTS_ADDLINE,OnAddLine);
  //Hook na zmiane kolorystyki AlphaControls
  PluginLink.HookEvent(AQQ_SYSTEM_COLORCHANGE,OnColorChange);
  //Hook na zaladowanie wszystkich modulow w AQQ (autoupdate awatarow)
  PluginLink.HookEvent(AQQ_SYSTEM_MODULESLOADED,OnModulesLoaded);
  //Hook na pobieranie adresow URL z roznych popup (tworzenie itemow w popup menu do akcji z tweetami)
  PluginLink.HookEvent(AQQ_SYSTEM_PERFORM_COPYDATA,OnPerformCopyData);
  //Hook na odbieranie nowej wiadomosci
  PluginLink.HookEvent(AQQ_CONTACTS_RECVMSG,OnRecvMsg);
  //Hook na zmiane kompozycji (pobranie stylu zalacznikow oraz zmiana skorkowania wtyczki)
  PluginLink.HookEvent(AQQ_SYSTEM_THEMECHANGED,OnThemeChanged);
  //Odczyt ustawien
  LoadSettings();
  //Pobranie stylu Attachment & Avatars
  GetThemeStyle();
  //Jezeli wszystkie moduly w AQQ zostaly juz zaladowany przed wczytaniem wtyczki
  if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
  {
	//Wlaczenie autoupdate awatarow
	AutoAvatarsUpdate();
	//Hook na pobieranie aktywnych zakladek
	PluginLink.HookEvent(AQQ_CONTACTS_BUDDY_PRIMARYTAB,OnPrimaryTab);
	PluginLink.CallService(AQQ_CONTACTS_BUDDY_FETCHALLTABS,0,0);
	PluginLink.UnhookEvent(OnPrimaryTab);
  }

  return 0;
}
//---------------------------------------------------------------------------

extern "C" int __declspec(dllexport) __stdcall Unload()
{
  //Anty "Abnormal program termination"
  hTweetForm->aForceDisconnect->Execute();
  //Wyladowanie hookow
  PluginLink.UnhookEvent(OnActiveTab);
  PluginLink.UnhookEvent(OnAddLine);
  PluginLink.UnhookEvent(OnColorChange);
  PluginLink.UnhookEvent(OnModulesLoaded);
  PluginLink.UnhookEvent(OnPerformCopyData);
  PluginLink.UnhookEvent(OnRecvMsg);
  PluginLink.UnhookEvent(OnThemeChanged);
  //Usuwanie elementow z interfejsu AQQ
  TPluginAction FastSettingsItem;
  ZeroMemory(&FastSettingsItem,sizeof(TPluginAction));
  FastSettingsItem.cbSize = sizeof(TPluginAction);
  FastSettingsItem.pszName = L"tweetIMFastSettingsItemButton";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&FastSettingsItem));
  TPluginAction InsertTagItem;
  ZeroMemory(&InsertTagItem,sizeof(TPluginAction));
  InsertTagItem.cbSize = sizeof(TPluginAction);
  InsertTagItem.pszName = L"tweetIMInsertTagItem";
  InsertTagItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertTagItem));
  TPluginAction InsertNickItem;
  ZeroMemory(&InsertNickItem,sizeof(TPluginAction));
  InsertNickItem.cbSize = sizeof(TPluginAction);
  InsertNickItem.pszName = L"tweetIMInsertNickItem";
  InsertNickItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertNickItem));
  TPluginAction SendPrivMsgItem;
  ZeroMemory(&SendPrivMsgItem,sizeof(TPluginAction));
  SendPrivMsgItem.cbSize = sizeof(TPluginAction);
  SendPrivMsgItem.pszName = L"tweetIMSendPrivMsgItem";
  SendPrivMsgItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SendPrivMsgItem));
  TPluginAction LikeLastTweetItem;
  ZeroMemory(&LikeLastTweetItem,sizeof(TPluginAction));
  LikeLastTweetItem.cbSize = sizeof(TPluginAction);
  LikeLastTweetItem.pszName = L"tweetIMLikeLastTweetItem";
  LikeLastTweetItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&LikeLastTweetItem));
  TPluginAction ShowTimelineItem;
  ZeroMemory(&ShowTimelineItem,sizeof(TPluginAction));
  ShowTimelineItem.cbSize = sizeof(TPluginAction);
  ShowTimelineItem.pszName = L"tweetIMShowTimelineItem";
  ShowTimelineItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowTimelineItem));
  TPluginAction ShowUserProfileItem;
  ZeroMemory(&ShowUserProfileItem,sizeof(TPluginAction));
  ShowUserProfileItem.cbSize = sizeof(TPluginAction);
  ShowUserProfileItem.pszName = L"tweetIMShowUserProfileItem";
  ShowUserProfileItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowUserProfileItem));
  TPluginAction SeparatorItem;
  ZeroMemory(&SeparatorItem,sizeof(TPluginAction));
  SeparatorItem.cbSize = sizeof(TPluginAction);
  SeparatorItem.pszName = L"tweetIMSeparatorItem";
  SeparatorItem.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SeparatorItem));
  TPluginAction UpdateCommandItem;
  ZeroMemory(&UpdateCommandItem,sizeof(TPluginAction));
  UpdateCommandItem.cbSize = sizeof(TPluginAction);
  UpdateCommandItem.pszName = L"tweetIMUpdateCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&UpdateCommandItem));
  TPluginAction IngCommandItem;
  ZeroMemory(&IngCommandItem,sizeof(TPluginAction));
  IngCommandItem.cbSize = sizeof(TPluginAction);
  IngCommandItem.pszName = L"tweetIMIngCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&IngCommandItem));
  TPluginAction ErsCommandItem;
  ZeroMemory(&ErsCommandItem,sizeof(TPluginAction));
  ErsCommandItem.cbSize = sizeof(TPluginAction);
  ErsCommandItem.pszName = L"tweetIMErsCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ErsCommandItem));
  TPluginAction UndoTweetCommandItem;
  ZeroMemory(&UndoTweetCommandItem,sizeof(TPluginAction));
  UndoTweetCommandItem.cbSize = sizeof(TPluginAction);
  UndoTweetCommandItem.pszName = L"tweetIMUndoTweetCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&UndoTweetCommandItem));
  TPluginAction SavedSearchesCommandItem;
  ZeroMemory(&SavedSearchesCommandItem,sizeof(TPluginAction));
  SavedSearchesCommandItem.cbSize = sizeof(TPluginAction);
  SavedSearchesCommandItem.pszName = L"tweetIMSavedSearcheCommandItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SavedSearchesCommandItem));
  TPluginAction CommandButton;
  ZeroMemory(&CommandButton,sizeof(TPluginAction));
  CommandButton.cbSize = sizeof(TPluginAction);
  CommandButton.pszName = L"tweetIMCommandButton";
  CommandButton.Handle = (int)hFrmSend;
  PluginLink.CallService(AQQ_CONTROLS_TOOLBAR "tbMain" AQQ_CONTROLS_DESTROYBUTTON ,0,(LPARAM)(&CommandButton));
  //Usuwanie PopUpMenu
  TPluginAction CommandPopUp;
  ZeroMemory(&CommandPopUp,sizeof(TPluginAction));
  CommandPopUp.cbSize = sizeof(TPluginAction);
  CommandPopUp.pszName = L"tweetIMCommandPopUp";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENU,0,(LPARAM)(&CommandPopUp));
  //Usuwanie serwisow
  PluginLink.DestroyServiceFunction(ServicetweetIMFastSettingsItem);
  PluginLink.DestroyServiceFunction(ServiceInsertTagItem);
  PluginLink.DestroyServiceFunction(ServiceInsertNickItem);
  PluginLink.DestroyServiceFunction(ServiceSendPrivMsgItem);
  PluginLink.DestroyServiceFunction(ServiceLikeLastTweetItem);
  PluginLink.DestroyServiceFunction(ServiceShowTimelineItem);
  PluginLink.DestroyServiceFunction(ServiceShowUserProfileItem);
  PluginLink.DestroyServiceFunction(ServiceUpdateCommandItem);
  PluginLink.DestroyServiceFunction(ServiceIngCommandItem);
  PluginLink.DestroyServiceFunction(ServiceErsCommandItem);
  PluginLink.DestroyServiceFunction(ServiceUndoTweetCommandItem);
  PluginLink.DestroyServiceFunction(ServiceSavedSearchesCommandItem);

  return 0;
}
//---------------------------------------------------------------------------

//Ustawienia wtyczki
extern "C" int __declspec(dllexport)__stdcall Settings()
{
  //Przypisanie uchwytu do formy ustawien
  if(!hTweetForm)
  {
	Application->Handle = (HWND)TweetForm;
	hTweetForm = new TTweetForm(Application);
  }
  //Pokaznie okna ustawien
  hTweetForm->Show();

  return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" __declspec(dllexport) PPluginInfo __stdcall AQQPluginInfo(DWORD AQQVersion)
{
  PluginInfo.cbSize = sizeof(TPluginInfo);
  PluginInfo.ShortName = L"tweet.IM";
  PluginInfo.Version = PLUGIN_MAKE_VERSION(1,0,2,0);
  PluginInfo.Description = L"Wtyczka przeznaczona dla osób u¿ywaj¹cych Twittera. Formatuje ona wszystkie wiadomoœci dla bota pochodz¹cego z serwisu tweet.IM.";
  PluginInfo.Author = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.AuthorMail = L"kontakt@beherit.pl";
  PluginInfo.Copyright = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.Homepage = L"http://beherit.pl";

  return &PluginInfo;
}
//---------------------------------------------------------------------------
