#ifndef MAIN_H
#define MAIN_H
#include "options.h"

#define MAIN_WINCLASS L"TorrServerLauncher"			// Класс основного окна
#define CHIL_WINCLASS L"TSLchild"					// Класс дочерних окон. Должен совпадать с указанным в ресурсах

enum { UM_TRAYACTION = WM_USER + 1, UM_NEWLINE, UM_SERVERSTARTED, UM_SERVERSTOPPED, UM_SETSTATUS };

extern OPTIONS_S opt;
extern HWND hMainWnd;

LPWSTR wStrReplace ( LPWSTR *dst, LPCWSTR src );
LPWSTR res_load( UINT idsText, LPWSTR *buf = NULL );
bool isX64();
LPCWSTR FileExists ( LPCWSTR szPath );
LPCWSTR checkTS();
LPWSTR ExePath( LPCWSTR prefix = NULL, LPCWSTR postfix = NULL );
void rcMessageBox( HWND hWnd, UINT idsText, UINT uType );

#endif // MAIN_H