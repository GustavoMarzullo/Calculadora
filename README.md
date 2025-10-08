# Calculadora de Sistemas de Equações Lineares

Uma aplicação elegante e moderna para Windows que resolve sistemas de equações lineares com interface gráfica nativa.

## 🎯 Características

- **Interface Moderna**: Design limpo com paleta off-white e bege elegante
- **Entrada Dinâmica**: Adicione variáveis conforme necessário (até 10x10)
- **Cálculo em Tempo Real**: Resultados atualizados automaticamente com debounce
- **Tratamento Inteligente**: Detecta sistemas sem solução ou com infinitas soluções
- **Algoritmo Robusto**: Eliminação Gaussiana com pivoteamento parcial
- **Nativo Windows**: Usa Win32 API, sem dependências externas

## 🚀 Compilação e Instalação

### Pré-requisitos

1. **MinGW-w64** (compilador GCC para Windows)
   - Baixe de: https://www.mingw-w64.org/downloads/
   - Ou instale via MSYS2: `pacman -S mingw-w64-x86_64-gcc`
   - Certifique-se de adicionar ao PATH do sistema

2. **Python 3** (opcional, para criar ícone personalizado)
   - `pip install Pillow`

### Compilação Automática

**Opção 1: Script Batch (Recomendado)**
```batch
build.bat
```

**Opção 2: Makefile**
```bash
make
# ou para release otimizada
make release
```

**Opção 3: Manual**
```bash
# Compilar recursos
windres resources.rc -o resources.o

# Compilar aplicação
g++ -std=c++17 -O2 -Wall -Wextra -mwindows -static-libgcc -static-libstdc++ main.cpp resources.o -o LinearCalculator.exe -lcomctl32 -lgdi32 -luser32 -lkernel32
```

### Criando Ícone Personalizado (Opcional)

```bash
python create_icon.py
```

## 📖 Como Usar

1. **Execute** `LinearCalculator.exe`
2. **Defina** o número de variáveis (2-10)
3. **Digite** os coeficientes da matriz e constantes
4. **Observe** a solução sendo calculada em tempo real

### Exemplo de Sistema 2x2:
```
2x₁ + 3x₂ = 7
1x₁ - 1x₂ = 1

Solução:
x₁ = 2.000000
x₂ = 1.000000
```

## 🔧 Estrutura do Projeto

```
├── main.cpp              # Interface GUI e lógica principal
├── LinearSolver.h        # Algoritmo de resolução (Eliminação Gaussiana)
├── resource.h            # Definições de recursos
├── resources.rc          # Arquivo de recursos Windows
├── calculator.ico        # Ícone da aplicação
├── build.bat            # Script de compilação automática
├── Makefile             # Makefile para compilação
├── create_icon.py       # Script para gerar ícone personalizado
└── README.md            # Este arquivo
```

## 🎨 Design e Interface

### Paleta de Cores
- **Fundo**: Off-white quente `RGB(250, 248, 245)`
- **Painéis**: Bege muito claro `RGB(245, 240, 235)`
- **Acentos**: Bege médio `RGB(210, 200, 185)`
- **Texto**: Marrom elegante `RGB(60, 55, 50)`

### Características Visuais
- Bordas arredondadas sutis
- Transparência sutil para efeito moderno
- Fontes Segoe UI para elegância
- Layout responsivo e intuitivo

## ⚙️ Algoritmo

### Eliminação Gaussiana com Pivoteamento Parcial

1. **Pivoteamento**: Seleciona o maior elemento em módulo como pivô
2. **Eliminação**: Reduz a matriz à forma escalonada
3. **Substituição Regressiva**: Calcula as variáveis de trás para frente
4. **Verificação**: Valida a solução substituindo na equação original

### Tratamento de Casos Especiais

- **Sistema Inconsistente**: Detecta quando não há solução
- **Infinitas Soluções**: Identifica sistemas indeterminados
- **Campos Vazios**: Mostra mensagem elegante durante digitação
- **Precisão Numérica**: Usa epsilon para comparações de ponto flutuante

## 🔍 Detecção de Problemas

A aplicação detecta automaticamente:

- ✅ **Solução Única**: Sistema bem determinado
- ❌ **Sem Solução**: Sistema inconsistente
- ♾️ **Infinitas Soluções**: Sistema indeterminado
- ⚠️ **Entrada Incompleta**: Campos não preenchidos

## 📊 Limitações

- Máximo de 10 variáveis (pode ser aumentado modificando o código)
- Precisão limitada por aritmética de ponto flutuante
- Sistemas muito mal condicionados podem ter precisão reduzida

## 🛠️ Personalização

### Modificar Cores
Edite as constantes em `main.cpp`:
```cpp
#define COLOR_BACKGROUND RGB(250, 248, 245)
#define COLOR_PANEL RGB(245, 240, 235)
#define COLOR_ACCENT RGB(210, 200, 185)
```

### Alterar Limite de Variáveis
Modifique a validação em `UpdateMatrixInputs()`:
```cpp
if (newSize > 0 && newSize <= 15 && newSize != currentSize) {
```

### Ajustar Debounce
Altere o tempo em `CalculationWorker()`:
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 500ms
```

## 🐛 Solução de Problemas

### Erro de Compilação
- Verifique se MinGW-w64 está instalado e no PATH
- Certifique-se de usar g++ versão 7.0 ou superior

### Ícone Não Aparece
- Execute `python create_icon.py` para gerar o ícone
- Ou substitua `calculator.ico` por um ícone personalizado

### Interface Não Responsiva
- Verifique se há loops infinitos no cálculo
- O debounce pode precisar ser ajustado

## 📝 Licença

Este projeto é de código aberto. Sinta-se livre para modificar e distribuir.

## 🤝 Contribuições

Contribuições são bem-vindas! Áreas de melhoria:

- Suporte para números complexos
- Exportação de resultados
- Temas personalizáveis
- Suporte para frações exatas

---

**Desenvolvido com ❤️ para resolver sistemas lineares de forma elegante e eficiente.**


