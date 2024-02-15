#ifndef OPTIONS_H
#define OPTIONS_H

enum { ONDEAD_NOACTION = 0, ONDEAD_CLOSE, ONDEAD_SHOW, ONDEAD_RESTART };
enum { ONCLICK_SHOWHIDE = 0, ONCLICK_OPENWEB, ONCLICK_RESTART };
#define AUTO -1

void	Autostart( bool yes );
bool	Autostart();

class wStr {
protected:
	LPWSTR val;
public:
	void reserve( DWORD size = 0 );
	LPWSTR get();
	wStr& set( LPCWSTR str1, LPCWSTR str2 = L"", LPCWSTR str3 = L"" );		// строка склейкой из 3-х строк
	wStr& set( UINT idstr1 );												// строка из ресурсов
	wStr( LPCWSTR str1 = NULL, LPCWSTR str2 = L"", LPCWSTR str3 = L"" );
	wStr( UINT idstr1 );
	~wStr();
};

class ARG { // аргументы командной строки (arg.v, arg.c)
public:
	LPWSTR * v;
	int	 c;
	ARG();
	// ~ARG();
	template<typename T>
	BOOL search( LPCWSTR name, T cb );
	BOOL search( LPCWSTR name );
};

class INI { // tsl.ini файл (ini.path)
public:
	wStr path;
	INI();
};

class OPT {

	class _int_ {
	private:
		const LPCWSTR name;
		INT val;
		BOOL noSave;
		BOOL loadINI();
		void loadREG();
		BOOL loadARG();
		BOOL saveINI();
		void saveREG();
	public:
		INT get();
		VOID set( INT val );
		INT load();
		void save( INT val );
		_int_( LPCWSTR nameInit, INT valInit );
	};

	class _str_ : public wStr {
	private:
		const LPCWSTR name;
		BOOL noSave;
		BOOL loadINI();
		void loadREG();
		BOOL loadARG();
		BOOL saveINI();
		void saveREG();
	public:
		LPWSTR load();
		void save( LPWSTR val );
		_str_( LPCWSTR nameInit, LPCWSTR valInit );
	};

public:
	_int_	GrabVerLines		{ L"GrabVerLines"		, 10				};	// В скольких первых строках вывода TS искать его версию. Версия нужна для заголовка окна и в эбауте. 0 - не искать.
	_int_	WindowX				{ L"WindowX"			, AUTO				};	// X координата окна. Если не указано - автоцентрирование
	_int_	WindowY				{ L"WindowY"			, AUTO				};	// Y координата окна. Если не указано - автоцентрирование
	_int_	WindowW				{ L"WindowW"			, AUTO				};	// Ширина окна. Если не указано - 2/3 экрана
	_int_	WindowH				{ L"WindowH"			, AUTO				};	// Высота окна. Если не указано - 2/3 экрана
	_int_	WindowMinW			{ L"WindowMinW"			, 320				};	// Минимальная ширина окна
	_int_	WindowMinH			{ L"WindowMinH"			, 240				};	// Минимальная высота окна
	_int_	WindowMax			{ L"WindowMax"			, 0					};	// Развернуть на весь экран. 0-нет, 1-да
	_int_	TextWrapping		{ L"TextWrapping"		, 0					};	// Переносить строки в консоли. 0-нет, 1-да
	_int_	MaxLines			{ L"MaxLines"			, 1000				};	// Строк в кольцевм буфере консоли
	_int_	ExitWhenClose		{ L"ExitWhenClose"		, 0					};	// Действие при закрытии окна. 0 - сворачивать в трей, 1 - выход
	_int_	OnTSdead			{ L"OnTSdead"			, ONDEAD_NOACTION	};	// Действие при падении TS. 0 - ничего не делать, 1 - закрыть программу, 2 - развернуть окно программы, 3 - перезапустить TS
	_int_	OnIconClick			{ L"OnIconClick"		, ONCLICK_SHOWHIDE	};	// Действие при клике по иконке в трее. 0 - показать/скрыть окно, 1 - открыть веб-интерфейс TS, 2 - рестарт TS
	_int_	DblIconClick		{ L"DblIconClick"		, 0					};	// Клик по иконке в трее. 0 - одинарный, 1 - двойной.
	_int_	ConsoleBkColor		{ L"ConsoleBkColor"		, 0x000000			};	// hex цвет фона 0xRRGGBB
	_int_	ConsoleFontColor	{ L"ConsoleFontColor"	, 0xBBBBBB			};	// hex цвет шрифта 0xRRGGBB 0xED9121 - морковный
	_int_	ConsoleFontSize		{ L"ConsoleFontSize"	, 9					};	// Размер шрифта
	_str_	ConsoleFontName		{ L"ConsoleFontName"	, L"Lucida Console"	};	// Название шрифта
	_str_	args				{ L"args"				, L""				};	// Аргументы командной строки TS
	OPT();
};

extern ARG arg;
extern INI ini;
extern OPT opt;

#endif // OPTIONS_H