#ifndef DIALOGS_H
#define DIALOGS_H

LRESULT CALLBACK MainWinProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK OptDlgProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
BOOL CALLBACK AboutDlgProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
int fontSize( HWND hWnd, int size );
void center_dlg( HWND hWnd );
void OnClick();

#endif // DIALOGS_H