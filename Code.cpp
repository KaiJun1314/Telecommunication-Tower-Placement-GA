#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<iomanip>
using namespace std;

//Station Tower Configuration
const int GRID_SIZE = 20;
const int COVERAGE_LEVEL = 3;
const float SIGNAL_STRENGTH[COVERAGE_LEVEL] = { 1.0, 0.75, 0.5 }; //High, Medium, Low
const int COVERAGE_RADIUS_EACH_LEVEL = 1;
const float BEST_SCENARIO = (GRID_SIZE * GRID_SIZE);

//Genetic Alogrithm Configuration
const int GENE = GRID_SIZE;
const int POP_SIZE = 50;
const float CROSSOVER_PROBABILITY = 0.9;
const float MUTATION_PROBABILITY = 0.2;
const int MAXIMUM_GENERATION = 30;
const float NUM_WEIGHTAGE = 20.0;
const float SIGNAL_WEIGHTAGE = 80.0;
const int CROSSOVER_POINT = 3;

//Data
vector<vector<float>>grid(GRID_SIZE, vector<float>(GRID_SIZE, 0));
int chromosome[POP_SIZE][GENE] = {};
float fitnessValue[POP_SIZE] = {};
int parents[2][GENE] = {};
int children[2][GENE] = {};
int survivor[POP_SIZE][GENE] = {};
int counterSurvivor = 0;
double averageFitness = 0;
double bestFitness = -1;
double bestChromosome[GENE] = {};
ofstream ACP_File("ACP_File.txt");
ofstream BSF_File("BSF_File.txt");
ofstream BC_File("BC_File.txt");
ofstream fout("Graph.csv");

//Function Prototype
void initializePopulation();
void printChromosome();
void evaluateChromosome();
void printGrid();
void calculateSignalvalue(int chromosome_index);
void parentSelectionTournament();
void parentSelectionProportionate();
void crossover();
void mutation();
void survivorSelection();
void copyChromosome();
void recordBestFitness();
void calculateAverageFitness();

int main() {
    srand(2);
    cout << "Initialize Of Population" << endl;
    initializePopulation();

    for (int i = 0; i < MAXIMUM_GENERATION; i++)
    {
        cout << "=================== Generation " << i + 1 << "===================\n";
        printChromosome();

        cout << "Fitness Evaluation" << endl;
        evaluateChromosome();
        calculateAverageFitness();
        recordBestFitness();

        counterSurvivor = 0;
        for (int j = 0; j < POP_SIZE; j += 2) {
            cout << " *** ITERATION " << j + 1 << " ***" << endl << endl;
            cout << "Parent Selection" << endl;
            parentSelectionTournament();

            cout << "Crossover" << endl;
            crossover();

            cout << "Mutation" << endl;
            mutation();

            cout << "Survival Selection" << endl;
            survivorSelection();
        }
        cout << "New Population" << endl;
        copyChromosome();
    }

    ACP_File.close();
    BSF_File.close();
    BC_File.close();
    fout.close();
    return 0;
}

//Print the value of grid
void printGrid() {
    for (vector<float> i : grid) {
        for (float j : i) {
            cout << setw(4) << j << " ";
        }
        cout << endl;
    }
}

//Intialize the chromosome of the population
void initializePopulation() {
    //srand(rand());
    for (int c = 0; c < POP_SIZE; c++) {
        for (int g = 0; g < GENE; g++) {
            chromosome[c][g] = rand() % (GRID_SIZE + 1);
        }
    }
}

//Print the value for each chromosome
void printChromosome() {
    for (int c = 0; c < POP_SIZE; c++) {
        cout << "\tChromosome " << c + 1 << "\t: ";
        for (int g = 0; g < GENE; g++) {
            cout << chromosome[c][g] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

//Calculate total signal receive at each box of grid
void calculateSignalvalue(int chromosome_index) {

    for (int g = 0; g < GENE; g++) {
        if (chromosome[chromosome_index][g] == 0) {
            continue;
        }
        else {
            int station_location = chromosome[chromosome_index][g] - 1;
            for (int i = COVERAGE_LEVEL; i > 0; i--) {
                float signal = 0.0;
                if (i == COVERAGE_LEVEL) {
                    signal = SIGNAL_STRENGTH[COVERAGE_LEVEL - 1];
                }
                else {
                    signal = SIGNAL_STRENGTH[i - 1] - SIGNAL_STRENGTH[i];
                }

                int radius = i * COVERAGE_RADIUS_EACH_LEVEL;
                for (int k = station_location - radius; k < station_location + radius + 1; k++) {
                    for (int l = g - radius; l < g + radius + 1; l++) {
                        try {
                            grid.at(k).at(l) += signal;
                        }
                        catch (...) { continue; }
                    }
                }
            }
        }
    }
}

//Evaluate the fitness value of each chromosome
void evaluateChromosome() {
    //for (int c = 0; c < 1; c++){
    for (int c = 0; c < POP_SIZE; c++) {
        calculateSignalvalue(c);
        float total = 0.0;
        for (vector<float> i : grid) {
            for (float j : i) {
                if (j > 1) {
                    total += 1 / j;
                }
                else {
                    total += j;
                }
            }
        }

        float num_station_used = 0.0;
        for (int g = 0; g < GENE; g++) {
            if (chromosome[c][g] != 0) {
                num_station_used += 1.0;
            }
        }

        float fitness = (total / BEST_SCENARIO * SIGNAL_WEIGHTAGE) + ((1 / (num_station_used)) * NUM_WEIGHTAGE);
        fitnessValue[c] = fitness;
        cout << "\tChromosome " << c + 1 << "\t: " << fitnessValue[c] << endl;

        //Print the Grid
        //printGrid();
        //system("pause");

        //Reintialize the grid for next chromosome
        for (vector<float>& i : grid) {
            i = vector<float>(GENE, 0);
        }
    }
    cout << endl;
}

//Parent Selection Strategy : Fitness Proportionate
void parentSelectionProportionate()
{
    float sumFitness = 0.0;
    float probabilities[POP_SIZE] = {};
    int parentIndex[2] = {};
    float randomNumber;

    //Calculate total fitness value
    for (int c = 0; c < POP_SIZE; c++) {
        sumFitness += fitnessValue[c];
    }

    //Calculate porport of each chromosome
    probabilities[0] = (fitnessValue[0] / sumFitness);
    for (int c = 1; c < POP_SIZE; c++) {
        probabilities[c] = (fitnessValue[c] / sumFitness);
        probabilities[c] += probabilities[c - 1];
    }

    //srand(rand());
    for (int p = 0; p < 2; p++) {
        float sumProbabilities = 0;
        do
        {
            randomNumber = ((float)rand() / RAND_MAX);
            for (int c = 0; c < POP_SIZE; c++) {
                if (randomNumber < probabilities[c]) {
                    parentIndex[p] = c;
                    for (int g = 0; g < GENE; g++) {
                        parents[p][g] = chromosome[c][g];
                    }
                    break;
                }
            }
        } while (parentIndex[0] == parentIndex[1]);
        cout << "\t Random Probability : " << randomNumber << endl;
        cout << "\t Parent " << (p + 1) << " : Chromosome " << (parentIndex[p] + 1) << endl << endl;
    }

    for (int p = 0; p < 2; p++) {
        cout << "\t Parent " << p + 1 << "\t: ";
        for (int g = 0; g < GENE; g++) {
            cout << parents[p][g] << " ";
        }
        cout << endl;
    }
    cout << endl;
}

//Parent Selection Strategy : Tournament
void parentSelectionTournament() {
    int indexPlayer1 = 0, indexPlayer2 = 0;
    int winner[2] = {};

    // tournament selection
    do {
        for (int p = 0; p < 2; p++) {
            indexPlayer1 = rand() % POP_SIZE;
            do {
                indexPlayer2 = rand() % POP_SIZE;
            } while (indexPlayer1 == indexPlayer2);

            cout << "\tPlayer " << indexPlayer1 + 1 << " vs Player " << indexPlayer2 + 1 << endl;

            // determine winner
            if (fitnessValue[indexPlayer1] > fitnessValue[indexPlayer2]) {
                winner[p] = indexPlayer1;
            }
            else {
                winner[p] = indexPlayer2;
            }

            cout << "\t\tWinner " << p + 1 << " is player " << winner[p] + 1 << endl << endl;
        } // end of tournament
    } while (winner[0] == winner[1]); // restart tournament if same winner

    // print winner (parents)
    for (int r = 0; r < 2; r++) {
        cout << "\tParent " << r + 1 << " (C" << winner[r] + 1 << ")\t\t\t: ";
        for (int g = 0; g < GENE; g++) {
            parents[r][g] = chromosome[winner[r]][g];
            cout << parents[r][g] << " ";
        }
        cout << endl;
    }

    cout << endl;
 }

//Crossover Operation
void crossover() {
    // copying children from parents
    for (int n = 0; n < 2; n++) {
        for (int g = 0; g < GENE; g++) {
            children[n][g] = parents[n][g];
        }
    }

    double probability = (rand() % 10) / 10.0;
    if (probability <= CROSSOVER_PROBABILITY) { // crossover happen
        int crossover_point[CROSSOVER_POINT + 1] = {};
        int* pos;

        do {
            //(rand());
            //Generate Crossover Point
            for (int i = 0; i < CROSSOVER_POINT; i++) {
                crossover_point[i] = rand() % GENE;
            }

            //Assert last point position for crossover from point 3 to last gene
            crossover_point[CROSSOVER_POINT] = GENE;

            //Sort crossover point
            sort(crossover_point, crossover_point + CROSSOVER_POINT + 1);
            pos = std::adjacent_find(crossover_point, crossover_point + CROSSOVER_POINT + 1);

        } while (pos != (crossover_point + CROSSOVER_POINT + 1));

        //Display crossover point
        cout << "\tCrossover at point " << crossover_point[0];
        for (int i = 1; i <= CROSSOVER_POINT - 1; i++) {
            cout << ", " << crossover_point[i];
        }
        cout << endl;

        // crossover process
        for (int i = 0; i < CROSSOVER_POINT; i += 2) {
            for (int g = crossover_point[i]; g < crossover_point[i + 1]; g++) {
                children[0][g] = parents[1][g];
                children[1][g] = parents[0][g];
            }
        }

        // print children
        for (int n = 0; n < 2; n++) {
            cout << "\tChildren " << n + 1 << "\t: ";
            int* end = crossover_point + CROSSOVER_POINT + 1;
            for (int g = 0; g < GENE; g++) {
                if (find(crossover_point, end, g) != end) //this is to mark the crossover point
                    cout << "** ";
                cout << children[n][g] << " ";
            }
            cout << endl;
        }
    }
    else { // crossover does not happen
        cout << "\tNo crossover" << endl;
    }
    cout << endl;
}

//Mutation operation 
void mutation() {
    for (int c = 0; c < 2; c++) {
        //srand(rand());
        float randomValue = (rand() % 11) / 10.0;
        if (randomValue < MUTATION_PROBABILITY) { // mutation happen
            int start_point = rand() % GENE;
            int end_point;
            int size;
            do {
                end_point = rand() % GENE;
                if (start_point > end_point) {
                    int tmp = start_point;
                    start_point = end_point;
                    end_point = tmp;
                }
                size = end_point - start_point;
            } while (size <= 5);

            cout << "\tMutation Start Point = " << start_point << endl;
            cout << "\tMutation End Point = " << end_point << endl;
            random_shuffle(&children[c][start_point], &children[c][start_point] + size);

            // print mutated child
            cout << "\tChildren " << c + 1 << " after mutation \t: ";
            for (int g = 0; g < GENE; g++) {
                if (g == start_point || g == end_point)
                    cout << "**";
                cout << children[c][g] << " ";
            }
            cout << endl;
        }
        else { // no mutation
            cout << "\tNo mutation for children " << c + 1 << endl;
        }
        cout << endl;
    }
}

//Survior Selection - Children Replace Parents
void survivorSelection()
{
    for (int c = 0; c < 2; c++) {
        for (int g = 0; g < GENE; g++) {
            survivor[counterSurvivor][g] = children[c][g];
        }
        counterSurvivor++;
    }

    for (int c = counterSurvivor - 2; c < counterSurvivor; c++) {
        cout << "\tSurvivor " << c + 1 << "\t:";
        for (int g = 0; g < GENE; g++) {
            cout << survivor[c][g] << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

//New population
void copyChromosome()
{
    for (int c = 0; c < POP_SIZE; c++) {
        for (int g = 0; g < GENE; g++) {
            chromosome[c][g] = survivor[c][g];
        }
    }
    printChromosome();
}

//Calculate Average Fitness
void calculateAverageFitness()
{
    double totalFitness = 0;
    for (int c = 0; c < POP_SIZE; c++) {
        totalFitness = totalFitness + fitnessValue[c];
    }

    averageFitness = totalFitness / POP_SIZE;
    cout << "\tAverage Fitness\t= " << averageFitness << endl;
    ACP_File << averageFitness << endl;
    fout << averageFitness << ",";
}

//Record Best Fitness
void recordBestFitness()
{
    for (int c = 0; c < POP_SIZE; c++) {
        if (fitnessValue[c] >= bestFitness) {
            bestFitness = fitnessValue[c];
            for (int g = 0; g < GENE; g++) {
                bestChromosome[g] = chromosome[c][g];
            }
        }
    }

    cout << "\tBest Fitness\t= " << bestFitness << endl;
    BSF_File << bestFitness << endl;
    cout << "\tBest Chromosome\t= ";
    for (int g = 0; g < GENE; g++) {
        cout << bestChromosome[g] << ' ';
        BC_File << bestChromosome[g] << ' ';
    }
    BC_File << endl;

    fout << bestFitness << ",,";
    for (int g = 0; g < GENE; g++) {
        fout << bestChromosome[g] << ",";
    }
    fout << endl;
    cout << endl << endl;
}