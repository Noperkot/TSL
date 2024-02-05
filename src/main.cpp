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
typedef BOOL (WINAPI *CWMFEx_t) (HWND, UINT, DWORD); // ChangeWindowMessageFilterEx from user32.lib

enum CMD_E : UINT { CMD_UNRECOGNIZED = 0, CMD_SILENT, CMD_CLOSE, CMD_STOP, CMD_START, CMD_RESTART, CMD_SHOW, CMD_HIDE, CMD_RESET, CMD_OPENWEB };
const LPCWSTR cmdArgs[] = { L"--silent", L"--close", L"--stop", L"--start", L"--restart", L"--show", L"--hide", L"--reset", L"--web" };

OPTIONS_S opt;
HWND hMainWnd;

CMD_E cmd_parser ( LPCWSTR args );
bool set_dir();

int WINAPI wWinMain ( HINSTANCE hInst, HINSTANCE, LPWSTR args, int ss ) {

	opt.load();

	HWND hWnd = FindWindow ( MAIN_WINCLASS, 0 );							// ищем запущенный экземпляр tsl (по классу окна)

	CMD_E cmd = cmd_parser ( args );
	switch ( cmd ) {														// первый распознанный параметр командной строки (остальные игнорируем)
		case CMD_CLOSE:
			if ( hWnd ) SendMessage ( hWnd, WM_DESTROY,  0, 0 );			// Закрываем запущенный экземпляр tsl
			return EXIT_SUCCESS;
		case CMD_STOP:
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND,  ID_STOP, 0 );					// Останавливаем сервер в запущенном экземпляре tsl
			return EXIT_SUCCESS;
		case CMD_START:
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND,  ID_START, 0 );					// Стартуем сервер в запущенном экземпляре tsl
			return EXIT_SUCCESS;
		case CMD_RESTART:
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND,  ID_RESTART, 0 );				// Рестартим сервер в запущенном экземпляре tsl
			return EXIT_SUCCESS;
		case CMD_HIDE:
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND,  ID_HIDE, 0 );					// Сворачиваем запущенный экземпляр tsl
			return EXIT_SUCCESS;
		case CMD_RESET:														// сброс настроек
			if ( hWnd ) SendMessage ( hWnd, WM_DESTROY,  0, 0 );
			opt_reset();
			rcMessageBox ( 0, STR_RESETTETTINGS, MB_OK | MB_APPLMODAL | MB_ICONWARNING );
			return EXIT_SUCCESS;
		case CMD_OPENWEB:
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND, ID_OPENWEB, 0 );
			return EXIT_SUCCESS;
		default:															// CMD_UNRECOGNIZED, CMD_SILENT, CMD_SHOW
			if ( !hWnd ) break;
			SendMessage ( hWnd, WM_COMMAND,  ID_SHOW, 0 );					// Показываем окно запущенного экземпляра tsl
			return EXIT_SUCCESS;				
	}

	if ( !set_dir() ) {														// устанавливаем рабочую директорию и проверяем наличие экзешника торрсервера
		if ( cmd != CMD_SILENT ) rcMessageBox ( 0, ( isX64() ) ? STR_NEED_TORSERVER64 : STR_NEED_TORSERVER32, MB_OK | MB_APPLMODAL | MB_ICONERROR );
		return EXIT_FAILURE; //EXIT_SUCCESS;
	}
	
	HICON hFavIcon = LoadIcon ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( FAV_ICON ) );
	
	{
		WNDCLASSEX wc = {0};
		// регистрируем класс дочерних окон
		wc.cbSize = sizeof ( WNDCLASSEX );
		GetClassInfoEx ( 0, WC_DIALOG, &wc );
		wc.lpszClassName = CHIL_WINCLASS;
		wc.hInstance = hInst;
		wc.style &= ~CS_GLOBALCLASS;
		if ( !RegisterClassEx ( &wc ) ) {
			if ( cmd != CMD_SILENT ) MessageBox ( 0, L"Unable to create a window class \"" CHIL_WINCLASS "\"", _PRODUCTNAME_, MB_OK | MB_APPLMODAL | MB_ICONERROR );
			return EXIT_FAILURE;
		}
		// регистрируем класс главного окна
		wc.lpszClassName = MAIN_WINCLASS;
		wc.lpfnWndProc = MainWinProc;
		wc.hIcon = hFavIcon;
		wc.hIconSm = hFavIcon;
		wc.lpszMenuName = MAKEINTRESOURCE ( IDR_FORMMENU );
		if ( !RegisterClassEx ( &wc ) ) {
			if ( cmd != CMD_SILENT )  MessageBox ( 0, L"Unable to create a window class \"" MAIN_WINCLASS "\"", _PRODUCTNAME_, MB_OK | MB_APPLMODAL | MB_ICONERROR );
			return EXIT_FAILURE;
		}

		int W = ( opt.WindowW == AUTO ) ? GetSystemMetrics ( SM_CXSCREEN ) * 2 / 3 : opt.WindowW;		// по умолчанию окно 2/3 экрана
		int H = ( opt.WindowH == AUTO ) ? GetSystemMetrics ( SM_CYSCREEN ) * 2 / 3 : opt.WindowH;
		int X = ( opt.WindowX == AUTO ) ? ( GetSystemMetrics ( SM_CXSCREEN ) - W ) / 2 : opt.WindowX;	// центрирование
		int Y = ( opt.WindowY == AUTO ) ? ( GetSystemMetrics ( SM_CYSCREEN ) - H ) / 2 : opt.WindowY;
		
		hMainWnd = CreateWindow (
		        MAIN_WINCLASS,
		        L"TorrServer",
		        WS_OVERLAPPEDWINDOW | ( ( opt.WindowMax ) ? WS_MAXIMIZE : 0 ),
		        X, Y, W, H,
		        NULL,
		        NULL,
		        hInst,
		        NULL
		    );
	}
	if ( hMainWnd ) {
		CWMFEx_t ChangeWindowMessageFilterEx = (CWMFEx_t)GetProcAddress(GetModuleHandleA("user32.lib"), "ChangeWindowMessageFilterEx");
		if(ChangeWindowMessageFilterEx) ChangeWindowMessageFilterEx( hMainWnd, WM_DESTROY, MSGFLT_ADD ); // позволяет принимать сообщение WM_DESTROY отправленное из под другого пользователя
		if ( cmd == CMD_SHOW ) ShowWindow ( hMainWnd, ( opt.WindowMax ) ? SW_MAXIMIZE : SW_SHOW );
		UpdateWindow ( hMainWnd );
		if ( cmd != CMD_STOP ) SendMessage ( hMainWnd, WM_COMMAND, ID_START, 0 ); // запускаем сервер
		if ( cmd == CMD_OPENWEB ) SendMessage ( hMainWnd, WM_COMMAND, ID_OPENWEB, 0 );
		HACCEL hAccelTable = LoadAccelerators ( GetModuleHandle ( NULL ), MAKEINTRESOURCE ( IDR_ACCEL1 ) );
		MSG msg;
		while ( GetMessage ( &msg, NULL, 0, 0 ) ) {
			if ( !TranslateAccelerator ( hMainWnd, hAccelTable, &msg ) ) {
				TranslateMessage ( &msg );
				DispatchMessage ( &msg );
			}
		}
		if(ChangeWindowMessageFilterEx) ChangeWindowMessageFilterEx( hMainWnd, WM_DESTROY, MSGFLT_REMOVE );
	} else MessageBox ( 0, L"Unable to create a window", _PRODUCTNAME_, MB_OK | MB_APPLMODAL | MB_ICONERROR );
	
	UnregisterClass ( CHIL_WINCLASS, hInst );
	UnregisterClass ( MAIN_WINCLASS, hInst );
	DestroyIcon( hFavIcon );
	return EXIT_SUCCESS;  //msg.wParam;
}

CMD_E cmd_parser ( LPCWSTR args ) { // парсер параметров командной строки
	CMD_E result = CMD_UNRECOGNIZED;
	int	 argc;
	LPWSTR * pArg = CommandLineToArgvW ( args, &argc );
	if ( argc > 0 ) {
		for ( UINT i = 0; i <  sizeof ( cmdArgs ) / sizeof ( *cmdArgs ) ; ) {
			if ( _wcsicmp ( pArg[0], cmdArgs[i++] ) == 0 ) {
				result = ( CMD_E ) i ;
				break;
			}
		}
	}
	GlobalFree ( pArg );
	return result;
}

bool set_dir() { // установка текущей директории из которой запущен tsl
	LPWSTR workDir = ExePath();
	* wcsrchr ( workDir, L'\\' ) = L'\0';
	bool res = ( SetCurrentDirectory ( workDir ) != 0 );
	free ( workDir );
	return res && ( checkTS() != NULL );
}


LPWSTR wStrReplace ( LPWSTR *dst, LPCWSTR src ) { // создание новой строки с освобождением памяти от предыдущей
	LPWSTR olddst = *dst;
	if ( src ) {
		*dst = ( LPWSTR ) malloc ( ( wcslen ( src ) + 1 ) * sizeof ( wchar_t ) );
		wcscpy ( *dst, src );
	} else  *dst = NULL;
	free ( olddst );
	return *dst;
}

LPWSTR rcString ( UINT idsText, LPWSTR *pBuf ) { // загрузка строки из ресурсов
	LPWSTR ptmp;
	int len = LoadString ( NULL, idsText, ( LPWSTR ) &ptmp, 0 );
	if ( len++ ) {
		ptmp = ( LPWSTR ) malloc ( len * sizeof ( *ptmp ) );
		LoadString ( NULL, idsText, ptmp, len  );
	} else ptmp = NULL;
	if ( pBuf ) {
		free ( *pBuf );
		*pBuf = ptmp;
	}
	return ptmp;
}

bool isX64() { // проверка разрядности ОС
	SYSTEM_INFO si;
	GetNativeSystemInfo ( &si );
	return ( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 );
}

LPCWSTR FileExists ( LPCWSTR szPath ) { // проверка существования файла
	DWORD dwAttrib = GetFileAttributes ( szPath );
	return ( dwAttrib != INVALID_FILE_ATTRIBUTES &&  ! ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) ) ? szPath : NULL;
}

LPCWSTR checkTS() {	// проверка существования экзешника ТС в соответствии с архитектурой
	LPCWSTR TSfname = NULL;
	if ( isX64() ) TSfname = FileExists ( L"./TorrServer-windows-amd64.exe" );
	if ( !TSfname ) TSfname = FileExists ( L"./TorrServer-windows-386.exe" );
	return TSfname;
}

LPWSTR ExePath ( LPCWSTR prefix, LPCWSTR postfix ) { // Создает строку с путем к исполняемому файлу tsl
	LPWSTR res = NULL;
	DWORD prefix_len = ( prefix ) ? wcslen ( prefix ) : 0;
	DWORD postfix_len = ( postfix ) ? wcslen ( postfix ) : 0;
	for ( DWORD len = MAX_PATH, copied = MAX_PATH; copied == len; len <<= 1 ) {	// несколько прогонов, каждый раз увеличивая буфер вдвое, пока путь в него не влезет.  для большинства случаев достаточно одной попытки.
		free ( res );
		res = ( LPWSTR ) malloc ( ( len + prefix_len + postfix_len ) * sizeof ( wchar_t ) );
		copied = GetModuleFileName ( NULL, res + prefix_len, len );
		if ( copied == 0 ) {
			free ( res );
			return NULL;
		}
	}
	if ( prefix ) memcpy ( res, prefix, prefix_len * sizeof ( wchar_t ) );
	if ( postfix ) wcscat ( res, postfix );
	return res;
}

void rcMessageBox ( HWND hWnd, UINT idsText, UINT uType ) { // MessageBox из ресурсов
	LPWSTR Tbuf = NULL;
	MessageBox ( hWnd, rcString ( idsText, &Tbuf ), _PRODUCTNAME_, uType );
	free ( Tbuf );
}



























