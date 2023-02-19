; собирать NSIS 3.04(к нему vt относится лояльней, чем к 3.08), подписывать MD5withRCA

Unicode True

!include x64.nsh ; ${RunningX64}
!include FileFunc.nsh ; ${GetOptions}
!include StrFunc.nsh ; ${StrRep}
${StrRep}

; ------------------------- Defines -------------------------------------
!define INSTALLER_VERSION "2.0.0.1"
!define COPYRIGHT_STR "Copyright © 2023 ${AUTHORS}"
!define APPNAME "TorrServer"
!define TSLEXE "tsl.exe"
!define OUTDIR "../build"
!define PRODUCT_PUBLISHER "Noperkot"
!define UNINST "Uninstall"
!define REG_UNINST_SUBKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define REG_RUN_SUBKEY "Software\Microsoft\Windows\CurrentVersion\Run"
!define INSTALLICON "../src/res/images/favicon_16-24-32-48(8-32colors).ico"
!define UNINSTALLICON "../src/res/images/faviconX_16-24-32-48(8-32colors).ico"
!define SETUPDIR "$INSTDIR\Setup"
!define LINKSDIR "$INSTDIR\Links"
!define SHORTCUTSDIR "$INSTDIR\Shortcuts"
!define ONLINE_INSTALLER "TorrServer_Setup.exe"

; ------------------------- Main settings -------------------------------
Name "${APPNAME}"
Caption "${CAPTION}"
UninstallCaption "${APPNAME} Uninstaller"
InstallDir "$APPDATA\${APPNAME}"
InstallDirRegKey HKCU "Software\${APPNAME}" ""
OutFile "${OUTDIR}/${INSTALLER}"
SetCompressor LZMA
; SetCompressor BZip2
; SetCompress off
ManifestDPIAware true
RequestExecutionLevel user
AllowRootDirInstall true
BrandingText " " ; убираем из окна инсталлятора строку строку "Nullsoft Install System v3.08"
SpaceTexts none ; убираем требуемое место на диске
; ShowInstDetails show
; ShowUnInstDetails show

; ------------------------- Variables -----------------------------------
Var	TorrServer_ver
Var	TSL_ver
Var installedTorrServer_ver
Var installedTSL_ver
Var alreadyInstalled
Var versionsText
Var AutorunCheckbox
Var DesktopShortcutCheckbox
Var ChromeCheckbox
Var FirefoxCheckbox

; ------------------------- MUI settings -------------------------------
!include "MUI2.nsh"
!define MUI_ICON ${INSTALLICON}
!define MUI_UNICON ${UNINSTALLICON}
; !define MUI_ABORTWARNING

!define MUI_PAGE_CUSTOMFUNCTION_PRE fWelcomePre
!insertmacro MUI_PAGE_WELCOME

!define MUI_PAGE_CUSTOMFUNCTION_PRE fDirectoryPre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW fDirectoryShow ; проверка существующей установки. если существует то пригасим выбор директории
!define MUI_DIRECTORYPAGE_TEXT_TOP $versionsText
!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

; !define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN "$INSTDIR\${TSLEXE}"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW fFinishShow ; генерируем дополнительные чекбоксы
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE fFinishLeave ; выполняем дополнительные функции
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_RESERVEFILE_LANGDLL

; -------------------------- Version Information -------------------------
VIProductVersion "${INSTALLER_VERSION}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "ProductName" "${CAPTION}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "CompanyName" "Noperkot"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "LegalCopyright" "${COPYRIGHT_STR}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "FileDescription" "${CAPTION}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "FileVersion" "${INSTALLER_VERSION}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "ProductVersion" "${PRODUCT_VERSION}"

; ------------------------------------------------------------------------

!macro SHARED prefix ; Общие функции, используемые как в инсталляторе, так и в деинсталляторе
	Function ${prefix}CloseTS ; Гасим торрсервер запущенный через tsl.exe
		FindWindow $0 "TorrServerLauncher"
		SendMessage $0 ${WM_DESTROY} 0 0
		Sleep 500
	FunctionEnd
!macroend
!insertmacro SHARED ""
!insertmacro SHARED "un."

!macro exit message
	MessageBox MB_OK|MB_ICONSTOP "${message}"
	Quit
!macroend

!macro setupShortcut ver
	CreateShortCut "${SETUPDIR}\$(_OLD_VERSIONS_)\${APPNAME}_${ver}_Setup.lnk" "$INSTDIR\${ONLINE_INSTALLER}" "/V ${ver}"
!macroend

!macro tslShortcut arg
	CreateShortCut "${SHORTCUTSDIR}\${TSLEXE}${arg}.lnk" "$INSTDIR\${TSLEXE}" "${arg}"
!macroend

Function fWelcomePre
	; проверка уже запущенного экземпляра
	System::Call 'kernel32::CreateMutex(i 0, i 0, t "TorrServerSetup") i .r1 ?e'
	Pop $0
	${IfNot} $0 == 0
		!insertmacro exit "$(_ALREADY_RUNNING_)"
	${EndIf}

	; пропуск страницы WELCOME
	ClearErrors
	${GetOptions} $CMDLINE "/SkipWelcome" $0
	${IfNot} ${Errors}
		Abort
	${EndIf}
FunctionEnd

Function fDirectoryPre
	;гасим кнопку "Назад"
	GetDlgItem $0 $HWNDPARENT 3
	EnableWindow $0 0

	; получаем версии устанавливаемых компонентов
	!insertmacro getVersions

	; получаем версии уже устанновленных компонентов
	ReadRegStr $installedTorrServer_ver HKCU "${REG_UNINST_SUBKEY}" "DisplayVersion"
	ReadRegStr $installedTSL_ver HKCU "${REG_UNINST_SUBKEY}" "TSLVersion"

	; проверка существующей установки
	ClearErrors
	ReadRegStr $0 HKCU "Software\${APPNAME}" ""
	${IfNot} ${Errors}
		${StrRep} $0 $0 '"' ''
		StrCpy $INSTDIR $0
	${EndIf}
	${If} ${FileExists} "$INSTDIR\${UNINST}.exe" ; Смотрим есть ли по этому пути файл Uninstall.exe
		StrCpy $alreadyInstalled 1
	${EndIf}

	; формируем текст на странице версий
	StrCpy $0 ""
	${IfNot} $installedTorrServer_ver == $TorrServer_ver
		StrCpy $0 "$0TorrServer $TorrServer_ver$\n$\n"
	${EndIf}
	${IfNot} $installedTSL_ver == $TSL_ver
		StrCpy $0 "$0TorrServer Launcher $TSL_ver"
	${EndIf}
	${If} $0 == ""
		StrCpy $versionsText "$(_NO_UPDATES_FOUND_)"
	${Else}
		${If} $alreadyInstalled == 1
			StrCpy $versionsText "$(_UPDATES_AVAILABLE_)"
		${Else}
			StrCpy $versionsText "$(_TO_INSTALL_)"
		${EndIf}
	${EndIf}
	StrCpy $versionsText "———  $versionsText  ———$\n$\n$\n$0"
FunctionEnd


Function fDirectoryShow
	;меняем шрифт в окне версий
	FindWindow $2 "#32770" "" $HWNDPARENT
	GetDlgItem $0 $2 1006
	CreateFont $R0 "Microsoft Sans Serif" "9" "700"
	${If} $installedTorrServer_ver == $TorrServer_ver
	${AndIf} $installedTSL_ver == $TSL_ver
		SetCtlColors $0 0x6D6D6D "transparent"
	${Else}
		SetCtlColors $0 0x0066CC "transparent"
	${EndIf}
	SendMessage $0 ${WM_SETFONT} $R0 0

	; если TS уже установлен
	${If} $alreadyInstalled == 1
		FindWindow $R1 "#32770" "" $HWNDPARENT
		GetDlgItem $R2 $R1 1019 ; гасим строку с путем
		EnableWindow $R2 0
		GetDlgItem $R2 $R1 1001 ; гасим кнопку выбора пути
		EnableWindow $R2 0
		; сообщаем что это переустановка
		SendMessage $mui.Header.Text ${WM_SETTEXT} 0 "STR:$(_REINSTALL_TEXT_)"
		SendMessage $mui.Header.SubText ${WM_SETTEXT} 0 "STR:$(_REINSTALL_SUBTEXT_)"
	${EndIf}
FunctionEnd

Function fFinishShow ; добавляем свои чекбоксы на финишную страницу

	${NSD_CreateCheckbox} 120u 110u 100% 10u "&$(_LAUNCH_ON_LOGON_)"
	Pop $AutorunCheckbox
	${NSD_SetState} $AutorunCheckbox 1
	SetCtlColors $AutorunCheckbox "" "ffffff"

	${NSD_CreateCheckbox} 120u 130u 100% 10u "&$(_DESKTOP_SHORTCUT_)"
	Pop $DesktopShortcutCheckbox
	${NSD_SetState} $DesktopShortcutCheckbox 1
	SetCtlColors $DesktopShortcutCheckbox "" "ffffff"

	${NSD_CreateCheckbox} 120u 155u 100% 10u "&$(_EXTENSION_FOR_) Chrome (web)"
	Pop $ChromeCheckbox
	${NSD_SetState} $ChromeCheckbox 0
	SetCtlColors $ChromeCheckbox "" "ffffff"

	${NSD_CreateCheckbox} 120u 175u 100% 10u "&$(_EXTENSION_FOR_) Firefox (web)"
	Pop $FirefoxCheckbox
	${NSD_SetState} $FirefoxCheckbox 0
	SetCtlColors $FirefoxCheckbox "" "ffffff"

FunctionEnd

Function fFinishLeave ; проверяем чекбоксы на финальной странице по кнопке "Готово"
	HideWindow

	${NSD_GetState} $mui.FinishPage.Run $0 ; перед стартом убиваем все левые процессы торрсервера
	${If} $0 <> 0
		KillProcDLL::KillProc "TorrServer-windows-386.exe"
		KillProcDLL::KillProc "TorrServer-windows-amd64.exe"
		KillProcDLL::KillProc "TorrServer.exe"
	${EndIf}

	${NSD_GetState} $AutorunCheckbox $0
	${If} $0 <> 0
		WriteRegStr HKCU "${REG_RUN_SUBKEY}" "${APPNAME}" '"$INSTDIR\${TSLEXE}" --silent'
	${EndIf}

	${NSD_GetState} $DesktopShortcutCheckbox $0
	${If} $0 <> 0
		CreateShortCut "$DESKTOP\${APPNAME}.lnk" '"$INSTDIR\${TSLEXE}"'
	${EndIf}

	${NSD_GetState} $ChromeCheckbox $0
	${If} $0 <> 0
		ExecShell "open" "https://chrome.google.com/webstore/detail/torrserver-adder/ihphookhabmjbgccflngglmidjloeefg"
	${EndIf}

	${NSD_GetState} $FirefoxCheckbox $0
	${If} $0 <> 0
		ExecShell "open" "https://addons.mozilla.org/firefox/addon/torrserver-adder"
	${EndIf}

FunctionEnd

!macro commonInstallSection

	Call CloseTS ; стопорим сервер(если запущен)
	DeleteRegValue HKCU "${REG_RUN_SUBKEY}" "${APPNAME}" ; удаляем автостарт
	Delete "$DESKTOP\${APPNAME}.lnk" ; удаляем иконку на рабочем столе
	SetOutPath "$INSTDIR"
	SetOverwrite on
	WriteUninstaller "$INSTDIR\${UNINST}.exe"	; деинсталлятор

	; создаем записи в реестре
	WriteRegStr HKCU "Software\${APPNAME}" "" '"$INSTDIR"'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "DisplayName" "${APPNAME}"
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "DisplayVersion" "$TorrServer_ver"
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "TSLVersion" "$TSL_ver"
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "UninstallString" '"$INSTDIR\${UNINST}.exe"'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "ModifyPath" '"$INSTDIR\${ONLINE_INSTALLER}" /SkipWelcome'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "DisplayIcon" '"$INSTDIR\${TSLEXE}",0'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegDWORD HKCU "${REG_UNINST_SUBKEY}" "NoModify" 0
	WriteRegDWORD HKCU "${REG_UNINST_SUBKEY}" "NoRepair" 1

	; создаем папку ярлыков установщиков
	CreateDirectory "${SETUPDIR}\$(_OLD_VERSIONS_)"
	CreateShortCut "${SETUPDIR}\$(_CHECK_UPDATES_).lnk" "$INSTDIR\${ONLINE_INSTALLER}" "/SkipWelcome"
	CreateShortCut "${SETUPDIR}\$(_UNINSTALL_).lnk" "$INSTDIR\${UNINST}.exe"
	!insertmacro setupShortcut "1.1.65"
	!insertmacro setupShortcut "1.1.68"
	!insertmacro setupShortcut "1.1.77"
	!insertmacro setupShortcut "MatriX.106"
	!insertmacro setupShortcut "MatriX.109"
	!insertmacro setupShortcut "MatriX.110"
	!insertmacro setupShortcut "MatriX.112"
	!insertmacro setupShortcut "MatriX.114"
	!insertmacro setupShortcut "MatriX.115"
	!insertmacro setupShortcut "MatriX.116"
	!insertmacro setupShortcut "MatriX.117"
	!insertmacro setupShortcut "MatriX.118"
	!insertmacro setupShortcut "MatriX.119"

	; создаем папку ярлыков запуска
	CreateDirectory "${SHORTCUTSDIR}"
	!insertmacro tslShortcut ""
	!insertmacro tslShortcut " --start"
	; !insertmacro tslShortcut " --stop"
	!insertmacro tslShortcut " --close"
	!insertmacro tslShortcut " --restart"
	!insertmacro tslShortcut " --show"
	!insertmacro tslShortcut " --hide"
	!insertmacro tslShortcut " --web"
	; !insertmacro tslShortcut " --reset"

	; создаем папку ссылок
	RMDir /r "$INSTDIR\Ссылки"
	CreateDirectory "${LINKSDIR}"
	WriteIniStr "${LINKSDIR}\$(_EXTENSION_FOR_) Chrome.url" "InternetShortcut" "URL" "https://chrome.google.com/webstore/detail/torrserver-adder/ihphookhabmjbgccflngglmidjloeefg"
	WriteIniStr "${LINKSDIR}\$(_EXTENSION_FOR_) Firefox.url" "InternetShortcut" "URL" "https://addons.mozilla.org/firefox/addon/torrserver-adder"
	WriteIniStr "${LINKSDIR}\TorrServer.url" "InternetShortcut" "URL" "https://github.com/YouROK/TorrServer"
	WriteIniStr "${LINKSDIR}\TSL.url" "InternetShortcut" "URL" "https://github.com/Noperkot/TSL"

	; создаем папку в меню "Старт"
	RMDir /r "$SMPROGRAMS\${APPNAME}"
	CreateDirectory "$SMPROGRAMS\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\${TSLEXE}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\$(_LINKS_).lnk" "${LINKSDIR}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\$(_SETUP_).lnk" "${SETUPDIR}"

!macroend


Section Uninstall
	Call un.CloseTS ; стопорим сервер(если запущен)
	;Remove from registry...
	DeleteRegKey HKCU "${REG_UNINST_SUBKEY}"
	DeleteRegKey HKCU "SOFTWARE\${APPNAME}"
	DeleteRegValue HKCU "${REG_RUN_SUBKEY}" "${APPNAME}"
	; Delete Shortcuts
	Delete "$DESKTOP\${APPNAME}.lnk"
	RMDir /r "$SMPROGRAMS\${APPNAME}"
	; Clean up Application
	Delete "$INSTDIR\${TSLEXE}"
	Delete "$INSTDIR\TorrServer-windows-386.exe"
	Delete "$INSTDIR\TorrServer-windows-amd64.exe"
	Delete "$INSTDIR\config.db"
	Delete "$INSTDIR\torrserver.db"
	Delete "$INSTDIR\torrserver.db.lock"
	Delete "$INSTDIR\rutor.ls"
	Delete "$INSTDIR\${ONLINE_INSTALLER}"
	RMDir /r "$INSTDIR\Ссылки"
	RMDir /r "${LINKSDIR}"
	RMDir /r "${SETUPDIR}"
	RMDir /r "${SHORTCUTSDIR}"
	Delete "$INSTDIR\${UNINST}.exe"
	RMDir "$INSTDIR\"
SectionEnd

LangString _ALREADY_RUNNING_ ${LANG_RUSSIAN} "Установка уже выполняется" ; $(_ALREADY_RUNNING_)
LangString _ALREADY_RUNNING_ ${LANG_ENGLISH} "The installer is already running"

LangString _REINSTALL_TEXT_ ${LANG_RUSSIAN} "Обновление установки" ; $(_REINSTALL_TEXT_)
LangString _REINSTALL_TEXT_ ${LANG_ENGLISH} "Updating the installation"

LangString _REINSTALL_SUBTEXT_ ${LANG_RUSSIAN} "Переустановка возможна только в существующую папку. Для выбора другого расположения удалите TorrServer и выполните установку заново." ; $(_REINSTALL_SUBTEXT_)
LangString _REINSTALL_SUBTEXT_ ${LANG_ENGLISH} "Reinstalling is only possible in an existing folder. To select a different location, delete TorrServer and perform the installation again."

LangString _EXTENSION_FOR_ ${LANG_RUSSIAN} "Расширение для" ; $(_EXTENSION_FOR_)
LangString _EXTENSION_FOR_ ${LANG_ENGLISH} "Extension for"

LangString _LAUNCH_ON_LOGON_ ${LANG_RUSSIAN} "Запускать при входе в Windows" ; $(_LAUNCH_ON_LOGON_)
LangString _LAUNCH_ON_LOGON_ ${LANG_ENGLISH} "Launch on logon"

LangString _DESKTOP_SHORTCUT_ ${LANG_RUSSIAN} "Ярлык на Рабочий стол" ; $(_DESKTOP_SHORTCUT_)
LangString _DESKTOP_SHORTCUT_ ${LANG_ENGLISH} "Create shortcut on Desktop"

LangString _LINKS_ ${LANG_RUSSIAN} "Ссылки" ; $(_LINKS_)
LangString _LINKS_ ${LANG_ENGLISH} "Links"

LangString _SETUP_ ${LANG_RUSSIAN} "Установка" ; $(_SETUP_)
LangString _SETUP_ ${LANG_ENGLISH} "Setup"

LangString _CHECK_UPDATES_ ${LANG_RUSSIAN} "Проверить обновления" ; $(_CHECK_UPDATES_)
LangString _CHECK_UPDATES_ ${LANG_ENGLISH} "Check for updates"

LangString _UNINSTALL_ ${LANG_RUSSIAN} "Удалить" ; $(_UNINSTALL_)
LangString _UNINSTALL_ ${LANG_ENGLISH} "Uninstall"

LangString _OLD_VERSIONS_ ${LANG_RUSSIAN} "Прошлые версии" ; $(_OLD_VERSIONS_)
LangString _OLD_VERSIONS_ ${LANG_ENGLISH} "Old versions"

LangString _NO_UPDATES_FOUND_ ${LANG_RUSSIAN} "Обновлений не найдено" ; $(_NO_UPDATES_FOUND_)
LangString _NO_UPDATES_FOUND_ ${LANG_ENGLISH} "No updates found"

LangString _UPDATES_AVAILABLE_ ${LANG_RUSSIAN} "Доступно обновление" ; $(_UPDATES_AVAILABLE_)
LangString _UPDATES_AVAILABLE_ ${LANG_ENGLISH} "Update available"

LangString _TO_INSTALL_ ${LANG_RUSSIAN} "Будут установлены следующие продукты:" ; $(_TO_INSTALL_)
LangString _TO_INSTALL_ ${LANG_ENGLISH} "The following products will be installed:"