#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace cord19 {

namespace fs = std::filesystem;

// Lightweight word-embedding index used for semantic query expansion.
//
// This is intentionally NOT transformer/LLM-based. It expects classic static
// embeddings (Word2Vec / GloVe / FastText-exported .vec/.txt). The semantic
// matching logic (cosine similarity + weighted expansion) is implemented here.
struct SemanticIndex {
    bool enabled = false;
    int dim = 0;

    // Row-major normalized vectors (L2=1), aligned with `terms`.
    std::vector<std::string> terms;              // row -> term
    std::vector<float> vecs;                    // row*dim + j
    std::unordered_map<std::string, uint32_t> term_to_row;

    // Load vectors from a text embedding file with lines:
    //   word v1 v2 ... vD
    // Supports optional header line: "<vocab> <dim>".
    //
    // To keep memory low, this loads vectors ONLY for `needed_terms`.
    bool load_from_text(const fs::path& path,
                        const std::unordered_set<std::string>& needed_terms);

    // Expand query tokens by nearest neighbors in embedding space.
    // Returns (term, weight) pairs. Original query terms always have weight 1.0.
    // Neighbors get weight ~= alpha * cosine_sim.
    std::vector<std::pair<std::string, float>> expand(
        const std::vector<std::string>& query_terms,
        int per_term = 3,
        int global_topk = 5,
        float min_sim = 0.55f,
        float alpha = 0.6f,
        int max_total_terms = 40) const;

private:
    const float* get_vec_ptr(const std::string& term) const;
    static void l2_normalize(std::vector<float>& v);

    // Returns top-k (row, sim) for a provided normalized query vector.
    std::vector<std::pair<uint32_t, float>> most_similar_to_vec(
        const float* qvec,
        int topk,
        float min_sim,
        const std::unordered_set<uint32_t>* banned_rows = nullptr) const;
};

} // namespace cord19
