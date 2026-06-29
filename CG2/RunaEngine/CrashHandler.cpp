#include "CrashHandler.h"
#include <Windows.h>
#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include <strsafe.h>

namespace {
LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);

	wchar_t filePath[MAX_PATH] = {};
	if (!CreateDirectoryW(L"./Dumps", nullptr)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			OutputDebugStringW(L"Failed to create Dumps directory.\n");
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	StringCchPrintfW(
		filePath,
		MAX_PATH,
		L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute
	);

	HANDLE dumpFileHandle = CreateFileW(
		filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		nullptr,
		CREATE_ALWAYS,
		0,
		nullptr
	);
	if (dumpFileHandle == INVALID_HANDLE_VALUE) {
		OutputDebugStringW(L"Failed to create dump file.\n");
		return EXCEPTION_EXECUTE_HANDLER;
	}

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{};
	minidumpInformation.ThreadId = GetCurrentThreadId();
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	BOOL isDumpSaved = MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		dumpFileHandle,
		MiniDumpNormal,
		&minidumpInformation,
		nullptr,
		nullptr
	);

	if (!isDumpSaved) {
		OutputDebugStringW(L"Failed to write minidump.\n");
	} else {
		OutputDebugStringW(L"Successfully saved minidump.\n");
	}

	CloseHandle(dumpFileHandle);
	return EXCEPTION_EXECUTE_HANDLER;
}
}

void CrashHandler::Initialize() {
	SetUnhandledExceptionFilter(ExportDump);
}
