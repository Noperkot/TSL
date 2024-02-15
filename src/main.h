#ifndef MAIN_H
#define MAIN_H
#include "options.h"

#define MAIN_WINCLASS L"TorrServerLauncher"			// Класс основного окна
#define CHIL_WINCLASS L"TSLchild"					// Класс дочерних окон. Должен совпадать с указанным в ресурсах

enum { UM_TRAYACTION = WM_USER + 1, UM_NEWLINE, UM_SERVERSTARTED, UM_SERVERSTOPPED, UM_SETSTATUS };

extern HWND hMainWnd;

bool isX64();
LPCWSTR FileExists( LPCWSTR szPath );
LPCWSTR checkTS();
void MessageBox( UINT idsText, UINT uType = MB_OK | MB_APPLMODAL );
void MessageBox( LPCWSTR text, UINT uType = MB_OK | MB_APPLMODAL );

#endif // MAIN_H