#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

class LinearSolver {
public:
    enum class SolutionStatus {
        UNIQUE_SOLUTION,
        NO_SOLUTION,
        INFINITE_SOLUTIONS,
        CALCULATION_ERROR
    };
    
    struct Solution {
        bool hasSolution;
        SolutionStatus status;
        std::vector<double> values;
        
        Solution() : hasSolution(false), status(SolutionStatus::CALCULATION_ERROR) {}
    };
    
private:
    static constexpr double EPSILON = 1e-10;
    
    // Função auxiliar para verificar se um número é praticamente zero
    bool IsZero(double value) const {
        return std::abs(value) < EPSILON;
    }
    
    // Função para encontrar o pivô na coluna
    int FindPivot(const std::vector<std::vector<double>>& matrix, int col, int startRow) const {
        int pivotRow = startRow;
        double maxAbs = std::abs(matrix[startRow][col]);
        
        for (int i = startRow + 1; i < static_cast<int>(matrix.size()); i++) {
            double currentAbs = std::abs(matrix[i][col]);
            if (currentAbs > maxAbs) {
                maxAbs = currentAbs;
                pivotRow = i;
            }
        }
        
        return (maxAbs > EPSILON) ? pivotRow : -1;
    }
    
    // Função para trocar duas linhas da matriz
    void SwapRows(std::vector<std::vector<double>>& matrix, int row1, int row2) const {
        if (row1 != row2) {
            std::swap(matrix[row1], matrix[row2]);
        }
    }
    
    // Eliminação Gaussiana com pivoteamento parcial
    Solution GaussianElimination(std::vector<std::vector<double>> augmentedMatrix) const {
        Solution solution;
        int n = static_cast<int>(augmentedMatrix.size());
        
        if (n == 0 || augmentedMatrix[0].size() != static_cast<size_t>(n + 1)) {
            solution.status = SolutionStatus::CALCULATION_ERROR;
            return solution;
        }
        
        std::vector<int> pivotCols(n, -1); // Para rastrear colunas de pivô
        int rank = 0;
        
        // Fase de eliminação (forward elimination)
        for (int col = 0; col < n && rank < n; col++) {
            // Encontrar pivô
            int pivotRow = FindPivot(augmentedMatrix, col, rank);
            
            if (pivotRow == -1) {
                // Coluna toda zero - pular para próxima coluna
                continue;
            }
            
            // Trocar linhas se necessário
            SwapRows(augmentedMatrix, rank, pivotRow);
            pivotCols[rank] = col;
            
            // Normalizar linha do pivô
            double pivot = augmentedMatrix[rank][col];
            for (int j = 0; j <= n; j++) {
                augmentedMatrix[rank][j] /= pivot;
            }
            
            // Eliminar elementos abaixo do pivô
            for (int i = rank + 1; i < n; i++) {
                if (!IsZero(augmentedMatrix[i][col])) {
                    double factor = augmentedMatrix[i][col];
                    for (int j = 0; j <= n; j++) {
                        augmentedMatrix[i][j] -= factor * augmentedMatrix[rank][j];
                    }
                }
            }
            
            rank++;
        }
        
        // Verificar consistência do sistema
        for (int i = rank; i < n; i++) {
            if (!IsZero(augmentedMatrix[i][n])) {
                // Linha da forma [0 0 ... 0 | c] onde c ≠ 0
                solution.status = SolutionStatus::NO_SOLUTION;
                return solution;
            }
        }
        
        // Verificar se há variáveis livres (infinitas soluções)
        if (rank < n) {
            solution.status = SolutionStatus::INFINITE_SOLUTIONS;
            return solution;
        }
        
        // Substituição regressiva (back substitution)
        solution.values.resize(n, 0.0);
        
        for (int i = rank - 1; i >= 0; i--) {
            int col = pivotCols[i];
            if (col == -1) continue;
            
            solution.values[col] = augmentedMatrix[i][n];
            
            for (int j = col + 1; j < n; j++) {
                solution.values[col] -= augmentedMatrix[i][j] * solution.values[j];
            }
        }
        
        // Marcar como solução válida
        solution.hasSolution = true;
        solution.status = SolutionStatus::UNIQUE_SOLUTION;
        
        return solution;
    }
    
    // Verificar se a solução encontrada é válida
    bool VerifySolution(const std::vector<std::vector<double>>& coefficients, 
                       const std::vector<double>& constants,
                       const std::vector<double>& solution) const {
        int n = static_cast<int>(solution.size());
        
        for (int i = 0; i < n; i++) {
            double sum = 0.0;
            for (int j = 0; j < n; j++) {
                sum += coefficients[i][j] * solution[j];
            }
            
            if (std::abs(sum - constants[i]) > EPSILON * 100) {
                return false;
            }
        }
        
        return true;
    }
    
public:
    // Método principal para resolver o sistema
    Solution Solve(const std::vector<std::vector<double>>& coefficients, 
                   const std::vector<double>& constants) const {
        Solution solution;
        
        if (coefficients.empty() || constants.empty() || 
            coefficients.size() != constants.size()) {
            solution.status = SolutionStatus::CALCULATION_ERROR;
            return solution;
        }
        
        int n = static_cast<int>(coefficients.size());
        
        // Verificar se a matriz é quadrada
        for (const auto& row : coefficients) {
            if (row.size() != static_cast<size_t>(n)) {
                solution.status = SolutionStatus::CALCULATION_ERROR;
                return solution;
            }
        }
        
        // Criar matriz aumentada [A|b]
        std::vector<std::vector<double>> augmentedMatrix(n, std::vector<double>(n + 1));
        
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                augmentedMatrix[i][j] = coefficients[i][j];
            }
            augmentedMatrix[i][n] = constants[i];
        }
        
        auto result = GaussianElimination(augmentedMatrix);
        
        // Verificar solução se encontrada
        if (result.hasSolution && !VerifySolution(coefficients, constants, result.values)) {
            result.hasSolution = false;
            result.status = SolutionStatus::CALCULATION_ERROR;
        }
        
        return result;
    }
    
    // Método para calcular o determinante (útil para diagnósticos)
    double CalculateDeterminant(const std::vector<std::vector<double>>& matrix) const {
        if (matrix.empty() || matrix.size() != matrix[0].size()) {
            return 0.0;
        }
        
        int n = static_cast<int>(matrix.size());
        auto tempMatrix = matrix; // Cópia para não modificar a original
        
        double det = 1.0;
        
        for (int i = 0; i < n; i++) {
            // Encontrar pivô
            int pivotRow = i;
            for (int k = i + 1; k < n; k++) {
                if (std::abs(tempMatrix[k][i]) > std::abs(tempMatrix[pivotRow][i])) {
                    pivotRow = k;
                }
            }
            
            if (IsZero(tempMatrix[pivotRow][i])) {
                return 0.0; // Determinante é zero
            }
            
            if (pivotRow != i) {
                std::swap(tempMatrix[i], tempMatrix[pivotRow]);
                det *= -1.0; // Troca de linha muda o sinal do determinante
            }
            
            det *= tempMatrix[i][i];
            
            // Eliminação
            for (int k = i + 1; k < n; k++) {
                double factor = tempMatrix[k][i] / tempMatrix[i][i];
                for (int j = i; j < n; j++) {
                    tempMatrix[k][j] -= factor * tempMatrix[i][j];
                }
            }
        }
        
        return det;
    }
};


