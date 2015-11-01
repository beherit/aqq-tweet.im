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

#include <vcl.h>
#include <windows.h>
#include "TweetFrm.h"
#include <inifiles.hpp>
#include <IdHashMessageDigest.hpp>
#include <fstream>
#include <XMLDoc.hpp>
#include <PluginAPI.h>
#include <LangAPI.hpp>
#pragma hdrstop

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------

//Uchwyt-do-formy-ustawien---------------------------------------------------
TSettingsForm* hSettingsForm;
//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//---------------------------------------------------------------------------
//Informacje o aktywnej zakladce
UnicodeString ActiveTabJID;
UnicodeString ActiveTabRes;
int ActiveTabUsrIdx;
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
//Uchwyt do okna timera
HWND hTimerFrm;
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
//TIMERY---------------------------------------------------------------------
#define TIMER_UPDATE_AVATARS_ONLOAD 10
#define TIMER_UPDATE_AVATARS 20
//FORWARD-AQQ-HOOKS----------------------------------------------------------
INT_PTR __stdcall OnActiveTab(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnAddLine(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnLangCodeChanged(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnModulesLoaded(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnPerformCopyData(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnPrimaryTab(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnRecvMsg(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnXMLDebug(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServicetweetIMFastSettingsItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceInsertTagItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceInsertNickItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceSendPrivMsgItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceFavLatestTweetItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceShowTimelineItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceShowUserProfileItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceUpdateCommandItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceIngCommandItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceErsCommandItem(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall ServiceUndoTweetCommandItem(WPARAM wParam, LPARAM lParam);
//FORWARD-TIMER--------------------------------------------------------------
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Otwarcie okna ustawien
void OpenSettingsForm()
{
	//Przypisanie uchwytu do formy ustawien
	if(!hSettingsForm)
	{
		Application->Handle = (HWND)SettingsForm;
		hSettingsForm = new TSettingsForm(Application);
	}
	//Pokaznie okna ustawien
	hSettingsForm->Show();
}
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

//Sprawdzanie czy wlaczona jest zaawansowana stylizacja okien
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
int GetBrightness()
{
	return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETBRIGHTNESS,0,0);
}
//---------------------------------------------------------------------------

//Kodowanie ciagu znakow do Base64
UnicodeString EncodeBase64(UnicodeString Str)
{
	return (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_BASE64,(WPARAM)Str.w_str(),3);
}
//---------------------------------------------------------------------------

//Dekodowanie ciagu znakow z Base64
UnicodeString DecodeBase64(UnicodeString Str)
{
	return (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_BASE64,(WPARAM)Str.w_str(),2);
}
//---------------------------------------------------------------------------

//Pobranie stylu labela
TColor GetWarningColor()
{
	//Odczyt pliku
	hSettingsForm->FileMemo->Lines->LoadFromFile(GetThemeDir()+"\\\\elements.xml");
	hSettingsForm->FileMemo->Text = "<content>" + hSettingsForm->FileMemo->Text + " </content>";
	_di_IXMLDocument XMLDoc = LoadXMLData(hSettingsForm->FileMemo->Text);
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

//Pobranie stylu awatarow
void GetThemeStyle()
{
	//Reset stylu
	AvatarStyle = "";
	//URL do aktuanie uzywanej kompozycji
	UnicodeString ThemeURL = GetThemeDir();
	//URL do domyslnej kompozycji
	UnicodeString ThemeURLW = (wchar_t*)(PluginLink.CallService(AQQ_FUNCTION_GETAPPPATH,0,0));
	ThemeURLW = StringReplace(ThemeURLW, "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
	ThemeURLW = ThemeURLW + "\\\\System\\\\Shared\\\\Themes\\\\Standard";
	//Pobieranie stylu awatarow
	if(FileExists(ThemeURL + "\\\\Message\\\\TweetAvatar.htm"))
	{
		//Pobieranie danych z pliku
		hSettingsForm->FileMemo->Lines->LoadFromFile(ThemeURL + "\\\\Message\\\\TweetAvatar.htm");
		AvatarStyle = hSettingsForm->FileMemo->Text;
		AvatarStyle = AvatarStyle.Trim();
		//Sprawdzanie zawartosci pliku
		if(AvatarStyle.Pos("CC_AVATAR"))
		{
			hSettingsForm->UsedAvatarsStyleLabel->Caption = GetLangStr("FromTheme");
		}
		else if(!StaticAvatarStyle.IsEmpty())
		{
			AvatarStyle = StaticAvatarStyle;
			hSettingsForm->UsedAvatarsStyleLabel->Caption = GetLangStr("Own");
		}
		else
		{
			AvatarStyle = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
			hSettingsForm->UsedAvatarsStyleLabel->Caption = GetLangStr("Default");
		}
	}
	else if(!StaticAvatarStyle.IsEmpty())
	{
		AvatarStyle = StaticAvatarStyle;
		hSettingsForm->UsedAvatarsStyleLabel->Caption = GetLangStr("Own");
	}
	else
	{
		AvatarStyle = "<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>";
		hSettingsForm->UsedAvatarsStyleLabel->Caption = GetLangStr("Default");
	}
	hSettingsForm->EditAvatarsStyleLabel->Left = hSettingsForm->UsedAvatarsStyleLabel->Left + hSettingsForm->Canvas->TextWidth(hSettingsForm->UsedAvatarsStyleLabel->Caption) + 6;
	hSettingsForm->AvatarsStyleGroupBox->Height = 42;
	hSettingsForm->EditAvatarsStyleLabel->Enabled = true;
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
			hSettingsForm->ManualAvatarsUpdateButton->Caption = GetLangStr("AbortUpdates");
			//Tworzenie katalogu z awatarami
			if(!DirectoryExists(AvatarsDir)) CreateDir(AvatarsDir);
			//Wlaczenie paska postepu
			hSettingsForm->ProgressBar->Position = 0;
			hSettingsForm->ProgressBar->Visible = true;
			hSettingsForm->ProgressLabel->Caption = GetLangStr("RetrievingData");
			hSettingsForm->ProgressLabel->Visible = true;
			//Wlaczenie paska postepu na taskbarze
			hSettingsForm->Taskbar->ProgressValue = 0;
			hSettingsForm->Taskbar->ProgressState = TTaskBarProgressState::Normal;
			//Pobieranie listy plikow
			hSettingsForm->FileListBox->Directory = "";
			hSettingsForm->FileListBox->Directory = GetPluginUserDirW() + "\\tweetIM\\Avatars";
			//Ignorowanie plikow *.tmp i plikow ze spacja (np. konflikty stworzone przez Dropbox'a)
			for(int Count=0;Count<hSettingsForm->FileListBox->Items->Count;Count++)
			{
				if(ExtractFileName(hSettingsForm->FileListBox->Items->Strings[Count]).Pos(".tmp")>0)
				{
					DeleteFile(hSettingsForm->FileListBox->Items->Strings[Count]);
					hSettingsForm->FileListBox->Items->Strings[Count] ="TMP_DELETE";
				}
				else if(ExtractFileName(hSettingsForm->FileListBox->Items->Strings[Count]).Pos(" ")>0)
				{
					DeleteFile(hSettingsForm->FileListBox->Items->Strings[Count]);
					hSettingsForm->FileListBox->Items->Strings[Count] = "TMP_DELETE";
				}
			}
			while(hSettingsForm->FileListBox->Items->IndexOf("TMP_DELETE")!=-1)
				hSettingsForm->FileListBox->Items->Delete(hSettingsForm->FileListBox->Items->IndexOf("TMP_DELETE"));
			//Ustawianie maksymalnego paska postepu
			hSettingsForm->ProgressBar->Max = hSettingsForm->FileListBox->Items->Count;
			//Ustawianie maksymalnego paska postepu na taskbarze
			hSettingsForm->Taskbar->ProgressMaxValue = hSettingsForm->FileListBox->Items->Count;
			//Wlacznie aktualizacji
			hSettingsForm->AutoAvatarsUpdateThread->Start();
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

//Usuwanie elementu szybkiego dostepu do ustawien wtyczki
void DestroytweetIMFastSettings()
{
  TPluginAction FastSettingsItem;
  ZeroMemory(&FastSettingsItem,sizeof(TPluginAction));
  FastSettingsItem.cbSize = sizeof(TPluginAction);
  FastSettingsItem.pszName = L"tweetIMFastSettingsItemButton";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&FastSettingsItem));
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

//Usuwanie menu dla elementow wtyczki
void DestroyPopupMenu()
{
	TPluginAction CommandPopUp;
	ZeroMemory(&CommandPopUp,sizeof(TPluginAction));
	CommandPopUp.cbSize = sizeof(TPluginAction);
	CommandPopUp.pszName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENU,0,(LPARAM)(&CommandPopUp));
}
//---------------------------------------------------------------------------
//Tworzenie menu dla elementow wtyczki
void BuildPopupMenu()
{
	TPluginAction CommandPopUp;
	ZeroMemory(&CommandPopUp,sizeof(TPluginAction));
	CommandPopUp.cbSize = sizeof(TPluginAction);
	CommandPopUp.pszName = L"tweetIMCommandPopUp";
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENU,0,(LPARAM)(&CommandPopUp));
}
//---------------------------------------------------------------------------

void DestroyInsertTagItem()
{
	TPluginAction InsertTagItem;
	ZeroMemory(&InsertTagItem,sizeof(TPluginAction));
	InsertTagItem.cbSize = sizeof(TPluginAction);
	InsertTagItem.pszName = L"tweetIMInsertTagItem";
	InsertTagItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertTagItem));
}
//---------------------------------------------------------------------------
void BuildInsertTagItem()
{
	TPluginAction InsertTagItem;
	ZeroMemory(&InsertTagItem,sizeof(TPluginAction));
	InsertTagItem.cbSize = sizeof(TPluginAction);
	InsertTagItem.pszName = L"tweetIMInsertTagItem";
	InsertTagItem.pszCaption = (GetLangStr("Insert") + " " + ItemCopyData).w_str();
	InsertTagItem.Position = 0;
	InsertTagItem.IconIndex = 11;
	InsertTagItem.pszService = L"stweetIMInsertTagItem";
	InsertTagItem.pszPopupName = L"popURL";
	InsertTagItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&InsertTagItem));
}
//---------------------------------------------------------------------------

void DestroyInsertNickItem()
{
	TPluginAction InsertNickItem;
	ZeroMemory(&InsertNickItem,sizeof(TPluginAction));
	InsertNickItem.cbSize = sizeof(TPluginAction);
	InsertNickItem.pszName = L"tweetIMInsertNickItem";
	InsertNickItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&InsertNickItem));
}
//---------------------------------------------------------------------------
void BuildInsertNickItem()
{
	TPluginAction InsertNickItem;
	ZeroMemory(&InsertNickItem,sizeof(TPluginAction));
	InsertNickItem.cbSize = sizeof(TPluginAction);
	InsertNickItem.pszName = L"tweetIMInsertNickItem";
	InsertNickItem.pszCaption = (GetLangStr("Insert") + " @" + ItemCopyData).w_str();
	InsertNickItem.Position = 0;
	InsertNickItem.IconIndex = 11;
	InsertNickItem.pszService = L"stweetIMInsertNickItem";
	InsertNickItem.pszPopupName = L"popURL";
	InsertNickItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&InsertNickItem));
}
//---------------------------------------------------------------------------

void DestroySendPrivMsgItem()
{
	TPluginAction SendPrivMsgItem;
	ZeroMemory(&SendPrivMsgItem,sizeof(TPluginAction));
	SendPrivMsgItem.cbSize = sizeof(TPluginAction);
	SendPrivMsgItem.pszName = L"tweetIMSendPrivMsgItem";
	SendPrivMsgItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SendPrivMsgItem));
}
//---------------------------------------------------------------------------
void BuildSendPrivMsgItem()
{
	TPluginAction SendPrivMsgItem;
	ZeroMemory(&SendPrivMsgItem,sizeof(TPluginAction));
	SendPrivMsgItem.cbSize = sizeof(TPluginAction);
	SendPrivMsgItem.pszName = L"tweetIMSendPrivMsgItem";
	SendPrivMsgItem.pszCaption = GetLangStr("PrivMsg").w_str();
	SendPrivMsgItem.Position = 0;
	SendPrivMsgItem.IconIndex = 8;
	SendPrivMsgItem.pszService = L"stweetIMSendPrivMsgItem";
	SendPrivMsgItem.pszPopupName = L"popURL";
	SendPrivMsgItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SendPrivMsgItem));
}
//---------------------------------------------------------------------------

void DestroyFavLatestTweetItem()
{
	TPluginAction FavLatestTweetItem;
	ZeroMemory(&FavLatestTweetItem,sizeof(TPluginAction));
	FavLatestTweetItem.cbSize = sizeof(TPluginAction);
	FavLatestTweetItem.pszName = L"tweetIMFavLatestTweetItem";
	FavLatestTweetItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&FavLatestTweetItem));
}
//---------------------------------------------------------------------------
void BuildFavLatestTweetItem()
{
	TPluginAction FavLatestTweetItem;
	ZeroMemory(&FavLatestTweetItem,sizeof(TPluginAction));
	FavLatestTweetItem.cbSize = sizeof(TPluginAction);
	FavLatestTweetItem.pszName = L"tweetIMFavLatestTweetItem";
	FavLatestTweetItem.pszCaption = GetLangStr("FavLatest").w_str();
	FavLatestTweetItem.Position = 0;
	FavLatestTweetItem.IconIndex = 157;
	FavLatestTweetItem.pszService = L"stweetIMFavLatestTweetItem";
	FavLatestTweetItem.pszPopupName = L"popURL";
	FavLatestTweetItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&FavLatestTweetItem));
}
//---------------------------------------------------------------------------

//Usuwanie elementu do pokazywania najnowszych tweetniec uzytkownika
void DestroyShowTimelineItem()
{
	TPluginAction ShowTimelineItem;
	ZeroMemory(&ShowTimelineItem,sizeof(TPluginAction));
	ShowTimelineItem.cbSize = sizeof(TPluginAction);
	ShowTimelineItem.pszName = L"tweetIMShowTimelineItem";
	ShowTimelineItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowTimelineItem));
}
//---------------------------------------------------------------------------
//Tworzenie elementu do pokazywania najnowszych tweetniec uzytkownika
void BuildShowTimelineItem()
{
	TPluginAction ShowTimelineItem;
	ZeroMemory(&ShowTimelineItem,sizeof(TPluginAction));
	ShowTimelineItem.cbSize = sizeof(TPluginAction);
	ShowTimelineItem.pszName = L"tweetIMShowTimelineItem";
	ShowTimelineItem.pszCaption = GetLangStr("ShowLatest").w_str();
	ShowTimelineItem.Position = 0;
	ShowTimelineItem.IconIndex = 21;
	ShowTimelineItem.pszService = L"stweetIMShowTimelineItem";
	ShowTimelineItem.pszPopupName = L"popURL";
	ShowTimelineItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&ShowTimelineItem));
}
//---------------------------------------------------------------------------

//Usuwanie elementu do pokazywania informacji o uzytkowniku
void DestroyShowUserProfileItem()
{
	TPluginAction ShowUserProfileItem;
	ZeroMemory(&ShowUserProfileItem,sizeof(TPluginAction));
	ShowUserProfileItem.cbSize = sizeof(TPluginAction);
	ShowUserProfileItem.pszName = L"tweetIMShowUserProfileItem";
	ShowUserProfileItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&ShowUserProfileItem));
}
//---------------------------------------------------------------------------
//Tworzenie elementu do pokazywania informacji o uzytkowniku
void BuildShowUserProfileItem()
{
	TPluginAction ShowUserProfileItem;
	ZeroMemory(&ShowUserProfileItem,sizeof(TPluginAction));
	ShowUserProfileItem.cbSize = sizeof(TPluginAction);
	ShowUserProfileItem.pszName = L"tweetIMShowUserProfileItem";
	ShowUserProfileItem.pszCaption = GetLangStr("UserInfo").w_str();
	ShowUserProfileItem.Position = 0;
	ShowUserProfileItem.IconIndex = 21;
	ShowUserProfileItem.pszService = L"stweetIMShowUserProfileItem";
	ShowUserProfileItem.pszPopupName = L"popURL";
	ShowUserProfileItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&ShowUserProfileItem));
}
//---------------------------------------------------------------------------

//Usuwanie separatora
void DestroySeparatorItem()
{
	TPluginAction SeparatorItem;
	ZeroMemory(&SeparatorItem,sizeof(TPluginAction));
	SeparatorItem.cbSize = sizeof(TPluginAction);
	SeparatorItem.pszName = L"tweetIMSeparatorItem";
	SeparatorItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&SeparatorItem));
}
//---------------------------------------------------------------------------
//Tworzenie separatora
void BuildSeparatorItem()
{
	TPluginAction SeparatorItem;
	ZeroMemory(&SeparatorItem,sizeof(TPluginAction));
	SeparatorItem.cbSize = sizeof(TPluginAction);
	SeparatorItem.pszName = L"tweetIMSeparatorItem";
	SeparatorItem.pszCaption = L"-";
	SeparatorItem.Position = 0;
	SeparatorItem.IconIndex = 0;
	SeparatorItem.pszPopupName = L"popURL";
	SeparatorItem.Handle = (int)hFrmSend;
	PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&SeparatorItem));
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
	//Usuwanie "Usun ostatni tweet"
	TPluginAction UndoTweetCommandItem;
	ZeroMemory(&UndoTweetCommandItem,sizeof(TPluginAction));
	UndoTweetCommandItem.cbSize = sizeof(TPluginAction);
	UndoTweetCommandItem.pszName = L"tweetIMUndoTweetCommandItem";
	PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM ,0,(LPARAM)(&UndoTweetCommandItem));
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
	//Usuwanie buttona z komendami bota
	DestroyCommandItems();
	//Tworzenie buttona z komendami bota
	if(hFrmSend)
	{
		//Tworzenie buttona w oknie rozmowy
		TPluginAction CommandButton;
		ZeroMemory(&CommandButton,sizeof(TPluginAction));
		CommandButton.cbSize = sizeof(TPluginAction);
		CommandButton.pszName = L"tweetIMCommandButton";
		CommandButton.Hint = GetLangStr("BotCommands").w_str();
		CommandButton.Position = 0;
		CommandButton.IconIndex = 131;
		CommandButton.pszPopupName = L"tweetIMCommandPopUp";
		CommandButton.Handle = (int)hFrmSend;
		PluginLink.CallService(AQQ_CONTROLS_TOOLBAR "tbMain" AQQ_CONTROLS_CREATEBUTTON,0,(LPARAM)(&CommandButton));
		//Tworzenie "Usun ostatni tweet"
		TPluginAction UndoTweetCommandItem;
		ZeroMemory(&UndoTweetCommandItem,sizeof(TPluginAction));
		UndoTweetCommandItem.cbSize = sizeof(TPluginAction);
		UndoTweetCommandItem.pszName = L"tweetIMUndoTweetCommandItem";
		UndoTweetCommandItem.pszCaption = GetLangStr("UndoTweet").w_str();
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
		ErsCommandItem.pszCaption = GetLangStr("Followers").w_str();
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
		IngCommandItem.pszCaption = GetLangStr("Followings").w_str();
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
		UpdateCommandItem.pszCaption = GetLangStr("FetchUnread").w_str();
		UpdateCommandItem.Position = 0;
		UpdateCommandItem.IconIndex = 19;
		UpdateCommandItem.pszService = L"stweetIMUpdateCommandItem";
		UpdateCommandItem.pszPopupName = L"tweetIMCommandPopUp";
		PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&UpdateCommandItem));
	}
}
//---------------------------------------------------------------------------

//Serwis szybkiego dostepu do ustawien wtyczki
INT_PTR __stdcall ServicetweetIMFastSettingsItem(WPARAM wParam, LPARAM lParam)
{
	//Otwarcie okna ustawien
	OpenSettingsForm();

	return 0;
}
//---------------------------------------------------------------------------

//Serwis do wstawiania tagu
INT_PTR __stdcall ServiceInsertTagItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceInsertNickItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceSendPrivMsgItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceFavLatestTweetItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceShowTimelineItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceShowUserProfileItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceUpdateCommandItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceIngCommandItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceErsCommandItem(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall ServiceUndoTweetCommandItem(WPARAM wParam, LPARAM lParam)
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

//Procka okna timera
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg==WM_TIMER)
	{
		//Aktualizacja awatarow po wlaczeniu wtyczki
		if(wParam==TIMER_UPDATE_AVATARS_ONLOAD)
		{
			//Zatrzymanie timera
			KillTimer(hTimerFrm,TIMER_UPDATE_AVATARS_ONLOAD);
			//Wlaczenie timera regularnej aktualizacji
				SetTimer(hTimerFrm,TIMER_UPDATE_AVATARS,3600000,(TIMERPROC)TimerFrmProc);
			//Sprawdzenie aktualizacji awatarow
			AutoAvatarsUpdate();
		}
		//Regularna aktualizacja awatarow
		else if(wParam==TIMER_UPDATE_AVATARS)
		{
			//Sprawdzenie aktualizacji awatarow
			AutoAvatarsUpdate();
		}

		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

//Hook na aktwyna zakladke lub okno rozmowy (pokazywanie menu do cytowania, tworzenie buttonow)
INT_PTR __stdcall OnActiveTab(WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall OnAddLine(WPARAM wParam, LPARAM lParam)
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
		//Pobieranie informacji	wiadomosci
		TPluginMessage AddLineMessage = *(PPluginMessage)lParam;
		UnicodeString MessageDate = (double)AddLineMessage.Date;
		UnicodeString MessageJID = (wchar_t*)AddLineMessage.JID;
		if(MessageJID.Pos("/")) MessageJID.Delete(MessageJID.Pos("/"),MessageJID.Length());
		UnicodeString Body = (wchar_t*)AddLineMessage.Body;
		Body = Body.Trim();
		//Zabezpieczenie przed bledem bota - dublowanie wiadomosci
		if((ContactJID!=MessageJID)||((ContactJID==MessageJID)&&(Body!=LastAddLineBody->ReadString("Body",ContactJID,""))&&(Body!=LastAddLineBody->ReadString("Body2",ContactJID,""))))
		{
			//Zabezpieczenie przed bledem bota - dublowanie wiadomosci
			if(ContactJID==MessageJID)
			{
				//Wyjatki
				if((Body!="Update request has been sent")
				&&(Body!="The message has been sent.")
				&&(!((Body.Pos("User ")==1)&&(Body.Pos(" now follows you"))))
				&&(!((Body.Pos("User ")==1)&&(Body.Pos(" doesn't follow you anymore"))))
				&&(!((Body.Pos("Message #")==1)&&(Body.Pos(" was erased")))))
				{
					//Zmiana miejsca poprzedniej zapamietanej wiadomosci
					LastAddLineBody->WriteString("Body2",ContactJID,LastAddLineBody->ReadString("Body",ContactJID,""));
					//Zapamietanie wiadomosci
					LastAddLineBody->WriteString("Body",ContactJID,Body);
				}
				//Tlumaczenie wybranych wiadomosci
				else
				{
					//Tlumaczenie
					if(Body=="Update request has been sent")
						Body = GetLangStr("UpdateRequest");
					else if(Body=="The message has been sent.")
						Body = GetLangStr("MessageSent");
					else if((Body.Pos("User ")==1)&&(Body.Pos(" now follows you")))
					{
						//Znaki HTMLowe
						if(Body.Pos("&quot;")) Body = StringReplace(Body, "&quot;", "\"", TReplaceFlags() << rfReplaceAll);
						//Wyciagnie loginu uzytkownika
						UnicodeString UserLogin = Body;
						UserLogin.Delete(1,UserLogin.Pos("\""));
						UserLogin.Delete(UserLogin.Pos("\""),UserLogin.Length());
						//Tworzenie odnosnika
						UserLogin = "<B><A HREF=\"http://aqq-link/?url=https://twitter.com/" + UserLogin + "\">@" + UserLogin + "</A></B>";
						//Tlumaczenie
						Body = GetLangStr("FollowsYou");
						Body = StringReplace(Body, "CC_USER", UserLogin, TReplaceFlags());
					}
					else if((Body.Pos("User ")==1)&&(Body.Pos(" doesn't follow you anymore")))
					{
						//Znaki HTMLowe
						if(Body.Pos("&quot;")) Body = StringReplace(Body, "&quot;", "\"", TReplaceFlags() << rfReplaceAll);
						//Wyciagnie loginu uzytkownika
						UnicodeString UserLogin = Body;
						UserLogin.Delete(1,UserLogin.Pos("\""));
						UserLogin.Delete(UserLogin.Pos("\""),UserLogin.Length());
						//Tworzenie odnosnika
						UserLogin = "<B><A HREF=\"http://aqq-link/?url=https://twitter.com/" + UserLogin + "\">@" + UserLogin + "</A></B>";
						//Tlumaczenie
						Body = GetLangStr("DoesentFollow");
						Body = StringReplace(Body, "CC_USER", UserLogin, TReplaceFlags());
					}
					else if((Body.Pos("Message #")==1)&&(Body.Pos(" was erased")))
						Body = GetLangStr("TweetErased");
					//Zwrocenie zmodyfikowanej wiadomosci;
					AddLineMessage.Body = Body.w_str();
					memcpy((PPluginMessage)lParam,&AddLineMessage, sizeof(TPluginMessage));
					return 2;
				}
			}

			//Znaki HTMLowe
			if(Body.Pos("&amp;")) Body = StringReplace(Body, "&amp;", "&", TReplaceFlags() << rfReplaceAll);
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
				&&(TempStr.LowerCase().Pos("href=\"")==0)
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
			Body = StringReplace(Body, "[CC_TAGS_LINK]", "<A HREF=\"http://aqq-link/?url=https://twitter.com/hashtag/", TReplaceFlags() << rfReplaceAll);
			Body = StringReplace(Body, "[CC_TAGS_LINK2]", "?src=hash\">", TReplaceFlags() << rfReplaceAll);
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
				&&(TempStr.LowerCase().Pos("href=\"")==0)
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
			Body = StringReplace(Body, "[CC_USERS_LINK]", "<A HREF=\"http://aqq-link/?url=https://twitter.com/", TReplaceFlags() << rfReplaceAll);
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
				Body = StringReplace(Body, TempStr + ":", "<B><A HREF=\"http://aqq-link/?url=https://twitter.com/" + UserLogin + "\" title=\"@" + UserLogin + "\">" + DisplayName + "</A></B>:", TReplaceFlags());
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
					Avatars = StringReplace(AvatarStyle, "CC_AVATAR", "<a href=\"http://aqq-link/?url=https://twitter.com/" + TweetSender + "\" title=\"@" + TweetSender + "\"><img class=\"twitter-avatar\" border=\"0px\" src=\"https://beherit.pl/tweetIM/?user=" + TweetSender + "\" width=\"" + IntToStr(AvatarSize) + "px\" height=\"" + IntToStr(AvatarSize) + "px\"></a>", TReplaceFlags() << rfReplaceAll);
					//Dodanie awatara do pobrania
					GetAvatarsList->Add(TweetSender+";"+"https://beherit.pl/tweetIM/?user="+TweetSender);
					//Wlaczenie watku
					if(!hSettingsForm->GetAvatarsThread->Active) hSettingsForm->GetAvatarsThread->Start();
				}
				//Awatar znajduje sie w folderze cache
				else Avatars = StringReplace(AvatarStyle, "CC_AVATAR", "<a href=\"http://aqq-link/?url=https://twitter.com/" + TweetSender + "\" title=\"@" + TweetSender + "\"><img class=\"twitter-avatar\" border=\"0px\" src=\"file:///" + AvatarsDirW + "/" + TweetSender + "\" width=\"" + IntToStr(AvatarSize) + "px\" height=\"" + IntToStr(AvatarSize) + "px\"></a>", TReplaceFlags() << rfReplaceAll);
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
INT_PTR __stdcall OnColorChange(WPARAM wParam, LPARAM lParam)
{
	//Okno ustawien zostalo juz stworzone
	if(hSettingsForm)
	{
		//Wlaczona zaawansowana stylizacja okien
		if(ChkSkinEnabled())
		{
			TPluginColorChange ColorChange = *(PPluginColorChange)wParam;
			hSettingsForm->sSkinManager->HueOffset = ColorChange.Hue;
			hSettingsForm->sSkinManager->Saturation = ColorChange.Saturation;
			hSettingsForm->sSkinManager->Brightness = ColorChange.Brightness;
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane lokalizacji
INT_PTR __stdcall OnLangCodeChanged(WPARAM wParam, LPARAM lParam)
{
	//Czyszczenie cache lokalizacji
	ClearLngCache();
	//Pobranie sciezki do katalogu prywatnego uzytkownika
	UnicodeString PluginUserDir = GetPluginUserDir();
	//Ustawienie sciezki lokalizacji wtyczki
	UnicodeString LangCode = (wchar_t*)lParam;
	LangPath = PluginUserDir + "\\\\Languages\\\\tweetIM\\\\" + LangCode + "\\\\";
	if(!DirectoryExists(LangPath))
	{
		LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETDEFLANGCODE,0,0);
		LangPath = PluginUserDir + "\\\\Languages\\\\tweetIM\\\\" + LangCode + "\\\\";
	}
	//Aktualizacja lokalizacji form wtyczki
	for(int i=0;i<Screen->FormCount;i++)
		LangForm(Screen->Forms[i]);
	//Poprawka pozycji komponentow
	if(hSettingsForm)
	{
		hSettingsForm->UsedAvatarsStyleLabel->Left = hSettingsForm->AvatarsStyleLabel->Left + hSettingsForm->Canvas->TextWidth(hSettingsForm->AvatarsStyleLabel->Caption) + 6;
		hSettingsForm->EditAvatarsStyleLabel->Left = hSettingsForm->UsedAvatarsStyleLabel->Left + hSettingsForm->Canvas->TextWidth(hSettingsForm->UsedAvatarsStyleLabel->Caption) + 6;
		hSettingsForm->AutoAvatarsUpdateComboBox->Left = hSettingsForm->AvatarsUpdateLabel->Left + hSettingsForm->Canvas->TextWidth(hSettingsForm->AvatarsUpdateLabel->Caption) + 6;
		hSettingsForm->LastAvatarsUpdateLabel->Left = hSettingsForm->LastAvatarsUpdateInfoLabel->Left + hSettingsForm->Canvas->TextWidth(hSettingsForm->LastAvatarsUpdateInfoLabel->Caption) + 6;
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zaladowanie wszystkich modulow w AQQ (autoupdate awatarow)
INT_PTR __stdcall OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	//Timer aktualizacji awatarow po wlaczeniu wtyczki
	SetTimer(hTimerFrm,TIMER_UPDATE_AVATARS_ONLOAD,300000,(TIMERPROC)TimerFrmProc);

	return 0;
}
//---------------------------------------------------------------------------

//Hook na pobieranie adresow URL z roznych popup (tworzenie itemow w popup menu do akcji z tweetami)
INT_PTR __stdcall OnPerformCopyData(WPARAM wParam, LPARAM lParam)
{
	//Domyslne usuwanie elementow
	DestroyInsertTagItem();
	DestroyInsertNickItem();
	DestroySendPrivMsgItem();
	DestroyFavLatestTweetItem();
	DestroyShowTimelineItem();
	DestroyShowUserProfileItem();
	DestroySeparatorItem();
	//Kasowanie zapamietanych danych
	ItemCopyData = "";
	//Jezeli zezwolono na sprawdzanie danych
	if(!BlockPerformCopyData)
	{
		//Pobranie danych
		UnicodeString CopyData = (wchar_t*)lParam;
		//Tagi
		if(CopyData.Pos("https://twitter.com/hashtag/")==1)
		{
			//Wyciaganie tag'u
			CopyData.Delete(1,CopyData.Pos("/hashtag/")+8);
			CopyData.Delete(CopyData.Pos("?src=hash"),CopyData.Length());
			if(!CopyData.IsEmpty())
			{
				//Kopiowanie tag'u
				ItemCopyData = "#" + CopyData;
				//Tworzenie separatora
				BuildSeparatorItem();
				//Tworzenie elementu wstawiania tagu
				BuildInsertTagItem();
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
				BuildSeparatorItem();
				//Tworzenie elementu do pokazywania informacji o uzytkowniku
				BuildShowUserProfileItem();
				//Tworzenie elementu do pokazywania najnowszych tweetniec uzytkownika
				BuildShowTimelineItem();
				//Tworzenie elementu lajkowania ostatniego tweetniecia
				BuildFavLatestTweetItem();
				//Tworzenie elementu wysylania prywatnej wiadomosci
				BuildSendPrivMsgItem();
				//Tworzenie elementu wstawiania nicku
				BuildInsertNickItem();
			}
		}
	}

	return 0;
}
//---------------------------------------------------------------------------

INT_PTR __stdcall OnPrimaryTab (WPARAM wParam, LPARAM lParam)
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
INT_PTR __stdcall OnRecvMsg(WPARAM wParam, LPARAM lParam)
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
		if((Body!=LastRecvMsgBody->ReadString("Body",ContactJID,""))&&(Body!=LastRecvMsgBody->ReadString("Body2",ContactJID,"")))
		{
			//Wyjatki
			if((Body!="Update request has been sent")
			&&(Body!="The message has been sent.")
			&&(!((Body.Pos("User ")==1)&&(Body.Pos(" now follows you"))))
			&&(!((Body.Pos("User ")==1)&&(Body.Pos(" doesn't follow you anymore"))))
			&&(!((Body.Pos("Message #")==1)&&(Body.Pos(" was erased")))))
			{
				//Zmiana miejsca poprzedniej zapamietanej wiadomosci
				LastRecvMsgBody->WriteString("Body2",ContactJID,LastRecvMsgBody->ReadString("Body",ContactJID,""));
				//Zapamietanie wiadomosci
				LastRecvMsgBody->WriteString("Body",ContactJID,Body);
			}
		}
		//Blokada wiadomosci
		else return 1;
	}

	return 0;
}
//---------------------------------------------------------------------------

//Hook na zmiane kompozycji (pobranie stylu zalacznikow oraz zmiana skorkowania wtyczki)
INT_PTR __stdcall OnThemeChanged(WPARAM wParam, LPARAM lParam)
{
	//Okno ustawien zostalo juz stworzone
	if(hSettingsForm)
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
				hSettingsForm->sSkinManager->SkinDirectory = ThemeSkinDir;
				hSettingsForm->sSkinManager->SkinName = "Skin.asz";
				//Ustawianie animacji AlphaControls
				if(ChkThemeAnimateWindows()) hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 200;
				else hSettingsForm->sSkinManager->AnimEffects->FormShow->Time = 0;
				hSettingsForm->sSkinManager->Effects->AllowGlowing = ChkThemeGlowing();
				//Zmiana kolorystyki AlphaControls
				hSettingsForm->sSkinManager->HueOffset = GetHUE();
				hSettingsForm->sSkinManager->Saturation = GetSaturation();
				hSettingsForm->sSkinManager->Brightness = GetBrightness();
				//Aktywacja skorkowania AlphaControls
				hSettingsForm->sSkinManager->Active = true;
			}
			//Brak pliku zaawansowanej stylizacji okien
			else hSettingsForm->sSkinManager->Active = false;
		}
		//Zaawansowana stylizacja okien wylaczona
		else hSettingsForm->sSkinManager->Active = false;
		//Kolor labelow
		if(hSettingsForm->sSkinManager->Active)
		{
			hSettingsForm->UsedAvatarsStyleLabel->Kind->Color = GetWarningColor();
			hSettingsForm->LastAvatarsUpdateLabel->Kind->Color = hSettingsForm->UsedAvatarsStyleLabel->Kind->Color;
		}
		else
		{
			hSettingsForm->UsedAvatarsStyleLabel->Kind->Color = clGreen;
			hSettingsForm->LastAvatarsUpdateLabel->Kind->Color = clGreen;
		}
	}
	//Pobranie stylu Avatars
	GetThemeStyle();

	return 0;
}
//---------------------------------------------------------------------------

//Hook na odbieranie pakietow XML
INT_PTR __stdcall OnXMLDebug(WPARAM wParam, LPARAM lParam)
{
	//Pobranie informacji nt. pakietu XML
	TPluginXMLChunk XMLChunk = *(PPluginXMLChunk)lParam ;
	//Pobranie nadawcy pakietu XML
	UnicodeString XMLSender = (wchar_t*)XMLChunk.From;
	//Pakiet wyslany od bota tweet.IM
	if(XMLSender.Pos("@twitter.tweet.im"))
	{
		//Pobranie pakietu XML
		UnicodeString XML = (wchar_t*)wParam;
		//Kodowanie pakietu
		XML = UTF8ToUnicodeString(XML.w_str());
		//Parsowanie pakietu XML
		_di_IXMLDocument XMLDoc = LoadXMLData(XML);
		_di_IXMLNode MainNode = XMLDoc->DocumentElement;
		int ItemsCount = MainNode->ChildNodes->GetCount();
		for(int Count=0;Count<ItemsCount;Count++)
		{
			_di_IXMLNode ChildNodes = MainNode->ChildNodes->GetNode(Count);
			//Wiadomosc zawiera dodatkowe informacje
			if(ChildNodes->NodeName=="x")
			{
				//Dodatkowe informacje o wiadomosci od bota tweet.IM
				if(ChildNodes->Attributes["xmlns"]=="http://process-one.net/threads")
				{
					//Zwykly tweet
					if(ChildNodes->Attributes["type"]=="tweet")
					{
						//Pobieranie informacji o nadawcy wiadomosci
						UnicodeString twitter_nick = ChildNodes->Attributes["twitter-nick"];
						UnicodeString avatar_url = ChildNodes->Attributes["avatar-url"];
						//Dane zostaly pobrane
						if((!twitter_nick.IsEmpty())&&(!avatar_url.IsEmpty()))
						{
							//Tworzenie katalogu z awatarami
							if(!DirectoryExists(AvatarsDir))
								CreateDir(AvatarsDir);
							//Awatara nie ma w folderze cache
							if(!FileExists(AvatarsDir + "\\\\" + twitter_nick))
							{
								//Dodanie awatara do pobrania
								GetAvatarsList->Add(twitter_nick+";"+avatar_url);
								//Wlaczenie watku
								if(!hSettingsForm->GetAvatarsThread->Active) hSettingsForm->GetAvatarsThread->Start();
							}
						}
					}
				}
			}
		}
	}

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
	else return 0;
}
//---------------------------------------------------------------------------

//Odczyt ustawien
void LoadSettings()
{
	TIniFile *Ini = new TIniFile(GetPluginUserDir()+"\\\\tweetIM\\\\Settings.ini");
	//Awatary
	AvatarSize = Ini->ReadInteger("Avatars","Size",25);
	StaticAvatarStyle = DecodeBase64(Ini->ReadString("Avatars64","Style","").Trim().w_str());
	if(StaticAvatarStyle=="<span style=\"display: inline-block; padding: 2px 4px 0px 1px; vertical-align: middle;\">CC_AVATAR</span>")
	{
		Ini->DeleteKey("Avatars64","Style");
		StaticAvatarStyle = "";
	}
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

extern "C" INT_PTR __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
	//Linkowanie wtyczki z komunikatorem
	PluginLink = *Link;
	//Sciezka folderu prywatnego wtyczek
	UnicodeString PluginUserDir = GetPluginUserDir();
	//Tworzenie katalogow lokalizacji
	if(!DirectoryExists(PluginUserDir+"\\\\Languages"))
		CreateDir(PluginUserDir+"\\\\Languages");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\tweetIM"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\tweetIM");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN");
	if(!DirectoryExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL"))
		CreateDir(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL");
	//Wypakowanie plikow lokalizacji
	//0D074B67F6AB5F7659D06FD79779A2F5
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\Const.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\Const.lng").w_str(),L"EN_CONST",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\Const.lng")!="0D074B67F6AB5F7659D06FD79779A2F5")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\Const.lng").w_str(),L"EN_CONST",L"DATA");
	//7A2609BAFE6DBF5A840F93DB5FEF23F4
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\TSettingsForm.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\TSettingsForm.lng").w_str(),L"EN_SETTINGSFRM",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\TSettingsForm.lng")!="7A2609BAFE6DBF5A840F93DB5FEF23F4")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\EN\\\\TSettingsForm.lng").w_str(),L"EN_SETTINGSFRM",L"DATA");
	//6619E51EEECA6D21F712EEC28F0BF8C0
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\Const.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\Const.lng").w_str(),L"PL_CONST",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\Const.lng")!="6619E51EEECA6D21F712EEC28F0BF8C0")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\Const.lng").w_str(),L"PL_CONST",L"DATA");
	//450D8FBA9846B257512E55861A88F427
	if(!FileExists(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\TSettingsForm.lng"))
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\TSettingsForm.lng").w_str(),L"PL_SETTINGSFRM",L"DATA");
	else if(MD5File(PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\TSettingsForm.lng")!="450D8FBA9846B257512E55861A88F427")
		ExtractRes((PluginUserDir+"\\\\Languages\\\\tweetIM\\\\PL\\\\TSettingsForm.lng").w_str(),L"PL_SETTINGSFRM",L"DATA");
	//Ustawienie sciezki lokalizacji wtyczki
	UnicodeString LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETLANGCODE,0,0);
	LangPath = PluginUserDir + "\\\\Languages\\\\tweetIM\\\\" + LangCode + "\\\\";
	if(!DirectoryExists(LangPath))
	{
		LangCode = (wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETDEFLANGCODE,0,0);
		LangPath = PluginUserDir + "\\\\Languages\\\\tweetIM\\\\" + LangCode + "\\\\";
	}
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
	//Tworzenie menu dla elementow wtyczki
	BuildPopupMenu();
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
	PluginLink.CreateServiceFunction(L"stweetIMFavLatestTweetItem",ServiceFavLatestTweetItem);
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
	//Tworzenie interfejsu szybkiego dostepu do ustawien wtyczki
	BuildtweetIMFastSettings();
	//Przypisanie uchwytu do formy ustawien
	if(!hSettingsForm)
	{
		Application->Handle = (HWND)SettingsForm;
		hSettingsForm = new TSettingsForm(Application);
	}
	//Definiowanie User-Agent dla polaczen HTTP
	hSettingsForm->IdHTTP->Request->UserAgent = "AQQ IM Plugin: tweet.IM/" + GetFileInfo(GetPluginDir().w_str(), L"FileVersion") + " (+http://beherit.pl)";
	hSettingsForm->AUIdHTTP->Request->UserAgent = hSettingsForm->IdHTTP->Request->UserAgent;
	//Hook na aktwyna zakladke lub okno rozmowy (pokazywanie menu do cytowania, tworzenie buttonow)
	PluginLink.HookEvent(AQQ_CONTACTS_BUDDY_ACTIVETAB,OnActiveTab);
	//Hook na pokazywane wiadomosci (formatowanie tweetow)
	PluginLink.HookEvent(AQQ_CONTACTS_ADDLINE,OnAddLine);
	//Hook na zmiane kolorystyki AlphaControls
	PluginLink.HookEvent(AQQ_SYSTEM_COLORCHANGEV2,OnColorChange);
	//Hook na zmiane lokalizacji
	PluginLink.HookEvent(AQQ_SYSTEM_LANGCODE_CHANGED,OnLangCodeChanged);
	//Hook na zaladowanie wszystkich modulow w AQQ (autoupdate awatarow)
	PluginLink.HookEvent(AQQ_SYSTEM_MODULESLOADED,OnModulesLoaded);
	//Hook na pobieranie adresow URL z roznych popup (tworzenie itemow w popup menu do akcji z tweetami)
	PluginLink.HookEvent(AQQ_SYSTEM_PERFORM_COPYDATA,OnPerformCopyData);
	//Hook na odbieranie nowej wiadomosci
	PluginLink.HookEvent(AQQ_CONTACTS_RECVMSG,OnRecvMsg);
	//Hook na zmiane kompozycji (pobranie stylu zalacznikow oraz zmiana skorkowania wtyczki)
	PluginLink.HookEvent(AQQ_SYSTEM_THEMECHANGED,OnThemeChanged);
	//Hook na odbieranie pakietow XML
	PluginLink.HookEvent(AQQ_SYSTEM_XMLDEBUG,OnXMLDebug);
	//Odczyt ustawien
	LoadSettings();
	//Pobranie stylu Avatars
	GetThemeStyle();
	//Rejestowanie klasy okna timera
	WNDCLASSEX wincl;
	wincl.cbSize = sizeof (WNDCLASSEX);
	wincl.style = 0;
	wincl.lpfnWndProc = TimerFrmProc;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hInstance = HInstance;
	wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
	wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	wincl.lpszMenuName = NULL;
	wincl.lpszClassName = L"TTweetIMTimer";
	wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wincl);
	//Tworzenie okna timera
	hTimerFrm = CreateWindowEx(0, L"TTweetIMTimer", L"",	0, 0, 0, 0, 0, NULL, NULL, HInstance, NULL);
	//Jezeli wszystkie moduly w AQQ zostaly juz zaladowany przed wczytaniem wtyczki
	if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
	{
		//Timer aktualizacji awatarow po wlaczeniu wtyczki
		SetTimer(hTimerFrm,TIMER_UPDATE_AVATARS_ONLOAD,300000,(TIMERPROC)TimerFrmProc);
		//Hook na pobieranie aktywnych zakladek
		PluginLink.HookEvent(AQQ_CONTACTS_BUDDY_PRIMARYTAB,OnPrimaryTab);
		PluginLink.CallService(AQQ_CONTACTS_BUDDY_FETCHALLTABS,0,0);
		PluginLink.UnhookEvent(OnPrimaryTab);
	}

	return 0;
}
//---------------------------------------------------------------------------

extern "C" INT_PTR __declspec(dllexport) __stdcall Unload()
{
	//Anty "Abnormal program termination"
	hSettingsForm->aForceDisconnect->Execute();
	//Zatrzymanie timerow
	for(int TimerID=10;TimerID<=20;TimerID=TimerID+10) KillTimer(hTimerFrm,TimerID);
	//Usuwanie okna timera
	DestroyWindow(hTimerFrm);
	//Wyrejestowanie klasy okna timera
	UnregisterClass(L"TTweetIMTimer",HInstance);
	//Wyladowanie hookow
	PluginLink.UnhookEvent(OnActiveTab);
	PluginLink.UnhookEvent(OnAddLine);
	PluginLink.UnhookEvent(OnColorChange);
	PluginLink.UnhookEvent(OnLangCodeChanged);
	PluginLink.UnhookEvent(OnModulesLoaded);
	PluginLink.UnhookEvent(OnPerformCopyData);
	PluginLink.UnhookEvent(OnRecvMsg);
	PluginLink.UnhookEvent(OnThemeChanged);
	PluginLink.UnhookEvent(OnXMLDebug);
	//Usuwanie elementow z interfejsu AQQ
	DestroytweetIMFastSettings();
	DestroyInsertTagItem();
	DestroyInsertNickItem();
	DestroySendPrivMsgItem();
	DestroyFavLatestTweetItem();
	DestroyShowTimelineItem();
	DestroyShowUserProfileItem();
	DestroySeparatorItem();
	DestroyCommandItems();
	DestroyPopupMenu();
	//Usuwanie serwisow
	PluginLink.DestroyServiceFunction(ServicetweetIMFastSettingsItem);
	PluginLink.DestroyServiceFunction(ServiceInsertTagItem);
	PluginLink.DestroyServiceFunction(ServiceInsertNickItem);
	PluginLink.DestroyServiceFunction(ServiceSendPrivMsgItem);
	PluginLink.DestroyServiceFunction(ServiceFavLatestTweetItem);
	PluginLink.DestroyServiceFunction(ServiceShowTimelineItem);
	PluginLink.DestroyServiceFunction(ServiceShowUserProfileItem);
	PluginLink.DestroyServiceFunction(ServiceUpdateCommandItem);
	PluginLink.DestroyServiceFunction(ServiceIngCommandItem);
	PluginLink.DestroyServiceFunction(ServiceErsCommandItem);
	PluginLink.DestroyServiceFunction(ServiceUndoTweetCommandItem);;

	return 0;
}
//---------------------------------------------------------------------------

//Ustawienia wtyczki
extern "C" INT_PTR __declspec(dllexport)__stdcall Settings()
{
	//Otwarcie okna ustawien
	OpenSettingsForm();

	return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" __declspec(dllexport) PPluginInfo __stdcall AQQPluginInfo(DWORD AQQVersion)
{
	PluginInfo.cbSize = sizeof(TPluginInfo);
	PluginInfo.ShortName = L"tweet.IM";
	PluginInfo.Version = PLUGIN_MAKE_VERSION(1,2,0,0);
	PluginInfo.Description = L"Wtyczka przeznaczona dla osób u¿ywaj¹cych Twittera. Formatuje ona wszystkie wiadomoœci dla bota pochodz¹cego z serwisu tweet.IM.";
	PluginInfo.Author = L"Krzysztof Grochocki";
	PluginInfo.AuthorMail = L"kontakt@beherit.pl";
	PluginInfo.Copyright = L"Krzysztof Grochocki";
	PluginInfo.Homepage = L"http://beherit.pl";
	PluginInfo.Flag = 0;
	PluginInfo.ReplaceDefaultModule = 0;

	return &PluginInfo;
}
//---------------------------------------------------------------------------
