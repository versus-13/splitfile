// SplitFile.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "plugin.hpp"
#include "SplitFileLng.h"
#include "version.h"
#include <initguid.h>
#include "guid.h"

#include "FarPlugin.h"

//static FarPlugin Context;

static struct PluginStartupInfo Info;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	FarPlugin::Instance().GetGlobalInfoW(Info);
	return;

	Info->StructSize = sizeof(struct GlobalInfo);
	Info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE);   //минимально необходимая версия Far Manager, смотрите изменения в API
	Info->Version = MAKEFARVERSION(3, 0, 0, 21, VS_RC);   //текущая версия плагина 3.0.0.21, релиз-кандидат
	Info->Guid = MainGuid;
	Info->Title = L"Align";
	Info->Description = L"Align block for Far Manager";
	Info->Author = L"Eugene Roshal & Far Group";
}

/*
Функция SetStartupInfoW вызывается один раз, перед всеми
другими функциями. Она передается плагину информацию,
необходимую для дальнейшей работы.
*/
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	FarPlugin::Instance().SetStartupInfoW(psi);
	return;

	Info = *psi;
}

/*
Функция GetPluginInfoW вызывается для получения информации о плагине
*/
void WINAPI GetPluginInfoW(struct PluginInfo *PInfo)
{
	FarPlugin::Instance().GetPluginInfoW(PInfo);
	return;


	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = Info.GetMsg(&MainGuid, MTitle);

	PInfo->StructSize = sizeof(struct PluginInfo);
	PInfo->Flags = 0; // PF_EDITOR | PF_DISABLEPANELS;
	PInfo->PluginMenu.Guids = &MenuGuid;
	PInfo->PluginMenu.Strings = PluginMenuStrings;
	PInfo->PluginMenu.Count = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
	PInfo->PluginConfig.Guids = &MenuGuid;
	PInfo->PluginConfig.Strings = PluginMenuStrings;
	PInfo->PluginConfig.Count = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
}

intptr_t WINAPI DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
{
	COORD *coords;

	switch (Msg)
	{
	case DN_INITDIALOG:
		break;

	case DN_DRAWDIALOG:
		break;

	case DN_RESIZECONSOLE:
		coords = (COORD*)Param2;
		break;

	default:
		break;
	}
	return Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

/*
Функция OpenPluginW вызывается при создании новой копии плагина.
*/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	return FarPlugin::Instance().OpenW(OInfo);

	const size_t ITEMS_COUNT = 1;
	FarDialogItem Items[ITEMS_COUNT];

	Items[0].Type = DI_TEXT;
	Items[0].X1 = 0;
	Items[0].Y1 = 0;
	Items[0].X2 = 10;
	Items[0].Y2 = 0;
	Items[0].Data = L"5555";



//	Info.Message(&MainGuid,           /* GUID */
//		NULL,
//		FMSG_WARNING | FMSG_LEFTALIGN,  /* Flags */
//		L"Contents",                  /* HelpTopic */
//		MsgItems,                     /* Items */
//		ARRAYSIZE(MsgItems),          /* ItemsNumber */
//		1);                           /* ButtonsNumber */

	FarDialogItem InitItems[] =
	{
		{ DI_DOUBLEBOX, 0, 0, 39, 8, 0, 0, 0, 0, 0 },
		{ DI_BUTTON, 20, 6, 20, 7, 0, 0, 0, 0, L"OK" },
		{ DI_BUTTON, 27, 6, 31, 7, DIF_CENTERGROUP, 0, 0, 1, L"Cancel" },
		{ DI_TEXT, 2, 1, 28, 2, 0, 0, L"text", DIF_LEFTTEXT, L"Enter name of file for" },
		{ DI_TEXT, 2, 2, 28, 3, 0, 0, L"target", DIF_LEFTTEXT, L"Export" },
		{ DI_EDIT, 2, 4, 37, 4, 0, 0, L"nameFile", DIF_HISTORY | DIF_SELECTONENTRY, L"name_file.txt" }
	};

	
	//HANDLE h = (HANDLE)Info.AdvControl(&MainGuid, ACTL_GETFARHWND, 0, NULL);
	SMALL_RECT r;
	Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &r);
	CONSOLE_SCREEN_BUFFER_INFO ScreenInfo;
	//GetConsoleScreenBufferInfo(h, &ScreenInfo);

	HANDLE dlg = Info.DialogInit(&MainGuid, &DialogGuid, -1, -1, 40, 20, L"Contents", InitItems, ARRAYSIZE(InitItems), NULL, FDLG_NONE, DlgProc, NULL);
	Info.DialogRun(dlg);
	Info.DialogFree(dlg);

	return NULL;
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* CInfo)
{
	return FarPlugin::Instance().ConfigureW(CInfo);
}

