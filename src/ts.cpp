#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include "main.h"
#include "ts.h"

PROCESS_INFORMATION pi;
HANDLE hThrd = NULL;

DWORD WINAPI Service_THRD( void *data ) {
	DWORD exitCode = EXIT_SUCCESS;
	if( data != NULL ) {																	// в data LPSTR на имя экзешника TS
		HANDLE hStdOutRd, hStdOutWr;
		SECURITY_ATTRIBUTES sa = { sizeof( SECURITY_ATTRIBUTES ), NULL, TRUE };				// { nLength, lpSecurityDescriptor, bInheritHandle }
		if( CreatePipe( &hStdOutRd, &hStdOutWr, &sa, 0 ) ) {
			SetHandleInformation( hStdOutRd, HANDLE_FLAG_INHERIT, 0 );
			STARTUPINFO si = {0};
			si.cb = sizeof( STARTUPINFO );
			si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
			si.hStdOutput = hStdOutWr;
			si.hStdError = hStdOutWr;
			LPWSTR cmdLine = wStrReplace( NULL, ( LPCWSTR ) data, L" ", opt.args.get() );
			memset( &pi, 0, sizeof( PROCESS_INFORMATION ) );
			if( CreateProcess( ( LPCWSTR ) data, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi ) ) {
				CloseHandle( hStdOutWr );
				SendMessage( hMainWnd, UM_SERVERSTARTED, 0, 0 );
				DWORD avail, bread;
				char ch;
				for( ;; ) {
					if( !ReadFile( hStdOutRd, &ch, sizeof( ch ), &bread, NULL ) ) break;	// ждем из пайпа хотя бы 1 байт или кончины дочернего процесса
					if( !PeekNamedPipe( hStdOutRd, NULL, 0, NULL, &avail, NULL ) ) break;	// смотрим сколько еще там осталось
					LPSTR buf = ( LPSTR ) malloc( avail + 2 );								// выделяем память с учетом уже прочитанного байта и терминального нуля
					*buf = ch;
					ReadFile( hStdOutRd, buf + 1, avail, &bread, NULL );					// дочитываем хвосты
					buf [bread + 1] = '\0';
					for( LPSTR p = strtok( buf, "\n" ); p; p = strtok( NULL, "\n" ) ) {		// разбиваем буфер на подстроки
						SendMessage( hMainWnd, UM_NEWLINE, ( WPARAM ) p, ( LPARAM ) 0 );	// отсылаем мультибайтную строку в консоль
					}
					free( buf );
				}
				GetExitCodeProcess( pi.hProcess, &exitCode );
				CloseHandle( pi.hThread );
				CloseHandle( pi.hProcess );
			} else {
				CloseHandle( hStdOutWr );
				exitCode = CREATEPROCESSERROR;
			}
			CloseHandle( hStdOutRd );
			free( cmdLine );
		} else exitCode = CREATEPIPEERROR;
	} else exitCode = TSNOTFOUND;
	PostMessage( hMainWnd, UM_SERVERSTOPPED, ( WPARAM )exitCode, ( LPARAM ) 0 );			// ставим сообщение в очередь. не ждем обработки
	return exitCode;
}

void StartServer( LPCWSTR tsFileName ) {
	if( !ServerRunning() ) {
		CloseHandle( hThrd );
		hThrd = CreateThread( NULL, 0, Service_THRD, ( void* ) tsFileName, 0, NULL );
	}
}

void StopServer() {
	if( ServerRunning() ) {
		TerminateProcess( pi.hProcess, PROCESSTERMINATED );									// убиваем процесс TS с кодом возврата PROCESSTERMINATED
		WaitForSingleObject( hThrd, INFINITE );												// ждём завершения перехватывающего потока
	}
}

bool ServerRunning() {
	return ( WaitForSingleObject( hThrd, 0 ) == WAIT_TIMEOUT );
}