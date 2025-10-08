@echo off
echo ====================================
echo  Calculadora de Sistemas Lineares
echo  Script de Compilacao para Windows
echo ====================================
echo.

REM Verificar se o MinGW está instalado
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRO: g++ nao encontrado!
    echo.
    echo Por favor, instale o MinGW-w64:
    echo 1. Baixe de: https://www.mingw-w64.org/downloads/
    echo 2. Ou instale via MSYS2: pacman -S mingw-w64-x86_64-gcc
    echo 3. Adicione o caminho do MinGW ao PATH do sistema
    echo.
    pause
    exit /b 1
)

where windres >nul 2>&1
if %errorlevel% neq 0 (
    echo ERRO: windres nao encontrado!
    echo Certifique-se de que o MinGW-w64 completo esta instalado.
    pause
    exit /b 1
)

echo Dependencias verificadas com sucesso!
echo.

REM Criar ícone padrão se não existir
if not exist "calculator.ico" (
    echo Criando icone padrao...
    echo NOTA: Substitua calculator.ico por um icone personalizado se desejar
    echo.
    REM Criar um arquivo ICO básico (placeholder)
    echo. > calculator.ico
)

echo Compilando recursos...
windres resources.rc -o resources.o
if %errorlevel% neq 0 (
    echo ERRO: Falha ao compilar recursos
    pause
    exit /b 1
)

echo Compilando aplicacao...
g++ -std=c++17 -O2 -Wall -Wextra -mwindows -static-libgcc -static-libstdc++ main.cpp resources.o -o LinearCalculator.exe -lcomctl32 -lgdi32 -luser32 -lkernel32

if %errorlevel% neq 0 (
    echo ERRO: Falha na compilacao
    pause
    exit /b 1
)

echo.
echo ====================================
echo  COMPILACAO CONCLUIDA COM SUCESSO!
echo ====================================
echo.
echo Executavel criado: LinearCalculator.exe
echo.

REM Criar pasta de distribuição
if not exist "dist" mkdir dist
copy LinearCalculator.exe dist\ >nul
echo Executavel copiado para: dist\LinearCalculator.exe
echo.

echo Deseja executar o programa agora? (s/n)
set /p choice=
if /i "%choice%"=="s" (
    echo Executando...
    start LinearCalculator.exe
) else (
    echo.
    echo Para executar manualmente: LinearCalculator.exe
    echo Ou use: dist\LinearCalculator.exe
)

echo.
pause


