#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>

using namespace std;
using TermID = uint32_t;

int main() {
    // number of barrels and input file
    const int NUM_BARRELS = 8;
    ifstream fin("../sampleFiles/inverted_index.txt");
    if (!fin.is_open()) {
        cerr << "cannot open ../sampleFiles/inverted_index.txt\n";
        return 1;
    }

    // open output barrel files with .idx extension
    vector<ofstream> barrels(NUM_BARRELS);
    for (int i = 0; i < NUM_BARRELS; ++i) {
        string fname = "../sampleFiles/barrel_" + to_string(i) + ".idx";
        barrels[i].open(fname);
        if (!barrels[i].is_open()) {
            cerr << "cannot write " << fname << "\n";
            return 1;
        }
    }

    // read each line, decide barrel by termID % 8, and write line there
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        TermID tid;
        size_t df;
        if (!(ss >> tid >> df)) continue;
        int b = static_cast<int>(tid % NUM_BARRELS);
        barrels[b] << line << "\n";
    }

    // done
    return 0;
}
