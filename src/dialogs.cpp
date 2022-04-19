#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "res/resource.h"
#include "res/commands.h"
#include "res/version.h"
#include "main.h"
#include "dialogs.h"
#include "ts.h"

LPWSTR TS_version = NULL;										// тут храним сграбленную версию TS

/************************************** MAIN ***********************************************/
LRESULT CALLBACK MainWinProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	static HWND hConsole;
	static HWND hStatusBar;
	static HMENU hFormMenu;
	static HMENU hTrayMenu;
	static HBRUSH BkBrush;	
	static HFONT hConsoleFont;
	static wchar_t tsUrl[23] = L"http://localhost:-----";
	static NOTIFYICONDATA TrayIconData = { 0 };					// Атрибуты значка
	static UINT _uTaskbarRestartMessage;
	static BYTE verChk;											// счетчик строк консоли в которых будет искаться версия TS. При 0 - не ищем.
	static LONG SB_Height;
	
	switch ( msg ) {

		case WM_CREATE: {
				// формируем структуру для иконки в трее
				TrayIconData.cbSize = sizeof ( NOTIFYICONDATA );
				TrayIconData.hWnd = hWnd;
				TrayIconData.uVersion = NOTIFYICON_VERSION;
				TrayIconData.uCallbackMessage = UM_TRAYACTION;
				TrayIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
				_uTaskbarRestartMessage = RegisterWindowMessage ( L"TaskbarCreated" );	// подписываемся на сообщение "TaskbarCreated" после падения explorer.exe
				SendMessage ( hWnd, _uTaskbarRestartMessage, 0, 0 );					// имитируем падение проводника для создания икони в трее
				hFormMenu = GetMenu ( hWnd );
				CheckMenuItem ( hFormMenu, IDM_AUTOSTART, MF_BYCOMMAND | ( Autostart() ) ? MF_CHECKED : MF_UNCHECKED ); // устанавливаем сохраненную галку автозагрузки
				hTrayMenu = LoadMenu ( NULL, MAKEINTRESOURCE ( IDR_TRAYMENU ) );		// подгружаем меню для иконки в трее

				hConsole = CreateWindow ( WC_EDIT,
				        NULL,
						ES_MULTILINE | ES_READONLY | WS_VSCROLL  | WS_CHILD | WS_VISIBLE | ( ( opt.TextWrapping ) ? 0 : WS_HSCROLL ),
				        0, 0, 0, 0,
				        hWnd,
				        NULL,
				        GetModuleHandle ( NULL ),
				        NULL );

				hConsoleFont = CreateFont ( fontSize ( hWnd, opt.ConsoleFontSize ), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
				        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
				        DEFAULT_PITCH | FF_DONTCARE, opt.ConsoleFontName );
				SendMessage ( hConsole, WM_SETFONT, ( WPARAM ) hConsoleFont, 0 );
				SendMessage ( hConsole, EM_SETLIMITTEXT, ( WPARAM ) 0, ( LPARAM ) 0 );
				
				BkBrush = CreateSolidBrush( opt.ConsoleBkColor );

				hStatusBar = CreateWindow ( STATUSCLASSNAME,
				        NULL,
				        CCS_BOTTOM | WS_CHILD | WS_VISIBLE,
				        0, 0, 0, 0,
				        hWnd,
				        NULL,
				        GetModuleHandle ( NULL ),
				        NULL );
				RECT rect;
				GetClientRect ( hStatusBar, &rect );
				SB_Height = rect.bottom - rect.top;
				int iStatusWidths[] = { 2, 2 + SB_Height * 8 / 10, -1 };				// позиции X разделителей статусбара. -1 - секция до конца
				SendMessage ( hStatusBar, SB_SETPARTS, 3, ( LPARAM ) iStatusWidths );	// разбиваем статусбар на части
				SendMessage ( hWnd, UM_SETSTATUS, ( WPARAM ) FALSE, 0 );
			}
			break;

		case WM_CTLCOLORSTATIC:
			if ( ( HWND ) lParam == hConsole ) {
				SetTextColor ( ( HDC ) wParam, opt.ConsoleFontColor );					// цвет шрифта
				SetBkMode ( ( HDC ) wParam, TRANSPARENT );				
				return (LRESULT) BkBrush;												// цвет фона
			} else return DefWindowProc ( hWnd, msg, wParam, lParam );
			break;

		case WM_SYSCOMMAND:
			switch ( wParam ) {
				case SC_CLOSE:
					if( opt.ExitWhenClose ) {
						DestroyWindow ( hWnd );
						break;
					}
				case SC_MINIMIZE:				
					SendMessage ( FindWindow ( CHIL_WINCLASS, 0 ), WM_SYSCOMMAND,  SC_CLOSE, 0 ); // перед скрытием главного окна закрываем дочерний диалог(если есть)
					ShowWindow ( hWnd, SW_HIDE );
					break;
				default:
					return DefWindowProc ( hWnd, msg, wParam, lParam );
			}
			break;

		case WM_DESTROY: {
				Shell_NotifyIcon ( NIM_DELETE, &TrayIconData );
				StopServer();
				DestroyMenu ( hTrayMenu );
				DeleteObject ( hConsoleFont );
				DeleteObject( BkBrush );				
				// сохраняем положение и размеры окна
				WINDOWPLACEMENT wndpl;
				wndpl.length = sizeof ( WINDOWPLACEMENT );
				GetWindowPlacement ( hWnd, &wndpl );
				if ( wndpl.showCmd == SW_MAXIMIZE ) reg_save ( L"WindowMax", 1 );
				else {
					RECT Rect;
					GetWindowRect ( hWnd, &Rect );
					if ( opt.WindowX != AUTO ) reg_save ( L"WindowX", Rect.left );
					if ( opt.WindowY != AUTO ) reg_save ( L"WindowY", Rect.top );
					reg_save ( L"WindowW", Rect.right - Rect.left );
					reg_save ( L"WindowH", Rect.bottom - Rect.top );
					reg_save ( L"WindowMax", 0 );
				}
				PostQuitMessage ( 0 );
			}
			break;

		case WM_COMMAND:

			switch ( LOWORD ( wParam ) ) {

				case ID_EXIT:
					DestroyWindow ( hWnd );
					break;

				case ID_SHOW:
					SetForegroundWindow ( hWnd );
					ShowWindow ( hWnd, ( GetWindowLong ( hWnd, GWL_STYLE ) & WS_MAXIMIZE ) ? SW_MAXIMIZE : SW_SHOW );
					break;

				case ID_HIDE:
					SendMessage ( hWnd, WM_SYSCOMMAND,  SC_MINIMIZE, 0 );
					break;

				case ID_SHOWHIDE:
					SendMessage ( hWnd, WM_COMMAND, ( IsWindowVisible ( hWnd ) ) ? ID_HIDE : ID_SHOW, 0 );
					break;

				case ID_STOP:
					StopServer ();
					break;
					
				case ID_START:
					if( !ServerRunning() ) {
						SetWindowText ( hConsole, L"" );
						verChk = opt.GrabVerLines;
						wStrReplace( &TS_version, L"TorrServer" );
						SetWindowText ( hWnd, TS_version );
						wcscpy ( tsUrl + 17, L"8090" );
						if ( opt.args ) { // ищем порт в параметрах командной строки
							int	 argc;
							LPWSTR * pArg = CommandLineToArgvW ( opt.args, &argc );
							argc--; // читаем до предпоследнего параметра(последний должен быть значением)
							for ( int i = 0; i < argc; i++ ) {
								if ( wcscmp ( pArg[i], L"-p" ) == 0 || wcscmp ( pArg[i], L"--port" ) == 0 ) {
									wcsncpy ( tsUrl + 17, pArg[i + 1], 5 );
									break;
								}
							}
							GlobalFree ( pArg );
						}
						StartServer ( checkTS() );
					}
					break;						

				case ID_RESTART:
					SetWindowText ( hConsole, L"" );
					SendMessage ( hWnd, WM_COMMAND, ID_STOP, 0 );
					SendMessage ( hWnd, WM_COMMAND, ID_START, 0 );
					break;

				case ID_OPENWEB:
					if( ServerRunning() ) ShellExecute ( NULL, L"open", tsUrl, NULL, NULL, SW_SHOW );
					break;

				case ID_COPY:
					SendMessage ( hConsole, EM_SETSEL, ( WPARAM ) 0, ( LPARAM ) - 1 );
					SendMessage ( hConsole, WM_COPY, ( WPARAM ) 0, ( LPARAM ) 0 );
					SendMessage ( hConsole, EM_SETSEL, ( WPARAM ) - 1, ( LPARAM ) 0 );
					break;

				case ID_CLEAR:
					SendMessage ( hConsole, WM_SETTEXT, 0, ( LPARAM ) L"" );
					break;

				case IDM_AUTOSTART:
					Autostart ( GetMenuState ( hFormMenu, IDM_AUTOSTART, MF_BYCOMMAND ) == MF_UNCHECKED );
					CheckMenuItem ( hFormMenu, IDM_AUTOSTART, MF_BYCOMMAND | ( Autostart() ) ? MF_CHECKED : MF_UNCHECKED );
					break;

				case IDM_ABOUT:
					DialogBox ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( IDD_ABOUT ), hWnd, ( DLGPROC ) AboutDlgProc );
					break;

				case IDM_OPTIONS:
					DialogBox ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( IDD_OPTIONS ), hWnd, ( DLGPROC ) OptDlgProc );
					break;

				default:
					return DefWindowProc ( hWnd, msg, wParam, lParam );
			}
			break;

		case UM_NEWLINE: {	// в wParam - новая строка в консоль (LPSTR)

				// конвертируем utf8 -> wchar
				int wsize = MultiByteToWideChar ( CP_UTF8, 0, ( LPSTR ) wParam, -1, NULL, 0 ) + 2;	// подсчитываем длину будущей вайдчарной строки (с учетом \r\n\0)
				LPWSTR wbuf = ( LPWSTR ) malloc ( wsize * sizeof ( *wbuf ) );						// выделяем память для вайдчарной строки
				MultiByteToWideChar ( CP_UTF8, 0, ( LPSTR ) wParam, -1, wbuf, wsize );				// конвертируем utf8 -> wchar, включая \0

				// ищем в присланной строке версию сервера(если еще не найдена)
				if ( verChk != 0 ) {
					LPCWSTR p = wcsstr ( wbuf, L"TorrServer" );
					if ( p ) p += 10;
					else {
						p = wcsstr ( wbuf, L"version:" );
						if ( p ) p += 8;
					}
					if ( p ) {																		// Формируем глобальную строку TS_version содержащую версию торрсервера
						free ( TS_version );
						TS_version = ( wchar_t* ) malloc ( ( 10 + wcslen ( p ) + 1 ) * sizeof ( wchar_t ) );
						wcscpy ( TS_version, L"TorrServer" );
						wcscat ( TS_version, p );
						wcstok ( TS_version, L"," );												// отсекаем мусор после запятой
						SetWindowText ( hWnd, TS_version );
						verChk = 0;
					} else verChk--;
				}
				wcscpy ( wbuf + wsize - 3, L"\r\n" );												// теперь можно добавить \r\n\0 в конец строки

				// добавляем строку в консоль
				SendMessage ( hConsole, WM_SETREDRAW, ( WPARAM ) FALSE, ( LPARAM ) 0 );				// запрещаем прорисовку
				int lCount = Edit_GetLineCount ( hConsole );										// сейчас строк в консоли
				if ( lCount > opt.MaxLines ) {														// строк больше допустимого, удаляем лишнее
					int cIdx = Edit_LineIndex ( hConsole, lCount - opt.MaxLines );
					Edit_SetSel ( hConsole, 0, cIdx );
					Edit_ReplaceSel ( hConsole, L"" );
				}
				int nLength = Edit_GetTextLength ( hConsole );
				Edit_SetSel ( hConsole, nLength, nLength );
				Edit_ReplaceSel ( hConsole, wbuf );													// вставляем в конец присланную строку
				free ( wbuf );																		// чистим вайдчарный буфер
				SendMessage ( hConsole, WM_VSCROLL, SB_BOTTOM, ( LPARAM ) NULL );					// скроллим до конца
				SendMessage ( hConsole, WM_SETREDRAW, ( WPARAM ) TRUE, ( LPARAM ) 0 );				// разрешаем прорисовку
			}
			break;

		case UM_SERVERSTARTED:																		// TS стартовал
			SendMessage ( hWnd, UM_SETSTATUS, ( WPARAM ) TRUE, 0 );
			break;

		case UM_SERVERSTOPPED:																		// TS умер
			SendMessage ( hWnd, UM_SETSTATUS, ( WPARAM ) FALSE, 0 );
			if( wParam == PROCESSTERMINATED) return 0;												// процесс TS был принудительно убит, никаких действий не требуется
			if( wParam > PROCESSTERMINATED ) SendMessage ( hWnd, WM_COMMAND, ID_SHOW, 0 );
			switch( wParam ){
				case TSNOTFOUND:
					rcMessageBox ( hWnd, ( isX64() ) ? STR_NEED_TORSERVER64 : STR_NEED_TORSERVER32, MB_OK | MB_APPLMODAL | MB_ICONERROR );				
					break;
				case CREATEPIPEERROR:
					MessageBox ( hWnd, L"CreatePipe error", _PRODUCTNAME_, MB_OK | MB_APPLMODAL | MB_ICONERROR );
					break;
				case CREATEPROCESSERROR:
					MessageBox ( hWnd, L"CreateProcess error", _PRODUCTNAME_, MB_OK | MB_APPLMODAL | MB_ICONERROR );
					break;
			}
			switch ( opt.OnTSdead ) {
				case ONDEAD_CLOSE:
					SendMessage ( hWnd, WM_COMMAND, ID_EXIT, 0 );
					break;
				case ONDEAD_SHOW:
					SendMessage ( hWnd, WM_COMMAND, ID_SHOW, 0 );
					break;					
				case ONDEAD_RESTART:
					SendMessage ( hWnd, WM_COMMAND, ID_RESTART, 0 );
					break;
			}
			break;

		case UM_SETSTATUS:	{
				LPWSTR StatusText = NULL;
				WORD working_icon, tray_icon;
				UINT MenuWebOpenEn;
				if ( wParam ) {
					res_load ( STR_WORKING, &StatusText );
					working_icon = CGREN_ICON;
					tray_icon = FAV_ICON;
					MenuWebOpenEn = MF_ENABLED;
				} else {
					res_load ( STR_STOPPED, &StatusText );
					working_icon = CRED_ICON;
					tray_icon = INACTIVE_ICON;
					MenuWebOpenEn = MF_GRAYED;
				}
				
				int iconSize = SB_Height * 5 / 10;
				static HICON hStatusIcon = NULL;
				DestroyIcon( hStatusIcon );
				hStatusIcon = ( HICON ) LoadImage (
				        GetModuleHandle ( NULL ),
				        MAKEINTRESOURCE ( working_icon ),
				        IMAGE_ICON,
				        iconSize,
				        iconSize,
				        LR_DEFAULTCOLOR
				    );

				SendMessage ( hStatusBar, SB_SETTEXT, 2, ( LPARAM ) StatusText );
				SendMessage ( hStatusBar, SB_SETICON, 1, ( LPARAM ) hStatusIcon );
				
				wcscpy ( TrayIconData.szTip, L"TorrServer " );
				wcscat ( TrayIconData.szTip, StatusText );
				DestroyIcon( TrayIconData.hIcon );
				TrayIconData.hIcon = LoadIcon ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( tray_icon ) );
				Shell_NotifyIcon ( NIM_MODIFY, &TrayIconData );
				free( StatusText );
				EnableMenuItem ( hFormMenu, ID_OPENWEB, MF_BYCOMMAND | MenuWebOpenEn );
				EnableMenuItem ( hTrayMenu, ID_OPENWEB, MF_BYCOMMAND | MenuWebOpenEn );
			}
			break;

		case UM_TRAYACTION: // действие над иконкой в трее
			switch ( lParam ) {
				case WM_RBUTTONUP: {
						POINT pCursor;
						GetCursorPos ( &pCursor );
						SetForegroundWindow ( hWnd ); // нужно чтобы меню на иконке скрывалось при клике в любую точку кроме него.
						TrackPopupMenu ( GetSubMenu ( hTrayMenu, 0 ), TPM_LEFTBUTTON | TPM_RIGHTALIGN, pCursor.x, pCursor.y, 0, hWnd, NULL );
						PostMessage ( hWnd, WM_NULL, 0, 0 ); // otherwise menu locks hDlg ???????
					}
					break;
				case WM_LBUTTONUP:
					if( !opt.DblIconClick ) OnClick();
					break;
				case WM_LBUTTONDBLCLK:
					if( opt.DblIconClick )  OnClick();
					break;
			}
			break;

		case WM_SIZE: { // ресайз окна
				RECT rect;
				GetClientRect ( hWnd, &rect );
				LONG x = rect.right - rect.left;
				LONG y = rect.bottom - rect.top - SB_Height;
				SetWindowPos ( hConsole, HWND_TOP, 0, 0, x, y, SWP_FRAMECHANGED );
				SendMessage ( hStatusBar, WM_SIZE, 0, 0 );
			}
			break;

		case WM_GETMINMAXINFO: // минимальные размеры окна
			( ( MINMAXINFO* ) lParam )->ptMinTrackSize.x = opt.WindowMinW;
			( ( MINMAXINFO* ) lParam )->ptMinTrackSize.y = opt.WindowMinH;
			break;

		default:
			if ( msg == _uTaskbarRestartMessage ) {					// восстановление иконки после падения проводника
				Shell_NotifyIcon ( NIM_ADD, &TrayIconData );
				break;
			}
			return DefWindowProc ( hWnd, msg, wParam, lParam );
	}
	return 0;
}

/************************************** OPTIONS ***********************************************/
BOOL CALLBACK OptDlgProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch ( msg ) {

		case WM_INITDIALOG: {
				SendMessage ( hWnd, WM_SETICON, ICON_SMALL, ( LPARAM ) LoadIcon ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( FAV_ICON ) ) );	// 	иконку в заголовок окна
				LPWSTR buf = NULL;
				// локализуем из ресурсов
				SetWindowText ( hWnd, res_load ( STR_OPTIONS, &buf ) );
				SetDlgItemText ( hWnd, IDC_CMDLINE_LABEL, res_load ( STR_TSARGS, &buf ) );
				SetDlgItemText ( hWnd, IDCANCEL, res_load ( STR_CANCEL, &buf ) );
				free ( buf );
				SetDlgItemText ( hWnd, IDC_ARGS, opt.args );
				center_dlg ( hWnd );
			}
			break;

		case WM_COMMAND:
			switch ( wParam ) {
				case IDOK: {
						int sz = GetWindowTextLength ( GetDlgItem ( hWnd, IDC_ARGS ) ) + 1;
						free ( opt.args );
						opt.args = ( LPWSTR ) malloc ( sz * sizeof ( *opt.args ) );
						GetDlgItemText ( hWnd, IDC_ARGS, opt.args, sz );
						reg_save ( L"args", opt.args );
					}
				case IDCANCEL:
					EndDialog ( hWnd, 0 );
					break;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}


/************************************** ABOUT ***********************************************/
#define TSL_URL L"https://github.com/Noperkot/TSL"
#define TS_URL  L"https://github.com/YouROK/TorrServer"
BOOL CALLBACK AboutDlgProc ( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	static HFONT hFont;

	switch ( msg ) {

		case WM_INITDIALOG: {
				SendMessage ( hWnd, WM_SETICON, ICON_SMALL, ( LPARAM ) LoadIcon ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( FAV_ICON ) ) );	// 	иконку в заголовок окна
				LPWSTR caption = res_load ( STR_ABOUT );
				SetWindowText ( hWnd, caption );
				free ( caption );
				hFont = CreateFont ( fontSize ( hWnd, 8 ), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft Sans Serif" );
				SendDlgItemMessage ( hWnd, IDC_TSVERSION,  WM_SETFONT, ( WPARAM ) hFont, ( LPARAM ) TRUE );
				SendDlgItemMessage ( hWnd, IDC_TSLVERSION, WM_SETFONT, ( WPARAM ) hFont, ( LPARAM ) TRUE );
				SetDlgItemText ( hWnd, IDC_TSLVERSION, _PRODUCTNAME_ L" v" _PRODUCTVERSION_ );
				SetDlgItemText ( hWnd, IDC_TSLCOPYRIGHT, _LEGALCOPYRIGHT_ );
				SetDlgItemText ( hWnd, IDC_TSVERSION, TS_version );
				SetDlgItemText ( hWnd, IDC_TSCOPYRIGHT, L"Copyright \x00A9 YouROK" );
				// SetDlgItemText ( hWnd, IDC_TSWWWLINK,  L"<a>" TS_URL  L"</a>" );
				// SetDlgItemText ( hWnd, IDC_TSLWWWLINK, L"<a>" TSL_URL L"</a>" );
				center_dlg ( hWnd );
			}
			break;

		case WM_DESTROY:
			DeleteObject ( hFont );
			break;

		case WM_COMMAND:
			switch ( wParam ) {
				case IDOK:
				case IDCANCEL:
					EndDialog ( hWnd, 0 );
					break;
				default:
					return FALSE;
			}
			break;

		case WM_NOTIFY:
			switch ( ( ( NMHDR * ) lParam )->code ) {
				case NM_CLICK:
					switch ( LOWORD ( wParam ) ) {
						case IDC_TSWWWLINK:
							ShellExecute ( NULL, L"open", TS_URL, NULL, NULL, SW_SHOW );
							// EndDialog ( hWnd, 0 );
							break;
						case IDC_TSLWWWLINK:
							ShellExecute ( NULL, L"open", TSL_URL, NULL, NULL, SW_SHOW );
							// EndDialog ( hWnd, 0 );
							break;
						default:
							return FALSE;
					}
					break;
				default:
					return FALSE;
			}
			break;

		default:
			return FALSE;
	}
	return TRUE;
}

int fontSize( HWND hWnd, int size ){
	HDC hDC = GetDC( hWnd );
	int height = -MulDiv( size, GetDeviceCaps( hDC,LOGPIXELSY ), 72 ); 
	ReleaseDC( hWnd, hDC );
	return height;
}

void center_dlg( HWND hWnd ){
	RECT rc, rcDlg, rcOwner;
	HWND hWndOwner = GetWindow ( hWnd, GW_OWNER );
	GetWindowRect ( hWndOwner, &rcOwner );
	GetWindowRect ( hWnd, &rcDlg );
	CopyRect ( &rc, &rcOwner );
	OffsetRect ( &rcDlg, -rcDlg.left, -rcDlg.top );
	OffsetRect ( &rc, -rc.left, -rc.top );
	OffsetRect ( &rc, -rcDlg.right, -rcDlg.bottom );
	SetWindowPos ( hWnd, HWND_TOP,
		rcOwner.left + ( rc.right / 2 ),
		rcOwner.top + ( rc.bottom / 2 ),
		0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

void OnClick(){
	switch( opt.OnIconClick ) {
		case ONCLICK_SHOWHIDE:
			SendMessage ( hMainWnd, WM_COMMAND, ( WPARAM ) ID_SHOWHIDE, 0 );
			break;
		case ONCLICK_OPENWEB:
			SendMessage ( hMainWnd, WM_COMMAND, ( WPARAM ) ID_OPENWEB, 0 );
			break;
		case ONCLICK_RESTART:
			SendMessage ( hMainWnd, WM_COMMAND, ( WPARAM ) ID_RESTART, 0 );
			break;
	}
}
