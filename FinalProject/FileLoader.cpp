#include "FileLoader.h"
#include <PathCch.h>

#pragma comment(lib, "Pathcch.lib")

HRESULT FileLoader::getDataAndSize(LPCWSTR Vsrc, LPBYTE* ShaderData, LPLONG SizeShaderData)
{
	TCHAR* path = new TCHAR[MAX_PATH];
	DWORD Sizepath = MAX_PATH;
	DWORD val = GetModuleFileName(NULL, path, Sizepath);
	PathCchRemoveFileSpec(path, Sizepath);
	HANDLE File = CreateFile(lstrcat(path, Vsrc), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (File == INVALID_HANDLE_VALUE){
		delete[] path;
		return E_FAIL;
	}
	BY_HANDLE_FILE_INFORMATION info;
	GetFileInformationByHandle(File, &info);
	DWORD size = info.nFileSizeLow, read;
	*SizeShaderData = size;
	*ShaderData = new BYTE[size];
	ReadFile(File, *ShaderData, size, &read, NULL);
	CloseHandle(File);
	delete[] path;
	return ERROR_SUCCESS;
}
