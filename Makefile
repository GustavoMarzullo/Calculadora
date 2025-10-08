# Makefile para Calculadora de Sistemas Lineares
# Compilação para Windows 64-bit

# Configurações do compilador
CXX = g++
WINDRES = windres
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -mwindows -static-libgcc -static-libstdc++
LDFLAGS = -lcomctl32 -lgdi32 -luser32 -lkernel32

# Arquivos
TARGET = LinearCalculator.exe
SOURCES = main.cpp
HEADERS = LinearSolver.h resource.h
RESOURCE_RC = resources.rc
RESOURCE_OBJ = resources.o
ICON = calculator.ico

# Regra principal
all: $(TARGET)

# Compilar recursos
$(RESOURCE_OBJ): $(RESOURCE_RC) $(ICON)
	$(WINDRES) $(RESOURCE_RC) -o $(RESOURCE_OBJ)

# Compilar executável
$(TARGET): $(SOURCES) $(HEADERS) $(RESOURCE_OBJ)
	$(CXX) $(CXXFLAGS) $(SOURCES) $(RESOURCE_OBJ) -o $(TARGET) $(LDFLAGS)

# Criar ícone padrão se não existir
$(ICON):
	@echo "Criando ícone padrão..."
	@echo "NOTA: Substitua calculator.ico por um ícone personalizado se desejar"

# Limpeza
clean:
	del /f $(TARGET) $(RESOURCE_OBJ) 2>nul || true

# Instalação (copia para uma pasta de distribuição)
install: $(TARGET)
	if not exist "dist" mkdir dist
	copy $(TARGET) dist\
	@echo "Executável copiado para a pasta dist/"

# Teste rápido (executa o programa)
test: $(TARGET)
	./$(TARGET)

# Informações do sistema
info:
	@echo "=== Informações de Compilação ==="
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Bibliotecas: $(LDFLAGS)"
	@echo "Target: $(TARGET)"
	@echo "================================="

# Verificar dependências
check:
	@echo "Verificando dependências..."
	@where g++ >nul 2>&1 || (echo "ERRO: g++ não encontrado. Instale MinGW-w64" && exit 1)
	@where windres >nul 2>&1 || (echo "ERRO: windres não encontrado. Instale MinGW-w64" && exit 1)
	@echo "Todas as dependências estão disponíveis!"

# Compilação para debug
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Compilação para release (otimizada)
release: CXXFLAGS += -DNDEBUG -s
release: $(TARGET)

.PHONY: all clean install test info check debug release


