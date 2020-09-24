#include "injector.h"


#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

bool IsCorrectTargetArchitecture(HANDLE hProc) {
	BOOL bTarget = FALSE;
	if (!IsWow64Process(hProc, &bTarget)) {
		printf("Can't confirm target process architecture: 0x%X\n", GetLastError());
		return false;
	}

	BOOL bHost = FALSE;
	IsWow64Process(GetCurrentProcess(), &bHost);

	return (bTarget == bHost);
}

DWORD GetProcessIdByName(wchar_t* name) {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE) {
		while (Process32Next(snapshot, &entry) == TRUE) {
			if (_wcsicmp(entry.szExeFile, name) == 0) {
				return entry.th32ProcessID;
			}
		}
	}

	CloseHandle(snapshot);
	return 0;
}

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	wchar_t* dllPath;
	DWORD PID;
	if (argc == 3) {
		dllPath = argv[1];
		PID = GetProcessIdByName(argv[2]);
	}
	else if (argc == 2) {
		dllPath = argv[1];
		std::string pname;
		printf("Process Name:\n");
		std::getline(std::cin, pname);

		char* vIn = (char*)pname.c_str();
		wchar_t* vOut = new wchar_t[strlen(vIn) + 1];
		mbstowcs_s(NULL, vOut, strlen(vIn) + 1, vIn, strlen(vIn));
		PID = GetProcessIdByName(vOut);
	}
	else {
		printf("Invalid Params\n");
		printf("Usage: dll_path [process_name]\n");
		system("pause");
		return 0;
	}

	if (PID == 0) {
		printf("Process not found\n");
		system("pause");
		return -1;
	}

	printf("Process pid: %d\n", PID);
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	if (!hProc) {
		DWORD Err = GetLastError();
		printf("OpenProcess failed: 0x%X\n", Err);
		system("PAUSE");
		return -2;
	}

	if (!IsCorrectTargetArchitecture(hProc)) {
		printf("Invalid Process Architecture.\n");
		CloseHandle(hProc);
		system("PAUSE");
		return -3;
	}

	printf("Mapping...\n");
	if (!ManualMap(hProc, dllPath))
	{
		CloseHandle(hProc);
		printf("Error while mapping.\n");
		system("PAUSE");
		return -4;
	}

	CloseHandle(hProc);
	printf("OK\n");
	return 0;
}
