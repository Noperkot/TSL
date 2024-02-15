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

ARG arg;
INI ini;
OPT opt;

void Autostart( bool yes ) {
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_RUN_SUBKEY, 0, KEY_WRITE, &hKey ) == ERROR_SUCCESS ) {
		if( yes ) {
			wStr tmp( L"\"", arg.v [ 0 ], L"\" --silent" );
			RegSetValueExW( hKey, TS_KEYNAME, 0, REG_SZ, ( const BYTE* ) tmp.get(), ( wcslen( tmp.get() ) + 1 ) * sizeof( wchar_t ) );
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
			wStr tmp;
			tmp.reserve( bufSize );
			RegQueryValueExW( hKey, TS_KEYNAME, NULL, NULL, ( BYTE* ) tmp.get(), &bufSize );
			result = ( wcsstr( tmp.get(), arg.v [ 0 ] ) != NULL );
		}
		RegCloseKey( hKey );
	}
	return result;
}

/* ****************** class wStr ****************** */

void wStr::reserve( DWORD size ) {
	free( val );
	val = ( size ) ? ( LPWSTR ) malloc( size ) : NULL;
}

LPWSTR wStr::get() {
	return val;
};

wStr& wStr::set( LPCWSTR str1, LPCWSTR str2, LPCWSTR str3 ) {
	LPWSTR oldval = val;
	if( str1 ) {
		size_t len1 = wcslen( str1 ), len12 = len1 + wcslen( str2 );
		val = ( LPWSTR ) malloc( ( len12 + wcslen( str3 ) + 1 ) * sizeof( wchar_t ) );
		wcscpy( val, str1 );
		wcscpy( val + len1, str2 );
		wcscpy( val + len12, str3 );
	} else val = NULL;
	free( oldval );
	return *this;
}

wStr& wStr::set( UINT idstr1 ) {
	free( val );
	int len = LoadStringW( NULL, idstr1, ( LPWSTR ) &val, 0 );
	if( len ) {
		val = ( LPWSTR ) malloc( ++len * sizeof( wchar_t ) );
		LoadStringW( NULL, idstr1, val, len );
	} else val = NULL;
	return *this;
}

wStr::wStr( LPCWSTR str1, LPCWSTR str2, LPCWSTR str3 ) : val( NULL ) {
	set( str1, str2, str3 );
}

wStr::wStr( UINT idstr1 ) : val( NULL ) {
	set( idstr1 );
}

wStr::~wStr() {
	free( val );
}

/* ****************** class ARG ****************** */

ARG::ARG( ) : v( CommandLineToArgvW( GetCommandLineW(), &c ) ) {}

// ARG::~ARG() {
// GlobalFree ( v );
// }

template<typename T>
BOOL ARG::search( LPCWSTR name, T cb ) {
	for( int i = 1; i < c ; i++ ) {
		if( wcsstr( v [ i ], name ) == v [ i ] ) {
			size_t namelen = wcslen( name );
			if( v [ i ] [ namelen ] == L'=' ) cb( v [ i ] + namelen + 1 );
			return TRUE;
		}
	}
	return FALSE;
}

BOOL ARG::search( LPCWSTR name ) {
	return search( name, []( LPCWSTR ) {} );
}

/* ****************** class INI ****************** */

INI::INI() : path( arg.v [ 0 ] ) {
	* wcsrchr( path.get(), L'\\' ) = L'\0';
	path.set( path.get(), L"\\tsl.ini" );
	DWORD dwAttrib = GetFileAttributes( path.get() );
	if( dwAttrib == INVALID_FILE_ATTRIBUTES || ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ) ) path.reserve();
}

/* ****************** class OPT::_int_ ****************** */

BOOL OPT::_int_::loadINI() {
	if( !ini.path.get() ) return FALSE;
	val = GetPrivateProfileIntW( TS_KEYNAME, name, val, ini.path.get() );
	return TRUE;
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
	return arg.search( name, [ this ]( LPCWSTR val ) {
		this->val = _wtoi( val );
	} );
}

BOOL OPT::_int_::saveINI() {
	if( !ini.path.get() ) return FALSE;
	wchar_t buf [ 12 ];
	_itow( val, buf, 10 );
	WritePrivateProfileStringW( TS_KEYNAME, name, buf, ini.path.get() );
	return TRUE;
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
	noSave = loadARG();
	if( !noSave ) if( !loadINI() ) loadREG();
	return val;
}
void OPT::_int_::save( INT val ) {
	set( val );
	if( !noSave ) if( !saveINI() ) saveREG();
}

OPT::_int_::_int_( LPCWSTR nameInit, INT valInit ) : name( nameInit ), val( valInit ), noSave( false ) {
	load();
}

/* ****************** class OPT::_str_ ****************** */

BOOL OPT::_str_::loadINI() {
	if( !ini.path.get() ) return FALSE;
	wchar_t buf [ 1024 ];
	if( GetPrivateProfileStringW( TS_KEYNAME, name, NULL, buf, 1024, ini.path.get() ) ) set( buf );
	return TRUE;
}

void OPT::_str_::loadREG() {
	HKEY hKey;
	if( RegOpenKeyEx( REG_HKEY, REG_SUBKEY, 0, KEY_READ, &hKey ) == ERROR_SUCCESS ) {
		DWORD Type;
		DWORD bufSize;
		if( RegQueryValueExW( hKey, name, NULL, &Type, NULL, &bufSize ) == ERROR_SUCCESS && Type == REG_SZ ) {
			reserve( bufSize );
			RegQueryValueExW( hKey, name, NULL, NULL, ( LPBYTE ) val, &bufSize );
		}
		RegCloseKey( hKey );
	}
}

BOOL OPT::_str_::loadARG() {
	return arg.search( name, [ this ]( LPCWSTR val ) {
		set( val );
	} );
}

BOOL OPT::_str_::saveINI() {
	if( !ini.path.get() ) return FALSE;
	WritePrivateProfileStringW( TS_KEYNAME, name, val, ini.path.get() );
	return TRUE;
}

void OPT::_str_::saveREG() {
	HKEY hKey;
	if( RegCreateKeyEx( REG_HKEY, REG_SUBKEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL ) == ERROR_SUCCESS ) {
		RegSetValueExW( hKey, name, 0, REG_SZ, * ( LPCBYTE* ) val, ( wcslen( val ) + 1 ) * sizeof( wchar_t ) );
		RegCloseKey( hKey );
	}
}

LPWSTR OPT::_str_::load() {
	noSave = loadARG();
	if( !noSave ) if( !loadINI() ) loadREG();
	return val;
}

void OPT::_str_::save( LPWSTR val ) {
	set( val );
	if( !noSave ) if( !saveINI() ) saveREG();
}

OPT::_str_::_str_( LPCWSTR nameInit, LPCWSTR valInit ) : wStr( valInit ), name( nameInit ), noSave( false ) {
	load();
}

/* ****************** class OPT ****************** */

DWORD rgb2bgr( DWORD rgb ) {
	return ( rgb & 0x00ff00 ) | ( rgb >> 16 & 0x0000ff ) | ( rgb << 16 & 0xff0000 );
}

OPT::OPT() {
	ConsoleBkColor.set(	rgb2bgr( ConsoleBkColor.get() ) ) ;
	ConsoleFontColor.set( rgb2bgr( ConsoleFontColor.get() ) );
}