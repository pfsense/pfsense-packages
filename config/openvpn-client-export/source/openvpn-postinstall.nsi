;--------------------------------
; OpenVPN NSIS Post-Installer
;--------------------------------

;--------------------------------
;Include Modern UI

Var /GLOBAL mui.FinishPage.Run
!define MUI_FINISHPAGE_RUN_VARIABLES

  !include "MUI2.nsh"
  !include "FileFunc.nsh"
  !include "LogicLib.nsh"

;--------------------------------
; General
;--------------------------------

  Name "OpenVPN Configuration"
  OutFile "openvpn-postinstall.exe"
  SetCompressor /SOLID lzma

  ShowInstDetails show

  !include "dotnet2.nsh"
;--------------------------------
;Include Settings
;--------------------------------

  !define MUI_ICON "openvpn-postinstall.ico"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
;--------------------------------

!define WELCOME_TITLE 'Welcome to OpenVPN installer.'

!define WELCOME_TEXT "This wizard will guide you through the installation of the OpenVPN client and configuration.$\r$\n$\r$\n\
This wil automaticaly install the configuration files needed for your connection. \
And if needed install the required DotNet2 framework."
  !define MUI_WELCOMEPAGE_TITLE '${WELCOME_TITLE}'
  ;!define MUI_WELCOMEPAGE_TITLE_3LINES
  !define MUI_WELCOMEPAGE_TEXT '${WELCOME_TEXT}'
  !insertmacro MUI_PAGE_WELCOME
  
  !insertmacro MUI_PAGE_INSTFILES
  
  
  !define MUI_FINISHPAGE_RUN "C:\User\test.lnk"
  !define MUI_FINISHPAGE_RUN_TEXT "Start OpenVPNManager."
  !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
  !define MUI_PAGE_CUSTOMFUNCTION_SHOW finish_show
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro Locate
  !insertmacro GetParameters
  !insertmacro GetOptions

;--------------------------------
;Languages
;--------------------------------

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Functions
;--------------------------------

Function .onInit
  Var /GLOBAL BINPATH
  Var /GLOBAL CONFPATH
  Var /GLOBAL OpenVPNManager
  
	IfFileExists ".\OpenVPNManager" InstallOpenVPNManager1 DontInstallOpenVPNManager1
	InstallOpenVPNManager1:
		strcpy $OpenVPNManager true
		!insertmacro CheckForDotNET2
		Goto OpenVPNManagerDone1
	DontInstallOpenVPNManager1:
		strcpy $OpenVPNManager false
	OpenVPNManagerDone1:
FunctionEnd

Function CopyConfFile
  CopyFiles $R9 $CONFPATH\$R7
  Push $0
FunctionEnd

Function ImportConfFile
  ExecWait "rundll32.exe cryptext.dll,CryptExtAddPFX $R9"
  Push $0
FunctionEnd

Function CopyOpenVPNManager
  DetailPrint "Installing OpenVPNManager..."
  DetailPrint "Installing in: $BINPATH\OpenVPNManager\"
  CreateDirectory "$BINPATH\OpenVPNManager"
  CreateDirectory "$BINPATH\OpenVPNManager\config"
  CopyFiles ".\OpenVPNManager\*.*" "$BINPATH\OpenVPNManager"
  CreateShortcut "$desktop\OpenVPNManager.lnk" "$BINPATH\OpenVPNManager\OpenVPNManager.exe"
  Push $0
FunctionEnd

Function finish_show
  ${If} $OpenVPNManager != "true"
	;If OpenVPNManager is not installed then dont give the option to run it. (hide and uncheck the checkbox)
	ShowWindow $mui.FinishPage.Run 0
	${NSD_Uncheck} $mui.FinishPage.Run
  ${EndIf}
FunctionEnd

Function LaunchLink
  ExecShell "" "$desktop\OpenVPNManager.lnk"
FunctionEnd
;--------------------------------
;Installer Sections
;--------------------------------

Section "Import Configuration" SectionImport
	${If} $OpenVPNManager == "true"	
		; OpenVPNManager needs dotnet2
		!insertmacro InstallDotNet2
	${Endif}
	
	ClearErrors
	ReadRegStr $BINPATH HKLM "Software\OpenVPN" ""
	IfErrors OpenVPNInstall OpenVPNAlreadyInstalled
	OpenVPNInstall:
		DetailPrint "Pausing installation while OpenVPN installer runs."
		ExecWait '".\openvpn-install.exe"' $1
		${if} $OpenVPNManager == "true"
			SetShellVarContext all
			Delete "$desktop\OpenVPN GUI.lnk"
			SetShellVarContext current
		${Endif}
		Pop $0
	OpenVPNAlreadyInstalled:

	ClearErrors
	ReadRegStr $BINPATH HKLM "Software\OpenVPN" ""
	IfErrors OpenVPNnotFound OpenVPNok
	OpenVPNnotFound:
		Abort "OpenVPN installation not found, installation aborted."
	OpenVPNok:
		DetailPrint "Completed OpenVPN installation."

	${If} $OpenVPNManager == "true"
		strcpy $OpenVPNManager true
		StrCpy $CONFPATH "$BINPATH\OpenVPNManager\config"
		call "CopyOpenVPNManager"
	${Else}
		strcpy $OpenVPNManager false
		ClearErrors
		ReadRegStr $CONFPATH HKLM "Software\OpenVPN" "config_dir"
		IfErrors configNotFound configFound
		configNotFound:
			ReadRegStr $CONFPATH HKLM "Software\OpenVPN" ""
			StrCpy $CONFPATH "$CONFPATH\config"
		configFound:
		
	${Endif}

	DetailPrint "Installing configuration files ..."
	${Locate} ".\config" "/L=F /M=*.ovpn" "CopyConfFile"

	DetailPrint "Installing certificate and key files ..."
	${Locate} ".\config" "/L=F /M=*.crt" "CopyConfFile"
	${Locate} ".\config" "/L=F /M=*.key" "CopyConfFile"
  
	${If} $OpenVPNManager == "true"
		DetailPrint "Registering OpenVPNManager service..."
		ExecWait '"$BINPATH\OpenVPNManager\OpenVPNManager.exe" /install'
		DetailPrint "Starting OpenVPNManager service..."
		SimpleSC::StartService "OpenVPNManager" "" 30
		Pop $0
	${Else}
		;DetailPrint "Starting OpenVPN Service..."
		;SimpleSC::StartService "OpenVPNService" "" 30
		;Pop $0
	${Endif}
  
	${GetParameters} $R0
	${GetOptions} $R0 "/Import" $R1
	IfErrors p12_copy p12_import
	p12_copy:
		${Locate} ".\config" "/L=F /M=*.p12" "CopyConfFile"
		Goto p12_done
	p12_import:
		${Locate} ".\config" "/L=F /M=*.p12" "ImportConfFile"
		Goto p12_done
	p12_done:

SectionEnd
;--------------------------------
;Descriptions
;--------------------------------

  ;Language strings
  LangString DESC_SectionImport ${LANG_ENGLISH} "Import OpenVPN Configurations and Key Files."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionImport} $(DESC_SectionImport)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END 

;--------------------------------
; END
;--------------------------------
