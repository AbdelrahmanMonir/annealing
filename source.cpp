#include <iostream>
#include <vector>   
#include <algorithm>
#include <cmath>    
#include <random>   
#include <iomanip>  
#include <fstream>  
#include <chrono>


using namespace std;

// Struct to represent components
struct Cell {
    int number;
    int row;
    int column;
};

// Struct to represent a net
struct Net {
    vector<int> connections;
};


void clearGrid(int numRows)
{
    // clear the board to animate the new output 
    for (int i = 0; i < numRows; ++i) {
        cout << "\033[1A\033[2K";
    }
}

// function that prints the grid out
void printGrid(const vector<Cell>& initialPlacement, int numRows, int numCols, int cellWidth) {
    vector<vector<int>> grid(numRows, vector<int>(numCols, -1)); //making a 2D vector 


    for (const Cell& cell : initialPlacement) {
        grid[cell.row][cell.column] = cell.number;
    }

    // Print the grid
    cout << endl << endl << "Initial Placement: " << endl;
    for (const std::vector<int>& row : grid) {
        for (const int& number : row) {
            if (number != -1) {
                cout << setw(cellWidth) << number << " ";
            }
            else {
                cout << setw(cellWidth) << "-" << " ";
            }
        }
        cout << endl;
    }
    cout << "Final Placement: " << endl;

}

void PrintZeroesandOnes(const vector<Cell>& initialPlacement, int numRows, int numCols, int cellWidth) {
    vector<vector<int>> grid(numRows, vector<int>(numCols, 1)); // Initialize all cells as non-empty (1)

    for (const Cell& cell : initialPlacement) {
        grid[cell.row][cell.column] = 0; // Set the cell as empty (0)
    }

    // Print the grid
    cout << endl << endl << "Grid: " << endl;
    for (const vector<int>& row : grid) {
        for (const int& value : row) {
            cout << setw(cellWidth) << value << " ";
        }
        cout <<endl;
    }
}
// Function to calculate the wirelength of a net
int calculateWireLength(const std::vector<Cell>& cells, const Net& net) {
    int minX = cells[net.connections[0]].column;
    int maxX = minX;
    int minY = cells[net.connections[0]].row;
    int maxY = minY;

    for (size_t i = 1; i < net.connections.size(); ++i) {
        int col = cells[net.connections[i]].column;
        int row = cells[net.connections[i]].row;
        minX = min(minX, col);
        maxX = max(maxX, col);
        minY = min(minY, row);
        maxY = max(maxY, row);

    }

    return (maxX - minX) + (maxY - minY);
}

// Function to calculate the total wirelength of the placement
int calculateTotalWirelength(const vector<Cell>& cells, const vector<Net>& nets) {

    int totalWirelength = 0;
    for (const Net& net : nets) {

        
        totalWirelength += calculateWireLength(cells, net);
    }
    return totalWirelength;
}

// Function to perform a random initial placement
vector<Cell> performInitialPlacement(int numRows, int numCols, int numCells) {
    vector<Cell> cells;
    cells.reserve(numCells); //intiating using .resize(), did not work so we decided to go with that

    random_device rd;
    mt19937 generator(rd());

    vector<int> index(numRows * numCols);
    for (int i = 0; i < numRows * numCols; ++i) {
        index[i] = i;
    }
    shuffle(index.begin(), index.end(), generator);

    for (int i = 0; i < numCells; ++i) {
        int row = index[i] / numCols;
        int col = index[i] % numCols;
        cells.push_back({ i, row, col });
    }

    return cells;
}


// Function to perform simulated annealing-based placement
void performPlacement(vector<Cell>& currentPlacement, const vector<Net>& nets, double initialTemperature, double finalTemperature, double coolingRate, int numRows, int numCols) {
    int currentWirelength = calculateTotalWirelength(currentPlacement, nets);
    int numCells = currentPlacement.size();
    double currentTemperature = initialTemperature;

    cout << endl;
    auto startTime = chrono::high_resolution_clock::now(); //starting clock
    int counter = 0;
    while (currentTemperature > finalTemperature) {
        int movesPerTemperature = 10 * numCells;

        for (int i = 0; i < movesPerTemperature; ++i) {
            random_device rd;
            mt19937 generator(rd());
            uniform_int_distribution<int> cellDistribution(0, numCells - 1);

            int randomCellIndex = cellDistribution(generator);
            Cell& cell = currentPlacement[randomCellIndex];

            uniform_int_distribution<int> rowDistribution(0, numRows - 1);
            uniform_int_distribution<int> colDistribution(0, numCols - 1);

            int oldRow = cell.row;
            int oldCol = cell.column;

            int newRow = rowDistribution(generator);
            int newCol = colDistribution(generator);

            if (newRow != oldRow || newCol != oldCol) {
                bool overlap = false; //see if there is an overlap
                for (const Cell& otherCell : currentPlacement) {
                    if (&otherCell != &cell && otherCell.row == newRow && otherCell.column == newCol) {
                        overlap = true;
                        break;
                    }
                }

                if (!overlap) {
                    cell.row = newRow;
                    cell.column = newCol;

                    int newWirelength = calculateTotalWirelength(currentPlacement, nets);
                    int deltaWirelength = newWirelength - currentWirelength;

                    if (deltaWirelength < 0 || exp(-deltaWirelength / currentTemperature) > static_cast<double>(rand()) / RAND_MAX) {
                        currentWirelength = newWirelength;
                    }
                    else {
                        cell.row = oldRow;
                        cell.column = oldCol;
                    }
                }
            }
        }

        currentTemperature *= coolingRate;

        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();


        if (counter)
            clearGrid(numRows + 4);

        printGrid(currentPlacement, numRows, numCols, 4);

        counter++;
    }
    cout << endl;
    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << "Final wirelength: " << currentWirelength << endl;

    cout << "Execution time: " << duration / 1000 << " seconds" << endl;
}



int main() {
    int numCells, numNets, numRows, numCols;
    int cellWidth = 4;


    vector<double> coolingRates = { 0.75, 0.8, 0.85, 0.9, 0.95 };

    for (double coolingRate : coolingRates) {
        ifstream inputFile("Text1.txt");
        if (inputFile.is_open()) {
            inputFile >> numCells >> numNets >> numRows >> numCols;

            vector<Cell> cells;
            cells.reserve(numCells);
            vector<Net> nets;
            nets.reserve(numNets);
            // Read net information
            for (int i = 0; i < numNets; ++i) {
                int numComponents;
                inputFile >> numComponents;
                // cout << endl << " number of components: "<< numComponents; 

                Net net;
                net.connections.resize(numComponents);
                for (int j = 0; j < numComponents; ++j) {
                    inputFile >> net.connections[j];
                }

                nets.push_back(net);
            }
            inputFile.close();


            // Perform the initial random placement
            vector<Cell> initialPlacement = performInitialPlacement(numRows, numCols, numCells);
            // Displaing the grid
            printGrid(initialPlacement, numRows, numCols, cellWidth);
            double total_wire_length = calculateTotalWirelength(initialPlacement, nets);
            cout << "Initial wirelength : " << total_wire_length << endl;
            double initialTemperature = 500 * total_wire_length;
            cout << "Inital temperature :" << initialTemperature << endl;
            double finalTemperature = 5e-6 * total_wire_length / numNets;
            cout << "Final temperature : " << finalTemperature << endl;

            vector<Cell>finalPlacement = initialPlacement;
            cout << "The final results for cooling rate : " << coolingRate << endl;
            performPlacement(finalPlacement, nets, initialTemperature, finalTemperature, coolingRate, numRows, numCols);
            PrintZeroesandOnes(finalPlacement, numRows, numCols, cellWidth);

            system("pause");
            system("clear");
        }
        else {
            cout << "File was not found. " << endl;
            return 1; 
        }
    }
    return NULL; 
}
