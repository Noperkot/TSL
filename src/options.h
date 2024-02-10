#ifndef OPTIONS_H
#define OPTIONS_H

enum { ONDEAD_NOACTION = 0, ONDEAD_CLOSE, ONDEAD_SHOW, ONDEAD_RESTART };
enum { ONCLICK_SHOWHIDE = 0, ONCLICK_OPENWEB, ONCLICK_RESTART };
#define AUTO -1

void	Autostart( bool yes );
bool	Autostart();

class OPT {

	class _int_ {
	private:
		OPT *parent;
		const LPCWSTR name;
		INT val;
		BOOL noSave;
		void loadINI();
		void loadREG();
		BOOL loadARG();
		void saveINI();
		void saveREG();
	public:
		INT get();
		VOID set( INT val );
		INT load();
		void save( INT val );
		_int_( OPT *parentInit, LPCWSTR nameInit, INT valInit );
	};

	class _str_ {
	private:
		OPT *parent;
		const LPCWSTR name;
		LPWSTR val;
		BOOL noSave;
		void loadINI();
		void loadREG();
		BOOL loadARG();
		void saveINI();
		void saveREG();
	public:
		LPWSTR get();
		VOID set( LPCWSTR val );
		LPWSTR load();
		void save( LPWSTR val );
		_str_( OPT *parentInit, LPCWSTR nameInit, LPCWSTR valInit );
	};

	class _arg_ {
	public:
		LPWSTR * v;
		int	 c;
		_arg_( OPT * parent );
		// ~_arg_();
		template<typename T>
		BOOL search( LPCWSTR name, T cb );
		BOOL search( LPCWSTR name );
	};

	class _ini_ {
	public:
		LPWSTR path;
		_ini_( OPT * parent );
	};

public:
	_arg_	arg					{ this };											// аргументы командной строки (arg.v, arg.c)
	_ini_	ini					{ this };											// tsl.ini файл (ini.path)
	_int_	GrabVerLines		{ this, L"GrabVerLines"		, 10				};	// В скольких первых строках вывода TS искать его версию. Версия нужна для заголовка окна и в эбауте. 0 - не искать.
	_int_	WindowX				{ this, L"WindowX"			, AUTO				};	// X координата окна. Если не указано - автоцентрирование
	_int_	WindowY				{ this, L"WindowY"			, AUTO				};	// Y координата окна. Если не указано - автоцентрирование
	_int_	WindowW				{ this, L"WindowW"			, AUTO				};	// Ширина окна. Если не указано - 2/3 экрана
	_int_	WindowH				{ this, L"WindowH"			, AUTO				};	// Высота окна. Если не указано - 2/3 экрана
	_int_	WindowMinW			{ this, L"WindowMinW"		, 320				};	// Минимальная ширина окна
	_int_	WindowMinH			{ this, L"WindowMinH"		, 240				};	// Минимальная высота окна
	_int_	WindowMax			{ this, L"WindowMax"		, 0					};	// Развернуть на весь экран. 0-нет, 1-да
	_int_	TextWrapping		{ this, L"TextWrapping"		, 0					};	// Переносить строки в консоли. 0-нет, 1-да
	_int_	MaxLines			{ this, L"MaxLines"			, 1000				};	// Строк в кольцевм буфере консоли
	_int_	ExitWhenClose		{ this, L"ExitWhenClose"	, 0					};	// Действие при закрытии окна. 0 - сворачивать в трей, 1 - выход
	_int_	OnTSdead			{ this, L"OnTSdead"			, ONDEAD_NOACTION	};	// Действие при падении TS. 0 - ничего не делать, 1 - закрыть программу, 2 - развернуть окно программы, 3 - перезапустить TS
	_int_	OnIconClick			{ this, L"OnIconClick"		, ONCLICK_SHOWHIDE	};	// Действие при клике по иконке в трее. 0 - показать/скрыть окно, 1 - открыть веб-интерфейс TS, 2 - рестарт TS
	_int_	DblIconClick		{ this, L"DblIconClick"		, 0					};	// Клик по иконке в трее. 0 - одинарный, 1 - двойной.
	_int_	ConsoleBkColor		{ this, L"ConsoleBkColor"	, 0x000000			};	// hex цвет фона 0xRRGGBB
	_int_	ConsoleFontColor	{ this, L"ConsoleFontColor"	, 0xBBBBBB			};	// hex цвет шрифта 0xRRGGBB 0xED9121 - морковный
	_int_	ConsoleFontSize		{ this, L"ConsoleFontSize"	, 9					};	// Размер шрифта
	_str_	ConsoleFontName		{ this, L"ConsoleFontName"	, L"Lucida Console"	};	// Название шрифта
	_str_	args				{ this, L"args"				, L""				};	// Аргументы командной строки TS
	OPT();
};

extern OPT opt;

#endif // OPTIONS_H