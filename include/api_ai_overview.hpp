#pragma once

#include <string>
#include "third_party/nlohmann/json.hpp"

namespace cord19 {

using json = nlohmann::json;

// Configuration for Azure OpenAI service
struct AzureOpenAIConfig {
    std::string endpoint;      // e.g., "https://your-resource.openai.azure.com"
    std::string api_key;
    std::string model;         // e.g., "gpt-5.2-chat"
    std::string api_version = "2024-02-15-preview"; // Azure OpenAI API version
};

// Generate an AI overview of search results using Azure OpenAI
// Takes the search results JSON and returns an AI-generated overview
json generate_ai_overview(const AzureOpenAIConfig& config, 
                          const std::string& query,
                          const json& search_results);

} // namespace cord19
