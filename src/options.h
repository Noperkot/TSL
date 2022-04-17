#ifndef OPTIONS_H
#define OPTIONS_H

enum { ONDEAD_NOACTION = 0, ONDEAD_CLOSE, ONDEAD_RESTART };
enum { ONCLICK_SHOWHIDE = 0, ONCLICK_OPENWEB };
#define AUTO -1

struct OPTIONS_S {
	INT		GrabVerLines;
	INT		WindowX;
	INT		WindowY;
	INT		WindowW;
	INT		WindowH;
	INT		WindowMinW;
	INT		WindowMinH;
	INT		WindowMax;
	INT		TextWrapping;
	INT		MaxLines;
	INT		ExitWhenClose;
	INT		OnTSdead;
	INT		OnIconClick;
	INT		ConsoleBkColor;
	INT		ConsoleFontColor;
	INT		ConsoleFontSize;
	LPWSTR 	ConsoleFontName;
	LPWSTR 	args;

	void load();
};

void	opt_reset ();
INT		reg_load  ( LPCWSTR ValName, CONST INT defaultVal = 0 );
LSTATUS reg_save  ( LPCWSTR ValName, CONST INT IntVal );
LSTATUS reg_load  ( LPCWSTR ValName, LPWSTR *StrVal, LPCWSTR defaultVal = L"" );
LSTATUS reg_save  ( LPCWSTR ValName, LPCWSTR StrVal );
void	Autostart ( bool yes );
bool	Autostart ();

#endif // OPTIONS_H