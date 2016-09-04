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

; Maximum Compression
SetCompressor /SOLID lzma

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "COPYING"
  !insertmacro MUI_PAGE_LICENSE "COPYING-Music.txt"
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
; Reserve Files

; If you are using solid compression, files that are required before
; the actual installation should be stored first in the data block,
; because this will make your installer start faster.

!insertmacro MUI_RESERVEFILE_LANGDLL
ReserveFile "${NSISDIR}\Plugins\*.dll"

;--------------------------------
; The stuff to install

Section "7kaa (required)" 7kaareq

  ;make section required
  SectionIn RO

  SetOutPath "$INSTDIR"
  File ".\README"
  Rename "$INSTDIR\README" "$INSTDIR\README.txt"
  File ".\COPYING"
  Rename "$INSTDIR\COPYING" "$INSTDIR\COPYING.txt"
  File ".\COPYING.uuid"
  Rename "$INSTDIR\COPYING.uuid" "$INSTDIR\COPYING-uuid.txt"
  File ".\doc\7kaa-hotkeys-2.14.5.png"
  File /r ".\data\encyc"
  File /r ".\data\encyc2"
  File /r ".\data\image"
  File /r ".\data\resource"
  File /r ".\data\scenari2"
  File /r ".\data\scenario"
  File /r ".\data\sound"
  File /r ".\data\sprite"
  File /r ".\data\tutorial"
  File .\src\client\7kaa.exe
  
  ;Reset Install path
  ;SetOutPath "$INSTDIR"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\7kaa "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "DisplayName" "Seven Kingdoms AA"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Music Files (Not GPL and not Public Domain)(can be disabled by user)
Section "Music" music

  SetOutPath "$INSTDIR"
  File ".\README-music.txt"
  File ".\COPYING-music.txt"
  File /r ".\data\music"

SectionEnd

Section "OpenAL" openal

  SetOutPath "$INSTDIR"
  File ".\OpenAL32.dll"

SectionEnd

Section "SDL" sdl

  SetOutPath "$INSTDIR"
  File ".\SDL2.dll"

SectionEnd

; Start Menu Shortcuts (can be disabled by the user)
Section "Start Menu Shortcuts" startshort

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
  ; Create shortcuts
  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Seven Kingdoms AA.lnk" "$INSTDIR\7kaa.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Hotkeys.lnk" "$INSTDIR\7kaa-hotkeys-2.14.5.png"
  
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString secreq ${LANG_ENGLISH} "Files required for 7kaa to run"
  LangString secmusic ${LANG_ENGLISH} "Music files for 7kaa"
  LangString secshort ${LANG_ENGLISH} "Start menu shortcuts"
  LangString secopenal ${LANG_ENGLISH} "OpenAL-soft shared library (recommended if not provided by your hardware driver)"
  LangString secsdl ${LANG_ENGLISH} "SDL shared library (recommended)"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${7kaareq} $(secreq)
      !insertmacro MUI_DESCRIPTION_TEXT ${music} $(secmusic)
      !insertmacro MUI_DESCRIPTION_TEXT ${startshort} $(secshort)
    !insertmacro MUI_DESCRIPTION_TEXT ${openal} $(secopenal)
    !insertmacro MUI_DESCRIPTION_TEXT ${sdl} $(secsdl)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\7kaa"
  DeleteRegKey HKLM "SOFTWARE\7kaa"
  DeleteRegKey HKCU "SOFTWARE\7kaa\Start Menu Folder"
  DeleteRegKey HKCU "SOFTWARE\7kaa"

  ; Remove the program files
  RMDir /r "$INSTDIR\encyc"
  RMDir /r "$INSTDIR\encyc2"
  RMDir /r "$INSTDIR\image"
  RMDir /r "$INSTDIR\resource"
  RMDir /r "$INSTDIR\scenari2"
  RMDir /r "$INSTDIR\scenario"
  RMDir /r "$INSTDIR\sound"
  RMDir /r "$INSTDIR\sprite"
  RMDir /r "$INSTDIR\tutorial"
  RMDir /r "$INSTDIR\music"
  Delete "$INSTDIR\7kaa.exe"
  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\7kaa-hotkeys-2.14.5.png"
  Delete "$INSTDIR\COPYING-music.txt"
  Delete "$INSTDIR\README-music.txt"
  Delete "$INSTDIR\OpenAL32.dll"
  Delete "$INSTDIR\SDL2.dll"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Seven Kingdoms AA.lnk"

  ; Remove directories if empty
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  RMDir "$INSTDIR"

SectionEnd

