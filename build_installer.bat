@echo off
echo ====================================
echo  Criando Instalador para Windows
echo  Calculadora de Sistemas Lineares
echo ====================================
echo.

REM Verificar se NSIS está instalado
where makensis >nul 2>nul
if %errorlevel% neq 0 (
    echo ❌ NSIS não encontrado!
    echo.
    echo Para criar o instalador, você precisa instalar o NSIS:
    echo 1. Baixe em: https://nsis.sourceforge.io/Download
    echo 2. Instale o NSIS
    echo 3. Adicione o NSIS ao PATH do sistema
    echo 4. Execute este script novamente
    echo.
    pause
    exit /b 1
)

echo ✓ NSIS encontrado

REM Verificar se o executável existe
if not exist "dist\LinearCalculator.exe" (
    echo ❌ Executável não encontrado!
    echo.
    echo Execute primeiro o build.bat para compilar o programa:
    echo   ./build.bat
    echo.
    pause
    exit /b 1
)

echo ✓ Executável encontrado

REM Verificar se o ícone existe
if not exist "calculator.ico" (
    echo ❌ Ícone não encontrado!
    echo.
    echo Execute o script create_icon.py para criar o ícone:
    echo   python create_icon.py
    echo.
    pause
    exit /b 1
)

echo ✓ Ícone encontrado

REM As imagens personalizadas são opcionais para o NSIS
echo ℹ️  Usando interface padrão do NSIS (sem imagens customizadas)

echo.
echo Compilando instalador...
makensis installer.nsi

if %errorlevel% equ 0 (
    echo.
    echo ✅ INSTALADOR CRIADO COM SUCESSO!
    echo.
    echo Arquivo: CalculadoraSistemasLineares_Setup.exe
    echo.
    if exist "CalculadoraSistemasLineares_Setup.exe" (
        for %%A in ("CalculadoraSistemasLineares_Setup.exe") do (
            echo Tamanho: %%~zA bytes
        )
    )
    echo.
    echo O instalador está pronto para distribuição!
    echo.
) else (
    echo.
    echo ❌ Erro ao criar o instalador
    echo Verifique o arquivo installer.nsi para possíveis erros
    echo.
)

pause
