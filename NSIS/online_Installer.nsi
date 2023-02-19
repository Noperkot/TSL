!define INSTALLER "TorrServer_Setup.exe"
!define CAPTION "TorrServer Online Installer"
!define TEMPDIR "$TEMP\TorrServerInstaller"
!define AUTHORS "Noperkot"
!define PRODUCT_VERSION ""


!macro abort message
	SetDetailsView show
	Abort
!macroend

!macro nop message
!macroend

!macro DownloadUrl timeout url tag
	NScurl::http GET "${url}" "${TEMPDIR}\${tag}" /TAG ${tag} /BACKGROUND /COMPLETETIMEOUT ${timeout} /CANCEL /END
	; /HEADER "Authorization: Basic [тут Base64 кодированная пара юзер:токен]" ; авторизация позволяет слать на api.github более 60 запросов в час
!macroend

!macro DownloadVerify cancelmacro tag
	NScurl::query /TAG "${tag}" "@URL@"
	Pop $0
	DetailPrint "$0"
	NScurl::query /TAG "${tag}" "@ERROR@"
	Pop $1
	DetailPrint "    $1"
	${If} $1 != "OK"
		!insertmacro ${cancelmacro} "$1$\n$0"
	${EndIf}
	NScurl::cancel /TAG "${tag}" /REMOVE
!macroend

!macro extractVersion tag output
	!insertmacro DownloadVerify exit ${tag}	; проверка успешности загрузки .json
	; получаем версию в ${output}
	StrCpy $0 "${TEMPDIR}\${tag}"
	ClearErrors
	nsJSON::Set /file $0
	${If} ${Errors}
		!insertmacro exit "$(_FILE_ERROR_)$\n$0"
	${EndIf}
	nsJSON::Get "tag_name" /end
	${If} ${Errors}
		!insertmacro exit "$(_PARSING_ERROR_)$\n$0"
	${EndIf}
	Pop ${output}
!macroend

!macro getVersions
	ClearErrors
	${GetOptions} $CMDLINE "/V" $0	; параметром командной строки /V можно указать желаемую версию торрсервера
	${IfNot} ${Errors}
		StrCpy $1 "tags/$0"
	${Else}
		StrCpy $1 "latest"
	${EndIf}
	!insertmacro downloadUrl 5s "https://api.github.com/repos/YouROK/TorrServer/releases/$1" "torrserver.json"	; ставим запрос в очередь
	!insertmacro downloadUrl 5s "https://api.github.com/repos/Noperkot/TSL/releases/latest"	 "tsl.json"			; -//-
	NScurl::wait /POPUP /CANCEL /END																			; ждем завершения всех запросов
	!insertmacro extractVersion "torrserver.json" $TorrServer_ver												; проверка ответа и извлечение версии
	!insertmacro extractVersion "tsl.json" $TSL_ver																; -//-
!macroend

!include common.nsh

Section Install

	Var	/Global TorrServerEXE
	${If} ${RunningX64}
		StrCpy $TorrServerEXE "TorrServer-windows-amd64.exe"
	${Else}
		StrCpy $TorrServerEXE "TorrServer-windows-386.exe"
	${EndIf}
	DetailPrint "$(_DOWNLOADING_)"
	!insertmacro DownloadUrl 5m "https://api.github.com/repos/YouROK/TorrServer/releases?per_page=20" "last20.json"
	!insertmacro DownloadUrl 5m "https://github.com/Noperkot/TSL/releases/download/$TSL_ver/${TSLEXE}" "${TSLEXE}"
	!insertmacro DownloadUrl 5m "https://github.com/YouROK/TorrServer/releases/download/$TorrServer_ver/$TorrServerEXE" "$TorrServerEXE"
	NScurl::wait /CANCEL /END
	SetDetailsPrint listonly
	!insertmacro DownloadVerify nop   "last20.json"
	!insertmacro DownloadVerify abort "${TSLEXE}"
	!insertmacro DownloadVerify abort "$TorrServerEXE"
	SetDetailsPrint both
	DetailPrint "$(_DOWNLOAD_COMPLETE_)"

	!insertmacro commonInstallSection

	Delete "$INSTDIR\${TSLEXE}"
	Rename "${TEMPDIR}\${TSLEXE}" "$INSTDIR\${TSLEXE}"
	Delete "$INSTDIR\$TorrServerEXE"
	Rename "${TEMPDIR}\$TorrServerEXE" "$INSTDIR\$TorrServerEXE"
	CopyFiles "$EXEPATH" "$INSTDIR\${ONLINE_INSTALLER}"	; инсталлятор

	; создаем ярлыки установщиков (после MatriX.119, но не более 20 последних)
	ClearErrors
	nsJSON::Set /file "${TEMPDIR}\last20.json"
	${IfNot} ${Errors}
		nsJSON::Get /count /end
		${IfNot} ${Errors}
			Pop $1
			IntOp $1 $1 - 1
			${ForEach} $2 0 $1 + 1
				nsJSON::Get /index $2 "tag_name" /end
				${IfNot} ${Errors}
					Pop $3
					${If} $3 == "MatriX.119"
						${Break}
					${EndIf}
					!insertmacro setupShortcut "$3"
				${EndIf}
			${Next}
		${EndIf}
	${EndIf}

SectionEnd

Function .onInit
	SetSilent normal
	RMDir /r ${TEMPDIR}
	CreateDirectory ${TEMPDIR}
FunctionEnd

Function .onGUIEnd
	RMDir /r "${TEMPDIR}"
FunctionEnd

LangString _GETTING_VERSIONS_ ${LANG_RUSSIAN} "Получение версий..." ; $(_GETTING_VERSIONS_)
LangString _GETTING_VERSIONS_ ${LANG_ENGLISH} "Getting products versions..."

LangString _FILE_ERROR_ ${LANG_RUSSIAN} "Ошибка открытия файла" ; $(_FILE_ERROR_)
LangString _FILE_ERROR_ ${LANG_ENGLISH} "File opening error"

LangString _PARSING_ERROR_ ${LANG_RUSSIAN} "Ошибка парсинга" ; $(_PARSING_ERROR_)
LangString _PARSING_ERROR_ ${LANG_ENGLISH} "Parsing error"

LangString _DOWNLOADING_ ${LANG_RUSSIAN} "Загрузка..." ; $(_DOWNLOADING_)
LangString _DOWNLOADING_ ${LANG_ENGLISH} "Downloading..."

LangString _DOWNLOAD_COMPLETE_ ${LANG_RUSSIAN} "Загрузка завершена" ; $(_DOWNLOAD_COMPLETE_)
LangString _DOWNLOAD_COMPLETE_ ${LANG_ENGLISH} "Download complete"