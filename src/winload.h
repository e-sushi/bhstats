#pragma once

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <TlHelp32.h>   
#include <string>


// main repository for anything to do with getting information from a window

//do this in a less silly way later
struct csgoProcInfo {
	DWORD pid;
	DWORD clientBaseAddr;
	HANDLE handle;
};

namespace winload {

	struct HandleAndPid { HANDLE handle; DWORD pid; };

	HandleAndPid GetProcessByName(const char* name) {
		DWORD pid = 0;

		HandleAndPid hap;

		// Create toolhelp snapshot.
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 process;
		ZeroMemory(&process, sizeof(process));
		process.dwSize = sizeof(process);

		// Walkthrough all processes.
		if (Process32First(snapshot, &process)) {
			do {
				// Compare process.szExeFile based on format of name, i.e., trim file path
				// trim .exe if necessary, etc.

				//scuffed fix to a wide char problem i should probably change later
				std::wstring ws(process.szExeFile);
				std::string str(ws.begin(), ws.end());
				if (str == std::string(name)) {
					pid = process.th32ProcessID;
					break;
				}
			} while (Process32Next(snapshot, &process));
		}

		CloseHandle(snapshot);

		hap.pid = pid;

		if (pid != 0) {
			hap.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
			return hap;
		}

		//return NULL;
	}

	uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName) {
		uintptr_t modBaseAddr = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);

		if (hSnap != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 modEntry;
			modEntry.dwSize = sizeof(modEntry);

			if (Module32First(hSnap, &modEntry)) {
				do {
					if (!_wcsicmp(modEntry.szModule, modName)) {
						modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
						break;
					}
				} while (Module32Next(hSnap, &modEntry));
			}
		}

		CloseHandle(hSnap);
		return modBaseAddr;
	}

	csgoProcInfo GetCSGOInfo() {
		csgoProcInfo cpi;
		HandleAndPid hap = GetProcessByName("csgo.exe");

		cpi.handle = hap.handle;
		cpi.pid = hap.pid;
		cpi.clientBaseAddr = GetModuleBaseAddress(cpi.pid, L"client.dll");

		return cpi;
	}



}