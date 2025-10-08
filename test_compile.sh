#!/bin/bash

echo "Testando compilação..."

# Compilar apenas o código principal para verificar erros
x86_64-w64-mingw32-g++ -std=c++17 -Wall -Wextra -c main.cpp -o main.o

if [ $? -eq 0 ]; then
    echo "✓ Compilação do main.cpp bem-sucedida"
    
    # Testar compilação dos recursos
    x86_64-w64-mingw32-windres resources.rc -o resources.o
    
    if [ $? -eq 0 ]; then
        echo "✓ Compilação dos recursos bem-sucedida"
        
        # Compilação final
        x86_64-w64-mingw32-g++ -std=c++17 -O2 -mwindows -static-libgcc -static-libstdc++ \
            main.o resources.o -o LinearCalculator.exe \
            -lcomctl32 -lgdi32 -luser32 -lkernel32
        
        if [ $? -eq 0 ]; then
            echo "✓ Compilação final bem-sucedida!"
            echo "Executável: LinearCalculator.exe"
            ls -la LinearCalculator.exe
        else
            echo "✗ Erro na compilação final"
        fi
    else
        echo "✗ Erro na compilação dos recursos"
    fi
else
    echo "✗ Erro na compilação do main.cpp"
fi

# Limpeza
rm -f main.o
