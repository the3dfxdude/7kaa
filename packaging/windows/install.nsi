; install.nsi
; Written by Thomas Atkinson 12/4/2012
; Licensed under the gpl
;
; Updated by Jesse Allen for 2.14.xx and 2.15.xx
;
; Requires NSIS to compile
;
; Install 7kaa in a directory, copy this into the directory, and use NSIS to
; create the installer. See build.sh for a quick rundown on how to use "make
; install". A few other files might need to be brought in from elsewhere.

;--------------------------------
; Set current working directory

; You can achieve this with /NOCD, but this might get rolled into Makefile
; and this makes more sense.
;!cd ..\..\dest

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

  ;!insertmacro MUI_PAGE_LICENSE "COPYING"
  ;!insertmacro MUI_PAGE_LICENSE "COPYING-Music.txt"
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
;ReserveFile "${NSISDIR}\Plugins\*.dll"

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
  File ".\7kaa-manual.pdf"
  File /r ".\ENCYC"
  File /r ".\ENCYC2"
  File /r ".\IMAGE"
  File /r ".\locale"
  File /r ".\RESOURCE"
  File /r ".\SCENARI2"
  File /r ".\SCENARIO"
  File /r ".\SOUND"
  File /r ".\SPRITE"
  File /r ".\TUTORIAL"
  File .\7kaa.exe
  File ".\SDL2.dll"
  File ".\libcurl-4.dll"
  
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
  File /r ".\MUSIC"

SectionEnd

Section "OpenAL" openal

  SetOutPath "$INSTDIR"
  File ".\OpenAL32.dll"

SectionEnd

; Start Menu Shortcuts (can be disabled by the user)
Section "Start Menu Shortcuts" startshort

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
  ; Create shortcuts
  CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Seven Kingdoms AA.lnk" "$INSTDIR\7kaa.exe"
  CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Manual.lnk" "$INSTDIR\7kaa-manual.pdf"
  
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString secreq ${LANG_ENGLISH} "Files required for 7kaa to run"
  LangString secmusic ${LANG_ENGLISH} "Music files for 7kaa"
  LangString secshort ${LANG_ENGLISH} "Start menu shortcuts"
  LangString secopenal ${LANG_ENGLISH} "OpenAL-soft shared library (recommended if not provided by your sound driver)"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${7kaareq} $(secreq)
      !insertmacro MUI_DESCRIPTION_TEXT ${music} $(secmusic)
      !insertmacro MUI_DESCRIPTION_TEXT ${startshort} $(secshort)
    !insertmacro MUI_DESCRIPTION_TEXT ${openal} $(secopenal)
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
  RMDir /r "$INSTDIR\ENCYC"
  RMDir /r "$INSTDIR\ENCYC2"
  RMDir /r "$INSTDIR\IMAGE"
  RMDir /r "$INSTDIR\locale"
  RMDir /r "$INSTDIR\RESOURCE"
  RMDir /r "$INSTDIR\SCENARI2"
  RMDir /r "$INSTDIR\SCENARIO"
  RMDir /r "$INSTDIR\SOUND"
  RMDir /r "$INSTDIR\SPRITE"
  RMDir /r "$INSTDIR\TUTORIAL"
  RMDir /r "$INSTDIR\MUSIC"
  Delete "$INSTDIR\7kaa.exe"
  Delete "$INSTDIR\COPYING.txt"
  Delete "$INSTDIR\README.txt"
  Delete "$INSTDIR\7kaa-manual.pdf"
  Delete "$INSTDIR\COPYING-music.txt"
  Delete "$INSTDIR\README-music.txt"
  Delete "$INSTDIR\OpenAL32.dll"
  Delete "$INSTDIR\SDL2.dll"
  Delete "$INSTDIR\libcurl-4.dll"
  Delete "$INSTDIR\Uninstall.exe"

  ; Remove shortcuts
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Seven Kingdoms AA.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Manual.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Hotkeys.lnk"

  ; Remove directories if empty
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  RMDir "$INSTDIR"

SectionEnd

