!include x64.nsh
; !include nsDialogs.nsh
; !include WinMessages.nsh

; ******** Configuration ********


!ifndef APPVERSION
	!define APPVERSION "MatriX.120"
	; 
	; !define APPVERSION "MatriX.119"	
	; !define APPVERSION "MatriX.118"
	; !define APPVERSION "MatriX.117"
	; !define APPVERSION "MatriX.116"
	; !define APPVERSION "MatriX.115"
	; !define APPVERSION "MatriX.114"
	; !define APPVERSION "MatriX.112"
	; !define APPVERSION "MatriX.110"
	; !define APPVERSION "MatriX.109"
	; !define APPVERSION "MatriX.106"
	; !define APPVERSION "1.1.77"
	; !define APPVERSION "1.1.68"
	; !define APPVERSION "1.1.65"
!endif

!define TSLVERSION "1.7.1"

!define COPYRIGHT_STR "Copyright © 2023 YouROK, Noperkot"


!define APPNAME "TorrServer"
!define TSDIR "TorrServers/${APPVERSION}"
!define TorrServerEXE32 "TorrServer-windows-386.exe"
!define TorrServerEXE64 "TorrServer-windows-amd64.exe"
!define TSLDIR "../build/signed/${TSLVERSION}"
!define TSLEXE "tsl.exe"
!define OUTDIR "../build"
!define OUTFILE "${OUTDIR}/${APPNAME}_${APPVERSION}_setup.exe"
!define PRODUCT_PUBLISHER "Noperkot"
!define UNINST "Uninstall"
!define REG_UNINST_SUBKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define REG_RUN_SUBKEY "Software\Microsoft\Windows\CurrentVersion\Run"
!define INSTALLICON "../src/res/images/favicon_16-32-48(8-32colors).ico"
!define UNINSTALLICON "../src/res/images/faviconX_16-32-48(8-32colors).ico"
; *******************************


; Main Install settings
Unicode True
Name "${APPNAME}"
Caption "${APPNAME} ${APPVERSION}"
UninstallCaption "${APPNAME}"
InstallDir "$APPDATA\${APPNAME}"
InstallDirRegKey HKCU "Software\${APPNAME}" ""
OutFile ${OUTFILE}
; SetCompressor BZip2
SetCompressor LZMA 
; SetCompress off
ManifestDPIAware true
RequestExecutionLevel user
AllowRootDirInstall true
BrandingText " " ; убираем из окна инсталятора строку строку "Nullsoft Install System v3.08"
SpaceTexts none ; убираем требуемое место на диске
; ShowInstDetails show
; ShowUnInstDetails show



; Modern interface settings
!include "MUI2.nsh"

!define MUI_ICON ${INSTALLICON}
!define MUI_UNICON ${UNINSTALLICON}

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_SHOW CheckInstDirReg ; проверка существующей установки. если существует то пригасим выбор директории
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${TSLEXE}"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW MyFinishShow ; генерируем дополнительные чекбоксы
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE MyFinishLeave ; выполняем дополнительные функции
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_RESERVEFILE_LANGDLL


;Version Information
VIProductVersion "${TSLVERSION}.0"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "CompanyName" "Noperkot"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "LegalCopyright" "${COPYRIGHT_STR}"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "FileDescription" "${APPNAME} ${APPVERSION} Setup"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "FileVersion" "${TSLVERSION}.0"
VIAddVersionKey  /LANG=${LANG_ENGLISH} "ProductVersion" "TS-${APPVERSION}, TSL-${TSLVERSION}"
;VIAddVersionKey /LANG=${LANG_ENGLISH} "OriginalFilename" "newsetup.exe"
;------------------------------------------------------------------------

; Create the shared function.
!macro MYMACRO un
	Function ${un}CloseTS ; Гасим торрсервер запущенный через tsl.exe
		FindWindow $0 "TorrServerLauncher"
		SendMessage $0 ${WM_DESTROY} 0 0
		Sleep 500 
	FunctionEnd
!macroend
; Insert function as an installer and uninstaller function.
!insertmacro MYMACRO ""
!insertmacro MYMACRO "un."

Section "TorrServerInstall" Section1
	
	Call CloseTS ; стопорим сервер(если запущен)	
	DeleteRegValue HKCU "${REG_RUN_SUBKEY}" "${APPNAME}" ; удаляем автостарт
	Delete "$DESKTOP\${APPNAME}.lnk" ; удаляем иконку на рабочем столе
	SetOutPath "$INSTDIR\"
	SetOverwrite on
	
	; файлы
	File "${TSLDIR}\${TSLEXE}"
	${If} ${RunningX64}
	File "${TSDIR}\${TorrServerEXE64}"
	${Else}
	File "${TSDIR}\${TorrServerEXE32}"
	${EndIf}
	
	; создаем папку ссылок
	CreateDirectory "$INSTDIR\$(_LINKS_)"
	WriteIniStr "$INSTDIR\$(_LINKS_)\$(_EXTENSION_FOR_) Chrome.url" "InternetShortcut" "URL" "https://chrome.google.com/webstore/detail/torrserver-adder/ihphookhabmjbgccflngglmidjloeefg"
	WriteIniStr "$INSTDIR\$(_LINKS_)\$(_EXTENSION_FOR_) Firefox.url" "InternetShortcut" "URL" "https://addons.mozilla.org/firefox/addon/torrserver-adder"
	WriteIniStr "$INSTDIR\$(_LINKS_)\TorrServer.url" "InternetShortcut" "URL" "https://github.com/YouROK/TorrServer"
	WriteIniStr "$INSTDIR\$(_LINKS_)\TSL.url" "InternetShortcut" "URL" "https://github.com/Noperkot/TSL"
	
	; создаем папку в меню "Старт"
	CreateDirectory "$SMPROGRAMS\${APPNAME}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\${TSLEXE}"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\$(_UNINSTALL_) ${APPNAME}.lnk" "$INSTDIR\${UNINST}.exe"
	CreateShortCut "$SMPROGRAMS\${APPNAME}\$(_LINKS_).lnk" "$INSTDIR\$(_LINKS_)"	

SectionEnd

Section -FinishSection
	
	; создаем записи в реестре
	WriteRegStr HKCU "Software\${APPNAME}" "" '"$INSTDIR"'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "DisplayName" "${APPNAME}"
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "UninstallString" '"$INSTDIR\${UNINST}.exe"'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "DisplayIcon" '"$INSTDIR\${TSLEXE}",0'
	WriteRegStr HKCU "${REG_UNINST_SUBKEY}" "Publisher" "${PRODUCT_PUBLISHER}"
	WriteRegDWORD HKCU "${REG_UNINST_SUBKEY}" "NoModify" 1
	WriteRegDWORD HKCU "${REG_UNINST_SUBKEY}" "NoRepair" 1
	
	; деинсталятор
	WriteUninstaller "$INSTDIR\${UNINST}.exe"		

SectionEnd


;Uninstall section
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
	Delete "$INSTDIR\${TorrServerEXE32}"
	Delete "$INSTDIR\${TorrServerEXE64}"
	Delete "$INSTDIR\config.db"
	Delete "$INSTDIR\torrserver.db"
	Delete "$INSTDIR\torrserver.db.lock"
	; RMDir /r "$INSTDIR\$(_LINKS_)"
	RMDir /r "$INSTDIR\Links"
	RMDir /r "$INSTDIR\Ссылки"	
	Delete "$INSTDIR\${UNINST}.exe"	
	RMDir "$INSTDIR\"

SectionEnd

/* Function un.onUninstSuccess
	HideWindow
	MessageBox MB_ICONINFORMATION|MB_OK "Удаление программы ${APPNAME} было успешно завершено."
FunctionEnd */

; Function un.onInit
	; MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Вы уверены в том, что желаете удалить ${APPNAME} и все компоненты программы?" IDYES +2
	; Abort
; FunctionEnd


Var AutorunCheckbox
Var DesktopShortcutCheckbox
; Var OpenDirCheckbox
Var ChromeCheckbox
Var FirefoxCheckbox

Function MyFinishShow ; добавляем свои чекбоксы на финальную страницу
	
	${NSD_CreateCheckbox} 120u 110u 100% 10u "&$(_LAUNCH_ON_LOGON_)"
	Pop $AutorunCheckbox
	${NSD_SetState} $AutorunCheckbox 1
	SetCtlColors $AutorunCheckbox "" "ffffff"
	
	${NSD_CreateCheckbox} 120u 130u 100% 10u "&$(_DESKTOP_SHORTCUT_)"
	Pop $DesktopShortcutCheckbox
	${NSD_SetState} $DesktopShortcutCheckbox 1
	SetCtlColors $DesktopShortcutCheckbox "" "ffffff"
	
	; ${NSD_CreateCheckbox} 120u 150u 100% 10u "&Открыть папку ${APPNAME}"
	; Pop $OpenDirCheckbox
	; ${NSD_SetState} $OpenDirCheckbox 1
	; SetCtlColors $OpenDirCheckbox "" "ffffff"
	
	${NSD_CreateCheckbox} 120u 155u 100% 10u "&$(_EXTENSION_FOR_) Chrome (web)"
	Pop $ChromeCheckbox
	${NSD_SetState} $ChromeCheckbox 0
	SetCtlColors $ChromeCheckbox "" "ffffff"
	
	${NSD_CreateCheckbox} 120u 175u 100% 10u "&$(_EXTENSION_FOR_) Firefox (web)"
	Pop $FirefoxCheckbox
	${NSD_SetState} $FirefoxCheckbox 0
	SetCtlColors $FirefoxCheckbox "" "ffffff"
	
FunctionEnd

Function MyFinishLeave ; проверяем чекбоксы на финальной странице по кнопке "Готово"
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
	
	; ${NSD_GetState} $OpenDirCheckbox $0
	; ${If} $0 <> 0
		; ExecShell "open" "$INSTDIR" 
	; ${EndIf}

	${NSD_GetState} $ChromeCheckbox $0
	${If} $0 <> 0
		ExecShell "open" "https://chrome.google.com/webstore/detail/torrserver-adder/ihphookhabmjbgccflngglmidjloeefg" 
	${EndIf}
	
	${NSD_GetState} $FirefoxCheckbox $0
	${If} $0 <> 0
		ExecShell "open" "https://addons.mozilla.org/firefox/addon/torrserver-adder"
	${EndIf}	
	
FunctionEnd

!include StrFunc.nsh
${StrRep} 
Function CheckInstDirReg ; проверяем существующую установку
	ReadRegStr $R0 HKCU "Software\${APPNAME}" "" ; читаем путь установки из реестра
    ${StrRep} $R0 $R0 '"' ''
	${If} ${FileExists} "$R0\${UNINST}.exe" ; Смотрим есть ли по этому пути файл Uninstall.exe, если да то гасим выбор директории
		FindWindow $R1 "#32770" "" $HWNDPARENT 
		GetDlgItem $R2 $R1 1019 ; гасим строку с путем
		EnableWindow $R2 0
		GetDlgItem $R2 $R1 1001 ; гасим кнопку выбора пути
		EnableWindow $R2 0
	${EndIf}   
FunctionEnd

!ifndef TorrServerEXE32
	; !include WinVer.nsh	
	Function .onInit
		${IfNot} ${RunningX64} ; проверка разрядности винды. ТЛЬКО ДЛЯ Matrix.111
			MessageBox MB_OK|MB_ICONSTOP $(_REQUIRE64WIN_)
			Quit
		${EndIf}
	FunctionEnd
!endif

LangString _LINKS_ ${LANG_RUSSIAN} "Ссылки" ; $(_LINKS_)
LangString _LINKS_ ${LANG_ENGLISH} "Links"

LangString _EXTENSION_FOR_ ${LANG_RUSSIAN} "Расширение для" ; $(_EXTENSION_FOR_)
LangString _EXTENSION_FOR_ ${LANG_ENGLISH} "Extension for"

LangString _LAUNCH_ON_LOGON_ ${LANG_RUSSIAN} "Запускать при входе в Windows" ; $(_LAUNCH_ON_LOGON_)
LangString _LAUNCH_ON_LOGON_ ${LANG_ENGLISH} "Launch on logon"

LangString _DESKTOP_SHORTCUT_ ${LANG_RUSSIAN} "Ярлык на Рабочий стол" ; $(_DESKTOP_SHORTCUT_)
LangString _DESKTOP_SHORTCUT_ ${LANG_ENGLISH} "Create shortcut on Desktop"

LangString _UNINSTALL_ ${LANG_RUSSIAN} "Удалить" ; $(_UNINSTALL_)
LangString _UNINSTALL_ ${LANG_ENGLISH} "Uninstall"

LangString _REQUIRE64WIN_ ${LANG_RUSSIAN} "Требуется 64 разрядная версия Windows" ; $(_REQUIRE64WIN_)
LangString _REQUIRE64WIN_ ${LANG_ENGLISH} "Requires a 64 bit version of Windows"

; eof