#pragma once
#include <Windows.h>

namespace FileLoader
{
	HRESULT getDataAndSize(LPCWSTR Vsrc, LPBYTE* ShaderData, LPLONG SizeShaderData);
}

