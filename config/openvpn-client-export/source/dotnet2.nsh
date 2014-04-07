; Plugin for installing .NET Framework v2.0
; Written by Christopher St. John
; for EncounterPRO Healthcare Resources, Inc.

!ifndef DOTNET2_INCLUDED
!define DOTNET2_INCLUDED

; -----------------------------------------
; Includes
    !include "WordFunc.nsh"
    !insertmacro VersionCompare
    !include LogicLib.nsh

; -----------------------------------------
; Defines
    ; Direct-download location of .NET 2.0 redist
    !define BASE_URL http://download.microsoft.com/download
    !define URL_DOTNET_1033 "${BASE_URL}/5/6/7/567758a3-759e-473e-bf8f-52154438565a/dotnetfx.exe"
    
; -----------------------------------------
; Variables
    Var DotNetVersion2
    Var InstallDotNet2
    
; -----------------------------------------
; Functions
Function GetDotNETVersion2
  Push $0
  Push $1

  System::Call "mscoree::GetCORVersion(w .r0, i 1024, *i r2) i .r1"
  StrCmp $1 0 +2
    StrCpy $0 0
  
  Pop $1
  Exch $0
FunctionEnd

; -----------------------------------------
; Macros
!macro CheckForDotNET2
    ; Check .NET version
    StrCpy $InstallDotNET2 "No"
    Call GetDotNETVersion2
    Pop $0
    StrCpy $DotNetVersion2 $0
  
    ${If} $0 == "not found"
        StrCpy $InstallDotNET2 "Yes"
        MessageBox MB_OK|MB_ICONINFORMATION "Installer requires that the .NET Framework 2.0 is installed. The .NET Framework will be downloaded and installed automatically during installation."
        Return
    ${EndIf}

    StrCpy $0 $0 "" 1 # skip "v"

    ${VersionCompare} $0 "2.0" $1
    ${If} $1 == 2
        StrCpy $InstallDotNET2 "Yes"
        MessageBox MB_OK|MB_ICONINFORMATION "Installer requires that the .NET Framework 2.0 is installed. The .NET Framework will be downloaded and installed automatically during installation."
        Return
    ${EndIf}
!macroend

!macro InstallDotNET2
    ; Get .NET if required
    ${If} $InstallDotNET2 == "Yes"
        DetailPrint "Downloading .NET Framework v2.0..."
        ;SetDetailsView hide
        NSISdl::download /TIMEOUT=30000 "${URL_DOTNET_1033}" "$INSTDIR\dotnetfx.exe"
        Pop $1

        ${If} $1 != "success"
            DetailPrint "Download failed: $1"
            Delete "$INSTDIR\dotnetfx.exe"
            Abort "Installation Cancelled"
        ${EndIf}

        DetailPrint "Installing .NET Framework v2.0..."
        ExecWait '"$INSTDIR\dotnetfx.exe" /q:a /c:"install /passive"' $1
        ${If} $1 == 0
            DetailPrint ".NET Framework v2.0 successfully installed."
        ${ElseIf} $1 == 3010
            MessageBox MB_OK ".NET Framework v2.0 has been installed and requires a reboot.  Please restart the computer and run this installer again."
            Abort ".NET Framework v2.0 requires reboot."
        ${Else}
            MessageBox MB_OK ".NET Framework v2.0 reports a failure during installation ($1).  Please try to install .NET Framework v2.0 via Windows Update before running this installer again."
            Abort ".NET Framework v2.0 installation failed ($1)."
        ${EndIf}
        Delete "$INSTDIR\dotnetfx.exe"  
    ${EndIf}
!macroend

!endif