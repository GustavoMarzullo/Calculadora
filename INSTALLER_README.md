# 📦 Instalador para Windows - Calculadora de Sistemas Lineares

Este diretório contém os arquivos necessários para criar um instalador profissional para Windows.

## 🛠️ Pré-requisitos

### Para Windows:
1. **NSIS (Nullsoft Scriptable Install System)**
   - Baixe em: https://nsis.sourceforge.io/Download
   - Instale e adicione ao PATH do sistema
   - Versão recomendada: 3.08 ou superior

### Para WSL/Linux:
```bash
# Ubuntu/Debian
sudo apt-get install nsis

# Fedora
sudo dnf install mingw32-nsis

# Arch Linux
sudo pacman -S nsis
```

## 🚀 Como Criar o Instalador

### Método 1: Windows
```cmd
# 1. Compile primeiro o programa
./build.bat

# 2. Crie o instalador
./build_installer.bat
```

### Método 2: WSL/Linux
```bash
# 1. Compile primeiro o programa
./build.sh

# 2. Crie o instalador
./build_installer.sh
```

### Método 3: Manual
```cmd
# Compile diretamente com NSIS
makensis installer.nsi
```

## 📁 Arquivos do Instalador

- `installer.nsi` - Script principal do instalador NSIS
- `LICENSE.txt` - Licença de uso do software
- `build_installer.bat` - Script para Windows
- `build_installer.sh` - Script para WSL/Linux
- `header.bmp` - Imagem do cabeçalho (criada automaticamente)
- `welcome.bmp` - Imagem de boas-vindas (criada automaticamente)

## ✨ Funcionalidades do Instalador

### 🎯 Instalação
- ✅ Interface moderna e profissional
- ✅ Instalação em `Program Files`
- ✅ Atalho no Menu Iniciar
- ✅ Atalho na Área de Trabalho (opcional)
- ✅ Associação de arquivos .calc (opcional)
- ✅ Registro no "Adicionar/Remover Programas"
- ✅ Detecção de versão anterior
- ✅ Desinstalação automática da versão antiga

### 🗑️ Desinstalação
- ✅ Remoção completa de arquivos
- ✅ Limpeza de atalhos
- ✅ Limpeza do registro
- ✅ Remoção de associações de arquivo

### 🔧 Opções Avançadas
- ✅ Instalação silenciosa: `/S`
- ✅ Diretório personalizado: `/D=C:\MeuDiretorio`
- ✅ Componentes selecionáveis
- ✅ Verificação de integridade

## 📋 Componentes do Instalador

1. **Programa Principal** (obrigatório)
   - Executável da calculadora
   - Ícone da aplicação
   - Atalhos no Menu Iniciar

2. **Atalho na Área de Trabalho** (opcional)
   - Acesso rápido na área de trabalho

3. **Associação de Arquivos** (opcional)
   - Associa arquivos .calc com a calculadora
   - Ícone personalizado para arquivos .calc

## 🎨 Personalização

### Modificar Aparência
Edite o arquivo `installer.nsi` e altere:
- `!define APP_NAME` - Nome da aplicação
- `!define APP_VERSION` - Versão
- `!define APP_PUBLISHER` - Seu nome/empresa
- Substitua `header.bmp` e `welcome.bmp` por suas imagens

### Adicionar Arquivos
No arquivo `installer.nsi`, na seção `SecMain`:
```nsis
File "meu_arquivo.txt"
File "pasta\*.*"
```

## 📊 Resultado Final

Após a compilação, você terá:
- **Arquivo**: `CalculadoraSistemasLineares_Setup.exe`
- **Tamanho**: ~2-3 MB
- **Compatibilidade**: Windows 7/8/10/11 (64-bit)
- **Assinatura**: Não assinado (adicione certificado se necessário)

## 🔒 Assinatura Digital (Opcional)

Para adicionar assinatura digital:
```cmd
signtool sign /f certificado.pfx /p senha CalculadoraSistemasLineares_Setup.exe
```

## 🐛 Solução de Problemas

### Erro: "makensis não reconhecido"
- Instale o NSIS e adicione ao PATH
- Reinicie o terminal/prompt

### Erro: "Arquivo não encontrado"
- Compile primeiro o programa com `./build.sh`
- Verifique se `dist/LinearCalculator.exe` existe

### Erro: "Permissão negada"
- Execute como administrador
- Verifique permissões dos arquivos

## 📞 Suporte

Para problemas com o instalador:
1. Verifique os pré-requisitos
2. Consulte a documentação do NSIS
3. Verifique os logs de compilação

---

**Desenvolvido por Gustavo Marzullo - 2024**
