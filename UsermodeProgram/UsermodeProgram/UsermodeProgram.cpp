#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include "Driver.h"
#include "Maths.h"

#include <Windows.h>
#include <sddl.h>
#include <accctrl.h>
#include <iomanip>
#include <stdio.h>
#include <conio.h>
#include <aclapi.h>
#include <Iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#include <stdlib.h>
#include <iostream>
#include <ole2.h>
#include <WinBase.h>
#include <olectl.h>
#include <chrono>
#include <thread>
#include <ratio>
#include <tchar.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <winnt.h>
#include <Winnetwk.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <algorithm>
#include <ctype.h>
#include <stack>
#include <VersionHelpers.h>
#include <stdint.h>
#include <tchar.h>
#include <tlhelp32.h>
#include <time.h>
#include <WinUser.h>
#include <stdio.h>
#include <stdlib.h>
#include <Psapi.h>
#include <memory>
#include <TimeAPI.h>
#include <sstream>
#include <fstream>
#include <cstdint>
#include <iomanip>
#include <vector>


typedef struct _KERNEL_READ_REQUEST
{
	ULONG ProtectedProgram;
	ULONG LSASS;
	ULONG CSRSS;
	ULONG CSRSS2;

} KERNEL_READ_REQUEST, *PKERNEL_READ_REQUEST;


typedef struct _KERNEL_UNLOADDRIVER
{
	ULONG UnloadDriver;

} KERNEL_UNLOADDRIVER, *PKERNEL_UNLOADDRIVER;


#define IO_READ_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0701 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define IO_UNLOADDRIVER_REQUEST CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0702 /* Our Custom Code */, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

HANDLE hDriver;
bool SendProcessIDs(ULONG PROTECTEDPROGRAM, ULONG LSASS, ULONG CSRSS, ULONG CSRSS2)
{
	hDriver = CreateFileA("\\\\.\\discordexp", GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

	if (hDriver == INVALID_HANDLE_VALUE)
		return false;

	DWORD Return, Bytes;
	KERNEL_READ_REQUEST ReadRequest;

	ReadRequest.ProtectedProgram = PROTECTEDPROGRAM;
	ReadRequest.LSASS = LSASS;
	ReadRequest.CSRSS = CSRSS;
	ReadRequest.CSRSS2 = CSRSS2;

	
	if (DeviceIoControl(hDriver, IO_READ_REQUEST, &ReadRequest,
		sizeof(ReadRequest), &ReadRequest, sizeof(ReadRequest), &Bytes, NULL))
	{
		return true;
	}
	else
	{
		return false;
	}
}

#pragma comment(lib,"ntdll.lib")


typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

using namespace std;

vector<DWORD> GetPIDs(wstring targetProcessName)
{
	vector<DWORD> pids;
	if (targetProcessName == L"")
		return pids;
	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W entry;
	entry.dwSize = sizeof entry;
	if (!Process32FirstW(snap, &entry))
		return pids;
	do {
		if (wstring(entry.szExeFile) == targetProcessName) {
			pids.emplace_back(entry.th32ProcessID);
		}
	} while (Process32NextW(snap, &entry));
	return pids;
}

#define MAX_PROCESSES 1024 
DWORD FindProcessId(__in_z LPCTSTR lpcszFileName)
{
	LPDWORD lpdwProcessIds;
	LPTSTR  lpszBaseName;
	HANDLE  hProcess;
	DWORD   i, cdwProcesses, dwProcessId = 0;

	lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES * sizeof(DWORD));
	if (lpdwProcessIds != NULL)
	{
		if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES * sizeof(DWORD), &cdwProcesses))
		{
			lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH * sizeof(TCHAR));
			if (lpszBaseName != NULL)
			{
				cdwProcesses /= sizeof(DWORD);
				for (i = 0; i < cdwProcesses; i++)
				{
					hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]);
					if (hProcess != NULL)
					{
						if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0)
						{
							if (!lstrcmpi(lpszBaseName, lpcszFileName))
							{
								dwProcessId = lpdwProcessIds[i];
								CloseHandle(hProcess);
								break;
							}
						}
						CloseHandle(hProcess);
					}
				}
				HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName);
			}
		}
		HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds);
	}
	return dwProcessId;
}

int main()
{
	Wrappers Driver("\\\\.\\discordexp");
	DWORD ProcessId;


	DWORD csrss1 = NULL;
	DWORD csrss2 = NULL;
	wstring we1 = L"";
	wstring lsassNoStr1 = we1 + L'c' + L's' + L'r' + L's' + L's' + L'.' + L'e' + L'x' + L'e';
	vector<DWORD> pidsLsass1 = GetPIDs(lsassNoStr1);
	if (pidsLsass1.empty())
		cout << "Bulunamadı" << endl;
	sort(pidsLsass1.begin(), pidsLsass1.end()); 
	csrss1 = pidsLsass1[0];
	csrss2 = pidsLsass1[1];
	if (!csrss1)
		cout << "Bulunamadı" << endl;
	if (!csrss2)
		cout << "Bulunamadı" << endl;

	DWORD pivotPID = NULL;
	wstring we = L"";
	wstring lsassNoStr = we + L'l' + L's' + L'a' + L's' + L's' + L'.' + L'e' + L'x' + L'e';
	vector<DWORD> pidsLsass = GetPIDs(lsassNoStr);
	if (pidsLsass.empty())
		cout << "Bulunamadı" << endl;
	sort(pidsLsass.begin(), pidsLsass.end()); 
	pivotPID = pidsLsass[0];
	if (!pivotPID)
		cout << "Bulunamadı" << endl;
	
	
	SendProcessIDs(GetCurrentProcessId(), pivotPID, csrss1, csrss2);
	Driver.SetTargetPid(FindProcessId(L"RainbowSix.exe"));
	
	DWORD_PTR MainModule = Driver.GetMainModule();


	

	DWORD64 GameManager = Driver.RPM(MainModule + Offset::GameManager, sizeof(DWORD64));
	DWORD64 EntityList = Driver.RPM(GameManager + Offset::EntityList, sizeof(ULONG));

	DWORD64 Entity = Driver.RPM(EntityList + 0x0, sizeof(DWORD64));

	
	DWORD64 pRender = Driver.RPM(GameManager + Offset::Renderer, sizeof(DWORD64));

	DWORD64 pGameRender = Driver.RPM(pRender + Offset::GameRenderer, sizeof(DWORD64));
	
	DWORD64 pEngineLink = Driver.RPM(pGameRender + Offset::EngineLink, sizeof(DWORD64));

	DWORD64 pEngine = Driver.RPM(pEngineLink + Offset::Engine, sizeof(DWORD64));
	
	DWORD64 pCamera = Driver.RPM(pEngine + Offset::Camera, sizeof(DWORD64));
	

	while (true)
	{
		
		for (int i = 0; i <= 12; i++)
		{
			DWORD_PTR entityInfo = Driver.RPM(i + Offset::EntityInfo, sizeof(DWORD_PTR));
			DWORD_PTR mainComp = Driver.RPM(entityInfo + Offset::MainComponent, sizeof(DWORD_PTR));
			DWORD_PTR weaponComp = Driver.RPM(mainComp + Offset::WeaponComp, sizeof(DWORD_PTR));
			DWORD_PTR weaponProc = Driver.RPM(weaponComp + Offset::WeaponProcessor, sizeof(DWORD_PTR));
			DWORD_PTR weapon = Driver.RPM(weaponProc + Offset::Weapon, sizeof(DWORD_PTR));
			DWORD_PTR weaponInfo = Driver.RPM(weapon + Offset::WeaponInfo, sizeof(DWORD_PTR));
			float spread = Driver.RPM(weaponInfo + Offset::Spread, sizeof(float));

			Driver.WPM(weaponInfo + Offset::Spread, 0, sizeof(float));


			float recoil = Driver.RPM(weaponInfo + Offset::Recoil, sizeof(float));
			float recoil2 = Driver.RPM(weaponInfo + Offset::Recoil2, sizeof(float));
			float recoil3 = Driver.RPM(weaponInfo + Offset::Recoil3, sizeof(float));
			float adsRecoil = Driver.RPM(weaponInfo + Offset::AdsRecoil, sizeof(float));

			Driver.WPM(recoil, 0, sizeof(float));
			Driver.WPM(recoil2, 0, sizeof(float));
			Driver.WPM(recoil3, 0, sizeof(float));
			Driver.WPM(adsRecoil, 0, sizeof(float));

		}
		Sleep(10);
	}
	return 0;
}