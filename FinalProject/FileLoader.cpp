#include "FileLoader.h"
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

//Función para leer archivos binarios, obtener su contenido y tamaño
HRESULT FileLoader::getDataAndSize(LPCWSTR Vsrc, LPBYTE* ShaderData, LPLONG SizeShaderData)
{
	TCHAR path[MAX_PATH];
	DWORD Sizepath = MAX_PATH;
	DWORD val = GetModuleFileName(NULL, path, Sizepath); //Se obtiene la ruta donde está la aplicación
	PathRemoveFileSpec(path);//Se remueve el nombre del archivo y nos quedamos con la ruta de la carpeta
	//donde está contenido la aplicación
	//lstrcat concatena el directoria con el nombre del archivo Vsrc
	HANDLE File = CreateFile(lstrcat(path, Vsrc), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (File == INVALID_HANDLE_VALUE){
		return E_FAIL; //Si no lo encontró retorna error
	}
	BY_HANDLE_FILE_INFORMATION info;
	GetFileInformationByHandle(File, &info); //Se obtiene información del tamaño del archivo
	DWORD size = info.nFileSizeLow, read;
	*SizeShaderData = size;
	*ShaderData = new BYTE[size];
	ReadFile(File, *ShaderData, size, &read, NULL); //se lee y almacena la información en ShaderData
	CloseHandle(File);//Se cierra el archivo
	return ERROR_SUCCESS;
}
