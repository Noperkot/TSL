!ifndef TS_VERSION
	!define TS_VERSION "MatriX.125"
	; !define TS_VERSION "MatriX.124"
	; !define TS_VERSION "MatriX.123"
	; !define TS_VERSION "MatriX.122"
	; !define TS_VERSION "MatriX.121"
	; !define TS_VERSION "MatriX.120"
	; !define TS_VERSION "MatriX.119"
	; !define TS_VERSION "MatriX.118"
	; !define TS_VERSION "MatriX.117"
	; !define TS_VERSION "MatriX.116"
	; !define TS_VERSION "MatriX.115"
	; !define TS_VERSION "MatriX.114"
	; !define TS_VERSION "MatriX.112"
	; !define TS_VERSION "MatriX.110"
	; !define TS_VERSION "MatriX.109"
	; !define TS_VERSION "MatriX.106"
	; !define TS_VERSION "1.1.77"
	; !define TS_VERSION "1.1.68"
	; !define TS_VERSION "1.1.65"
!endif
!define TSL_VERSION "1.7.1"
!define INSTALLER "TorrServer_${TS_VERSION}_Setup.exe"
!define CAPTION "TorrServer ${TS_VERSION} Installer"
!define AUTHORS "YouROK, Noperkot"
!define PRODUCT_VERSION "TS-${TS_VERSION}, TSL-${TSL_VERSION}"
!define TSDIR "TorrServers\${TS_VERSION}"
!define TSLDIR "..\build\signed\${TSL_VERSION}"

!macro getVersions
	StrCpy $TorrServer_ver ${TS_VERSION}
	StrCpy $TSL_ver ${TSL_VERSION}
!macroend

!include common.nsh

Section Install
	!insertmacro commonInstallSection
	File "..\build\signed\${ONLINE_INSTALLER}"	; онлайн инсталлятор
	File "${TSLDIR}\${TSLEXE}"
	${If} ${RunningX64}
		File "${TSDIR}\TorrServer-windows-amd64.exe"
	${Else}
		File "${TSDIR}\TorrServer-windows-386.exe"
	${EndIf}
SectionEnd

Function .onInit
	SetSilent normal
FunctionEnd