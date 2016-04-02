#include "stdafx.h"
#include "FarPlugin.h"
#include "guid.h"
#include "SplitFileLng.h"


FarPlugin::FarPlugin()
{
	m_SizeX = 0;
	m_SizeY = 0;
}


FarPlugin::~FarPlugin()
{
}

void FarPlugin::GetGlobalInfoW(GlobalInfo *info)
{
	info->StructSize = sizeof(struct GlobalInfo);
	info->MinFarVersion = MAKEFARVERSION(3, 0, 0, 2927, VS_RELEASE);   //минимально необходимая версия Far Manager, смотрите изменения в API
	info->Version = MAKEFARVERSION(3, 0, 0, 21, VS_RC);   //текущая версия плагина 3.0.0.21, релиз-кандидат
	info->Guid = MainGuid;
	info->Title = L"Align";
	info->Description = L"Align block for Far Manager";
	info->Author = L"Eugene Roshal & Far Group";
}

void FarPlugin::SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	m_Info = *psi;
}

void FarPlugin::GetPluginInfoW(struct PluginInfo *info)
{
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0] = m_Info.GetMsg(&MainGuid, MTitle);

	info->StructSize = sizeof(struct PluginInfo);
	info->Flags = 0; // PF_EDITOR | PF_DISABLEPANELS;
	info->PluginMenu.Guids = &MenuGuid;
	info->PluginMenu.Strings = PluginMenuStrings;
	info->PluginMenu.Count = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);
	info->PluginConfig.Guids = &MenuGuid;
	info->PluginConfig.Strings = PluginMenuStrings;
	info->PluginConfig.Count = sizeof(PluginMenuStrings) / sizeof(PluginMenuStrings[0]);

}

intptr_t WINAPI FarPlugin::DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2)
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
	return FarPlugin::Instance().m_Info.DefDlgProc(hDlg, Msg, Param1, Param2);
}

std::wstring FarPlugin::CheckSize(std::wstring s) {
	wchar_t Measure = '\0';
	std::wstring r;
	
	for (std::wstring::iterator it = s.end(); it != s.begin();) {
		it--;
		if (it == s.end())
			continue;
		if (*it == ' ')
			continue;
		if ((*it == 'K') || (*it == 'M') || (*it == 'G')) {
			if (r.length() > 0)
				return L"";
			if (Measure != '\0')
				return L"";
			Measure = *it;
		}
		if (((*it >= '0') && (*it <= '9')) || (*it == '.')) {
			r = (*it) + r;
		}
	}

	double d;
	try {
		d = wcstod(r.c_str(), NULL);
	}
	catch (...) {
		return L"";
	}

	if (Measure == '\0')
		m_PartSize = (ULONGLONG)d;
	else {
		d *= 1024;
		if (Measure == 'K')
			m_PartSize = (ULONGLONG)d;
		else {
			d *= 1024;
			if (Measure == 'M')
				m_PartSize = (ULONGLONG)d;
			else {
				d *= 1024;
				m_PartSize = (ULONGLONG)d;
			}
		}
	}

	if (Measure == '\0')
		return r;

	return r + Measure;
}

void FarPlugin::OpenSettingsHandle() {
	FarSettingsCreate fct = { sizeof(FarSettingsCreate), MainGuid, 0 };
	if (!m_Info.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_LOCAL, &fct))
		return;

	m_SettingsHandle = fct.Handle;
}

void FarPlugin::CloseSettingsHandle() {
	m_Info.SettingsControl(m_SettingsHandle, SCTL_FREE, 0, NULL);
}

std::wstring FarPlugin::ReadSetting(std::wstring id) {
	FarSettingsItem item = { sizeof(FarSettingsItem), 0, id.c_str(), FST_STRING };
	if (m_Info.SettingsControl(m_SettingsHandle, SCTL_GET, 0, &item)) {
		return item.String;
	}
}

void FarPlugin::SaveSetting(std::wstring id, std::wstring value) {
	FarSettingsItem item = { sizeof(FarSettingsItem), 0, id.c_str(), FST_STRING };

	item.String = value.c_str();
	m_Info.SettingsControl(m_SettingsHandle, SCTL_SET, 0, &item);
}

void FarPlugin::OpenSettings(std::wstring s, int save) {
	FarSettingsCreate fct = { sizeof(FarSettingsCreate), MainGuid, 0 };
	if (!m_Info.SettingsControl(INVALID_HANDLE_VALUE, SCTL_CREATE, PSL_LOCAL, &fct))
		return;
	
	HANDLE handle = fct.Handle;

	FarSettingsItem item1 = { sizeof(FarSettingsItem), 0, SIZE_PARAM_NAME, FST_STRING };
	if (m_Info.SettingsControl(handle, SCTL_GET, 0, &item1)) {
		m_Size = CheckSize(item1.String);
	}

	FarSettingsItem item2 = { sizeof(FarSettingsItem), 0, DIGITS_PARAM_NAME, FST_STRING };
	if (m_Info.SettingsControl(handle, SCTL_GET, 0, &item2)) {
		try {
			m_Digits = wcstol(item2.String, NULL, 10);
		}
		catch (...) {
			m_Digits = 3;
		}
	}

	std::wstring r;
	FarSettingsItem item3 = { sizeof(FarSettingsItem), 0, HISTORY_PARAM_NAME, FST_STRING };
	if (m_Info.SettingsControl(handle, SCTL_GET, 0, &item3)) {
		r = item3.String;
	}

	ParseHistory(r, s);

	if (save) {
		r.clear();
		for (std::vector<std::wstring>::iterator it = m_FileHistory.begin(); it != m_FileHistory.end(); it++) {
			if (r.length() > 0)
				r += '\1';
			r += *it;
		}
		
		item3.String = r.c_str();
		m_Info.SettingsControl(handle, SCTL_SET, 0, &item3);

		item2.String = (std::to_wstring(m_Digits)).c_str();
		m_Info.SettingsControl(handle, SCTL_SET, 0, &item2);

		item1.String = m_Size.c_str();
		m_Info.SettingsControl(handle, SCTL_SET, 0, &item1);
	}

	m_Info.SettingsControl(handle, SCTL_FREE, 0, NULL);
}

void FarPlugin::AppendToHistory(std::wstring newvalue, std::wstring appended) {
	if (newvalue != appended)
		m_FileHistory.push_back(newvalue);
}

void FarPlugin::ParseHistory(std::wstring saved, std::wstring appended) {
	m_FileHistory.clear();
	m_FileHistory.push_back(appended);
	int j = 0;
	while (m_FileHistory.size() < LOCAL_HISTORY_DEPTH) {
		int i = saved.find('\1', j + 1);
		if (i >= 0) {
			AppendToHistory(saved.substr(j, i - j), appended);
			j = i + 1;
		}
		else {
			if (j < saved.length())
				AppendToHistory(saved.substr(j), appended);
			break;
		}
	}
}

HANDLE FarPlugin::OpenW(const struct OpenInfo *OInfo)
{
	std::wstring dir = GetCurrentDir();
	if (!IsValidDir(dir))
		return INVALID_HANDLE_VALUE;

	std::vector<std::wstring> selfiles = GetSelectedFiles();
	if (selfiles.size())
	{
		JoinFiles(selfiles);
		return NULL;
	}

	std::wstring cfile = GetCurrentFile();
	if ((cfile.empty()) || (cfile == L"..") || (cfile == L"."))
		return NULL;

	SMALL_RECT r;
	if (m_Info.AdvControl(&MainGuid, ACTL_GETFARRECT, 0, &r))
		m_SizeX = r.Right - r.Left + 1;


	int HalfX = (int)((m_SizeX - 10) / 2);
	std::wstring caption = L" Split file \"" + cfile + L"\" ";

	OpenSettingsHandle();
	std::wstring size = CheckSize(ReadSetting(SIZE_PARAM_NAME));
	int Digits;
	try {
		Digits = wcstol(ReadSetting(DIGITS_PARAM_NAME).c_str(), NULL, 10);
	}
	catch (...) {
		Digits = 0;
	}
	ParseHistory(ReadSetting(HISTORY_PARAM_NAME), cfile);
	//OpenSettings(cfile, 0);

	std::wstring digits_str = std::to_wstring(Digits);
	FarDialogItem InitItems[] =
	{
		{ DI_SINGLEBOX,		2,			6,	m_SizeX - 11,	8,			0, 0, 0, 0, 0 },
		{ DI_DOUBLEBOX,		2,			1,	m_SizeX - 11,	8,			0, 0, 0, 0, 0 },
		{ DI_TEXT,			2,			1,	m_SizeX - 11,	1,			0, 0, L"caption", DIF_CENTERGROUP, caption.c_str() },
		{ DI_TEXT,			4,			2,	m_SizeX - 14,	2,			0, 0, L"text1", DIF_LEFTTEXT, L"Output files:" },
		{ DI_COMBOBOX,		4,			3,	m_SizeX - 14,	3,			0, 0, L"outfile", DIF_HISTORY | DIF_SELECTONENTRY, cfile.c_str() },

		{ DI_TEXT,			4,			5,	10,				5,			0, 0, L"text2", DIF_LEFTTEXT, L"Digits:" },
		{ DI_EDIT,			11,			5,	HalfX - 2,		5,			0, 0, L"digits", DIF_HISTORY | DIF_SELECTONENTRY, digits_str.c_str() },

		{ DI_TEXT,			HalfX,		5,	HalfX + 10,		5,			0, 0, L"text3", DIF_LEFTTEXT, L"Part size:" },
		{ DI_EDIT,			HalfX + 11,	5,	m_SizeX - 14,	5,			0, 0, L"size", DIF_HISTORY | DIF_SELECTONENTRY, size.c_str() },

		{ DI_BUTTON, 0, 7, 0, 7, 0, 0, 0, DIF_CENTERGROUP, L"OK" },
		{ DI_BUTTON, 0, 7, 0, 7, 0, 0, 0, DIF_CENTERGROUP, L"Cancel" }
	};

	FarListItem *items = new FarListItem[m_FileHistory.size()];
	for (int k = 0; k < m_FileHistory.size(); k++) {
		items[k].Flags = (LIF_SELECTED && (!k));
		items[k].Text = m_FileHistory.at(k).c_str();
	}
	FarList lst;
	lst.ItemsNumber = m_FileHistory.size();
	lst.StructSize = sizeof(lst);
	lst.Items = items;
	InitItems[4].ListItems = &lst;

	HANDLE dlg = m_Info.DialogInit(&MainGuid, &DialogGuid, -1, -1, m_SizeX - 8, 10, L"Contents", InitItems, ARRAYSIZE(InitItems), NULL, FDLG_NONE, DlgProc, NULL);
	intptr_t res = m_Info.DialogRun(dlg);

	cfile = GetDlgItemData(dlg, 4);
	try {
		Digits = wcstol(GetDlgItemData(dlg, 6).c_str(), NULL, 10);
	}
	catch (...) {
		Digits = 0;
	}
	size = CheckSize(GetDlgItemData(dlg, 8));

	m_Info.DialogFree(dlg);

	if ((res > -1) && (res != 10)) {
		std::wstring r;
		ParseHistory(ReadSetting(HISTORY_PARAM_NAME), cfile);
		for (std::vector<std::wstring>::iterator it = m_FileHistory.begin(); it != m_FileHistory.end(); it++) {
			if (r.length() > 0)
				r += '\1';
			r += *it;
		}
		SaveSetting(HISTORY_PARAM_NAME, r);
		SaveSetting(SIZE_PARAM_NAME, size);
		SaveSetting(DIGITS_PARAM_NAME, std::to_wstring(Digits));
		//OpenSettings(cfile, 1);
		CloseSettingsHandle();

		SplitFile(cfile, Digits, m_PartSize);
	}

	return NULL;
}

void FarPlugin::SplitFile(std::wstring filename, int digits, ULONGLONG partsize) {

}

std::wstring FarPlugin::GetDlgItemData(HANDLE dlg, int id) {
	const int DATA_LENGTH = 1024;
	wchar_t dd[DATA_LENGTH];
	FarDialogItemData d = { sizeof(FarDialogItemData), DATA_LENGTH - 1, dd };
	dd[m_Info.SendDlgMessage(dlg, DM_GETTEXT, id, &d)] = 0;
	return dd;
}

intptr_t FarPlugin::ConfigureW(const ConfigureInfo* CInfo)
{
	return 0;
}

std::wstring FarPlugin::GetCurrentFile()
{
	size_t size = m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, 0);
	PluginPanelItem* PPI = (PluginPanelItem*)malloc(size);
	FarGetPluginPanelItem pi = { sizeof(FarGetPluginPanelItem), size, PPI };
	m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETCURRENTPANELITEM, 0, &pi);
	std::wstring r;
	if (PPI->FileSize > 0)
		r = PPI->FileName;
	else
		r = L"";
	free(PPI);

	return r;
}

std::wstring FarPlugin::GetCurrentDir()
{
	size_t size = m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIRECTORY, 0, 0);
	FarPanelDirectory* FPD = (FarPanelDirectory*)malloc(size);
	FPD->PluginId = MainGuid;
	FPD->StructSize = size;
	m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELDIRECTORY, size, FPD);
	std::wstring s = FPD->Name;
	free(FPD);
	return s;
}

int FarPlugin::IsValidDir(std::wstring dir)
{
	if (dir.length() < 3)
		return false;

	if ((dir.at(1) == ':') && (dir.at(2) == '\\'))
		return true;

	if ((dir.at(0) == '\\') && (dir.at(1) == '\\'))
		return true;

	return false;
}

std::vector<std::wstring> FarPlugin::GetSelectedFiles()
{
	std::vector<std::wstring> r;

	struct PanelInfo PInfo;
	m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &PInfo);
	if (PInfo.SelectedItemsNumber == 0)
		return r;

	size_t size = m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, 0, 0);

	for (int i = 0; i < PInfo.SelectedItemsNumber; i++)
	{
		PluginPanelItem* PPI = (PluginPanelItem*)malloc(size);
		FarGetPluginPanelItem pi = { sizeof(FarGetPluginPanelItem), size, PPI };
		m_Info.PanelControl(PANEL_ACTIVE, FCTL_GETSELECTEDPANELITEM, i, &pi);
		if ((PPI->Flags & PPIF_SELECTED) == PPIF_SELECTED)
			r.push_back(PPI->FileName);
		free(PPI);
	}

	return r;
}

void FarPlugin::JoinFiles(std::vector<std::wstring> files)
{

}