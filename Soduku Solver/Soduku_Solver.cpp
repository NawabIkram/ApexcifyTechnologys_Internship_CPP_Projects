#include <iostream>
#include <vector>
#include <iomanip>
#include <cctype>
#include <string>
#include <limits>
#include <chrono>
#include <sstream>

using namespace std;

// SUDOKU GRID CLASS

/**
 * SudokuGrid class - Manages the 9x9 Sudoku grid data
 * Single Responsibility Principle (SRP): Only manages grid data and basic operations
 */
class SudokuGrid {
private:
    static const int GRID_SIZE = 9;
    static const int SUBGRID_SIZE = 3;
    vector<vector<int>> grid;
    
public:
    // Constructor
    SudokuGrid() : grid(GRID_SIZE, vector<int>(GRID_SIZE, 0)) {}
    
    // Set value at position
    bool setValue(int row, int col, int value) {
        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            return false;
        }
        if (value < 0 || value > 9) {
            return false;
        }
        grid[static_cast<size_t>(row)][static_cast<size_t>(col)] = value;
        return true;
    }
    
    // Get value at position
    int getValue(int row, int col) const {
        if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
            return -1;
        }
        return grid[static_cast<size_t>(row)][static_cast<size_t>(col)];
    }
    
    // Check if cell is empty
    bool isEmpty(int row, int col) const {
        return grid[static_cast<size_t>(row)][static_cast<size_t>(col)] == 0;
    }
    
    // Get grid size
    int getSize() const { return GRID_SIZE; }
    int getSubgridSize() const { return SUBGRID_SIZE; }
    
    // Copy grid
    SudokuGrid copy() const {
        SudokuGrid newGrid;
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                newGrid.setValue(i, j, grid[static_cast<size_t>(i)][static_cast<size_t>(j)]);
            }
        }
        return newGrid;
    }
    
    // Check if grid is full
    bool isFull() const {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[static_cast<size_t>(i)][static_cast<size_t>(j)] == 0) return false;
            }
        }
        return true;
    }
    
    // Find next empty cell
    bool findEmptyCell(int& row, int& col) const {
        for (row = 0; row < GRID_SIZE; row++) {
            for (col = 0; col < GRID_SIZE; col++) {
                if (grid[static_cast<size_t>(row)][static_cast<size_t>(col)] == 0) {
                    return true;
                }
            }
        }
        return false;
    }
    
    // Access operator for convenience
    vector<int>& operator[](int index) {
        return grid[static_cast<size_t>(index)];
    }
    
    const vector<int>& operator[](int index) const {
        return grid[static_cast<size_t>(index)];
    }
};

// ============================================================================
// SUDOKU VALIDATOR CLASS
// ============================================================================

/**
 * SudokuValidator class - Validates moves according to Sudoku rules
 * Single Responsibility Principle (SRP): Only handles validation logic
 */
class SudokuValidator {
private:
    const SudokuGrid& grid;
    
    bool isValidInRow(int row, int num) const {
        for (int col = 0; col < grid.getSize(); col++) {
            if (grid.getValue(row, col) == num) {
                return false;
            }
        }
        return true;
    }
    
    bool isValidInColumn(int col, int num) const {
        for (int row = 0; row < grid.getSize(); row++) {
            if (grid.getValue(row, col) == num) {
                return false;
            }
        }
        return true;
    }
    
    bool isValidInSubgrid(int row, int col, int num) const {
        int subgridSize = grid.getSubgridSize();
        int startRow = (row / subgridSize) * subgridSize;
        int startCol = (col / subgridSize) * subgridSize;
        
        for (int i = 0; i < subgridSize; i++) {
            for (int j = 0; j < subgridSize; j++) {
                if (grid.getValue(startRow + i, startCol + j) == num) {
                    return false;
                }
            }
        }
        return true;
    }
    
public:
    SudokuValidator(const SudokuGrid& g) : grid(g) {}
    
    // Check if placing a number is valid
    bool isValidMove(int row, int col, int num) const {
        // Check if position is empty
        if (!grid.isEmpty(row, col)) {
            return false;
        }
        
        // Check row, column, and subgrid
        return isValidInRow(row, num) &&
               isValidInColumn(col, num) &&
               isValidInSubgrid(row, col, num);
    }
    
    // Validate entire puzzle
    bool isValidPuzzle() const {
        // Check all non-zero cells
        for (int i = 0; i < grid.getSize(); i++) {
            for (int j = 0; j < grid.getSize(); j++) {
                int value = grid.getValue(i, j);
                if (value != 0) {
                    // Temporarily clear cell to check validity
                    SudokuGrid tempGrid = grid.copy();
                    tempGrid.setValue(i, j, 0);
                    SudokuValidator tempValidator(tempGrid);
                    
                    if (!tempValidator.isValidMove(i, j, value)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};

// ============================================================================
// SUDOKU SOLVER CLASS (Backtracking Algorithm)
// ============================================================================

/**
 * SudokuSolver class - Implements backtracking algorithm to solve puzzles
 * Single Responsibility Principle (SRP): Only handles solving logic
 */
class SudokuSolver {
private:
    SudokuGrid grid;
    int solutionsFound;
    vector<SudokuGrid> solutions;
    bool m_findAllSolutions;
    
    bool solveRecursive() {
        int row, col;
        
        // Find next empty cell
        if (!grid.findEmptyCell(row, col)) {
            solutionsFound++;
            if (m_findAllSolutions) {
                solutions.push_back(grid.copy());
                return false; // Continue searching for more solutions
            }
            return true; // Puzzle solved
        }
        
        // Try numbers 1-9
        for (int num = 1; num <= grid.getSize(); num++) {
            SudokuValidator validator(grid);
            if (validator.isValidMove(row, col, num)) {
                // Place number
                grid.setValue(row, col, num);
                
                // Recursively solve
                if (solveRecursive()) {
                    if (!m_findAllSolutions) {
                        return true;
                    }
                }
                
                // Backtrack
                grid.setValue(row, col, 0);
            }
        }
        
        return false; // No solution found from this branch
    }
    
public:
    SudokuSolver() : solutionsFound(0), m_findAllSolutions(false) {}
    
    // Solve a single solution (standard)
    bool solve(SudokuGrid& puzzle) {
        grid = puzzle;
        solutionsFound = 0;
        m_findAllSolutions = false;
        
        // First validate the puzzle
        SudokuValidator validator(grid);
        if (!validator.isValidPuzzle()) {
            return false;
        }
        
        // Solve recursively
        bool solved = solveRecursive();
        
        if (solved) {
            puzzle = grid;
        }
        
        return solved;
    }
    
    // Find all possible solutions
    vector<SudokuGrid> findAllSolutions(SudokuGrid& puzzle) {
        grid = puzzle;
        solutions.clear();
        solutionsFound = 0;
        m_findAllSolutions = true;
        
        // First validate the puzzle
        SudokuValidator validator(grid);
        if (!validator.isValidPuzzle()) {
            return solutions;
        }
        
        solveRecursive();
        
        return solutions;
    }
    
    // Check if puzzle has multiple solutions
    bool hasMultipleSolutions(SudokuGrid& puzzle) {
        vector<SudokuGrid> allSolutions = findAllSolutions(puzzle);
        return allSolutions.size() > 1;
    }
    
    int getSolutionsCount() const {
        return solutionsFound;
    }
};

// ============================================================================
// DISPLAY MANAGER CLASS
// ============================================================================

/**
 * DisplayManager class - Handles all console output formatting
 * Single Responsibility Principle (SRP): Only manages display logic
 */
class DisplayManager {
private:
    static const int CELL_WIDTH = 3;
    
    void printHorizontalLine(int size, int subgridSize) const {
        cout << "  ";
        for (int i = 0; i < size; i++) {
            cout << "+";
            for (int j = 0; j < CELL_WIDTH; j++) {
                cout << "-";
            }
            if ((i + 1) % subgridSize == 0 && i != size - 1) {
                cout << "+";
            }
        }
        cout << "+\n";
    }
    
public:
    void showWelcome() const {
        cout << "\n";
        cout << "============================================================\n";
        cout << "                   SUDOKU SOLVER                            \n";
        cout << "                  Backtracking Algorithm                    \n";
        cout << "============================================================\n";
        cout << "\n";
    }
    
    void showMenu() const {
        cout << "\n";
        cout << "============================================================\n";
        cout << "                       MAIN MENU                            \n";
        cout << "============================================================\n";
        cout << "1. Enter Sudoku Puzzle (9x9)\n";
        cout << "2. Solve with Sample Puzzle\n";
        cout << "3. Solve with Hard Puzzle\n";
        cout << "4. Solve with Empty Puzzle\n";
        cout << "5. Find All Solutions (for puzzles with multiple solutions)\n";
        cout << "6. Exit\n";
        cout << "------------------------------------------------------------\n";
        cout << "Enter your choice (1-6): ";
    }
    
    void showPuzzle(const SudokuGrid& grid, const string& title = "Sudoku Puzzle") const {
        cout << "\n";
        cout << "============================================================\n";
        cout << "  " << title << "\n";
        cout << "============================================================\n";
        
        int size = grid.getSize();
        int subgridSize = grid.getSubgridSize();
        
        for (int i = 0; i < size; i++) {
            // Print horizontal line before each subgrid row
            if (i % subgridSize == 0) {
                printHorizontalLine(size, subgridSize);
            }
            
            // Print row content
            cout << "  ";
            for (int j = 0; j < size; j++) {
                cout << "| ";
                int value = grid.getValue(i, j);
                if (value == 0) {
                    cout << "  ";
                } else {
                    cout << value << " ";
                }
                if ((j + 1) % subgridSize == 0 && j != size - 1) {
                    cout << "| ";
                }
            }
            cout << "|\n";
        }
        printHorizontalLine(size, subgridSize);
        cout << "\n";
    }
    
    void showSolution(const SudokuGrid& solution, double durationMs) const {
        cout << "\n";
        cout << "============================================================\n";
        cout << "                     SOLUTION FOUND                         \n";
        cout << "============================================================\n";
        showPuzzle(solution, "Solved Sudoku Puzzle");
        cout << "Time taken: " << fixed << setprecision(3) << durationMs << " ms\n";
    }
    
    void showMultipleSolutions(const vector<SudokuGrid>& solutions, double durationMs) const {
        cout << "\n";
        cout << "============================================================\n";
        cout << "           MULTIPLE SOLUTIONS FOUND                         \n";
        cout << "============================================================\n";
        cout << "Total solutions found: " << solutions.size() << "\n";
        cout << "Time taken: " << fixed << setprecision(3) << durationMs << " ms\n\n";
        
        size_t maxShow = (solutions.size() < 5) ? solutions.size() : 5;
        for (size_t i = 0; i < maxShow; i++) {
            stringstream ss;
            ss << "Solution " << (i + 1);
            showPuzzle(solutions[i], ss.str());
        }
        
        if (solutions.size() > 5) {
            cout << "... and " << (solutions.size() - 5) << " more solutions\n";
        }
    }
    
    void showError(const string& message) const {
        cout << "\n[ERROR] " << message << "\n";
    }
    
    void showSuccess(const string& message) const {
        cout << "\n[SUCCESS] " << message << "\n";
    }
    
    void showInfo(const string& message) const {
        cout << "\n[INFO] " << message << "\n";
    }
    
    void pressAnyKey() const {
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }
    
    void clearScreen() const {
        for (int i = 0; i < 50; i++) {
            cout << "\n";
        }
    }
};

// ============================================================================
// INPUT HANDLER CLASS
// ============================================================================

/**
 * InputHandler class - Manages user input for Sudoku puzzles
 * Single Responsibility Principle (SRP): Only handles input collection
 */
class InputHandler {
private:
    DisplayManager& display;
    
    bool isValidSudokuInput(int value) const {
        return value >= 0 && value <= 9;
    }
    
public:
    InputHandler(DisplayManager& disp) : display(disp) {}
    
    SudokuGrid getPuzzleFromUser() const {
        SudokuGrid grid;
        
        cout << "\n";
        cout << "============================================================\n";
        cout << "              ENTER SUDOKU PUZZLE (9x9)                     \n";
        cout << "============================================================\n";
        cout << "Instructions:\n";
        cout << "  Enter numbers row by row\n";
        cout << "  Use 0 for empty cells\n";
        cout << "  Separate numbers with spaces\n";
        cout << "------------------------------------------------------------\n\n";
        
        for (int i = 0; i < 9; i++) {
            cout << "Row " << (i + 1) << ": ";
            for (int j = 0; j < 9; j++) {
                int value;
                cin >> value;
                
                while (!isValidSudokuInput(value)) {
                    cout << "Invalid input! Enter a number between 0 and 9: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cin >> value;
                }
                
                grid.setValue(i, j, value);
            }
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return grid;
    }
    
    SudokuGrid getSamplePuzzle() const {
        SudokuGrid grid;
        
        // Sample medium difficulty puzzle
        int puzzle[9][9] = {
            {5, 3, 0, 0, 7, 0, 0, 0, 0},
            {6, 0, 0, 1, 9, 5, 0, 0, 0},
            {0, 9, 8, 0, 0, 0, 0, 6, 0},
            {8, 0, 0, 0, 6, 0, 0, 0, 3},
            {4, 0, 0, 8, 0, 3, 0, 0, 1},
            {7, 0, 0, 0, 2, 0, 0, 0, 6},
            {0, 6, 0, 0, 0, 0, 2, 8, 0},
            {0, 0, 0, 4, 1, 9, 0, 0, 5},
            {0, 0, 0, 0, 8, 0, 0, 7, 9}
        };
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid.setValue(i, j, puzzle[i][j]);
            }
        }
        
        return grid;
    }
    
    SudokuGrid getHardPuzzle() const {
        SudokuGrid grid;
        
        // Hard difficulty puzzle
        int puzzle[9][9] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 3, 0, 8, 5},
            {0, 0, 1, 0, 2, 0, 0, 0, 0},
            {0, 0, 0, 5, 0, 7, 0, 0, 0},
            {0, 0, 4, 0, 0, 0, 1, 0, 0},
            {0, 9, 0, 0, 0, 0, 0, 0, 0},
            {5, 0, 0, 0, 0, 0, 0, 7, 3},
            {0, 0, 2, 0, 1, 0, 0, 0, 0},
            {0, 0, 0, 0, 4, 0, 0, 0, 9}
        };
        
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                grid.setValue(i, j, puzzle[i][j]);
            }
        }
        
        return grid;
    }
    
    SudokuGrid getEmptyPuzzle() const {
        return SudokuGrid();
    }
};

// ============================================================================
// SUDOKU APP CLASS (Facade Pattern)
// ============================================================================

/**
 * SudokuApp class - Main application controller
 * Facade Pattern: Provides simplified interface to complex subsystem
 */
class SudokuApp {
private:
    DisplayManager display;
    InputHandler inputHandler;
    
    void solveAndDisplay(SudokuGrid puzzle, bool findAll = false) {
        display.showPuzzle(puzzle, "Original Sudoku Puzzle");
        
        auto start = chrono::high_resolution_clock::now();
        
        SudokuSolver solver;
        bool solved = false;
        vector<SudokuGrid> solutions;
        
        if (findAll) {
            solutions = solver.findAllSolutions(puzzle);
            solved = !solutions.empty();
        } else {
            solved = solver.solve(puzzle);
        }
        
        auto end = chrono::high_resolution_clock::now();
        double durationMs = chrono::duration<double, milli>(end - start).count();
        
        if (findAll) {
            if (solutions.empty()) {
                display.showError("No solutions found for this puzzle!");
            } else if (solutions.size() == 1) {
                display.showSuccess("Found exactly 1 solution!");
                display.showSolution(solutions[0], durationMs);
            } else {
                display.showMultipleSolutions(solutions, durationMs);
            }
        } else {
            if (solved) {
                display.showSuccess("Puzzle solved successfully!");
                display.showSolution(puzzle, durationMs);
            } else {
                display.showError("No solution exists for this puzzle!");
            }
        }
    }
    
public:
    SudokuApp() : display(), inputHandler(display) {}
    
    void run() {
        bool running = true;
        
        while (running) {
            display.clearScreen();
            display.showWelcome();
            display.showMenu();
            
            int choice;
            cin >> choice;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            SudokuGrid puzzle;
            
            switch (choice) {
                case 1:
                    puzzle = inputHandler.getPuzzleFromUser();
                    solveAndDisplay(puzzle);
                    break;
                    
                case 2:
                    puzzle = inputHandler.getSamplePuzzle();
                    solveAndDisplay(puzzle);
                    break;
                    
                case 3:
                    puzzle = inputHandler.getHardPuzzle();
                    solveAndDisplay(puzzle);
                    break;
                    
                case 4:
                    puzzle = inputHandler.getEmptyPuzzle();
                    display.showInfo("Solving empty puzzle...");
                    solveAndDisplay(puzzle);
                    break;
                    
                case 5:
                    puzzle = inputHandler.getSamplePuzzle();
                    display.showInfo("Finding all possible solutions...");
                    solveAndDisplay(puzzle, true);
                    break;
                    
                case 6:
                    running = false;
                    display.showInfo("Thank you for using Sudoku Solver!");
                    break;
                    
                default:
                    display.showError("Invalid choice! Please select 1-6.");
            }
            
            if (running && choice >= 1 && choice <= 5) {
                display.pressAnyKey();
            }
        }
    }
};

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================

int main() {
    try {
        SudokuApp app;
        app.run();
    } catch (const exception& e) {
        cerr << "\nFatal error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}