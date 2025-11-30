#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cctype>
#include <cstdint>

using namespace std;
using DocID = string;          // Use cord_uid string as document ID
using TermID = uint32_t;       // Numeric term ID as before

struct TermOcc {
    TermID tid;
    vector<uint32_t> pos;
};

// Lexicon mapping term string → term ID, and forward index mapping docID(cord_uid) → list of term occurrences
unordered_map<string, TermID> term_to_id;
unordered_map<DocID, vector<TermOcc>> forward_index;

// CSV parser that handles quoted fields and commas inside quotes
vector<string> parse_csv(const string& line) {
    vector<string> cols;
    string cur;
    bool inq = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') {
            if (inq && i + 1 < line.size() && line[i + 1] == '"') {
                cur.push_back('"');
                i++;
            } else inq = !inq;
        }
        else if (c == ',' && !inq) {
            cols.push_back(cur);
            cur.clear();
        }
        else cur.push_back(c);
    }
    cols.push_back(cur);
    return cols;
}

// Tokenizer that lowercases text and keeps only alphabetic characters and spaces
vector<string> tokenize(const string& s) {
    string cleaned;
    cleaned.reserve(s.size());
    for (char c : s) {
        if (isalpha((unsigned char)c) || isspace((unsigned char)c))
            cleaned.push_back(tolower((unsigned char)c));
        else
            cleaned.push_back(' ');
    }
    stringstream ss(cleaned);
    string tok;
    vector<string> out;
    while (ss >> tok) out.push_back(tok);
    return out;
}

// Main: load lexicon, read metadata, build forward index keyed by cord_uid, and write forward_index.txt
int main() {
    // Load lexicon.txt into term_to_id map
    {
        ifstream lx("../sampleFiles/lexicon.txt");
        if (!lx.is_open()) {
            cerr << "lexicon.txt not found\n";
            return 1;
        }
        string term;
        TermID tid;
        uint32_t df;
        while (lx >> term >> tid >> df)
            term_to_id[term] = tid;
    }

    // Open metadata.csv for reading
    ifstream fin("../sampleFiles/metadata.csv");
    if (!fin.is_open()) {
        cerr << "metadata.csv not found\n";
        return 1;
    }

    // Read and parse header to locate column indices (title, authors, abstract, cord_uid)
    string header;
    getline(fin, header);
    auto head = parse_csv(header);

    int title_col = -1, authors_col = -1, abs_col = -1, cord_col = -1;

    for (size_t i = 0; i < head.size(); i++) {
        string h = head[i];
        for (char &c : h) c = tolower(c);
        if (h == "title")    title_col = (int)i;
        if (h == "authors")  authors_col = (int)i;
        if (h == "abstract") abs_col = (int)i;
        if (h == "cord_uid") cord_col = (int)i;
    }

    // Ensure required columns exist (title, abstract, cord_uid)
    if (title_col == -1 || abs_col == -1 || cord_col == -1) {
        cerr << "title/abstract/cord_uid column missing\n";
        return 1;
    }

    // Process each data row and build per-document term positions
    string line;
    while (getline(fin, line)) {
        auto cols = parse_csv(line);

        // Basic safety: ensure we have at least up to cord_uid, title, abstract
        if (cols.size() <= (size_t)abs_col ||
            cols.size() <= (size_t)title_col ||
            cols.size() <= (size_t)cord_col)
            continue;

        string cord_uid = cols[cord_col];
        if (cord_uid.empty()) continue;

        string title    = cols[title_col];
        string authors  = (authors_col != -1 && cols.size() > (size_t)authors_col ? cols[authors_col] : "");
        string abstract = cols[abs_col];

        string text = title + " " + authors + " " + abstract;
        auto tokens = tokenize(text);

        unordered_map<TermID, vector<uint32_t>> mp;  // Map termID → positions in this doc

        for (uint32_t i = 0; i < tokens.size(); i++) {
            auto it = term_to_id.find(tokens[i]);
            if (it != term_to_id.end())
                mp[it->second].push_back(i);
        }

        vector<TermOcc> v;
        v.reserve(mp.size());
        for (auto &p : mp)
            v.push_back({ p.first, p.second });

        forward_index[cord_uid] = v;  // Use cord_uid as the document ID key
    }

    // Write forward_index.txt using cord_uid as document identifier
    ofstream fout("../sampleFiles/forward_index.txt");
    if (!fout.is_open()) {
        cerr << "cannot write forward_index.txt\n";
        return 1;
    }

    for (auto &entry : forward_index) {
        const DocID &doc_id = entry.first;        
        auto &terms = entry.second;
        if (terms.empty()) continue;

        fout << doc_id << " " << terms.size() << " ";
        for (size_t i = 0; i < terms.size(); i++) {
            fout << terms[i].tid << ":";
            for (size_t j = 0; j < terms[i].pos.size(); j++) {
                fout << terms[i].pos[j];
                if (j + 1 < terms[i].pos.size()) fout << ",";
            }
            if (i + 1 < terms.size()) fout << ";";
        }
        fout << "\n";
    }

    return 0;
}
