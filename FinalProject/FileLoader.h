#pragma once
#include <Windows.h>

namespace FileLoader
{
	//Función para leer archivos binarios
	HRESULT getDataAndSize(LPCWSTR Vsrc, LPBYTE* ShaderData, LPLONG SizeShaderData);
}

