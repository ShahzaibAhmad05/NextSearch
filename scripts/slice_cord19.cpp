/*
 * Script used to slice a subset from the HUGE cord19 dataset 
 * 
 * Example run:
 * slice_cord19.exe --in_root D:\cord19 --out_root D:\cord19_sliced --n 5000 --prefer either --require_body
 */

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Args {
    std::string in_root;
    std::string out_root;
    int n = 2000;
    int seed = 1337;
    std::string prefer = "either"; // "pmc", "pdf", "either"
    bool require_body = false;
};

// Parse command line arguments
Args parse_args(int argc, char* argv[]) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--in_root" && i + 1 < argc) {
            args.in_root = argv[++i];
        } else if (arg == "--out_root" && i + 1 < argc) {
            args.out_root = argv[++i];
        } else if (arg == "--n" && i + 1 < argc) {
            args.n = std::stoi(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            args.seed = std::stoi(argv[++i]);
        } else if (arg == "--prefer" && i + 1 < argc) {
            args.prefer = argv[++i];
            if (args.prefer != "pmc" && args.prefer != "pdf" && args.prefer != "either") {
                std::cerr << "Error: --prefer must be one of: pmc, pdf, either" << std::endl;
                std::exit(1);
            }
        } else if (arg == "--require_body") {
            args.require_body = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: slice_cord19 --in_root <path> --out_root <path> [options]\n"
                      << "Options:\n"
                      << "  --in_root <path>    Input directory (e.g., D:\\cord19)\n"
                      << "  --out_root <path>   Output directory (e.g., D:\\cord19_sliced)\n"
                      << "  --n <num>           Number of rows/docs to keep (default: 2000)\n"
                      << "  --seed <num>        Random seed (default: 1337)\n"
                      << "  --prefer <choice>   Prefer pmc, pdf, or either (default: either)\n"
                      << "  --require_body      Only keep docs with at least one json file\n";
            std::exit(0);
        }
    }
    
    if (args.in_root.empty() || args.out_root.empty()) {
        std::cerr << "Error: --in_root and --out_root are required" << std::endl;
        std::exit(1);
    }
    
    return args;
}

// Parse semicolon-separated paths
std::vector<std::string> parse_semicolon_paths(const std::string& s) {
    std::vector<std::string> result;
    if (s.empty()) {
        return result;
    }
    
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, ';')) {
        // Trim whitespace
        part.erase(0, part.find_first_not_of(" \t\r\n"));
        part.erase(part.find_last_not_of(" \t\r\n") + 1);
        if (!part.empty()) {
            result.push_back(part);
        }
    }
    return result;
}

// Safe copy with directory creation
void safe_copy(const fs::path& src, const fs::path& dst) {
    fs::create_directories(dst.parent_path());
    if (!fs::exists(dst)) {
        fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
    }
}

// Parse CSV line respecting quoted fields
std::vector<std::string> parse_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    
    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (c == '"') {
            if (in_quotes && i + 1 < line.length() && line[i + 1] == '"') {
                // Escaped quote
                field += '"';
                ++i;
            } else {
                // Toggle quote state
                in_quotes = !in_quotes;
            }
        } else if (c == ',' && !in_quotes) {
            // End of field
            fields.push_back(field);
            field.clear();
        } else {
            field += c;
        }
    }
    fields.push_back(field); // Last field
    return fields;
}

// Write CSV field (with quoting if necessary)
std::string csv_escape(const std::string& s) {
    if (s.find(',') != std::string::npos || 
        s.find('"') != std::string::npos || 
        s.find('\n') != std::string::npos) {
        std::string escaped = "\"";
        for (char c : s) {
            if (c == '"') {
                escaped += "\"\"";
            } else {
                escaped += c;
            }
        }
        escaped += "\"";
        return escaped;
    }
    return s;
}

int main(int argc, char* argv[]) {
    Args args = parse_args(argc, argv);
    
    fs::path in_root(args.in_root);
    fs::path out_root(args.out_root);
    fs::path in_meta = in_root / "metadata.csv";
    fs::path out_meta = out_root / "metadata.csv";
    
    if (!fs::exists(in_meta)) {
        std::cerr << "Error: metadata.csv not found at: " << in_meta << std::endl;
        return 1;
    }
    
    fs::create_directories(out_root);
    
    // Read all metadata rows
    std::ifstream infile(in_meta);
    if (!infile) {
        std::cerr << "Error: Could not open " << in_meta << std::endl;
        return 1;
    }
    
    std::string header_line;
    std::getline(infile, header_line);
    std::vector<std::string> fieldnames = parse_csv_line(header_line);
    
    // Find column indices
    int pdf_json_idx = -1;
    int pmc_json_idx = -1;
    for (size_t i = 0; i < fieldnames.size(); ++i) {
        if (fieldnames[i] == "pdf_json_files") pdf_json_idx = i;
        if (fieldnames[i] == "pmc_json_files") pmc_json_idx = i;
    }
    
    // Read all rows
    std::vector<std::vector<std::string>> rows;
    std::string line;
    while (std::getline(infile, line)) {
        if (!line.empty()) {
            rows.push_back(parse_csv_line(line));
        }
    }
    infile.close();
    
    // Shuffle rows
    std::mt19937 rng(args.seed);
    std::shuffle(rows.begin(), rows.end(), rng);
    
    // Process rows
    std::vector<std::vector<std::string>> kept;
    int copied_files = 0;
    
    for (const auto& row : rows) {
        std::vector<std::string> pdfs;
        std::vector<std::string> pmcs;
        
        if (pdf_json_idx >= 0 && pdf_json_idx < (int)row.size()) {
            pdfs = parse_semicolon_paths(row[pdf_json_idx]);
        }
        if (pmc_json_idx >= 0 && pmc_json_idx < (int)row.size()) {
            pmcs = parse_semicolon_paths(row[pmc_json_idx]);
        }
        
        // Check if we should skip this row
        if (args.require_body && pdfs.empty() && pmcs.empty()) {
            continue;
        }
        
        // Check preference
        if (args.prefer == "pmc" && pmcs.empty()) {
            continue;
        }
        if (args.prefer == "pdf" && pdfs.empty()) {
            continue;
        }
        
        // Copy referenced json files
        std::vector<std::string> all_files;
        all_files.insert(all_files.end(), pdfs.begin(), pdfs.end());
        all_files.insert(all_files.end(), pmcs.begin(), pmcs.end());
        
        for (const auto& rel : all_files) {
            fs::path src = in_root / rel;
            if (fs::exists(src)) {
                fs::path dst = out_root / rel;
                try {
                    safe_copy(src, dst);
                    copied_files++;
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Failed to copy " << src << ": " << e.what() << std::endl;
                }
            }
        }
        
        kept.push_back(row);
        if ((int)kept.size() >= args.n) {
            break;
        }
    }
    
    if (kept.empty()) {
        std::cerr << "Error: No rows were kept. Try removing --require_body or changing --prefer." << std::endl;
        return 1;
    }
    
    // Write sliced metadata.csv
    std::ofstream outfile(out_meta);
    if (!outfile) {
        std::cerr << "Error: Could not create " << out_meta << std::endl;
        return 1;
    }
    
    // Write header
    for (size_t i = 0; i < fieldnames.size(); ++i) {
        if (i > 0) outfile << ",";
        outfile << csv_escape(fieldnames[i]);
    }
    outfile << "\n";
    
    // Write rows
    for (const auto& row : kept) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) outfile << ",";
            outfile << csv_escape(row[i]);
        }
        outfile << "\n";
    }
    outfile.close();
    
    // Optionally copy extra files
    std::vector<std::string> extras = {
        "metadata.readme", 
        "json_schema.txt", 
        "COVID.DATA.LIC.AGMT.pdf"
    };
    
    for (const auto& extra : extras) {
        fs::path src = in_root / extra;
        if (fs::exists(src)) {
            try {
                safe_copy(src, out_root / extra);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to copy " << extra << ": " << e.what() << std::endl;
            }
        }
    }
    
    std::cout << "Kept rows: " << kept.size() << std::endl;
    std::cout << "Copied JSON files: " << copied_files << std::endl;
    std::cout << "Output: " << out_root << std::endl;
    
    return 0;
}
