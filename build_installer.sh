#!/bin/bash

echo "===================================="
echo " Criando Instalador para Windows"
echo " Calculadora de Sistemas Lineares"
echo "===================================="
echo

# Verificar se makensis está disponível
if ! command -v makensis &> /dev/null; then
    echo "❌ NSIS não encontrado!"
    echo
    echo "Para criar o instalador no WSL/Linux, instale o NSIS:"
    echo "Ubuntu/Debian: sudo apt-get install nsis"
    echo "Fedora: sudo dnf install mingw32-nsis"
    echo "Arch: sudo pacman -S nsis"
    echo
    echo "Ou baixe em: https://nsis.sourceforge.io/Download"
    echo
    exit 1
fi

echo "✓ NSIS encontrado"

# Verificar se o executável existe
if [ ! -f "dist/LinearCalculator.exe" ]; then
    echo "❌ Executável não encontrado!"
    echo
    echo "Execute primeiro o build.sh para compilar o programa:"
    echo "  ./build.sh"
    echo
    exit 1
fi

echo "✓ Executável encontrado"

# Verificar se o ícone existe
if [ ! -f "calculator.ico" ]; then
    echo "❌ Ícone não encontrado!"
    echo
    echo "Execute o script create_icon.py para criar o ícone:"
    echo "  python3 create_icon.py"
    echo
    exit 1
fi

echo "✓ Ícone encontrado"

# As imagens personalizadas são opcionais para o NSIS
echo "ℹ️  Usando interface padrão do NSIS (sem imagens customizadas)"

echo
echo "Compilando instalador..."
makensis installer.nsi

if [ $? -eq 0 ]; then
    echo
    echo "✅ INSTALADOR CRIADO COM SUCESSO!"
    echo
    echo "Arquivo: CalculadoraSistemasLineares_Setup.exe"
    echo
    if [ -f "CalculadoraSistemasLineares_Setup.exe" ]; then
        size=$(stat -f%z "CalculadoraSistemasLineares_Setup.exe" 2>/dev/null || stat -c%s "CalculadoraSistemasLineares_Setup.exe" 2>/dev/null)
        echo "Tamanho: $size bytes"
    fi
    echo
    echo "O instalador está pronto para distribuição!"
    echo
else
    echo
    echo "❌ Erro ao criar o instalador"
    echo "Verifique o arquivo installer.nsi para possíveis erros"
    echo
    exit 1
fi
