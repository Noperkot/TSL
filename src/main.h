#ifndef MAIN_H
#define MAIN_H
#include "options.h"

#define MAIN_WINCLASS L"TorrServerLauncher"			// Класс основного окна
#define CHIL_WINCLASS L"TSLchild"					// Класс дочерних окон. Должен совпадать с указанным в ресурсах

enum { UM_TRAYACTION = WM_USER + 1, UM_NEWLINE, UM_SERVERSTARTED, UM_SERVERSTOPPED, UM_SETSTATUS };

extern HWND hMainWnd;

LPWSTR wStrReplace( LPWSTR *dst, LPCWSTR str1 = NULL, LPCWSTR str2 = L"", LPCWSTR str3 = L"" );
LPWSTR rcString( LPWSTR *dst, UINT idsText );
bool isX64();
LPCWSTR FileExists( LPCWSTR szPath );
LPCWSTR checkTS();
void MessageBox( UINT idsText, UINT uType = MB_OK | MB_APPLMODAL );
void MessageBox( LPCWSTR text, UINT uType = MB_OK | MB_APPLMODAL );

#endif // MAIN_H