#ifndef UNICODE
	#define UNICODE
#endif
#include <windows.h>
#include "options.h"
#include "main.h"

#define REG_HKEY		HKEY_CURRENT_USER
#define REG_SUBKEY		L"Software\\TorrServer"
#define REG_RUN_SUBKEY	L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define REG_RUN_VALNAME	L"TorrServer"

struct {
	LPCWSTR name;
	DWORD type;
	LPCVOID defVal;
} const opt_enum [ sizeof ( OPTIONS_S ) / sizeof ( LPCVOID ) ] = {					// в 32-х разрядном приложении что INT что LPVOID все по 4 байта
	{ L"GrabVerLines",				REG_DWORD,	( LPCVOID ) 10 },					// В скольких первых строках вывода TS искать его версию. Версия нужна для заголовка окна и в эбауте. 0 - не искать.
	{ L"WindowX",					REG_DWORD,	( LPCVOID ) AUTO },					// X координата окна. Если не указано - автоцентрирование
	{ L"WindowY",					REG_DWORD,	( LPCVOID ) AUTO },					// Y координата окна. Если не указано - автоцентрирование
	{ L"WindowW",					REG_DWORD,	( LPCVOID ) AUTO },					// Ширина окна. Если не указано - 2/3 экрана
	{ L"WindowH",					REG_DWORD,	( LPCVOID ) AUTO },					// Высота окна. Если не указано - 2/3 экрана
	{ L"WindowMinW",				REG_DWORD,	( LPCVOID ) 320 },					// Минимальная ширина окна
	{ L"WindowMinH",				REG_DWORD,	( LPCVOID ) 240 },					// Минимальная высота окна
	{ L"WindowMax",					REG_DWORD,	( LPCVOID ) 0 },					// Развернуть на весь экран. 0-нет, 1-да
	{ L"TextWrapping",				REG_DWORD,	( LPCVOID ) 0 },					// Переносить строки в консоли. 0-нет, 1-да
	{ L"MaxLines",					REG_DWORD,	( LPCVOID ) 1000 },					// Строк в кольцевм буфере консоли
	{ L"ExitWhenClose",				REG_DWORD,	( LPCVOID ) 0 },					// Действие при закрытии окна. 0 - сворачивать в трей, 1 - выход
	{ L"OnTSdead",					REG_DWORD,	( LPCVOID ) ONDEAD_NOACTION },		// Действие при падении TS. 0 - ничего не делать, 1 - закрыть программу, 2 - развернуть окно программы, 3 - перезапустить TS
	{ L"OnIconClick",				REG_DWORD,	( LPCVOID ) ONCLICK_SHOWHIDE },		// Действие при клике по иконке в трее. 0 - показать/скрыть окно, 1 - открыть веб-интерфейс TS, 2 - рестарт TS
	{ L"DblIconClick",				REG_DWORD,	( LPCVOID ) 0 },					// Клик по иконке в трее. 0 - одинарный, 1 - двойной.
	{ L"ConsoleBkColor",			REG_DWORD,	( LPCVOID ) 0x000000 },				// hex цвет фона 0xRRGGBB
	{ L"ConsoleFontColor",			REG_DWORD,	( LPCVOID ) 0xBBBBBB },				// hex цвет шрифта 0xRRGGBB 0xED9121 - морковный
	{ L"ConsoleFontSize",			REG_DWORD,	( LPCVOID ) 9 },					// Размер шрифта
	{ L"ConsoleFontName",			REG_SZ,		( LPCVOID ) L"Lucida Console" },	// Название шрифта
	{ L"args",						REG_SZ,		( LPCVOID ) NULL }					// Аргументы командной строки TS
};


DWORD rgb2bgr ( DWORD rgb ) {
	return ( rgb & 0x00ff00 ) | ( rgb >> 16 & 0x0000ff ) | ( rgb << 16 & 0xff0000 );
}

void OPTIONS_S::load() {
	memset ( this, 0, sizeof ( *this ) );
	for ( DWORD i = 0; i < sizeof ( opt_enum ) / sizeof ( *opt_enum ); i++ ) {
		switch ( opt_enum[i].type ) {
			case REG_SZ: {
					LPWSTR *ppws = &reinterpret_cast<LPWSTR*> ( this ) [i];
					if ( reg_load ( opt_enum[i].name, ppws ) != ERROR_SUCCESS ) {
						wStrReplace ( ppws, ( LPCWSTR ) opt_enum[i].defVal );
					}
				}
				break;
			case REG_DWORD:
				reinterpret_cast<INT*> ( this ) [i] = reg_load ( opt_enum[i].name, ( INT ) opt_enum[i].defVal );
				break;
		}
	}
	ConsoleBkColor		= rgb2bgr ( ConsoleBkColor ) ;
	ConsoleFontColor	= rgb2bgr ( ConsoleFontColor );
}


void opt_reset() {
	HKEY hKey;
	for ( DWORD i = 0; i < sizeof ( opt_enum ) / sizeof ( *opt_enum ); i++ ) {
		if ( RegOpenKeyEx ( REG_HKEY, REG_SUBKEY, 0, KEY_WRITE, &hKey ) == ERROR_SUCCESS ) {
			RegDeleteValue ( hKey, opt_enum[i].name );
			RegCloseKey ( hKey );
		}
	}
	Autostart ( false );
}


INT reg_load ( LPCWSTR ValName, CONST INT defaultVal ) {
	HKEY hKey;
	INT res = defaultVal;
	if ( RegOpenKeyEx ( REG_HKEY, REG_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		DWORD Type;
		DWORD bufSize;
		if ( RegQueryValueEx ( hKey, ValName, NULL, &Type, NULL, &bufSize ) == ERROR_SUCCESS ) {
			if ( Type == REG_DWORD ) RegQueryValueEx ( hKey, ValName, NULL, NULL, ( BYTE* ) &res, &bufSize );
		}
		RegCloseKey ( hKey );
	}
	return res;
}

LSTATUS reg_save ( LPCWSTR ValName, CONST INT IntVal ) {
	HKEY hKey;
	LSTATUS result = RegCreateKeyEx ( REG_HKEY, REG_SUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL );
	if ( result == ERROR_SUCCESS ) {
		result = RegSetValueEx ( hKey, ValName, 0, REG_DWORD, ( const BYTE* ) &IntVal, sizeof ( IntVal ) );
		RegCloseKey ( hKey );
	}
	return result;
}


LSTATUS reg_load ( LPCWSTR ValName, LPWSTR *StrVal, LPCWSTR defaultVal ) {
	DWORD bufSize;
	HKEY hKey;
	DWORD Type;
	wStrReplace ( StrVal, defaultVal );
	LSTATUS result = RegOpenKeyEx ( REG_HKEY, REG_SUBKEY, 0, KEY_READ, &hKey );
	if ( result == ERROR_SUCCESS ) {
		result = RegQueryValueEx ( hKey, ValName, NULL, &Type, NULL, &bufSize );
		if ( result == ERROR_SUCCESS ) {
			if ( Type == REG_SZ ) {
				free ( *StrVal );
				*StrVal = ( LPWSTR ) malloc ( bufSize );
				result = RegQueryValueEx ( hKey, ValName, NULL, NULL, ( BYTE* ) *StrVal, &bufSize );
			} else result = ERROR_FILE_NOT_FOUND;
		}
		RegCloseKey ( hKey );
	}
	return result;
}

LSTATUS reg_save ( LPCWSTR ValName, LPCWSTR StrVal ) {
	HKEY hKey;
	LSTATUS result = RegCreateKeyEx ( REG_HKEY, REG_SUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL );
	if ( result == ERROR_SUCCESS ) {
		result = RegSetValueEx ( hKey, ValName, 0, REG_SZ, ( const BYTE* ) StrVal, ( wcslen ( StrVal ) + 1 ) * sizeof ( wchar_t ) );
		RegCloseKey ( hKey );
	}
	return result;
}

void Autostart ( bool yes ) {
	HKEY hKey;
	if ( RegOpenKeyEx ( REG_HKEY, REG_RUN_SUBKEY, 0, KEY_WRITE, &hKey ) == ERROR_SUCCESS ) {
		if ( yes ) {
			LPWSTR cmdLine = ExePath ( L"\"", L"\" --silent" );
			RegSetValueEx ( hKey, REG_RUN_VALNAME, 0, REG_SZ, ( const BYTE* ) cmdLine, ( wcslen ( cmdLine ) + 1 ) * sizeof ( wchar_t ) );
			free ( cmdLine );
		} else RegDeleteValue ( hKey, REG_RUN_VALNAME );
		RegCloseKey ( hKey );
	}
}

bool Autostart() {
	bool result = false;
	HKEY hKey;
	DWORD bufSize;
	if (  RegOpenKeyEx ( REG_HKEY, REG_RUN_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		if ( RegQueryValueEx ( hKey, REG_RUN_VALNAME, NULL, NULL, NULL, &bufSize ) == ERROR_SUCCESS ) {
			wchar_t *regVal = ( wchar_t* ) malloc ( bufSize );
			RegQueryValueEx ( hKey, REG_RUN_VALNAME, NULL, NULL, ( BYTE* ) regVal, &bufSize );
			LPWSTR exePath = ExePath();
			result = wcsstr ( regVal, exePath ) != NULL;
			free ( exePath );
			free ( regVal );
		}
		RegCloseKey ( hKey );
	}
	return result;
}












