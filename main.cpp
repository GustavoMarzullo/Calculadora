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
#include "LinearSolver.h"
#include "resource.h"

// Constantes para IDs dos controles
#define ID_MATRIX_SIZE 1001
#define ID_SOLVE_BUTTON 1002
#define ID_RESULT_TEXT 1003
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
    std::vector<std::vector<HWND>> matrixInputs;
    std::vector<HWND> constantInputs;
    LinearSolver solver;
    
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
        
        // Área de resultado
        hwndSolutionLabel = CreateWindow(TEXT("STATIC"), TEXT("Solução:"),
            WS_VISIBLE | WS_CHILD,
            450, 70, 80, 25,
            hwndMain, nullptr, GetModuleHandle(nullptr), nullptr);
        
        hwndResultText = CreateWindow(TEXT("EDIT"), TEXT(""),
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL,
            450, 100, 300, 400,
            hwndMain, (HMENU)(UINT_PTR)ID_RESULT_TEXT, GetModuleHandle(nullptr), nullptr);
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
        std::vector<HWND> toKeep = {hwndMatrixSize, hwndResultText, hwndTitleLabel, hwndVariablesLabel, hwndSolutionLabel};
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
        
        // Ajustar tamanho da janela se necessário
        RECT windowRect, clientRect;
        GetWindowRect(hwndMain, &windowRect);
        GetClientRect(hwndMain, &clientRect);
        
        int minWindowWidth = totalWidth + 350; // espaço para resultado
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
        
        // Reposicionar área de resultado dinamicamente
        int resultX = totalWidth + 50;
        int resultY = 70;
        int resultWidth = 300;
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
    
    LRESULT HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_COMMAND:
                if (HIWORD(wParam) == EN_CHANGE) {
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



