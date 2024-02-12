#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include "options.h"
#include "main.h"

#define TS_KEYNAME		L"TorrServer"
#define REG_HKEY		HKEY_CURRENT_USER
#define REG_SUBKEY		L"Software\\" TS_KEYNAME
#define REG_RUN_SUBKEY	L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

OPT opt;

void Autostart( bool yes ) {
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_RUN_SUBKEY, 0, KEY_WRITE, &hKey ) == ERROR_SUCCESS ) {
		if( yes ) {
			LPWSTR tmp = wStrReplace( NULL, L"\"", opt.arg.v[0], L"\" --silent" );
			RegSetValueExW( hKey, TS_KEYNAME, 0, REG_SZ, ( const BYTE* ) tmp, ( wcslen( tmp ) + 1 ) * sizeof( wchar_t ) );
			free( tmp );
		} else RegDeleteValueW( hKey, TS_KEYNAME );
		RegCloseKey( hKey );
	}
}

bool Autostart() {
	bool result = false;
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_RUN_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		DWORD Type;
		DWORD bufSize;
		if( RegQueryValueExW( hKey, TS_KEYNAME, NULL, &Type, NULL, &bufSize ) == ERROR_SUCCESS && Type == REG_SZ ) {
			LPWSTR tmp = ( LPWSTR ) malloc( bufSize );
			RegQueryValueExW( hKey, TS_KEYNAME, NULL, NULL, ( BYTE* ) tmp, &bufSize );
			result = ( wcsstr( tmp, opt.arg.v[0] ) != NULL );
			free( tmp );
		}
		RegCloseKey( hKey );
	}
	return result;
}

void OPT::_int_::loadINI() {
	val = GetPrivateProfileIntW( TS_KEYNAME, name, val, parent->ini.path );
}

void OPT::_int_::loadREG() {
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		DWORD Type;
		DWORD bufSize;
		if( RegQueryValueExW( hKey, name, NULL, &Type, NULL, &bufSize ) == ERROR_SUCCESS && Type == REG_DWORD ) {
			RegQueryValueExW( hKey, name, NULL, NULL, ( LPBYTE ) &val, &bufSize );
		}
		RegCloseKey( hKey );
	}
}

BOOL OPT::_int_::loadARG() {
	return parent->arg.search( name, [this]( LPCWSTR val ) {
		this->val = _wtoi( val );
	} );
}

void OPT::_int_::saveINI() {
	wchar_t buf[12];
	_itow( val, buf, 10 );
	WritePrivateProfileStringW( TS_KEYNAME, name, buf, parent->ini.path );
}

void OPT::_int_::saveREG() {
	HKEY hKey;
	if( RegCreateKeyEx( REG_HKEY, REG_SUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL ) == ERROR_SUCCESS ) {
		RegSetValueExW( hKey, name, 0, REG_DWORD, ( LPCBYTE ) &val, sizeof( val ) );
		RegCloseKey( hKey );
	}
}

INT OPT::_int_::get() {
	return this->val;
}

VOID OPT::_int_::set( INT val ) {
	this->val = val;
}

INT OPT::_int_::load() {
	if( !loadARG() ) {
		if( parent->ini.path ) loadINI();
		else loadREG();
		noSave = false;
	} else noSave = true;
	return val;
}
void OPT::_int_::save( INT val ) {
	set( val );
	if( !noSave ) {
		if( parent->ini.path ) saveINI();
		else saveREG();
	}
}

OPT::_int_::_int_( OPT *parentInit, LPCWSTR nameInit, INT valInit ): parent( parentInit ), name( nameInit ), val( valInit ), noSave( false ) {
	load();
}

void OPT::_str_::loadINI() {
	wchar_t buf[1024];
	if( GetPrivateProfileStringW( TS_KEYNAME, name, NULL, buf, 1024, parent->ini.path ) ) wStrReplace( &val, buf );
}

void OPT::_str_::loadREG() {
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		DWORD Type;
		DWORD bufSize;
		if( RegQueryValueExW( hKey, name, NULL, &Type, NULL, &bufSize ) == ERROR_SUCCESS && Type == REG_SZ ) {
			free( val );
			val = ( LPWSTR ) malloc( bufSize );
			RegQueryValueExW( hKey, name, NULL, NULL, ( LPBYTE ) val, &bufSize );
		}
		RegCloseKey( hKey );
	}
}

BOOL OPT::_str_::loadARG() {
	return parent->arg.search( name, [this]( LPCWSTR val ) {
		wStrReplace( &this->val, val );
	} );
}

void OPT::_str_::saveINI() {
	WritePrivateProfileStringW( TS_KEYNAME, name, val, parent->ini.path );
}

void OPT::_str_::saveREG() {
	HKEY hKey;
	if( RegCreateKeyEx( REG_HKEY, REG_SUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL ) == ERROR_SUCCESS ) {
		RegSetValueExW( hKey, name, 0, REG_SZ, *( LPCBYTE* ) val, ( wcslen( val ) + 1 ) * sizeof( wchar_t ) );
		RegCloseKey( hKey );
	}
}

LPWSTR OPT::_str_::get() {
	return this->val;
}

VOID OPT::_str_::set( LPCWSTR val ) {
	wStrReplace( &this->val, val );
}

LPWSTR OPT::_str_::load() {
	if( !loadARG() ) {
		if( parent->ini.path ) loadINI();
		else loadREG();
		noSave = false;
	} else noSave = true;
	return val;
}

void OPT::_str_::save( LPWSTR val ) {
	set( val );
	if( !noSave ) {
		if( parent->ini.path ) saveINI();
		else saveREG();
	}
}

OPT::_str_::_str_( OPT *parentInit, LPCWSTR nameInit, LPCWSTR valInit ): parent( parentInit ), name( nameInit ), val( NULL ), noSave( false ) {
	set( valInit );
	load();
}

OPT::_arg_::_arg_( OPT * parent ): v( CommandLineToArgvW( GetCommandLineW(), &c ) ) {}

/*
OPT::_arg_::~_arg_(){
	GlobalFree ( v );
}
*/

template<typename T>
BOOL OPT::_arg_::search( LPCWSTR name, T cb ) {
	for( int i = 1; i < c ; i++ ) {
		if( wcsstr( v[i], name ) == v[i] ) {
			size_t namelen = wcslen( name );
			if( v[i][namelen] == L'=' ) cb( v[i] + namelen + 1 );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL OPT::_arg_::search( LPCWSTR name ) {
	return search( name, []( LPCWSTR ) {} );
}

OPT::_ini_::_ini_( OPT * parent ): path( NULL ) {
	wStrReplace( &path, parent->arg.v[0] );
	* wcsrchr( path, L'\\' ) = L'\0';
	wStrReplace( &path, L"", path, L"\\tsl.ini" );
	DWORD dwAttrib = GetFileAttributes( path );
	if( dwAttrib == INVALID_FILE_ATTRIBUTES || ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) ) wStrReplace( &path );
}

DWORD rgb2bgr( DWORD rgb ) {
	return ( rgb & 0x00ff00 ) | ( rgb >> 16 & 0x0000ff ) | ( rgb << 16 & 0xff0000 );
}

OPT::OPT() {
	ConsoleBkColor.set(	rgb2bgr( ConsoleBkColor.get() ) ) ;
	ConsoleFontColor.set( rgb2bgr( ConsoleFontColor.get() ) );
}