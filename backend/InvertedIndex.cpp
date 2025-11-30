#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace std;
using DocID = string;            // Using cord_uid as document ID
using TermID = uint32_t;         // Numeric term ID

// Structure representing one posting entry (doc + positions)
struct Posting {
    DocID doc_id;
    vector<uint32_t> pos;
};

// Inverted index: termID → list of postings
unordered_map<TermID, vector<Posting>> inv;

// Removes leading whitespace from a string
static inline void ltrim(string& s) {
    while (!s.empty() && (s[0]==' ' || s[0]=='\t' || s[0]=='\r'))
        s.erase(s.begin());
}

int main() {
    // Read forward_index.txt to build inverted index
    ifstream fin("../sampleFiles/forward_index.txt");
    if (!fin.is_open()) {
        cerr << "forward_index.txt not found\n";
        return 1;
    }

    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);

        DocID d;                 // Document ID (cord_uid)
        size_t nt;               // Number of terms
        ss >> d >> nt;

        string rest;
        getline(ss, rest);
        ltrim(rest);             // Trim leading whitespace from remaining text
        if (rest.empty()) continue;

        stringstream rs(rest);
        string block;

        // Parse each "termID:poslist" block separated by semicolons
        while (getline(rs, block, ';')) {
            ltrim(block);
            if (block.empty()) continue;

            size_t pos_colon = block.find(':');
            if (pos_colon == string::npos) continue;

            TermID tid = static_cast<TermID>(stoul(block.substr(0, pos_colon)));
            string pos_str = block.substr(pos_colon + 1);

            vector<uint32_t> positions;
            stringstream ps(pos_str);
            string t;

            // Extract all positions for this term in this document
            while (getline(ps, t, ',')) {
                ltrim(t);
                if (!t.empty())
                    positions.push_back(static_cast<uint32_t>(stoul(t)));
            }

            if (positions.empty()) continue;

            inv[tid].push_back({ d, positions });   // Add posting entry
        }
    }

    // Sort posting lists by doc_id to keep them ordered
    for (auto &kv : inv) {
        auto &plist = kv.second;
        sort(plist.begin(), plist.end(),
             [](auto &a, auto &b) { return a.doc_id < b.doc_id; });
    }

    // Write inverted index to file
    ofstream fout("../sampleFiles/inverted_index.txt");
    if (!fout.is_open()) {
        cerr << "cannot write inverted_index.txt\n";
        return 1;
    }

    // Serialize format: termID df doc:pos,pos;doc:pos...
    for (auto &kv : inv) {
        TermID tid = kv.first;
        auto &plist = kv.second;

        fout << tid << " " << plist.size() << " ";
        for (size_t i = 0; i < plist.size(); i++) {
            fout << plist[i].doc_id << ":";  
            for (size_t j = 0; j < plist[i].pos.size(); j++) {
                fout << plist[i].pos[j];
                if (j + 1 < plist[i].pos.size()) fout << ",";
            }
            if (i + 1 < plist.size()) fout << ";";
        }
        fout << "\n";
    }

    return 0;
}
