#pragma once

#include <string>
#include "third_party/nlohmann/json.hpp"

namespace cord19 {

using json = nlohmann::json;

// Forward declarations
struct Engine;
struct AzureOpenAIConfig;
class StatsTracker;

// Generate an AI summary of a document abstract using Azure OpenAI with caching
// Takes the cord_uid and returns an AI-generated summary of the abstract
// Uses Engine's AI cache to save on API costs (24hr expiry, LRU eviction)
// is_authorized: if true, counter won't decrement; if false, counter decrements
json generate_ai_summary(const AzureOpenAIConfig& config, 
                         const std::string& cord_uid,
                         Engine* engine,
                         StatsTracker* stats = nullptr,
                         bool is_authorized = false);

} // namespace cord19
