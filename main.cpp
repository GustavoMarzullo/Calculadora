#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <iostream>
#include "LinearSolver.h"
#include "resource.h"

// Estrutura para armazenar um cálculo no histórico
struct CalculationHistory {
    int size;
    std::vector<std::vector<double>> matrix;
    std::vector<double> constants;
    LinearSolver::Solution solution;
    std::basic_string<TCHAR> timestamp;
    std::basic_string<TCHAR> description;
    std::basic_string<TCHAR> customName;
    
    // Construtor padrão
    CalculationHistory() : size(0) {}
    
    CalculationHistory(int n, const std::vector<std::vector<double>>& m, 
                      const std::vector<double>& c, const LinearSolver::Solution& s) 
        : size(n), matrix(m), constants(c), solution(s) {
        
        // Criar timestamp
        SYSTEMTIME st;
        GetLocalTime(&st);
        TCHAR timeStr[64];
        _stprintf_s(timeStr, TEXT("%02d/%02d/%04d %02d:%02d:%02d"), 
                   st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
        timestamp = timeStr;
        
        // Criar descrição
        TCHAR desc[256];
        if (solution.hasSolution) {
            _stprintf_s(desc, TEXT("Sistema %dx%d - Solução única"), size, size);
        } else {
            switch (solution.status) {
                case LinearSolver::SolutionStatus::NO_SOLUTION:
                    _stprintf_s(desc, TEXT("Sistema %dx%d - Sem solução"), size, size);
                    break;
                case LinearSolver::SolutionStatus::INFINITE_SOLUTIONS:
                    _stprintf_s(desc, TEXT("Sistema %dx%d - Infinitas soluções"), size, size);
                    break;
                default:
                    _stprintf_s(desc, TEXT("Sistema %dx%d - Erro"), size, size);
                    break;
            }
        }
        description = desc;
    }
};

// Constantes para IDs dos controles
#define ID_MATRIX_SIZE 1001
#define ID_SOLVE_BUTTON 1002
#define ID_RESULT_TEXT 1003
#define ID_HISTORY_BUTTON 1004
#define ID_SAVE_BUTTON 1005
#define ID_CLEAR_BUTTON 1006
#define ID_CLEAR_HISTORY 1007
#define ID_HISTORY_LIST 1008
#define ID_RESTORE_CALCULATION 1009
#define ID_DELETE_ITEM 1010
#define ID_RENAME_ITEM 1011
#define ID_NAME_INPUT 1012
#define ID_NAME_OK 1013
#define ID_NAME_CANCEL 1014
#define ID_MATRIX_START 2000

// Cores da paleta elegante
#define COLOR_BG RGB(250, 248, 245)    // Off-white quente
#define COLOR_PANEL RGB(245, 240, 235)         // Bege muito claro
#define COLOR_ACCENT RGB(210, 200, 185)        // Bege médio
#define COLOR_TEXT RGB(60, 55, 50)             // Marrom escuro elegante
#define COLOR_BORDER RGB(220, 210, 195)        // Borda sutil

class CalculatorApp {
private:
    HWND hwndMain;
    HWND hwndMatrixSize;
    HWND hwndResultText;
    HWND hwndTitleLabel;
    HWND hwndVariablesLabel;
    HWND hwndSolutionLabel;
    HWND hwndHistoryButton;
    HWND hwndSaveButton;
    HWND hwndClearButton;
    HWND hwndHistoryWindow;
    HWND hwndNameDialog;
    std::vector<std::vector<HWND>> matrixInputs;
    std::vector<HWND> constantInputs;
    LinearSolver solver;
    
    // Histórico de cálculos
    std::vector<CalculationHistory> calculationHistory;
    static constexpr int MAX_HISTORY_ITEMS = 50;
    static constexpr const TCHAR* HISTORY_FILE = TEXT("calculator_history.dat");
    
    // Último cálculo realizado (para gravar manualmente)
    std::vector<std::vector<double>> lastMatrix;
    std::vector<double> lastConstants;
    LinearSolver::Solution lastSolution;
    bool hasLastCalculation = false;
    
    // Debounce para cálculos
    std::mutex calculationMutex;
    std::thread calculationThread;
    bool shouldCalculate = false;
    bool calculationRunning = false;
    
    // Brushes para cores personalizadas
    HBRUSH hBrushBackground;
    HBRUSH hBrushPanel;
    HBRUSH hBrushAccent;
    HFONT hFontMain;
    HFONT hFontTitle;
    
    int currentSize = 2;
    
public:
    // Método para acessar a janela principal
    HWND GetMainWindow() const { return hwndMain; }
    
    CalculatorApp() {
        // Inicializar variáveis
        hwndMain = nullptr;
        hwndMatrixSize = nullptr;
        hwndResultText = nullptr;
        hwndTitleLabel = nullptr;
        hwndVariablesLabel = nullptr;
        hwndSolutionLabel = nullptr;
        hwndHistoryButton = nullptr;
        hwndSaveButton = nullptr;
        hwndClearButton = nullptr;
        hwndHistoryWindow = nullptr;
        hwndNameDialog = nullptr;
        
        // Criar brushes para cores
        hBrushBackground = CreateSolidBrush(COLOR_BG);
        hBrushPanel = CreateSolidBrush(COLOR_PANEL);
        hBrushAccent = CreateSolidBrush(COLOR_ACCENT);
        
        // Criar fontes elegantes
        hFontMain = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Segoe UI"));
            
        hFontTitle = CreateFont(20, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Segoe UI"));
    }
    
    ~CalculatorApp() {
        // Cleanup
        if (calculationThread.joinable()) {
            calculationRunning = false;
            calculationThread.join();
        }
        
        // Fechar janela de histórico se estiver aberta
        if (hwndHistoryWindow && IsWindow(hwndHistoryWindow)) {
            DestroyWindow(hwndHistoryWindow);
            hwndHistoryWindow = nullptr;
        }
        
        // Fechar janela de nome se estiver aberta
        if (hwndNameDialog && IsWindow(hwndNameDialog)) {
            DestroyWindow(hwndNameDialog);
            hwndNameDialog = nullptr;
        }
        
        DeleteObject(hBrushBackground);
        DeleteObject(hBrushPanel);
        DeleteObject(hBrushAccent);
        DeleteObject(hFontMain);
        DeleteObject(hFontTitle);
    }
    
    bool Initialize(HINSTANCE hInstance) {
        // Registrar classe da janela
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CALCULATOR));
        wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CALCULATOR));
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = hBrushBackground;
        wc.lpszClassName = TEXT("LinearEquationCalculator");
        
        if (!RegisterClassEx(&wc)) {
            return false;
        }
        
        // Criar janela principal
        hwndMain = CreateWindowEx(
            WS_EX_LAYERED,
            TEXT("LinearEquationCalculator"),
            TEXT("Calculadora de Sistemas Lineares"),
            WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT,
            800, 600,
            nullptr, nullptr, hInstance, this
        );
        
        if (!hwndMain) {
            return false;
        }
        
        // Aplicar transparência sutil para efeito moderno
        SetLayeredWindowAttributes(hwndMain, 0, 250, LWA_ALPHA);
        
        CreateControls();
        UpdateMatrixInputs();
        
        // Carregar histórico do arquivo
        LoadHistoryFromFile();
        
        ShowWindow(hwndMain, SW_SHOW);
        UpdateWindow(hwndMain);
        
        // Iniciar thread de cálculo
        calculationRunning = true;
        calculationThread = std::thread(&CalculatorApp::CalculationWorker, this);
        
        return true;
    }
    
    void CreateControls() {
        // Título elegante
        hwndTitleLabel = CreateWindow(TEXT("STATIC"), TEXT("Sistema de Equações Lineares"),
            WS_VISIBLE | WS_CHILD | SS_CENTER,
            50, 20, 700, 30,
            hwndMain, nullptr, GetModuleHandle(nullptr), nullptr);
        
        // Label para tamanho da matriz
        hwndVariablesLabel = CreateWindow(TEXT("STATIC"), TEXT("Número de variáveis:"),
            WS_VISIBLE | WS_CHILD,
            50, 70, 150, 25,
            hwndMain, nullptr, GetModuleHandle(nullptr), nullptr);
        
        // Input para tamanho da matriz
        hwndMatrixSize = CreateWindow(TEXT("EDIT"), TEXT("2"),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_TABSTOP,
            210, 68, 60, 25,
            hwndMain, (HMENU)(UINT_PTR)ID_MATRIX_SIZE, GetModuleHandle(nullptr), nullptr);
        
        // Botão de histórico
        hwndHistoryButton = CreateWindow(TEXT("BUTTON"), TEXT("Histórico"),
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
            280, 68, 80, 25,
            hwndMain, (HMENU)(UINT_PTR)ID_HISTORY_BUTTON, GetModuleHandle(nullptr), nullptr);
        
        // Botão de gravar
        hwndSaveButton = CreateWindow(TEXT("BUTTON"), TEXT("Gravar"),
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
            370, 68, 70, 25,
            hwndMain, (HMENU)(UINT_PTR)ID_SAVE_BUTTON, GetModuleHandle(nullptr), nullptr);
        
        // Botão de limpar
        hwndClearButton = CreateWindow(TEXT("BUTTON"), TEXT("Limpar"),
            WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON,
            450, 68, 70, 25,
            hwndMain, (HMENU)(UINT_PTR)ID_CLEAR_BUTTON, GetModuleHandle(nullptr), nullptr);
        
        // Área de resultado (posicionada mais à direita para evitar sobreposição)
        hwndSolutionLabel = CreateWindow(TEXT("STATIC"), TEXT("Solução:"),
            WS_VISIBLE | WS_CHILD,
            500, 70, 80, 25,
            hwndMain, nullptr, GetModuleHandle(nullptr), nullptr);
        
        hwndResultText = CreateWindow(TEXT("EDIT"), TEXT(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            500, 100, 280, 400,
            hwndMain, (HMENU)(UINT_PTR)ID_RESULT_TEXT, GetModuleHandle(nullptr), nullptr);
        
        // Aplicar fontes aos controles
        SendMessage(hwndMatrixSize, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SendMessage(hwndHistoryButton, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SendMessage(hwndSaveButton, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SendMessage(hwndClearButton, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SendMessage(hwndResultText, WM_SETFONT, (WPARAM)hFontMain, TRUE);
    }
    
    void UpdateMatrixInputs() {
        // Limpar inputs existentes de forma mais segura
        for (auto& row : matrixInputs) {
            for (auto hwnd : row) {
                if (IsWindow(hwnd)) {
                    DestroyWindow(hwnd);
                }
            }
        }
        for (auto hwnd : constantInputs) {
            if (IsWindow(hwnd)) {
                DestroyWindow(hwnd);
            }
        }
        
        // Limpeza AGRESSIVA: destruir TODOS os controles exceto os principais
        std::vector<HWND> toKeep = {hwndMatrixSize, hwndResultText, hwndTitleLabel, hwndVariablesLabel, hwndSolutionLabel, hwndHistoryButton, hwndSaveButton, hwndClearButton};
        std::vector<HWND> toDestroy;
        
        HWND hChild = GetWindow(hwndMain, GW_CHILD);
        while (hChild) {
            HWND hNext = GetWindow(hChild, GW_HWNDNEXT);
            
            // Verificar se é um controle que deve ser mantido
            bool shouldKeep = false;
            for (HWND keepHwnd : toKeep) {
                if (hChild == keepHwnd) {
                    shouldKeep = true;
                    break;
                }
            }
            
            if (!shouldKeep) {
                toDestroy.push_back(hChild);
            }
            
            hChild = hNext;
        }
        
        // Destruir todos os controles marcados
        for (HWND hwnd : toDestroy) {
            if (IsWindow(hwnd)) {
                DestroyWindow(hwnd);
            }
        }
        
        matrixInputs.clear();
        constantInputs.clear();
        
        // Criar novos inputs
        matrixInputs.resize(currentSize);
        constantInputs.resize(currentSize);
        
        // Layout responsivo baseado no número de variáveis
        int startY = 120;
        int cellWidth = std::max(45, std::min(70, 400 / currentSize)); // Largura adaptativa
        int cellHeight = std::max(28, std::min(35, 300 / currentSize)); // Altura adaptativa
        int spacing = std::max(2, std::min(8, 50 / currentSize)); // Espaçamento adaptativo
        
        // Calcular largura total necessária para a matriz
        int matrixWidth = currentSize * cellWidth + (currentSize - 1) * spacing;
        int totalWidth = matrixWidth + 80 + cellWidth; // matriz + espaço + constante
        
        // Calcular posição dos botões para determinar layout
        int buttonX = 280;
        if (totalWidth > 250) {
            buttonX = std::max(280, totalWidth - 200); // Mais espaço para 3 botões
        }
        int buttonsEndX = buttonX + 90 + 70 + 70; // posição do último botão + largura (3 botões)
        int resultX = std::max(550, buttonsEndX + 20); // pelo menos 20px de espaço
        int resultWidth = 280;
        
        // Ajustar tamanho da janela se necessário
        RECT windowRect, clientRect;
        GetWindowRect(hwndMain, &windowRect);
        GetClientRect(hwndMain, &clientRect);
        
        int minWindowWidth = resultX + resultWidth + 50; // espaço para resultado + margem
        int minWindowHeight = 400 + currentSize * (cellHeight + spacing) + 100;
        int currentWindowWidth = windowRect.right - windowRect.left;
        int currentWindowHeight = windowRect.bottom - windowRect.top;
        
        if (currentWindowWidth < minWindowWidth || currentWindowHeight < minWindowHeight) {
            SetWindowPos(hwndMain, nullptr, 0, 0, 
                        std::max(currentWindowWidth, minWindowWidth), 
                        std::max(currentWindowHeight, minWindowHeight),
                        SWP_NOMOVE | SWP_NOZORDER);
        }
        
        for (int i = 0; i < currentSize; i++) {
            matrixInputs[i].resize(currentSize);
            
            for (int j = 0; j < currentSize; j++) {
                int x = 50 + j * (cellWidth + spacing);
                int y = startY + i * (cellHeight + spacing);
                
                matrixInputs[i][j] = CreateWindow(TEXT("EDIT"), TEXT("0"),
                    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | WS_TABSTOP,
                    x, y, cellWidth, cellHeight,
                    hwndMain, (HMENU)(UINT_PTR)(ID_MATRIX_START + i * currentSize + j),
                    GetModuleHandle(nullptr), nullptr);
                
                SendMessage(matrixInputs[i][j], WM_SETFONT, (WPARAM)hFontMain, TRUE);
            }
            
            // Label "=" - perfeitamente alinhado com os campos
            int equalX = 50 + currentSize * (cellWidth + spacing) + 10;
            int equalY = startY + i * (cellHeight + spacing);
            HWND hEqual = CreateWindow(TEXT("STATIC"), TEXT("="),
                WS_VISIBLE | WS_CHILD | SS_CENTER | SS_CENTERIMAGE,
                equalX, equalY, 20, cellHeight,
                hwndMain, nullptr, GetModuleHandle(nullptr), nullptr);
            
            // Aplicar fonte ao sinal de igual
            SendMessage(hEqual, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            
            // Input para constante
            int constX = equalX + 30;
            constantInputs[i] = CreateWindow(TEXT("EDIT"), TEXT("0"),
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_CENTER | WS_TABSTOP,
                constX, equalY, cellWidth, cellHeight,
                hwndMain, (HMENU)(UINT_PTR)(ID_MATRIX_START + 1000 + i),
                GetModuleHandle(nullptr), nullptr);
            
            SendMessage(constantInputs[i], WM_SETFONT, (WPARAM)hFontMain, TRUE);
        }
        
        // Aplicar fontes aos controles principais
        SendMessage(hwndMatrixSize, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SendMessage(hwndResultText, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        
        // Reposicionar botões para evitar sobreposição
        if (IsWindow(hwndHistoryButton) && IsWindow(hwndSaveButton) && IsWindow(hwndClearButton)) {
            SetWindowPos(hwndHistoryButton, nullptr, buttonX, 68, 80, 25, SWP_NOZORDER);
            SetWindowPos(hwndSaveButton, nullptr, buttonX + 90, 68, 70, 25, SWP_NOZORDER);
            SetWindowPos(hwndClearButton, nullptr, buttonX + 170, 68, 70, 25, SWP_NOZORDER);
        }
        
        // Reposicionar área de resultado dinamicamente
        int resultY = 70;
        int resultHeight = std::max(200, currentSize * (cellHeight + spacing) + 50);
        
        // Reposicionar o label "Solução:"
        if (IsWindow(hwndSolutionLabel)) {
            SetWindowPos(hwndSolutionLabel, nullptr, resultX, resultY, 80, 25, SWP_NOZORDER);
        }
        
        // Reposicionar área de texto do resultado
        SetWindowPos(hwndResultText, nullptr, resultX, resultY + 30, resultWidth, resultHeight, SWP_NOZORDER);
        
        // Forçar atualização da interface
        InvalidateRect(hwndMain, nullptr, TRUE);
        UpdateWindow(hwndMain);
        
        // Definir foco inicial no primeiro campo da matriz
        if (!matrixInputs.empty() && !matrixInputs[0].empty()) {
            SetFocus(matrixInputs[0][0]);
        }
        
        TriggerCalculation();
    }
    
    void TriggerCalculation() {
        std::lock_guard<std::mutex> lock(calculationMutex);
        shouldCalculate = true;
    }
    
    void ClearAllVariables() {
        // Limpar todos os campos da matriz
        for (auto& row : matrixInputs) {
            for (auto hwnd : row) {
                if (IsWindow(hwnd)) {
                    SetWindowText(hwnd, TEXT("0"));
                }
            }
        }
        
        // Limpar todos os campos de constantes
        for (auto hwnd : constantInputs) {
            if (IsWindow(hwnd)) {
                SetWindowText(hwnd, TEXT("0"));
            }
        }
        
        // Limpar área de resultado
        if (IsWindow(hwndResultText)) {
            SetWindowText(hwndResultText, TEXT("Digite os coeficientes da matriz e as constantes..."));
        }
        
        // Resetar último cálculo
        hasLastCalculation = false;
        lastMatrix.clear();
        lastConstants.clear();
        
        // Definir foco no primeiro campo
        if (!matrixInputs.empty() && !matrixInputs[0].empty() && IsWindow(matrixInputs[0][0])) {
            SetFocus(matrixInputs[0][0]);
        }
        
        // Forçar recálculo
        TriggerCalculation();
    }
    
    void SaveCurrentCalculation() {
        if (!hasLastCalculation) {
            MessageBox(hwndMain, TEXT("Não há cálculo para gravar.\n\nRealize um cálculo primeiro digitando os valores da matriz e constantes."), 
                      TEXT("Nenhum Cálculo"), MB_OK | MB_ICONINFORMATION);
            return;
        }
        
        // Verificar se é um cálculo válido para gravar
        if (!lastSolution.hasSolution && lastSolution.status == LinearSolver::SolutionStatus::CALCULATION_ERROR) {
            MessageBox(hwndMain, TEXT("Não é possível gravar um cálculo com erro.\n\nVerifique se todos os campos estão preenchidos corretamente."), 
                      TEXT("Cálculo Inválido"), MB_OK | MB_ICONWARNING);
            return;
        }
        
        // Verificar se já existe no histórico (evitar duplicatas)
        if (!calculationHistory.empty()) {
            const auto& last = calculationHistory[0];
            if (last.size == currentSize && last.matrix == lastMatrix && last.constants == lastConstants) {
                MessageBox(hwndMain, TEXT("Este cálculo já está gravado no histórico."), 
                          TEXT("Já Gravado"), MB_OK | MB_ICONINFORMATION);
                return;
            }
        }
        
        // Mostrar diálogo para inserir nome personalizado
        ShowNameDialog();
    }
    
    void ShowNameDialog() {
        if (hwndNameDialog && IsWindow(hwndNameDialog)) {
            SetForegroundWindow(hwndNameDialog);
            return;
        }
        
        // Registrar classe da janela de nome se necessário
        static bool nameClassRegistered = false;
        if (!nameClassRegistered) {
            WNDCLASSEX wcName = {};
            wcName.cbSize = sizeof(WNDCLASSEX);
            wcName.style = CS_HREDRAW | CS_VREDRAW;
            wcName.lpfnWndProc = NameDialogProc;
            wcName.hInstance = GetModuleHandle(nullptr);
            wcName.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_CALCULATOR));
            wcName.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcName.hbrBackground = hBrushBackground;
            wcName.lpszClassName = TEXT("NameDialog");
            RegisterClassEx(&wcName);
            nameClassRegistered = true;
        }
        
        // Criar janela de diálogo
        hwndNameDialog = CreateWindowEx(
            WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
            TEXT("NameDialog"),
            TEXT("Nome do Cálculo"),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
            hwndMain, nullptr, GetModuleHandle(nullptr), this
        );
        
        if (hwndNameDialog) {
            // Label de instrução
            CreateWindow(TEXT("STATIC"), TEXT("Digite um nome para este cálculo:"),
                WS_VISIBLE | WS_CHILD,
                20, 20, 350, 20,
                hwndNameDialog, nullptr, GetModuleHandle(nullptr), nullptr);
            
            // Campo de entrada
            HWND hwndNameInput = CreateWindow(TEXT("EDIT"), TEXT(""),
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                20, 50, 340, 25,
                hwndNameDialog, (HMENU)(UINT_PTR)ID_NAME_INPUT, GetModuleHandle(nullptr), nullptr);
            
            // Botões
            CreateWindow(TEXT("BUTTON"), TEXT("OK"),
                WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                200, 90, 80, 30,
                hwndNameDialog, (HMENU)(UINT_PTR)ID_NAME_OK, GetModuleHandle(nullptr), nullptr);
            
            CreateWindow(TEXT("BUTTON"), TEXT("Cancelar"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                290, 90, 80, 30,
                hwndNameDialog, (HMENU)(UINT_PTR)ID_NAME_CANCEL, GetModuleHandle(nullptr), nullptr);
            
            // Aplicar fontes
            SendMessage(hwndNameInput, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            
            // Definir foco no campo de entrada
            SetFocus(hwndNameInput);
            
            // Gerar nome sugerido
            TCHAR suggestedName[64];
            if (lastSolution.hasSolution) {
                _stprintf_s(suggestedName, TEXT("Sistema %dx%d - Solução"), currentSize, currentSize);
            } else {
                _stprintf_s(suggestedName, TEXT("Sistema %dx%d"), currentSize, currentSize);
            }
            SetWindowText(hwndNameInput, suggestedName);
            
            // Selecionar todo o texto
            SendMessage(hwndNameInput, EM_SETSEL, 0, -1);
        }
    }
    
    LRESULT HandleNameDialogMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_KEYDOWN:
                if (wParam == VK_RETURN) {
                    // Enter pressionado - confirmar
                    HWND hwndNameInput = GetDlgItem(hwnd, ID_NAME_INPUT);
                    TCHAR customName[256];
                    GetWindowText(hwndNameInput, customName, 256);
                    
                    // Adicionar ao histórico com nome personalizado
                    AddToHistory(lastMatrix, lastConstants, lastSolution, customName);
                    
                    // Mostrar confirmação
                    TCHAR confirmMsg[512];
                    _stprintf_s(confirmMsg, TEXT("✓ Cálculo '%s' gravado com sucesso!\n\nTotal de itens no histórico: %d"), 
                               customName, static_cast<int>(calculationHistory.size()));
                    MessageBox(hwndMain, confirmMsg, TEXT("Gravado no Histórico"), MB_OK | MB_ICONINFORMATION);
                    
                    // Fechar diálogo
                    DestroyWindow(hwnd);
                    hwndNameDialog = nullptr;
                    return 0;
                } else if (wParam == VK_ESCAPE) {
                    // Escape pressionado - cancelar
                    DestroyWindow(hwnd);
                    hwndNameDialog = nullptr;
                    return 0;
                }
                break;
                
            case WM_COMMAND:
                if (HIWORD(wParam) == BN_CLICKED) {
                    if (LOWORD(wParam) == ID_NAME_OK) {
                        // Obter o nome digitado
                        HWND hwndNameInput = GetDlgItem(hwnd, ID_NAME_INPUT);
                        TCHAR customName[256];
                        GetWindowText(hwndNameInput, customName, 256);
                        
                        // Adicionar ao histórico com nome personalizado
                        AddToHistory(lastMatrix, lastConstants, lastSolution, customName);
                        
                        // Mostrar confirmação
                        TCHAR confirmMsg[512];
                        _stprintf_s(confirmMsg, TEXT("✓ Cálculo '%s' gravado com sucesso!\n\nTotal de itens no histórico: %d"), 
                                   customName, static_cast<int>(calculationHistory.size()));
                        MessageBox(hwndMain, confirmMsg, TEXT("Gravado no Histórico"), MB_OK | MB_ICONINFORMATION);
                        
                        // Fechar diálogo
                        DestroyWindow(hwnd);
                        hwndNameDialog = nullptr;
                        return 0;
                    } else if (LOWORD(wParam) == ID_NAME_CANCEL) {
                        // Cancelar
                        DestroyWindow(hwnd);
                        hwndNameDialog = nullptr;
                        return 0;
                    }
                }
                break;
                
            case WM_CLOSE:
                DestroyWindow(hwnd);
                hwndNameDialog = nullptr;
                return 0;
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    void AddToHistory(const std::vector<std::vector<double>>& matrix, 
                     const std::vector<double>& constants, 
                     const LinearSolver::Solution& solution, 
                     const std::basic_string<TCHAR>& customName = TEXT("")) {
        // Não adicionar ao histórico se há campos vazios ou erro
        if (!solution.hasSolution && solution.status == LinearSolver::SolutionStatus::CALCULATION_ERROR) {
            return;
        }
        
        // Verificar se já existe um cálculo idêntico recente (evitar duplicatas)
        if (!calculationHistory.empty()) {
            const auto& last = calculationHistory[0];
            if (last.size == currentSize && last.matrix == matrix && last.constants == constants) {
                return; // Não adicionar duplicata
            }
        }
        
        // Criar entrada do histórico
        CalculationHistory entry(currentSize, matrix, constants, solution);
        
        // Definir nome personalizado se fornecido
        if (!customName.empty()) {
            entry.customName = customName;
        }
        
        // Adicionar ao início da lista
        calculationHistory.insert(calculationHistory.begin(), entry);
        
        // Limitar o número de itens no histórico
        if (calculationHistory.size() > MAX_HISTORY_ITEMS) {
            calculationHistory.resize(MAX_HISTORY_ITEMS);
        }
        
        // Salvar no arquivo
        SaveHistoryToFile();
    }
    
    void ShowHistoryWindow() {
        if (hwndHistoryWindow && IsWindow(hwndHistoryWindow)) {
            // Se a janela já existe, apenas trazê-la para frente
            SetForegroundWindow(hwndHistoryWindow);
            return;
        }
        
        // Registrar classe da janela de histórico se necessário
        static bool classRegistered = false;
        if (!classRegistered) {
            WNDCLASSEX wcHistory = {};
            wcHistory.cbSize = sizeof(WNDCLASSEX);
            wcHistory.style = CS_HREDRAW | CS_VREDRAW;
            wcHistory.lpfnWndProc = HistoryWindowProc;
            wcHistory.hInstance = GetModuleHandle(nullptr);
            wcHistory.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_CALCULATOR));
            wcHistory.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcHistory.hbrBackground = hBrushBackground;
            wcHistory.lpszClassName = TEXT("HistoryWindow");
            RegisterClassEx(&wcHistory);
            classRegistered = true;
        }
        
        // Criar janela de histórico
        hwndHistoryWindow = CreateWindowEx(
            WS_EX_TOOLWINDOW,
            TEXT("HistoryWindow"),
            TEXT("Histórico de Cálculos"),
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 550, 400,
            hwndMain, nullptr, GetModuleHandle(nullptr), this
        );
        
        if (hwndHistoryWindow) {
            // Criar lista de histórico
            HWND hwndList = CreateWindow(TEXT("LISTBOX"), nullptr,
                WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                10, 10, 510, 300,
                hwndHistoryWindow, (HMENU)(UINT_PTR)ID_HISTORY_LIST, GetModuleHandle(nullptr), nullptr);
            
            // Botões
            CreateWindow(TEXT("BUTTON"), TEXT("Restaurar"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                10, 320, 100, 30,
                hwndHistoryWindow, (HMENU)(UINT_PTR)ID_RESTORE_CALCULATION, GetModuleHandle(nullptr), nullptr);
            
            CreateWindow(TEXT("BUTTON"), TEXT("Apagar Item"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                120, 320, 100, 30,
                hwndHistoryWindow, (HMENU)(UINT_PTR)ID_DELETE_ITEM, GetModuleHandle(nullptr), nullptr);
            
            CreateWindow(TEXT("BUTTON"), TEXT("Renomear"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                230, 320, 90, 30,
                hwndHistoryWindow, (HMENU)(UINT_PTR)ID_RENAME_ITEM, GetModuleHandle(nullptr), nullptr);
            
            CreateWindow(TEXT("BUTTON"), TEXT("Limpar Tudo"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                330, 320, 100, 30,
                hwndHistoryWindow, (HMENU)(UINT_PTR)ID_CLEAR_HISTORY, GetModuleHandle(nullptr), nullptr);
            
            CreateWindow(TEXT("BUTTON"), TEXT("Fechar"),
                WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                440, 320, 70, 30,
                hwndHistoryWindow, (HMENU)IDCANCEL, GetModuleHandle(nullptr), nullptr);
            
            // Preencher lista com histórico
            UpdateHistoryList(hwndList);
            
            // Aplicar fontes
            SendMessage(hwndList, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        }
    }
    
    void UpdateHistoryList(HWND hwndList) {
        // Limpar lista
        SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
        
        // Adicionar itens do histórico
        for (size_t i = 0; i < calculationHistory.size(); i++) {
            const auto& entry = calculationHistory[i];
            TCHAR listItem[512];
            
            if (!entry.customName.empty()) {
                // Mostrar nome personalizado primeiro
                _stprintf_s(listItem, TEXT("★ %s (%s)"), 
                           entry.customName.c_str(), entry.timestamp.c_str());
            } else {
                // Mostrar descrição padrão
                _stprintf_s(listItem, TEXT("%s - %s"), 
                           entry.timestamp.c_str(), entry.description.c_str());
            }
            
            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)listItem);
        }
    }
    
    void RestoreCalculationFromHistory(int index) {
        if (index < 0 || index >= static_cast<int>(calculationHistory.size())) {
            return;
        }
        
        const auto& entry = calculationHistory[index];
        
        // Ajustar o tamanho da matriz se necessário
        if (entry.size != currentSize) {
            currentSize = entry.size;
            TCHAR sizeStr[10];
            _stprintf_s(sizeStr, TEXT("%d"), currentSize);
            SetWindowText(hwndMatrixSize, sizeStr);
            UpdateMatrixInputs();
        }
        
        // Restaurar valores da matriz
        for (int i = 0; i < entry.size; i++) {
            for (int j = 0; j < entry.size; j++) {
                if (i < static_cast<int>(matrixInputs.size()) && 
                    j < static_cast<int>(matrixInputs[i].size())) {
                    TCHAR valueStr[32];
                    _stprintf_s(valueStr, TEXT("%.6g"), entry.matrix[i][j]);
                    SetWindowText(matrixInputs[i][j], valueStr);
                }
            }
        }
        
        // Restaurar constantes
        for (int i = 0; i < entry.size; i++) {
            if (i < static_cast<int>(constantInputs.size())) {
                TCHAR valueStr[32];
                _stprintf_s(valueStr, TEXT("%.6g"), entry.constants[i]);
                SetWindowText(constantInputs[i], valueStr);
            }
        }
        
        // Fechar janela de histórico
        if (hwndHistoryWindow && IsWindow(hwndHistoryWindow)) {
            DestroyWindow(hwndHistoryWindow);
            hwndHistoryWindow = nullptr;
        }
        
        // Focar na janela principal
        SetForegroundWindow(hwndMain);
        
        // Recalcular
        TriggerCalculation();
    }
    
    void DeleteHistoryItem(int index) {
        if (index < 0 || index >= static_cast<int>(calculationHistory.size())) {
            return;
        }
        
        // Confirmar exclusão
        const auto& entry = calculationHistory[index];
        TCHAR confirmMsg[512];
        
        if (!entry.customName.empty()) {
            _stprintf_s(confirmMsg, TEXT("Deseja realmente apagar o cálculo '%s'?"), 
                       entry.customName.c_str());
        } else {
            _stprintf_s(confirmMsg, TEXT("Deseja realmente apagar este cálculo?\n\n%s"), 
                       entry.description.c_str());
        }
        
        if (MessageBox(hwndHistoryWindow, confirmMsg, TEXT("Confirmar Exclusão"), 
                      MB_YESNO | MB_ICONQUESTION) == IDYES) {
            // Remover item do histórico
            calculationHistory.erase(calculationHistory.begin() + index);
            
            // Salvar alterações
            SaveHistoryToFile();
            
            MessageBox(hwndHistoryWindow, TEXT("Item removido do histórico."), 
                      TEXT("Sucesso"), MB_OK | MB_ICONINFORMATION);
        }
    }
    
    void RenameHistoryItem(int index) {
        if (index < 0 || index >= static_cast<int>(calculationHistory.size())) {
            return;
        }
        
        auto& entry = calculationHistory[index];
        
        // Criar diálogo simples para renomear
        TCHAR newName[256];
        
        // Usar nome atual ou descrição como valor inicial
        if (!entry.customName.empty()) {
            _tcscpy_s(newName, entry.customName.c_str());
        } else {
            _tcscpy_s(newName, entry.description.c_str());
        }
        
        // Mostrar diálogo de entrada simples (usando InputBox simulado)
        if (ShowInputDialog(TEXT("Renomear Cálculo"), TEXT("Digite o novo nome:"), newName, 256)) {
            // Atualizar nome
            entry.customName = newName;
            
            // Salvar alterações
            SaveHistoryToFile();
            
            MessageBox(hwndHistoryWindow, TEXT("Item renomeado com sucesso."), 
                      TEXT("Sucesso"), MB_OK | MB_ICONINFORMATION);
        }
    }
    
    bool ShowInputDialog(const TCHAR* title, const TCHAR* prompt, TCHAR* buffer, int bufferSize) {
        // Implementação simples usando MessageBox + InputBox
        // Para simplicidade, vamos usar um diálogo modal básico
        
        // Criar janela de diálogo temporária
        HWND hwndInputDialog = CreateWindowEx(
            WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
            TEXT("NameDialog"), // Reutilizar classe já registrada
            title,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
            hwndHistoryWindow, nullptr, GetModuleHandle(nullptr), this
        );
        
        if (!hwndInputDialog) return false;
        
        // Label de prompt
        CreateWindow(TEXT("STATIC"), prompt,
            WS_VISIBLE | WS_CHILD,
            20, 20, 350, 20,
            hwndInputDialog, nullptr, GetModuleHandle(nullptr), nullptr);
        
        // Campo de entrada
        HWND hwndInput = CreateWindow(TEXT("EDIT"), buffer,
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
            20, 50, 340, 25,
            hwndInputDialog, (HMENU)(UINT_PTR)ID_NAME_INPUT, GetModuleHandle(nullptr), nullptr);
        
        // Botões
        CreateWindow(TEXT("BUTTON"), TEXT("OK"),
            WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            200, 90, 80, 30,
            hwndInputDialog, (HMENU)(UINT_PTR)ID_NAME_OK, GetModuleHandle(nullptr), nullptr);
        
        CreateWindow(TEXT("BUTTON"), TEXT("Cancelar"),
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            290, 90, 80, 30,
            hwndInputDialog, (HMENU)(UINT_PTR)ID_NAME_CANCEL, GetModuleHandle(nullptr), nullptr);
        
        // Aplicar fonte e selecionar texto
        SendMessage(hwndInput, WM_SETFONT, (WPARAM)hFontMain, TRUE);
        SetFocus(hwndInput);
        SendMessage(hwndInput, EM_SETSEL, 0, -1);
        
        // Loop de mensagens modal simples
        bool result = false;
        bool dialogActive = true;
        
        while (dialogActive && IsWindow(hwndInputDialog)) {
            MSG msg;
            if (GetMessage(&msg, nullptr, 0, 0)) {
                // Verificar se é uma mensagem para nossa janela de diálogo
                if (msg.hwnd == hwndInputDialog || IsChild(hwndInputDialog, msg.hwnd)) {
                    // Verificar se Enter foi pressionado
                    if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
                        GetWindowText(hwndInput, buffer, bufferSize);
                        result = true;
                        dialogActive = false;
                        continue; // Não processar mais esta mensagem
                    }
                    // Verificar se Escape foi pressionado
                    else if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE) {
                        result = false;
                        dialogActive = false;
                        continue; // Não processar mais esta mensagem
                    }
                    else if (msg.message == WM_COMMAND && HIWORD(msg.wParam) == BN_CLICKED) {
                        if (LOWORD(msg.wParam) == ID_NAME_OK) {
                            GetWindowText(hwndInput, buffer, bufferSize);
                            result = true;
                            dialogActive = false;
                            continue; // Não processar mais esta mensagem
                        } else if (LOWORD(msg.wParam) == ID_NAME_CANCEL) {
                            result = false;
                            dialogActive = false;
                            continue; // Não processar mais esta mensagem
                        }
                    } else if (msg.message == WM_CLOSE) {
                        result = false;
                        dialogActive = false;
                        continue; // Não processar mais esta mensagem
                    }
                    
                    // Processar outras mensagens do diálogo
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                } else {
                    // Mensagens para outras janelas - processar normalmente
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            } else {
                // GetMessage retornou 0 (WM_QUIT) ou -1 (erro)
                break;
            }
        }
        
        if (IsWindow(hwndInputDialog)) {
            DestroyWindow(hwndInputDialog);
        }
        
        return result;
    }
    
    void ClearHistory() {
        calculationHistory.clear();
        SaveHistoryToFile(); // Salvar após limpar
        
        // Atualizar lista se a janela estiver aberta
        if (hwndHistoryWindow && IsWindow(hwndHistoryWindow)) {
            HWND hwndList = GetDlgItem(hwndHistoryWindow, ID_HISTORY_LIST);
            if (hwndList) {
                UpdateHistoryList(hwndList);
            }
        }
    }
    
    void SaveHistoryToFile() {
        try {
            std::ofstream file(HISTORY_FILE, std::ios::binary);
            if (!file.is_open()) {
                return; // Falha silenciosa se não conseguir abrir
            }
            
            // Escrever número de itens
            size_t count = calculationHistory.size();
            file.write(reinterpret_cast<const char*>(&count), sizeof(count));
            
            // Escrever cada item do histórico
            for (const auto& item : calculationHistory) {
                // Escrever tamanho da matriz
                file.write(reinterpret_cast<const char*>(&item.size), sizeof(item.size));
                
                // Escrever matriz
                for (int i = 0; i < item.size; i++) {
                    for (int j = 0; j < item.size; j++) {
                        file.write(reinterpret_cast<const char*>(&item.matrix[i][j]), sizeof(double));
                    }
                }
                
                // Escrever constantes
                for (int i = 0; i < item.size; i++) {
                    file.write(reinterpret_cast<const char*>(&item.constants[i]), sizeof(double));
                }
                
                // Escrever solução
                file.write(reinterpret_cast<const char*>(&item.solution.hasSolution), sizeof(bool));
                file.write(reinterpret_cast<const char*>(&item.solution.status), sizeof(LinearSolver::SolutionStatus));
                
                size_t valuesCount = item.solution.values.size();
                file.write(reinterpret_cast<const char*>(&valuesCount), sizeof(valuesCount));
                for (const auto& value : item.solution.values) {
                    file.write(reinterpret_cast<const char*>(&value), sizeof(double));
                }
                
                // Escrever timestamp (como string simples)
                size_t timestampLen = item.timestamp.length();
                file.write(reinterpret_cast<const char*>(&timestampLen), sizeof(timestampLen));
                file.write(reinterpret_cast<const char*>(item.timestamp.c_str()), timestampLen * sizeof(TCHAR));
                
                // Escrever descrição
                size_t descLen = item.description.length();
                file.write(reinterpret_cast<const char*>(&descLen), sizeof(descLen));
                file.write(reinterpret_cast<const char*>(item.description.c_str()), descLen * sizeof(TCHAR));
                
                // Escrever nome personalizado
                size_t customNameLen = item.customName.length();
                file.write(reinterpret_cast<const char*>(&customNameLen), sizeof(customNameLen));
                if (customNameLen > 0) {
                    file.write(reinterpret_cast<const char*>(item.customName.c_str()), customNameLen * sizeof(TCHAR));
                }
            }
            
            file.close();
        } catch (...) {
            // Falha silenciosa em caso de erro
        }
    }
    
    void LoadHistoryFromFile() {
        try {
            std::ifstream file(HISTORY_FILE, std::ios::binary);
            if (!file.is_open()) {
                return; // Arquivo não existe ou não pode ser aberto
            }
            
            calculationHistory.clear();
            
            // Ler número de itens
            size_t count;
            file.read(reinterpret_cast<char*>(&count), sizeof(count));
            
            if (count > MAX_HISTORY_ITEMS) {
                count = MAX_HISTORY_ITEMS; // Limitar para evitar problemas
            }
            
            // Ler cada item do histórico
            for (size_t idx = 0; idx < count; idx++) {
                CalculationHistory item;
                
                // Ler tamanho da matriz
                file.read(reinterpret_cast<char*>(&item.size), sizeof(item.size));
                
                if (item.size <= 0 || item.size > 20) {
                    break; // Dados corrompidos
                }
                
                // Ler matriz
                item.matrix.resize(item.size);
                for (int i = 0; i < item.size; i++) {
                    item.matrix[i].resize(item.size);
                    for (int j = 0; j < item.size; j++) {
                        file.read(reinterpret_cast<char*>(&item.matrix[i][j]), sizeof(double));
                    }
                }
                
                // Ler constantes
                item.constants.resize(item.size);
                for (int i = 0; i < item.size; i++) {
                    file.read(reinterpret_cast<char*>(&item.constants[i]), sizeof(double));
                }
                
                // Ler solução
                file.read(reinterpret_cast<char*>(&item.solution.hasSolution), sizeof(bool));
                file.read(reinterpret_cast<char*>(&item.solution.status), sizeof(LinearSolver::SolutionStatus));
                
                size_t valuesCount;
                file.read(reinterpret_cast<char*>(&valuesCount), sizeof(valuesCount));
                item.solution.values.resize(valuesCount);
                for (size_t i = 0; i < valuesCount; i++) {
                    file.read(reinterpret_cast<char*>(&item.solution.values[i]), sizeof(double));
                }
                
                // Ler timestamp
                size_t timestampLen;
                file.read(reinterpret_cast<char*>(&timestampLen), sizeof(timestampLen));
                if (timestampLen > 0 && timestampLen < 1000) { // Validação básica
                    std::vector<TCHAR> timestampBuffer(timestampLen + 1);
                    file.read(reinterpret_cast<char*>(timestampBuffer.data()), timestampLen * sizeof(TCHAR));
                    timestampBuffer[timestampLen] = 0;
                    item.timestamp = timestampBuffer.data();
                }
                
                // Ler descrição
                size_t descLen;
                file.read(reinterpret_cast<char*>(&descLen), sizeof(descLen));
                if (descLen > 0 && descLen < 1000) { // Validação básica
                    std::vector<TCHAR> descBuffer(descLen + 1);
                    file.read(reinterpret_cast<char*>(descBuffer.data()), descLen * sizeof(TCHAR));
                    descBuffer[descLen] = 0;
                    item.description = descBuffer.data();
                }
                
                // Ler nome personalizado (pode não existir em arquivos antigos)
                size_t customNameLen = 0;
                if (file.read(reinterpret_cast<char*>(&customNameLen), sizeof(customNameLen))) {
                    if (customNameLen > 0 && customNameLen < 1000) { // Validação básica
                        std::vector<TCHAR> customNameBuffer(customNameLen + 1);
                        file.read(reinterpret_cast<char*>(customNameBuffer.data()), customNameLen * sizeof(TCHAR));
                        customNameBuffer[customNameLen] = 0;
                        item.customName = customNameBuffer.data();
                    }
                }
                
                calculationHistory.push_back(item);
            }
            
            file.close();
        } catch (...) {
            // Em caso de erro, limpar histórico corrompido
            calculationHistory.clear();
        }
    }
    
    void CalculationWorker() {
        while (calculationRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Debounce de 300ms
            
            bool needsCalculation = false;
            {
                std::lock_guard<std::mutex> lock(calculationMutex);
                needsCalculation = shouldCalculate;
                shouldCalculate = false;
            }
            
            if (needsCalculation) {
                PerformCalculation();
            }
        }
    }
    
    void PerformCalculation() {
        try {
            // Coletar dados da matriz
            std::vector<std::vector<double>> matrix(currentSize, std::vector<double>(currentSize));
            std::vector<double> constants(currentSize);
            
            bool hasEmptyFields = false;
            
            for (int i = 0; i < currentSize; i++) {
                for (int j = 0; j < currentSize; j++) {
                    TCHAR buffer[32];
                    GetWindowText(matrixInputs[i][j], buffer, 32);
                    
                    if (_tcslen(buffer) == 0) {
                        hasEmptyFields = true;
                        matrix[i][j] = 0.0;
                    } else {
                        // Validar se é um número válido
                        TCHAR* endPtr;
                        double value = _tcstod(buffer, &endPtr);
                        if (*endPtr == '\0') {
                            matrix[i][j] = value;
                        } else {
                            // Valor inválido, tratar como campo vazio
                            matrix[i][j] = 0.0;
                            hasEmptyFields = true;
                        }
                    }
                }
                
                TCHAR buffer[32];
                GetWindowText(constantInputs[i], buffer, 32);
                
                if (_tcslen(buffer) == 0) {
                    hasEmptyFields = true;
                    constants[i] = 0.0;
                } else {
                    // Validar se é um número válido
                    TCHAR* endPtr;
                    double value = _tcstod(buffer, &endPtr);
                    if (*endPtr == '\0') {
                        constants[i] = value;
                    } else {
                        // Valor inválido, tratar como campo vazio
                        constants[i] = 0.0;
                        hasEmptyFields = true;
                    }
                }
            }
            
            std::basic_string<TCHAR> result;
            
            if (hasEmptyFields) {
                result = TEXT("Digite os coeficientes da matriz e as constantes...");
            } else {
                auto solution = solver.Solve(matrix, constants);
                
                // Armazenar último cálculo (não adicionar automaticamente ao histórico)
                lastMatrix = matrix;
                lastConstants = constants;
                lastSolution = solution;
                hasLastCalculation = true;
                
                if (solution.hasSolution) {
                    result = TEXT("Solução encontrada:\r\n\r\n");
                    for (size_t i = 0; i < solution.values.size(); i++) {
                        std::basic_stringstream<TCHAR> ss;
                        ss << TEXT("x") << (i + 1) << TEXT(" = ") << std::fixed << std::setprecision(6) << solution.values[i] << TEXT("\r\n");
                        result += ss.str();
                    }
                } else {
                    switch (solution.status) {
                        case LinearSolver::SolutionStatus::NO_SOLUTION:
                            result = TEXT("Sistema inconsistente.\r\nNão há solução.");
                            break;
                        case LinearSolver::SolutionStatus::INFINITE_SOLUTIONS:
                            result = TEXT("Sistema indeterminado.\r\nInfinitas soluções.");
                            break;
                        default:
                            result = TEXT("Erro no cálculo.");
                            break;
                    }
                }
            }
            
            // Atualizar UI na thread principal
            PostMessage(hwndMain, WM_USER + 1, 0, (LPARAM)new std::basic_string<TCHAR>(result));
            
        } catch (...) {
            PostMessage(hwndMain, WM_USER + 1, 0, (LPARAM)new std::basic_string<TCHAR>(TEXT("Erro no cálculo")));
        }
    }
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        CalculatorApp* app = nullptr;
        
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<CalculatorApp*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        } else {
            app = reinterpret_cast<CalculatorApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        
        if (app) {
            return app->HandleMessage(hwnd, uMsg, wParam, lParam);
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    static LRESULT CALLBACK NameDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        CalculatorApp* app = nullptr;
        
        if (uMsg == WM_CREATE) {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<CalculatorApp*>(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        } else {
            app = reinterpret_cast<CalculatorApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        
        if (app) {
            return app->HandleNameDialogMessage(hwnd, uMsg, wParam, lParam);
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    static LRESULT CALLBACK HistoryWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        CalculatorApp* app = nullptr;
        
        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
            app = reinterpret_cast<CalculatorApp*>(pCreate->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        } else {
            app = reinterpret_cast<CalculatorApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }
        
        if (app) {
            return app->HandleHistoryMessage(hwnd, uMsg, wParam, lParam);
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_KEYDOWN:
                // Verificar se Enter foi pressionado no campo de número de variáveis
                if (wParam == VK_RETURN && GetFocus() == hwndMatrixSize) {
                    // Forçar atualização do tamanho da matriz
                    TCHAR buffer[10];
                    GetWindowText(hwndMatrixSize, buffer, 10);
                    int newSize = _ttoi(buffer);
                    
                    // Validação e atualização
                    if (newSize >= 2 && newSize <= 10 && newSize != currentSize) {
                        currentSize = newSize;
                        PostMessage(hwndMain, WM_USER + 2, 0, 0);
                    } else if (newSize > 10) {
                        SetWindowText(hwndMatrixSize, TEXT("10"));
                        currentSize = 10;
                        PostMessage(hwndMain, WM_USER + 2, 0, 0);
                    } else if (newSize < 2) {
                        SetWindowText(hwndMatrixSize, TEXT("2"));
                        currentSize = 2;
                        PostMessage(hwndMain, WM_USER + 2, 0, 0);
                    }
                    
                    // Mover foco para o primeiro campo da matriz
                    if (!matrixInputs.empty() && !matrixInputs[0].empty() && IsWindow(matrixInputs[0][0])) {
                        SetFocus(matrixInputs[0][0]);
                    }
                    
                    return 0;
                }
                break;
                
            case WM_COMMAND:
                if (HIWORD(wParam) == BN_CLICKED) {
                    if (LOWORD(wParam) == ID_HISTORY_BUTTON) {
                        ShowHistoryWindow();
                    } else if (LOWORD(wParam) == ID_SAVE_BUTTON) {
                        SaveCurrentCalculation();
                    } else if (LOWORD(wParam) == ID_CLEAR_BUTTON) {
                        ClearAllVariables();
                    }
                } else if (HIWORD(wParam) == EN_CHANGE) {
                    if (LOWORD(wParam) == ID_MATRIX_SIZE) {
                        TCHAR buffer[10];
                        GetWindowText(hwndMatrixSize, buffer, 10);
                        int newSize = _ttoi(buffer);
                        
                        // Validação mais robusta
                        if (newSize > 0 && newSize <= 10 && newSize != currentSize) {
                            currentSize = newSize;
                            // Pequeno delay para garantir que a mudança seja processada
                            PostMessage(hwndMain, WM_USER + 2, 0, 0);
                        } else if (newSize > 10) {
                            // Limitar a 10 variáveis
                            SetWindowText(hwndMatrixSize, TEXT("10"));
                            currentSize = 10;
                            PostMessage(hwndMain, WM_USER + 2, 0, 0);
                        } else if (newSize < 1 && _tcslen(buffer) > 0) {
                            // Mínimo de 1 variável
                            SetWindowText(hwndMatrixSize, TEXT("1"));
                            currentSize = 1;
                            PostMessage(hwndMain, WM_USER + 2, 0, 0);
                        }
                    } else if (LOWORD(wParam) >= ID_MATRIX_START) {
                        TriggerCalculation();
                    }
                }
                break;
                
            case WM_USER + 1: // Atualizar resultado
                {
                    std::basic_string<TCHAR>* result = reinterpret_cast<std::basic_string<TCHAR>*>(lParam);
                    SetWindowText(hwndResultText, result->c_str());
                    delete result;
                }
                break;
                
            case WM_USER + 2: // Atualizar matriz com delay
                UpdateMatrixInputs();
                break;
                
            case WM_CTLCOLORSTATIC:
            case WM_CTLCOLOREDIT:
                {
                    HDC hdc = (HDC)wParam;
                    SetTextColor(hdc, COLOR_TEXT);
                    SetBkColor(hdc, COLOR_PANEL);
                    return (LRESULT)hBrushPanel;
                }
                
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hwnd, &ps);
                    
                    // Desenhar fundo gradiente sutil
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    FillRect(hdc, &rect, hBrushBackground);
                    
                    EndPaint(hwnd, &ps);
                }
                return 0;
                
            case WM_SIZE:
                // Redimensionamento da janela - invalidar para redesenhar
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
                
            case WM_GETMINMAXINFO:
                {
                    // Definir tamanho mínimo da janela baseado no número de variáveis
                    MINMAXINFO* pMinMax = reinterpret_cast<MINMAXINFO*>(lParam);
                    
                    // Calcular tamanho mínimo baseado no layout atual
                    int cellWidth = std::max(45, std::min(70, 400 / currentSize));
                    int cellHeight = std::max(28, std::min(35, 300 / currentSize));
                    int spacing = std::max(2, std::min(8, 50 / currentSize));
                    int matrixWidth = currentSize * cellWidth + (currentSize - 1) * spacing;
                    
                    int minWidth = matrixWidth + 80 + cellWidth + 350 + 100; // matriz + espaço + constante + resultado + margem
                    int minHeight = 400 + currentSize * (cellHeight + spacing) + 100;
                    
                    pMinMax->ptMinTrackSize.x = std::max(800, minWidth);
                    pMinMax->ptMinTrackSize.y = std::max(500, minHeight);
                }
                return 0;
                
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    
    LRESULT HandleHistoryMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_COMMAND:
                if (HIWORD(wParam) == BN_CLICKED) {
                    switch (LOWORD(wParam)) {
                        case ID_RESTORE_CALCULATION:
                            {
                                HWND hwndList = GetDlgItem(hwnd, ID_HISTORY_LIST);
                                int selection = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                                if (selection != LB_ERR) {
                                    RestoreCalculationFromHistory(selection);
                                }
                            }
                            break;
                            
                        case ID_DELETE_ITEM:
                            {
                                HWND hwndList = GetDlgItem(hwnd, ID_HISTORY_LIST);
                                int selection = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                                if (selection != LB_ERR) {
                                    DeleteHistoryItem(selection);
                                    UpdateHistoryList(hwndList);
                                } else {
                                    MessageBox(hwnd, TEXT("Selecione um item para apagar."), 
                                             TEXT("Nenhum Item Selecionado"), MB_OK | MB_ICONINFORMATION);
                                }
                            }
                            break;
                            
                        case ID_RENAME_ITEM:
                            {
                                HWND hwndList = GetDlgItem(hwnd, ID_HISTORY_LIST);
                                int selection = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                                if (selection != LB_ERR) {
                                    RenameHistoryItem(selection);
                                    UpdateHistoryList(hwndList);
                                } else {
                                    MessageBox(hwnd, TEXT("Selecione um item para renomear."), 
                                             TEXT("Nenhum Item Selecionado"), MB_OK | MB_ICONINFORMATION);
                                }
                            }
                            break;
                            
                        case ID_CLEAR_HISTORY:
                            if (MessageBox(hwnd, TEXT("Deseja realmente limpar todo o histórico?"), 
                                         TEXT("Confirmar"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
                                ClearHistory();
                            }
                            break;
                            
                        case IDCANCEL:
                            DestroyWindow(hwnd);
                            hwndHistoryWindow = nullptr;
                            break;
                    }
                } else if (HIWORD(wParam) == LBN_DBLCLK && LOWORD(wParam) == ID_HISTORY_LIST) {
                    // Duplo clique na lista - restaurar cálculo
                    HWND hwndList = GetDlgItem(hwnd, ID_HISTORY_LIST);
                    int selection = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                    if (selection != LB_ERR) {
                        RestoreCalculationFromHistory(selection);
                    }
                }
                break;
                
            case WM_CLOSE:
                DestroyWindow(hwnd);
                hwndHistoryWindow = nullptr;
                break;
                
            case WM_DESTROY:
                hwndHistoryWindow = nullptr;
                break;
        }
        
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/) {
    // Inicializar Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
    
    CalculatorApp app;
    
    if (!app.Initialize(hInstance)) {
        MessageBox(nullptr, TEXT("Falha ao inicializar a aplicação"), TEXT("Erro"), MB_OK | MB_ICONERROR);
        return -1;
    }
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        // Processar mensagens de teclado para navegação com TAB
        if (!IsDialogMessage(app.GetMainWindow(), &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    return static_cast<int>(msg.wParam);
}



