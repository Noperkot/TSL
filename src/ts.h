#ifndef TS_H
#define TS_H

enum ExitCodes_E { PROCESSTERMINATED = 31415, TSNOTFOUND, CREATEPIPEERROR, CREATEPROCESSERROR };
void StartServer( LPCWSTR tsFileName );
void StopServer();
bool ServerRunning();

#endif // TS_H
