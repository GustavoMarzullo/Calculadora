#!/bin/bash

echo "===================================="
echo " Calculadora de Sistemas Lineares"
echo " Script de Compilação para WSL/Linux"
echo "===================================="
echo

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Função para verificar se um comando existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Verificar dependências
echo "Verificando dependências..."

# Verificar compilador C++ para Windows
if command_exists x86_64-w64-mingw32-g++; then
    CXX="x86_64-w64-mingw32-g++"
    WINDRES="x86_64-w64-mingw32-windres"
    echo -e "${GREEN}✓${NC} MinGW-w64 cross-compiler encontrado"
elif command_exists i686-w64-mingw32-g++; then
    CXX="i686-w64-mingw32-g++"
    WINDRES="i686-w64-mingw32-windres"
    echo -e "${YELLOW}⚠${NC} Usando MinGW-w64 32-bit (recomendado: 64-bit)"
else
    echo -e "${RED}✗${NC} Compilador MinGW-w64 não encontrado!"
    echo
    echo "Para instalar no Ubuntu/Debian:"
    echo "  sudo apt update"
    echo "  sudo apt install gcc-mingw-w64-x86-64 g++-mingw-w64-x86-64"
    echo
    echo "Para instalar no Arch/Manjaro:"
    echo "  sudo pacman -S mingw-w64-gcc"
    echo
    echo "Para instalar no Fedora:"
    echo "  sudo dnf install mingw64-gcc-c++"
    echo
    exit 1
fi

# Verificar se o ícone existe
if [ ! -f "calculator.ico" ]; then
    echo -e "${YELLOW}⚠${NC} Ícone não encontrado, criando ícone padrão..."
    if command_exists python3; then
        python3 create_icon.py
        if [ $? -eq 0 ]; then
            echo -e "${GREEN}✓${NC} Ícone criado com sucesso"
        else
            echo -e "${YELLOW}⚠${NC} Falha ao criar ícone, usando placeholder"
            # Criar um arquivo placeholder
            touch calculator.ico
        fi
    else
        echo -e "${YELLOW}⚠${NC} Python3 não encontrado, criando placeholder"
        touch calculator.ico
    fi
fi

echo
echo "Configuração:"
echo "  Compilador: $CXX"
echo "  Windres: $WINDRES"
echo

# Limpar arquivos anteriores
echo "Limpando arquivos anteriores..."
rm -f LinearCalculator.exe resources.o main.o

# Compilar recursos
echo "Compilando recursos..."
$WINDRES resources.rc -o resources.o
if [ $? -ne 0 ]; then
    echo -e "${RED}✗${NC} Erro ao compilar recursos"
    exit 1
fi
echo -e "${GREEN}✓${NC} Recursos compilados"

# Compilar aplicação
echo "Compilando aplicação..."
$CXX -std=c++17 -O2 -Wall -Wextra -mwindows -static-libgcc -static-libstdc++ \
    main.cpp resources.o -o LinearCalculator.exe \
    -lcomctl32 -lgdi32 -luser32 -lkernel32

if [ $? -ne 0 ]; then
    echo -e "${RED}✗${NC} Erro na compilação"
    exit 1
fi

echo -e "${GREEN}✓${NC} Aplicação compilada com sucesso"
echo

# Verificar se o executável foi criado
if [ -f "LinearCalculator.exe" ]; then
    file_size=$(stat -f%z "LinearCalculator.exe" 2>/dev/null || stat -c%s "LinearCalculator.exe" 2>/dev/null)
    echo -e "${GREEN}===================================="
    echo " COMPILAÇÃO CONCLUÍDA COM SUCESSO!"
    echo "====================================${NC}"
    echo
    echo "Executável criado: LinearCalculator.exe"
    echo "Tamanho: $(echo $file_size | numfmt --to=iec 2>/dev/null || echo $file_size bytes)"
    echo
    
    # Criar pasta de distribuição
    mkdir -p dist
    cp LinearCalculator.exe dist/
    echo "Executável copiado para: dist/LinearCalculator.exe"
    echo
    
    # Verificar dependências do executável
    echo "Verificando dependências..."
    if command_exists objdump; then
        echo "DLLs necessárias:"
        objdump -p LinearCalculator.exe | grep "DLL Name:" | head -10
    fi
    
    echo
    echo -e "${BLUE}Para executar no Windows:${NC}"
    echo "1. Copie LinearCalculator.exe para um sistema Windows"
    echo "2. Execute diretamente (duplo-clique)"
    echo "3. Ou use: dist/LinearCalculator.exe"
    echo
    echo -e "${BLUE}Para testar no WSL (se X11 estiver configurado):${NC}"
    echo "wine LinearCalculator.exe"
    
else
    echo -e "${RED}✗${NC} Executável não foi criado"
    exit 1
fi

echo
echo -e "${GREEN}Compilação finalizada!${NC}"
