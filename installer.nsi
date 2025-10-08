; Instalador para Calculadora de Sistemas Lineares
; Criado com NSIS (Nullsoft Scriptable Install System)

!define APP_NAME "Calculadora de Sistemas Lineares"
!define APP_VERSION "1.0.0"
!define APP_PUBLISHER "Gustavo Marzullo"
!define APP_URL "https://github.com/gustavomarzullo/calculadora-sistemas-lineares"
!define APP_EXE "LinearCalculator.exe"
!define APP_ICON "calculator.ico"

; Configurações do instalador
Name "${APP_NAME}"
OutFile "CalculadoraSistemasLineares_Setup.exe"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "Software\${APP_NAME}" "InstallDir"
RequestExecutionLevel admin

; Interface moderna
!include "MUI2.nsh"
!include "FileFunc.nsh"

; Configurações da interface
!define MUI_ABORTWARNING
!define MUI_ICON "${APP_ICON}"
!define MUI_UNICON "${APP_ICON}"

; Páginas do instalador
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\${APP_EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Executar ${APP_NAME}"
!insertmacro MUI_PAGE_FINISH

; Páginas do desinstalador
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Idiomas
!insertmacro MUI_LANGUAGE "PortugueseBR"

; Informações da versão
VIProductVersion "${APP_VERSION}.0"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "ProductName" "${APP_NAME}"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "Comments" "Calculadora profissional para resolver sistemas de equações lineares"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "CompanyName" "${APP_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "LegalCopyright" "© 2024 ${APP_PUBLISHER}"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "FileDescription" "${APP_NAME} - Instalador"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "FileVersion" "${APP_VERSION}"
VIAddVersionKey /LANG=${LANG_PORTUGUESEBR} "ProductVersion" "${APP_VERSION}"

; Seção principal (obrigatória)
Section "Programa Principal" SecMain
    SectionIn RO
    
    ; Definir diretório de saída
    SetOutPath "$INSTDIR"
    
    ; Arquivos do programa
    File "dist\${APP_EXE}"
    File "${APP_ICON}"
    
    ; Criar atalho no menu iniciar
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_ICON}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Desinstalar.lnk" "$INSTDIR\Uninstall.exe"
    
    ; Registrar no sistema
    WriteRegStr HKLM "Software\${APP_NAME}" "InstallDir" "$INSTDIR"
    WriteRegStr HKLM "Software\${APP_NAME}" "Version" "${APP_VERSION}"
    
    ; Criar desinstalador
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    ; Registrar no Adicionar/Remover Programas
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${APP_ICON}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${APP_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "URLInfoAbout" "${APP_URL}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
    
    ; Calcular tamanho da instalação (aproximado)
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "EstimatedSize" 3072
SectionEnd

; Seção opcional - Atalho na área de trabalho
Section "Atalho na Área de Trabalho" SecDesktop
    CreateShortCut "$DESKTOP\${APP_NAME}.lnk" "$INSTDIR\${APP_EXE}" "" "$INSTDIR\${APP_ICON}"
SectionEnd

; Seção opcional - Associação de arquivos
Section "Associar arquivos .calc" SecFileAssoc
    ; Registrar extensão de arquivo
    WriteRegStr HKCR ".calc" "" "CalculadoraSistemas.Document"
    WriteRegStr HKCR "CalculadoraSistemas.Document" "" "Arquivo de Cálculo de Sistemas Lineares"
    WriteRegStr HKCR "CalculadoraSistemas.Document\DefaultIcon" "" "$INSTDIR\${APP_ICON}"
    WriteRegStr HKCR "CalculadoraSistemas.Document\shell\open\command" "" '"$INSTDIR\${APP_EXE}" "%1"'
    
    ; Atualizar cache de ícones
    System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
SectionEnd

; Descrições das seções
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "Arquivos principais do programa (obrigatório)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Criar atalho na área de trabalho para acesso rápido"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFileAssoc} "Associar arquivos .calc com a calculadora"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Função executada antes da instalação
Function .onInit
    ; Verificar se já está instalado
    ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString"
    StrCmp $R0 "" done
    
    MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "${APP_NAME} já está instalado. $\n$\nClique 'OK' para remover a versão anterior ou 'Cancelar' para cancelar a instalação." \
    IDOK uninst
    Abort
    
    uninst:
        ClearErrors
        ExecWait '$R0 /S _?=$INSTDIR'
        
        IfErrors no_remove_uninstaller done
        no_remove_uninstaller:
    
    done:
FunctionEnd

; Desinstalador
Section "Uninstall"
    ; Remover arquivos
    Delete "$INSTDIR\${APP_EXE}"
    Delete "$INSTDIR\${APP_ICON}"
    Delete "$INSTDIR\Uninstall.exe"
    
    ; Remover atalhos
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Desinstalar.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"
    Delete "$DESKTOP\${APP_NAME}.lnk"
    
    ; Remover diretório se estiver vazio
    RMDir "$INSTDIR"
    
    ; Remover registros
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKLM "Software\${APP_NAME}"
    
    ; Remover associação de arquivos
    DeleteRegKey HKCR ".calc"
    DeleteRegKey HKCR "CalculadoraSistemas.Document"
    
    ; Atualizar cache de ícones
    System::Call 'shell32.dll::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)'
SectionEnd
