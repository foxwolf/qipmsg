; QIpMsg user-level installer.
; Build after windeployqt has filled a directory that contains qipmsg.exe:
;   makensis -DVERSION=2.30.7 -DDEPLOYDIR=C:\path\to\win-deploy packaging\qipmsg.nsi
; VERSION and DEPLOYDIR are required. Do not hardcode the product version here.

Unicode true
RequestExecutionLevel user
ManifestDPIAware true

!ifndef VERSION
  !error "Pass -DVERSION=x.y.z"
!endif
!ifndef DEPLOYDIR
  !error "Pass -DDEPLOYDIR=path-to-windeployqt-output"
!endif
!ifndef OUTFILE
  !define OUTFILE "qipmsg-${VERSION}-setup.exe"
!endif

Name "QIpMsg ${VERSION}"
OutFile "${OUTFILE}"
InstallDir "$LOCALAPPDATA\QIpMsg"
InstallDirRegKey HKCU "Software\QIpMsg" "InstallDir"

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "QIpMsg"
  SetOutPath "$INSTDIR"
  File /r "${DEPLOYDIR}\*.*"
  WriteUninstaller "$INSTDIR\uninstall.exe"
  CreateDirectory "$SMPROGRAMS\QIpMsg"
  CreateShortcut "$SMPROGRAMS\QIpMsg\QIpMsg.lnk" "$INSTDIR\qipmsg.exe"
  CreateShortcut "$SMPROGRAMS\QIpMsg\Uninstall QIpMsg.lnk" "$INSTDIR\uninstall.exe"
  WriteRegStr HKCU "Software\QIpMsg" "InstallDir" "$INSTDIR"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "DisplayName" "QIpMsg"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "DisplayVersion" "${VERSION}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "InstallLocation" "$INSTDIR"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg" \
      "NoRepair" 1
SectionEnd

Section "Uninstall"
  Delete "$SMPROGRAMS\QIpMsg\QIpMsg.lnk"
  Delete "$SMPROGRAMS\QIpMsg\Uninstall QIpMsg.lnk"
  RMDir "$SMPROGRAMS\QIpMsg"
  RMDir /r "$INSTDIR"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\QIpMsg"
  DeleteRegKey HKCU "Software\QIpMsg"
SectionEnd
