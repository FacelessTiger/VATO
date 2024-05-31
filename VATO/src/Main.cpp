#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

#include <string>

HANDLE FindHandleByName(const wchar_t* name, bool skipFirst = false)
{
	PROCESSENTRY32 entry = {
		.dwSize = sizeof(PROCESSENTRY32)
	};

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(snapshot, &entry))
	{
		while (Process32Next(snapshot, &entry))
		{
			if (wcscmp(entry.szExeFile, name) == 0)
			{
				if (skipFirst)
				{
					skipFirst = false;
					continue;
				}

				HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
				CloseHandle(snapshot);
				return process;
			}
		}
	}

	return nullptr;
}

void StartLeague(const std::wstring& clientLocation)
{
	std::wstring leagueConst = L"\"" + clientLocation + L"\" --launch-product=league_of_legends --launch-patchline=live";
	wchar_t* league = new wchar_t[leagueConst.size() + 1];

	std::memcpy(league, leagueConst.data(), (leagueConst.size() + 1) * sizeof(wchar_t));

	STARTUPINFO si = {
		.cb = sizeof(STARTUPINFO)
	};
	PROCESS_INFORMATION pi = {};
	if (!CreateProcess(NULL, league, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		MessageBox(NULL, std::to_wstring(GetLastError()).c_str(), L"Error", MB_ICONEXCLAMATION | MB_OK);
	
	delete league;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	wchar_t* clientLocation = new wchar_t[strlen(__argv[1]) + 1];
	mbstowcs(clientLocation, __argv[1], strlen(__argv[1]) + 1);

	// If we're already open, then just start league and exit
	if (FindHandleByName(L"VanguardTest.exe", true))
	{
		StartLeague(clientLocation);
		return 0;
	}

	SC_HANDLE scm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT | GENERIC_WRITE);

	SC_HANDLE vgc = OpenService(scm, L"vgc", GENERIC_READ | SERVICE_CHANGE_CONFIG);
	SC_HANDLE vgk = OpenService(scm, L"vgk", GENERIC_READ | SERVICE_CHANGE_CONFIG);
	if (!vgc || !vgk)
	{
		std::cout << GetLastError() << std::endl;
		CloseServiceHandle(vgc);
		CloseServiceHandle(vgk);
		return 1;
	}

	SERVICE_STATUS vgcStatus;
	SERVICE_STATUS vgkStatus;
	if (!QueryServiceStatus(vgc, &vgcStatus) || !QueryServiceStatus(vgk, &vgkStatus))
	{
		CloseServiceHandle(vgc);
		CloseServiceHandle(vgk);
		CloseServiceHandle(scm);
		return 1;
	}

	if (vgkStatus.dwCurrentState == SERVICE_STOPPED)
	{
		int messageBox = MessageBox(NULL, L"Vanguard is disabled. Do you want to enable it and restart?", L"Restart?", MB_ICONEXCLAMATION | MB_YESNO);
		if (messageBox == IDYES)
		{
			ChangeServiceConfig(vgc, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			ChangeServiceConfig(vgk, SERVICE_NO_CHANGE, SERVICE_SYSTEM_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

			// Enable shutdown privilege, then reboot the system
			HANDLE token;
			LUID luid;
			OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token);
			LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &luid);

			TOKEN_PRIVILEGES tp = {
				.PrivilegeCount = 1,
				.Privileges = {{
					.Luid = luid,
					.Attributes = SE_PRIVILEGE_ENABLED
				}}
			};
			AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL, 0);
			ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, SHTDN_REASON_MINOR_OTHER);

			return 0; // Should be impossible to get to, but just in case
		}
		else
		{
			CloseServiceHandle(vgc);
			CloseServiceHandle(vgk);
			CloseServiceHandle(scm);
			return 0;
		}
	}

	// Vanguard should be confirmed enabled if we get here
	// Launch game and pause until riot client is closed
	StartLeague(clientLocation);
	HANDLE riotClient = FindHandleByName(L"RiotClientServices.exe"); // We have to find the handle ourselves, since the above can close if riot client is already open
	WaitForSingleObject(riotClient, INFINITE);

	// Disable and stop vanguard
	ChangeServiceConfig(vgc, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	ChangeServiceConfig(vgk, SERVICE_NO_CHANGE, SERVICE_DISABLED, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	system("net stop vgk & taskkill /IM vgtray.exe"); // TODO: find a way to do this without system hack

	CloseServiceHandle(vgc);
	CloseServiceHandle(vgk);
	CloseServiceHandle(scm);

	MessageBox(NULL, L"Riot client closed. Disabled vanguard!", L"Disabled Vanguard", MB_OK);
	delete clientLocation;

	return 0;
}