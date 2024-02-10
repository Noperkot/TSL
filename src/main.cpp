#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include "res/resource.h"
#include "res/version.h"
#include "res/commands.h"
#include "main.h"
#include "dialogs.h"

#define MSGFLT_ADD 1
#define MSGFLT_REMOVE 2
typedef BOOL ( WINAPI *CWMFEx_t )( HWND, UINT, DWORD );  // ChangeWindowMessageFilterEx from user32.lib
bool set_dir();

HWND hMainWnd = 0;
BOOL silent = false;

int WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR args, int ss ) {
	enum { CMD_STOP, CMD_START, CMD_RESTART, CMD_HIDE, CMD_SHOW, CMD_OPENWEB };
	silent = opt.arg.search( L"--silent" );
	UINT act;
	{
		HWND hWnd = FindWindow( MAIN_WINCLASS, 0 );							// ищем запущенный экземпляр tsl (по классу окна)
		if( opt.arg.search( L"--close" ) ) {
			if( hWnd ) SendMessage( hWnd, WM_DESTROY,  0, 0 );
			return EXIT_SUCCESS;
		}
		struct {
			LPCWSTR cmd;
			UINT msg;
		} const  actions[] = {
			{ L"--stop", ID_STOP },
			{ L"--start", ID_START },
			{ L"--restart", ID_RESTART },
			{ L"--hide", ID_HIDE },
			{ L"--show", ID_SHOW },
			{ L"--web", ID_OPENWEB },
		};
		for( act = 0; act <  sizeof( actions ) / sizeof( *actions ); act++ ) {
			if( opt.arg.search( actions[act].cmd ) ) {
				if( hWnd ) {
					SendMessage( hWnd, WM_COMMAND,  actions[act].msg, 0 );
					return EXIT_SUCCESS;
				} else break;
			}
		}
		if( hWnd ) {
			SendMessage( hWnd, WM_COMMAND,  ID_SHOW, 0 );
			return EXIT_SUCCESS;
		}
	}
	if( !set_dir() ) {														// устанавливаем рабочую директорию и проверяем наличие экзешника торрсервера
		MessageBox( ( isX64() ) ? STR_NEED_TORSERVER64 : STR_NEED_TORSERVER32, MB_OK | MB_APPLMODAL | MB_ICONERROR );
		return EXIT_FAILURE;
	}
	HICON hFavIcon = LoadIcon( GetModuleHandle( NULL ), MAKEINTRESOURCE( TSLOGO_ICON ) );
	{
		WNDCLASSEX wc = {0};
		// регистрируем класс дочерних окон
		wc.cbSize = sizeof( WNDCLASSEX );
		GetClassInfoEx( 0, WC_DIALOG, &wc );
		wc.lpszClassName = CHIL_WINCLASS;
		wc.hInstance = hInst;
		wc.style &= ~CS_GLOBALCLASS;
		if( !RegisterClassEx( &wc ) ) {
			MessageBox( L"Unable to create a window class \"" CHIL_WINCLASS "\"", MB_OK | MB_APPLMODAL | MB_ICONERROR );
			return EXIT_FAILURE;
		}
		// регистрируем класс главного окна
		wc.lpszClassName = MAIN_WINCLASS;
		wc.lpfnWndProc = MainWinProc;
		wc.hIcon = hFavIcon;
		wc.hIconSm = hFavIcon;
		wc.lpszMenuName = MAKEINTRESOURCE( IDR_FORMMENU );
		if( !RegisterClassEx( &wc ) ) {
			MessageBox( L"Unable to create a window class \"" MAIN_WINCLASS "\"", MB_OK | MB_APPLMODAL | MB_ICONERROR );
			return EXIT_FAILURE;
		}
		int W = ( opt.WindowW.get() == AUTO ) ? GetSystemMetrics( SM_CXSCREEN ) * 2 / 3 : opt.WindowW.get();		// по умолчанию окно 2/3 экрана
		int H = ( opt.WindowH.get() == AUTO ) ? GetSystemMetrics( SM_CYSCREEN ) * 2 / 3 : opt.WindowH.get();
		int X = ( opt.WindowX.get() == AUTO ) ? ( GetSystemMetrics( SM_CXSCREEN ) - W ) / 2 : opt.WindowX.get();	// центрирование
		int Y = ( opt.WindowY.get() == AUTO ) ? ( GetSystemMetrics( SM_CYSCREEN ) - H ) / 2 : opt.WindowY.get();
		hMainWnd = CreateWindow(
		               MAIN_WINCLASS,
		               L"TorrServer",
		               WS_OVERLAPPEDWINDOW | ( ( opt.WindowMax.get() ) ? WS_MAXIMIZE : 0 ),
		               X, Y, W, H,
		               NULL,
		               NULL,
		               hInst,
		               NULL
		           );
	}
	if( hMainWnd ) {
		CWMFEx_t ChangeWindowMessageFilterEx = ( CWMFEx_t ) GetProcAddress( GetModuleHandleA( "user32.lib" ), "ChangeWindowMessageFilterEx" );
		if( ChangeWindowMessageFilterEx ) ChangeWindowMessageFilterEx( hMainWnd, WM_DESTROY, MSGFLT_ADD );   // позволяет принимать сообщение WM_DESTROY отправленное из под другого пользователя
		if( act == CMD_SHOW ) ShowWindow( hMainWnd, ( opt.WindowMax.get() ) ? SW_MAXIMIZE : SW_SHOW );
		UpdateWindow( hMainWnd );
		if( act != CMD_STOP ) SendMessage( hMainWnd, WM_COMMAND, ID_START, 0 );   // запускаем сервер
		if( act == CMD_OPENWEB ) SendMessage( hMainWnd, WM_COMMAND, ID_OPENWEB, 0 );
		HACCEL hAccelTable = LoadAccelerators( GetModuleHandle( NULL ), MAKEINTRESOURCE( IDR_ACCEL1 ) );
		MSG msg;
		while( GetMessage( &msg, NULL, 0, 0 ) ) {
			if( !TranslateAccelerator( hMainWnd, hAccelTable, &msg ) ) {
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		if( ChangeWindowMessageFilterEx ) ChangeWindowMessageFilterEx( hMainWnd, WM_DESTROY, MSGFLT_REMOVE );
	} else MessageBox( L"Unable to create a window", MB_OK | MB_APPLMODAL | MB_ICONERROR );
	UnregisterClass( CHIL_WINCLASS, hInst );
	UnregisterClass( MAIN_WINCLASS, hInst );
	DestroyIcon( hFavIcon );
	return EXIT_SUCCESS;  //msg.wParam;
}

bool set_dir() { // установка текущей директории из которой запущен tsl
	LPWSTR workDir = wStrReplace( NULL, opt.arg.v[0] );
	* wcsrchr( workDir, L'\\' ) = L'\0';
	bool res = ( SetCurrentDirectory( workDir ) != 0 );
	free( workDir );
	return res && ( checkTS() != NULL );
}

LPWSTR wStrReplace( LPWSTR *dst, LPCWSTR str1, LPCWSTR str2, LPCWSTR str3 ) {  // создание новой строки склейкой str* с освобождением памяти от предыдущей
	LPWSTR newstr = NULL;
	if( str1 ) {
		newstr = ( LPWSTR ) malloc( ( wcslen( str1 ) + wcslen( str2 ) + wcslen( str3 ) + 1 ) * sizeof( wchar_t ) );
		wcscpy( newstr, str1 );
		wcscat( newstr, str2 );
		wcscat( newstr, str3 );
	}
	if( dst ) {
		LPWSTR oldstr = *dst;
		*dst = newstr;
		free( oldstr );
	}
	return newstr;
}

LPWSTR rcString( LPWSTR *dst, UINT idsText ) {  // загрузка строки из ресурсов
	LPWSTR ptmp;
	int len = LoadString( NULL, idsText, ( LPWSTR ) &ptmp, 0 );
	if( len++ ) {
		ptmp = ( LPWSTR ) malloc( len * sizeof( *ptmp ) );
		LoadString( NULL, idsText, ptmp, len );
	} else ptmp = NULL;
	if( dst ) {
		free( *dst );
		*dst = ptmp;
	}
	return ptmp;
}

bool isX64() {  // проверка разрядности ОС
	SYSTEM_INFO si;
	GetNativeSystemInfo( &si );
	return ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 );
}

LPCWSTR FileExists( LPCWSTR szPath ) {  // проверка существования файла
	DWORD dwAttrib = GetFileAttributes( szPath );
	return ( dwAttrib != INVALID_FILE_ATTRIBUTES &&  !( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) ) ? szPath : NULL;
}

LPCWSTR checkTS() {	// проверка существования экзешника ТС в соответствии с архитектурой
	LPCWSTR TSfname = NULL;
	if( isX64() ) TSfname = FileExists( L"./TorrServer-windows-amd64.exe" );
	if( !TSfname ) TSfname = FileExists( L"./TorrServer-windows-386.exe" );
	return TSfname;
}

void MessageBox( UINT idsText, UINT uType ) {  // MessageBox строки из ресурсов
	if( !silent ) {
		LPWSTR buf = rcString( NULL, idsText );
		MessageBox( hMainWnd, buf, _PRODUCTNAME_, uType );
		free( buf );
	}
}

void MessageBox( LPCWSTR text, UINT uType ) {
	if( !silent ) MessageBox( hMainWnd, text, _PRODUCTNAME_, uType );
}