#include <iostream>
#include <vector>
#include <iomanip>
#include "LinearSolver.h"

void testCase(const std::string& name, 
              const std::vector<std::vector<double>>& matrix,
              const std::vector<double>& constants) {
    std::cout << "\n=== " << name << " ===" << std::endl;
    
    LinearSolver solver;
    auto solution = solver.Solve(matrix, constants);
    
    std::cout << "Status: ";
    switch (solution.status) {
        case LinearSolver::SolutionStatus::UNIQUE_SOLUTION:
            std::cout << "Solução única" << std::endl;
            break;
        case LinearSolver::SolutionStatus::NO_SOLUTION:
            std::cout << "Sem solução" << std::endl;
            break;
        case LinearSolver::SolutionStatus::INFINITE_SOLUTIONS:
            std::cout << "Infinitas soluções" << std::endl;
            break;
        case LinearSolver::SolutionStatus::CALCULATION_ERROR:
            std::cout << "Erro de cálculo" << std::endl;
            break;
    }
    
    if (solution.hasSolution) {
        std::cout << "Solução:" << std::endl;
        for (size_t i = 0; i < solution.values.size(); i++) {
            std::cout << "x" << (i+1) << " = " << std::fixed << std::setprecision(6) 
                      << solution.values[i] << std::endl;
        }
        
        // Verificar substituindo de volta
        std::cout << "Verificação:" << std::endl;
        for (size_t i = 0; i < matrix.size(); i++) {
            double sum = 0.0;
            for (size_t j = 0; j < matrix[i].size(); j++) {
                sum += matrix[i][j] * solution.values[j];
            }
            std::cout << "Equação " << (i+1) << ": " << sum 
                      << " = " << constants[i] 
                      << " (erro: " << std::abs(sum - constants[i]) << ")" << std::endl;
        }
    }
}

int main() {
    std::cout << "Testando LinearSolver..." << std::endl;
    
    // Teste 1: Sistema 2x2 com solução única
    testCase("Sistema 2x2 - Solução única",
        {{2, 3}, {1, -1}},
        {7, 1});
    
    // Teste 2: Sistema 3x3 com solução única  
    testCase("Sistema 3x3 - Solução única",
        {{1, 2, 3}, {2, -1, 1}, {3, 0, -1}},
        {9, 8, 3});
    
    // Teste 3: Sistema inconsistente
    testCase("Sistema inconsistente",
        {{1, 2}, {2, 4}},
        {3, 7});
    
    // Teste 4: Sistema com infinitas soluções
    testCase("Sistema com infinitas soluções",
        {{1, 2}, {2, 4}},
        {3, 6});
    
    // Teste 5: Matriz identidade
    testCase("Matriz identidade",
        {{1, 0}, {0, 1}},
        {5, 3});
    
    return 0;
}
