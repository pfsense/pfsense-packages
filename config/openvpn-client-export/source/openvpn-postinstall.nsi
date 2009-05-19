;--------------------------------
; OpenVPN NSIS Post-Installer
;--------------------------------

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"
  !include "FileFunc.nsh"
  !include "LogicLib.nsh"

;--------------------------------
; General
;--------------------------------

  Name "OpenVPN Configuration"
  OutFile "openvpn-postinstall.exe"
  SetCompressor /SOLID lzma

  ShowInstDetails show

;--------------------------------
;Include Settings
;--------------------------------

  !define MUI_ICON "openvpn-postinstall.ico"
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
;--------------------------------

  !insertmacro MUI_PAGE_INSTFILES
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

  Var /GLOBAL CONFPATH
  ReadRegStr $CONFPATH HKLM "Software\OpenVPN" "config_dir"

FunctionEnd

Function CopyConfFile

  CopyFiles $R9 $CONFPATH\$R7
  Push $0

FunctionEnd

Function ImportConfFile

  ExecWait "rundll32.exe cryptext.dll,CryptExtAddPFX $R9"
  Push $0

FunctionEnd

;--------------------------------
;Installer Sections
;--------------------------------

Section "Imort Configuration" SectionImport

  DetailPrint "Installing configuration files ..."
  ${Locate} ".\config" "/L=F /M=*.ovpn" "CopyConfFile"

  DetailPrint "Installing certificate and key files ..."
  ${Locate} ".\config" "/L=F /M=*.crt" "CopyConfFile"
  ${Locate} ".\config" "/L=F /M=*.key" "CopyConfFile"

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
