#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

char CallFunctionAssemblyCode[] = {
	0x55,					// push   ebp
	0x8b, 0xec,				// mov    ebp, esp
	0x8b, 0x45, 0x08,		// mov    eax, [ebp+0x8]
	0x8b, 0x08,				// mov    ecx, [eax]
	0x89, 0x4d, 0xfc,		// mov    [ebp-0x4], ecx
	0x8b, 0x55, 0x08,		// mov    edx, [ebp+0x8]
	0x8b, 0x42, 0x04,		// mov    eax, [edx+0x4]
	0x89, 0x45, 0xf4,		// mov    [ebp-0xC], eax
	0x8b, 0x4d, 0x08,		// mov    ecx, [ebp+0x8]
	0x8b, 0x51, 0x08,		// mov    edx, [ecx+0x8]
	0x89, 0x55, 0xf8,		// mov    [ebp-0x8], edx
	0x8b, 0x45, 0xfc,		// mov    eax, [ebp-0x4]
	0x89, 0x45, 0xf0,		// mov    [ebp-0x10], eax
	0x8b, 0x4d, 0xf8,		// mov    ecx, [ebp-0x8]
	0x51,					// push   ecx
	0x8b, 0x55, 0xf4,		// mov    edx, [ebp-0xC]
	0x52,					// push   edx
	0xff, 0x55, 0xf0,		// call   [ebp-0x10]
	0x83, 0xc4, 0x08,		// sub    esp, 0x2
	0x8b, 0xe5,				// mov    esp, ebp
	0x5d,                   // pop    ebp
	0xc3					// ret
};

struct CallFunctionArgs {
	DWORD* functionAddress;
	int* variableAddress;
	int value;
};

HANDLE GetProcess(char* processName) {
	HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);

	do {
		if (!_stricmp(entry.szExeFile, processName)) {
			CloseHandle(handle);
			return OpenProcess(PROCESS_ALL_ACCESS, false, entry.th32ProcessID);
		}
	} while (Process32Next(handle, &entry));

	return false;
}

int main() {
	HANDLE hProcess = GetProcess("cpptrash.exe");

	CallFunctionArgs callFunctionArgs;
	callFunctionArgs.functionAddress = (DWORD*)0xC315F0;
	callFunctionArgs.variableAddress = (int*)0xC35000;
	callFunctionArgs.value = 1000000;

	LPVOID* dataRegion = (LPVOID*)VirtualAllocEx(hProcess, 0, sizeof(callFunctionArgs), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	LPVOID* executableRegion = (LPVOID*)VirtualAllocEx(hProcess, 0, sizeof(CallFunctionAssemblyCode), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (!dataRegion || !executableRegion) {
		return 0;
	}

	WriteProcessMemory(hProcess, executableRegion, CallFunctionAssemblyCode, sizeof(CallFunctionAssemblyCode), 0);
	WriteProcessMemory(hProcess, dataRegion, &callFunctionArgs.functionAddress, 4, 0);
	WriteProcessMemory(hProcess, dataRegion + 0x1, &callFunctionArgs.variableAddress, 4, 0);
	WriteProcessMemory(hProcess, dataRegion + 0x2, &callFunctionArgs.value, 4, 0);

	HANDLE crt = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)executableRegion, dataRegion, 0, 0);
	
	if (!crt) {
		return 0;
	}

	WaitForSingleObject(crt, INFINITE);

	VirtualFreeEx(hProcess, dataRegion, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, executableRegion, 0, MEM_RELEASE);

	std::cout << "Done." << std::endl;
	std::cin.get();
}
