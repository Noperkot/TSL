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

static wStr TS_version( L"TorrServer" );										// тут храним версию TS

/************************************** MAIN ***********************************************/
LRESULT CALLBACK MainWinProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
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

	switch( msg ) {

	case WM_CREATE: {
		TrayIconData.cbSize = sizeof( NOTIFYICONDATA );
		TrayIconData.hWnd = hWnd;
		TrayIconData.uVersion = NOTIFYICON_VERSION;
		TrayIconData.uCallbackMessage = UM_TRAYACTION;
		TrayIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		_uTaskbarRestartMessage = RegisterWindowMessage( L"TaskbarCreated" );	// подписываемся на сообщение "TaskbarCreated" после падения explorer.exe
		SendMessage( hWnd, _uTaskbarRestartMessage, 0, 0 );						// имитируем падение проводника для создания икони в трее
		hFormMenu = GetMenu( hWnd );
		hTrayMenu = LoadMenu( NULL, MAKEINTRESOURCE( IDR_TRAYMENU ) );			// подгружаем меню для иконки в трее
		hConsole = CreateWindow( WC_EDIT,
		                         NULL,
		                         ES_MULTILINE | ES_READONLY | WS_VSCROLL  | WS_CHILD | WS_VISIBLE | ( ( opt.TextWrapping.get() ) ? 0 : WS_HSCROLL ),
		                         0, 0, 0, 0,
		                         hWnd,
		                         NULL,
		                         GetModuleHandle( NULL ),
		                         NULL );
		hConsoleFont = CreateFont( fontSize( hWnd, opt.ConsoleFontSize.get() ), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		                           CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		                           DEFAULT_PITCH | FF_DONTCARE, opt.ConsoleFontName.get() );
		SendMessage( hConsole, WM_SETFONT, ( WPARAM ) hConsoleFont, 0 );
		SendMessage( hConsole, EM_SETLIMITTEXT, ( WPARAM ) 0, ( LPARAM ) 0 );
		BkBrush = CreateSolidBrush( opt.ConsoleBkColor.get() );
		hStatusBar = CreateWindow( STATUSCLASSNAME,
		                           NULL,
		                           CCS_BOTTOM | WS_CHILD | WS_VISIBLE,
		                           0, 0, 0, 0,
		                           hWnd,
		                           NULL,
		                           GetModuleHandle( NULL ),
		                           NULL );
		RECT rect;
		GetClientRect( hStatusBar, &rect );
		SB_Height = rect.bottom - rect.top;
		int iStatusWidths[] = { 2, 2 + SB_Height * 8 / 10, -1 };				// позиции X разделителей статусбара. -1 - секция до конца
		SendMessage( hStatusBar, SB_SETPARTS, 3, ( LPARAM ) iStatusWidths );	// разбиваем статусбар на части
		SendMessage( hWnd, UM_SETSTATUS, ( WPARAM ) IDS_STOPPED, 0 );
	}
	break;

	case WM_CTLCOLORSTATIC:
		if( ( HWND ) lParam == hConsole ) {
			SetTextColor( ( HDC ) wParam, opt.ConsoleFontColor.get() );			// цвет шрифта
			SetBkMode( ( HDC ) wParam, TRANSPARENT );
			return ( LRESULT ) BkBrush;											// цвет фона
		} else return DefWindowProc( hWnd, msg, wParam, lParam );
		break;

	case WM_SYSCOMMAND:
		switch( wParam ) {
		case SC_CLOSE:
			if( opt.ExitWhenClose.get() ) {
				DestroyWindow( hWnd );
				break;
			}
		case SC_MINIMIZE:
			SendMessage( FindWindow( CHIL_WINCLASS, 0 ), WM_SYSCOMMAND,  SC_CLOSE, 0 );   // перед скрытием главного окна закрываем дочерний диалог(если есть)
			ShowWindow( hWnd, SW_HIDE );
			break;
		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;

	case WM_DESTROY: {
		Shell_NotifyIcon( NIM_DELETE, &TrayIconData );
		StopServer();
		DestroyMenu( hTrayMenu );
		DeleteObject( hConsoleFont );
		DeleteObject( BkBrush );
		// сохраняем положение и размеры окна
		WINDOWPLACEMENT wndpl;
		wndpl.length = sizeof( WINDOWPLACEMENT );
		GetWindowPlacement( hWnd, &wndpl );
		if( wndpl.showCmd == SW_MAXIMIZE ) opt.WindowMax.save( 1 );
		else {
			RECT Rect;
			GetWindowRect( hWnd, &Rect );
			if( opt.WindowX.get() != AUTO ) opt.WindowX.save( Rect.left );
			if( opt.WindowY.get() != AUTO ) opt.WindowY.save( Rect.top );
			opt.WindowW.save( Rect.right - Rect.left );
			opt.WindowH.save( Rect.bottom - Rect.top );
			opt.WindowMax.save( 0 );
		}
		PostQuitMessage( 0 );
	}
	break;

	case WM_COMMAND:

		switch( LOWORD( wParam ) ) {

		case ID_EXIT:
			DestroyWindow( hWnd );
			break;

		case ID_SHOW:
			SetForegroundWindow( hWnd );
			ShowWindow( hWnd, ( GetWindowLong( hWnd, GWL_STYLE ) & WS_MAXIMIZE ) ? SW_MAXIMIZE : SW_SHOW );
			break;

		case ID_HIDE:
			SendMessage( hWnd, WM_SYSCOMMAND,  SC_MINIMIZE, 0 );
			break;

		case ID_SHOWHIDE:
			SendMessage( hWnd, WM_COMMAND, ( IsWindowVisible( hWnd ) ) ? ID_HIDE : ID_SHOW, 0 );
			break;

		case ID_STOP:
			StopServer();
			break;

		case ID_START:
			if( !ServerRunning() ) {
				SetWindowText( hConsole, L"" );
				verChk = opt.GrabVerLines.get();
				wcscpy( tsUrl + 17, L"8090" );
				wStr tmp( opt.args.get() );
				LPWSTR p = wcstok( tmp.get(), L" =" );
				while( p ) {  // ищем порт в параметрах запуска TS
					if( wcscmp( p, L"-p" ) == 0 || wcscmp( p, L"--p" ) == 0 || wcscmp( p, L"-port" ) == 0 || wcscmp( p, L"--port" ) == 0 ) {
						p = wcstok( NULL, L" =" );
						if( p && _wtoi( p ) ) wcsncpy( tsUrl + 17, p, 5 );
						break;
					}
					p = wcstok( NULL, L" =" );
				}
				PostMessage( hWnd, UM_SETSTATUS, ( WPARAM ) IDS_STARTING, 0 );
				StartServer( checkTS() );
			}
			break;

		case ID_RESTART:
			SetWindowText( hConsole, L"" );
			SendMessage( hWnd, WM_COMMAND, ID_STOP, 0 );
			SendMessage( hWnd, WM_COMMAND, ID_START, 0 );
			break;

		case ID_OPENWEB:
			if( ServerRunning() ) ShellExecute( NULL, L"open", tsUrl, NULL, NULL, SW_SHOW );
			break;

		case ID_COPY:
			SendMessage( hConsole, EM_SETSEL, ( WPARAM ) 0, ( LPARAM ) - 1 );
			SendMessage( hConsole, WM_COPY, ( WPARAM ) 0, ( LPARAM ) 0 );
			SendMessage( hConsole, EM_SETSEL, ( WPARAM ) - 1, ( LPARAM ) 0 );
			break;

		case ID_CLEAR:
			SendMessage( hConsole, WM_SETTEXT, 0, ( LPARAM ) L"" );
			break;

		case IDM_AUTOSTART:
			Autostart( GetMenuState( hFormMenu, IDM_AUTOSTART, MF_BYCOMMAND ) == MF_UNCHECKED );
			break;

		case IDM_ABOUT:
			DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_ABOUT ), hWnd, ( DLGPROC ) AboutDlgProc );
			break;

		case IDM_OPTIONS:
			DialogBox( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDD_OPTIONS ), hWnd, ( DLGPROC ) OptDlgProc );
			break;

		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
		}
		break;

	case UM_NEWLINE: {	// в wParam - новая строка в консоль (LPSTR)

		// конвертируем utf8 -> wchar
		int wsize = MultiByteToWideChar( CP_UTF8, 0, ( LPSTR ) wParam, -1, NULL, 0 ) + 2;	// подсчитываем длину будущей вайдчарной строки (с учетом \r\n\0)
		LPWSTR wbuf = ( LPWSTR ) malloc( wsize * sizeof( *wbuf ) );							// выделяем память для вайдчарной строки
		MultiByteToWideChar( CP_UTF8, 0, ( LPSTR ) wParam, -1, wbuf, wsize );				// конвертируем utf8 -> wchar, включая \0

		// ищем в присланной строке версию сервера(если еще не найдена)
		if( verChk != 0 ) {
			LPCWSTR p = wcsstr( wbuf, L"TorrServer" );
			if( p ) p += 10;
			else {
				p = wcsstr( wbuf, L"version:" );
				if( p ) p += 8;
			}
			if( p ) {																		// Формируем глобальную строку TS_version содержащую версию торрсервера
				TS_version.set( L"TorrServer", p );
				wcstok( TS_version.get(), L"," );											// отсекаем мусор после запятой
				SendMessage( hWnd, UM_SETSTATUS, ( WPARAM ) IDS_WORKING, 0 );
				verChk = 0;
			} else verChk--;
		}
		wcscpy( wbuf + wsize - 3, L"\r\n" );												// теперь можно добавить \r\n\0 в конец строки

		// добавляем строку в консоль
		SendMessage( hConsole, WM_SETREDRAW, ( WPARAM ) FALSE, ( LPARAM ) 0 );				// запрещаем прорисовку
		int lCount = Edit_GetLineCount( hConsole );											// сейчас строк в консоли
		if( lCount > opt.MaxLines.get() ) {													// строк больше допустимого, удаляем лишнее
			int cIdx = Edit_LineIndex( hConsole, lCount - opt.MaxLines.get() );
			Edit_SetSel( hConsole, 0, cIdx );
			Edit_ReplaceSel( hConsole, L"" );
		}
		int nLength = Edit_GetTextLength( hConsole );
		Edit_SetSel( hConsole, nLength, nLength );
		Edit_ReplaceSel( hConsole, wbuf );													// вставляем в конец присланную строку
		free( wbuf );																		// чистим вайдчарный буфер
		SendMessage( hConsole, WM_VSCROLL, SB_BOTTOM, ( LPARAM ) NULL );					// скроллим до конца
		SendMessage( hConsole, WM_SETREDRAW, ( WPARAM ) TRUE, ( LPARAM ) 0 );				// разрешаем прорисовку
	}
	break;

	case UM_SERVERSTARTED:																	// TS стартовал
		SendMessage( hWnd, UM_SETSTATUS, ( WPARAM ) IDS_WORKING, 0 );
		break;

	case UM_SERVERSTOPPED:																	// TS умер
		TS_version.set( L"TorrServer" );
		SendMessage( hWnd, UM_SETSTATUS, ( WPARAM ) IDS_STOPPED, 0 );
		if( wParam == PROCESSTERMINATED ) return 0;											// процесс TS был принудительно убит, никаких действий не требуется
		if( wParam > PROCESSTERMINATED ) SendMessage( hWnd, WM_COMMAND, ID_SHOW, 0 );
		switch( wParam ) {
		case TSNOTFOUND:
			MessageBox( ( isX64() ) ? IDS_NEED_TORSERVER64 : IDS_NEED_TORSERVER32, MB_OK | MB_APPLMODAL | MB_ICONERROR );
			break;
		case CREATEPIPEERROR:
			MessageBox( L"CreatePipe error", MB_OK | MB_APPLMODAL | MB_ICONERROR );
			break;
		case CREATEPROCESSERROR:
			MessageBox( L"Create TorrServer process error", MB_OK | MB_APPLMODAL | MB_ICONERROR );
			break;
		}
		switch( opt.OnTSdead.get() ) {
		case ONDEAD_CLOSE:
			SendMessage( hWnd, WM_COMMAND, ID_EXIT, 0 );
			break;
		case ONDEAD_SHOW:
			SendMessage( hWnd, WM_COMMAND, ID_SHOW, 0 );
			break;
		case ONDEAD_RESTART:
			SendMessage( hWnd, WM_COMMAND, ID_RESTART, 0 );
			break;
		}
		break;

	case UM_SETSTATUS:	{
		struct {
			WORD sbIcon;
			WORD trIcon;
			UINT webEn;
		} st;
		if( wParam == IDS_WORKING )				st = {IDI_CGRN, IDI_TSLOGO, MF_ENABLED};
		else if( wParam == IDS_STARTING )		st = {IDI_CGRN, IDI_TSLOGO, MF_GRAYED};
		else /* ( wParam == IDS_STOPPED ) */	st = {IDI_CRED, IDI_TSSTOP, MF_GRAYED};
		int sbIconSize = SB_Height * 5 / 10;
		static HICON hStatusIcon = NULL;
		DestroyIcon( hStatusIcon );
		hStatusIcon = ( HICON ) LoadImage(
		                  GetModuleHandle( NULL ),
		                  MAKEINTRESOURCE( st.sbIcon ),
		                  IMAGE_ICON,
		                  sbIconSize,
		                  sbIconSize,
		                  LR_DEFAULTCOLOR
		              );
		SendMessage( hStatusBar, SB_SETICON, 1, ( LPARAM ) hStatusIcon );
		SendMessage( hStatusBar, SB_SETTEXT, 2, ( LPARAM ) wStr( wParam ).get() );
		wcscpy( TrayIconData.szTip, TS_version.get() );
		DestroyIcon( TrayIconData.hIcon );
		TrayIconData.hIcon = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( st.trIcon ) );
		Shell_NotifyIcon( NIM_MODIFY, &TrayIconData );
		EnableMenuItem( hFormMenu, ID_OPENWEB, MF_BYCOMMAND | st.webEn );
		EnableMenuItem( hTrayMenu, ID_OPENWEB, MF_BYCOMMAND | st.webEn );
		SetWindowText( hWnd, TS_version.get() );
	}
	break;

	case UM_TRAYACTION: // действие над иконкой в трее
		switch( lParam ) {
		case WM_RBUTTONUP: {
			POINT pCursor;
			GetCursorPos( &pCursor );
			SetForegroundWindow( hWnd );  // нужно чтобы меню на иконке скрывалось при клике в любую точку кроме него.
			TrackPopupMenu( GetSubMenu( hTrayMenu, 0 ), TPM_LEFTBUTTON | TPM_RIGHTALIGN, pCursor.x, pCursor.y, 0, hWnd, NULL );
			PostMessage( hWnd, WM_NULL, 0, 0 );  // otherwise menu locks hDlg ???????
		}
		break;
		case WM_LBUTTONUP:
			if( !opt.DblIconClick.get() ) OnClick();
			break;
		case WM_LBUTTONDBLCLK:
			if( opt.DblIconClick.get() )  OnClick();
			break;
		}
		break;

	case WM_SIZE: { // ресайз окна
		RECT rect;
		GetClientRect( hWnd, &rect );
		LONG x = rect.right - rect.left;
		LONG y = rect.bottom - rect.top - SB_Height;
		SetWindowPos( hConsole, HWND_TOP, 0, 0, x, y, SWP_FRAMECHANGED );
		SendMessage( hStatusBar, WM_SIZE, 0, 0 );
	}
	break;

	case WM_GETMINMAXINFO: // минимальные размеры окна
		( ( MINMAXINFO* ) lParam )->ptMinTrackSize.x = opt.WindowMinW.get();
		( ( MINMAXINFO* ) lParam )->ptMinTrackSize.y = opt.WindowMinH.get();
		break;

	case WM_INITMENUPOPUP: // клик по меню
		if( ( HMENU )wParam == GetSubMenu( hFormMenu, 1 ) ) {  // это подменю настроек
			CheckMenuItem( ( HMENU )wParam, IDM_AUTOSTART, MF_BYCOMMAND | ( Autostart() ) ? MF_CHECKED : MF_UNCHECKED );  // устанавливаем галку автозагрузки в соответствии со значением в реестре
		}
		break;

	default:
		if( msg == _uTaskbarRestartMessage ) {					// восстановление иконки после падения проводника
			Shell_NotifyIcon( NIM_ADD, &TrayIconData );
			break;
		}
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}
	return 0;
}

/************************************** OPTIONS ***********************************************/
BOOL CALLBACK OptDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	switch( msg ) {

	case WM_INITDIALOG: {
		SendMessage( hWnd, WM_SETICON, ICON_BIG, ( LPARAM ) LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TSLOGO ) ) );	// 	иконку в заголовок окна
		// локализуем из ресурсов
		SetWindowText( hWnd, wStr( IDS_OPTIONS ).get() );
		SetDlgItemText( hWnd, IDC_CMDLINE_LABEL, wStr( IDS_TSARGS ).get() );
		SetDlgItemText( hWnd, IDCANCEL, wStr( IDS_CANCEL ).get() );
		SetDlgItemText( hWnd, IDC_ARGS, opt.args.get() );
		center_dlg( hWnd );
	}
	break;

	case WM_COMMAND:
		switch( wParam ) {
		case IDOK: {
			int sz = GetWindowTextLength( GetDlgItem( hWnd, IDC_ARGS ) ) + 1;
			LPWSTR tmp = ( LPWSTR ) malloc( sz * sizeof( wchar_t ) );
			GetDlgItemText( hWnd, IDC_ARGS, tmp, sz );
			opt.args.save( tmp );
			free( tmp );
		}
		case IDCANCEL:
			EndDialog( hWnd, 0 );
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

/************************************** ABOUT ***********************************************/

BOOL CALLBACK AboutDlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {

	static HFONT hFont;

	switch( msg ) {

	case WM_INITDIALOG: {
		SendMessage( hWnd, WM_SETICON, ICON_BIG, ( LPARAM ) LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDI_TSLOGO ) ) );	// 	иконку в заголовок окна
		SetWindowText( hWnd, wStr( IDS_ABOUT ).get() );
		hFont = CreateFont( fontSize( hWnd, 8 ), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft Sans Serif" );
		SendDlgItemMessage( hWnd, IDC_TSVERSION,  WM_SETFONT, ( WPARAM ) hFont, ( LPARAM ) TRUE );
		SendDlgItemMessage( hWnd, IDC_TSLVERSION, WM_SETFONT, ( WPARAM ) hFont, ( LPARAM ) TRUE );
		SetDlgItemText( hWnd, IDC_TSLVERSION, _PRODUCTNAME_ L" v" _PRODUCTVERSION_ );
		SetDlgItemText( hWnd, IDC_TSLCOPYRIGHT, _LEGALCOPYRIGHT_ );
		SetDlgItemText( hWnd, IDC_TSVERSION, TS_version.get() );
		SetDlgItemText( hWnd, IDC_TSCOPYRIGHT, L"Copyright \x00A9 YouROK" );
		// SetDlgItemText ( hWnd, IDC_TSWWWLINK,  L"<a>" _TS_URL_  L"</a>" );
		// SetDlgItemText ( hWnd, IDC_TSLWWWLINK, L"<a>" _TSL_URL_ L"</a>" );
		center_dlg( hWnd );
	}
	break;

	case WM_DESTROY:
		DeleteObject( hFont );
		break;

	case WM_COMMAND:
		switch( wParam ) {
		case IDOK:
		case IDCANCEL:
			EndDialog( hWnd, 0 );
			break;
		default:
			return FALSE;
		}
		break;

	case WM_NOTIFY:
		switch( ( ( NMHDR * ) lParam )->code ) {
		case NM_CLICK:
			switch( LOWORD( wParam ) ) {
			case IDC_TSWWWLINK:
				ShellExecute( NULL, L"open", _TS_URL_, NULL, NULL, SW_SHOW );
				// EndDialog ( hWnd, 0 );
				break;
			case IDC_TSLWWWLINK:
				ShellExecute( NULL, L"open", _TSL_URL_, NULL, NULL, SW_SHOW );
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

int fontSize( HWND hWnd, int size ) {
	HDC hDC = GetDC( hWnd );
	int height = -MulDiv( size, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );
	ReleaseDC( hWnd, hDC );
	return height;
}

void center_dlg( HWND hWnd ) {
	RECT rc, rcDlg, rcOwner;
	HWND hWndOwner = GetWindow( hWnd, GW_OWNER );
	GetWindowRect( hWndOwner, &rcOwner );
	GetWindowRect( hWnd, &rcDlg );
	CopyRect( &rc, &rcOwner );
	OffsetRect( &rcDlg, -rcDlg.left, -rcDlg.top );
	OffsetRect( &rc, -rc.left, -rc.top );
	OffsetRect( &rc, -rcDlg.right, -rcDlg.bottom );
	SetWindowPos( hWnd, HWND_TOP,
	              rcOwner.left + ( rc.right / 2 ),
	              rcOwner.top + ( rc.bottom / 2 ),
	              0, 0, SWP_NOSIZE | SWP_NOZORDER );
}

void OnClick() {
	switch( opt.OnIconClick.get() ) {
	case ONCLICK_SHOWHIDE:
		SendMessage( hMainWnd, WM_COMMAND, ( WPARAM ) ID_SHOWHIDE, 0 );
		break;
	case ONCLICK_OPENWEB:
		SendMessage( hMainWnd, WM_COMMAND, ( WPARAM ) ID_OPENWEB, 0 );
		break;
	case ONCLICK_RESTART:
		SendMessage( hMainWnd, WM_COMMAND, ( WPARAM ) ID_RESTART, 0 );
		break;
	}
}