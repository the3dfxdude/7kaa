; install.nsi (v0.1)
; Written by Thomas Atkinson 12/4/2012
; Licensed under the gpl
; Requires NSIS to compile
;
; Installs 7kaa in a directory the user selects and creates an uninstaller

;--------------------------------
; Set current working directory

; You can achieve this with /NOCD, but this might get rolled into Makefile
; and this makes more sense.
!cd ..\..

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
; The name of the program to install
Name "Seven Kingdoms: Ancient Adversaries"

; The installer file name
OutFile "7kaa-install-win32.exe"

; The default installation directory
InstallDir $PROGRAMFILES\7kaa

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\7kaa" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\7kaa" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "Seven Kingdoms AA"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

 
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------


; The stuff to install
Section "7kaa (required)" 7kaareq

  ;make section required
  SectionIn RO

  SetOutPath $INSTDIR
  File ".\README"
  File ".\COPYING"
  File /r .\data\*.*
  File .\src\client\7kaa.exe
  
  ;Reset Install path
  ;SetOutPath "$INSTDIR"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\7kaa "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "DisplayName" "7kaa"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Music Files (Non GPL and Public Domain)(can be disabled by user)
; The music won't be in this installer.
;Section "Music" music
;
;        SetOutPath "$INSTDIR\music"
;        File ".\music\*.*"
; 
;SectionEnd

; Start Menu Shortcuts (can be disabled by the user)
Section "Start Menu Shortcuts" startshort

    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\7kaa.lnk" "$INSTDIR\7kaa.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString secreq ${LANG_ENGLISH} "Files required for 7kaa to run"
  LangString secmusic ${LANG_ENGLISH} "Music files for 7kaa. These are not licensed under the GPL or part of the Public Domain and may only be distributed with 7kaa"
  LangString secshort ${LANG_ENGLISH} "Start menu shortcuts"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${7kaareq} $(secreq)
;        !insertmacro MUI_DESCRIPTION_TEXT ${music} $(secmusic)
	!insertmacro MUI_DESCRIPTION_TEXT ${startshort} $(secshort)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa"
  DeleteRegKey HKLM SOFTWARE\7kaa

  ; Remove files and uninstaller
  Delete "$INSTDIR\*.*"
  Delete "$INSTDIR\encyc\firm\*.*"
  Delete "$INSTDIR\encyc\god\*.*"
  Delete "$INSTDIR\encyc\monster\*.*"
  Delete "$INSTDIR\encyc\seat\*.*"
  Delete "$INSTDIR\encyc\unit\*.*" 
  Delete "$INSTDIR\encyc2\god\*.*"
  Delete "$INSTDIR\encyc2\seat\*.*"
  Delete "$INSTDIR\encyc2\unit\*.*"  
  Delete "$INSTDIR\image\*.*"
  Delete "$INSTDIR\resource\*.*" 
  Delete "$INSTDIR\scenari2\*.*"
  Delete "$INSTDIR\scenario\*.*" 
  Delete "$INSTDIR\sound\*.*"
  Delete "$INSTDIR\sprite\*.*"
  Delete "$INSTDIR\tutorial\*.*"

  
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\7kaa\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\7kaa"
  RMDir "$INSTDIR"

SectionEnd

