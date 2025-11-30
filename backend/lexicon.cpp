#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cctype>
#include <cstdint>

using namespace std;
using DocID = uint32_t;
using TermID = uint32_t;

// Lexicon entry storing term ID and document frequency
struct LexiconEntry { TermID term_id; uint32_t doc_freq = 0; };

unordered_map<string, LexiconEntry> lexicon;   // Global lexicon map
TermID next_term_id = 1;                      // Next available term ID

// Tokenizes input string into lowercase alphabetic tokens
vector<string> tokenize(const string& s) {
    string cleaned; cleaned.reserve(s.size());
    for (char c : s) cleaned.push_back((isalpha((unsigned char)c) || isspace((unsigned char)c)) ? tolower(c) : ' ');
    stringstream ss(cleaned);
    string tok; vector<string> out;
    while (ss >> tok) out.push_back(tok);
    return out;
}

// Parses CSV line with quoted field support
vector<string> parse_csv(const string& line) {
    vector<string> cols; string cur; bool inq = false;
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];
        if (c == '"') { if (inq && i+1 < line.size() && line[i+1]=='"') cur.push_back('"'), i++; else inq=!inq; }
        else if (c == ',' && !inq) cols.push_back(cur), cur.clear();
        else cur.push_back(c);
    }
    cols.push_back(cur); return cols;
}

// Indexes a document by counting unique tokens
void index_doc(DocID id, const string& text) {
    (void)id; auto tokens = tokenize(text);
    unordered_set<string> seen;
    for (auto& t : tokens) {
        if (seen.count(t)) continue;
        auto it = lexicon.find(t);
        if (it == lexicon.end()) lexicon[t] = { next_term_id++, 1 };
        else it->second.doc_freq++;
        seen.insert(t);
    }
}

// Main: reads CSV, extracts fields, indexes docs, writes lexicon
int main() {
    ifstream fin("../sampleFiles/metadata.csv");
    if (!fin.is_open()) return cerr << "metadata.csv not found\n", 1;

    string header;
    if (!getline(fin, header)) return cerr << "empty metadata.csv\n", 1;

    auto head = parse_csv(header);
    int title_col = -1, authors_col = -1, abs_col = -1;

    // Detect required column positions
    for (size_t i = 0; i < head.size(); i++) {
        string h = head[i]; for (auto &ch : h) ch = tolower(ch);
        if (h == "title") title_col = i;
        if (h == "authors") authors_col = i;
        if (h == "abstract") abs_col = i;
    }

    if (title_col == -1 && abs_col == -1) return cerr << "no title or abstract column found\n", 1;

    DocID doc_id = 1;
    string line;

    // Read CSV rows and index documents
    while (getline(fin, line)) {
        if (line.empty()) continue;
        auto cols = parse_csv(line);

        int max_needed = max(title_col, abs_col);
        if (authors_col != -1) max_needed = max(max_needed, authors_col);
        if ((int)cols.size() <= max_needed) continue;

        string title = (title_col != -1 ? cols[title_col] : "");
        string authors = (authors_col != -1 ? cols[authors_col] : "");
        string abstract = (abs_col != -1 ? cols[abs_col] : "");

        if (title.empty() && abstract.empty()) { doc_id++; continue; }

        string text = title + " " + authors + " " + abstract;
        index_doc(doc_id, text);
        doc_id++;
    }

    // Write final lexicon to file
    ofstream fout("../sampleFiles/lexicon.txt");
    if (!fout.is_open()) return cerr << "cannot write lexicon.txt\n", 1;

    for (auto &p : lexicon)
        fout << p.first << " " << p.second.term_id << " " << p.second.doc_freq << "\n";

    return 0;
}
