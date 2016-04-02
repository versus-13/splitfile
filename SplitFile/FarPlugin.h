#pragma once

#include "plugin.hpp"
#include <string>
#include <vector>

class FarPlugin
{
private:
	const static int LOCAL_HISTORY_DEPTH = 5;
	const wchar_t *HISTORY_PARAM_NAME = L"history_outfile";
	const wchar_t *SIZE_PARAM_NAME = L"size";
	const wchar_t *DIGITS_PARAM_NAME = L"digits";

	struct PluginStartupInfo m_Info;
	std::vector<std::wstring> m_FileHistory;
	std::wstring m_Size;
	ULONGLONG m_PartSize;
	int m_Digits;

	FarPlugin();
	~FarPlugin();
	FarPlugin& operator=(FarPlugin&);
	static intptr_t WINAPI DlgProc(HANDLE hDlg, intptr_t Msg, intptr_t Param1, void *Param2);
	void OpenSettings(std::wstring s, int save);
	void ParseHistory(std::wstring saved, std::wstring appended);
	void AppendToHistory(std::wstring newvalue, std::wstring appended);
	std::wstring CheckSize(std::wstring s);
	std::wstring GetDlgItemData(HANDLE dlg, int id);

	HANDLE m_SettingsHandle = INVALID_HANDLE_VALUE;
	void OpenSettingsHandle();
	void CloseSettingsHandle();
	std::wstring ReadSetting(std::wstring id);
	void SaveSetting(std::wstring id, std::wstring value);

	void SplitFile(std::wstring filename, int digits, ULONGLONG partsize);

public:
	static FarPlugin &Instance()
	{
		static FarPlugin m_Instance;
		return m_Instance;
	}

	void GetGlobalInfoW(GlobalInfo *info);
	void SetStartupInfoW(const struct PluginStartupInfo *psi);
	void GetPluginInfoW(struct PluginInfo *info);
	HANDLE OpenW(const struct OpenInfo *OInfo);
	intptr_t ConfigureW(const ConfigureInfo* CInfo);

private:
	UINT m_SizeX, m_SizeY;

	std::wstring GetCurrentFile();
	std::wstring GetCurrentDir();
	std::vector<std::wstring> FarPlugin::GetSelectedFiles();

	static int IsValidDir(std::wstring dir);

	void JoinFiles(std::vector<std::wstring> files);
};

