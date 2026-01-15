#include "api_ai_overview.hpp"
#include "third_party/httplib.h"
#include <iostream>
#include <sstream>

namespace cord19 {

// Helper function to construct the system prompt for AI overview generation
static std::string build_system_prompt() {
    return R"(You are an AI assistant that generates concise, informative overviews of search results.
Your task is to analyze the provided search results and create a comprehensive summary that:
1. Answers the user's query directly
2. Synthesizes information from multiple sources
3. Highlights key findings and relevant details
4. Maintains accuracy and avoids speculation
5. Cites specific documents when appropriate

Keep your overview clear, factual, and helpful.)";
}

// Helper function to build the user prompt with search results
static std::string build_user_prompt(const std::string& query, const json& search_results) {
    std::ostringstream oss;
    oss << "User Query: " << query << "\n\n";
    oss << "Search Results:\n\n";
    
    // Extract and format the search results
    if (search_results.contains("results") && search_results["results"].is_array()) {
        const auto& results = search_results["results"];
        int rank = 1;
        
        for (const auto& result : results) {
            oss << "Document " << rank << ":\n";
            
            if (result.contains("title")) {
                oss << "Title: " << result["title"].get<std::string>() << "\n";
            }
            
            if (result.contains("cord_uid")) {
                oss << "ID: " << result["cord_uid"].get<std::string>() << "\n";
            }
            
            if (result.contains("bm25_score")) {
                oss << "Relevance Score: " << result["bm25_score"].get<double>() << "\n";
            }
            
            if (result.contains("url")) {
                oss << "URL: " << result["url"].get<std::string>() << "\n";
            }
            
            if (result.contains("author")) {
                oss << "Author: " << result["author"].get<std::string>() << "\n";
            }
            
            if (result.contains("publish_time")) {
                oss << "Published: " << result["publish_time"].get<std::string>() << "\n";
            }
            
            oss << "\n";
            rank++;
        }
    }
    
    oss << "Please provide a comprehensive AI overview based on these search results.";
    return oss.str();
}

json generate_ai_overview(const AzureOpenAIConfig& config,
                          const std::string& query,
                          const json& search_results) {
    json response_json;
    
    try {
        // Validate endpoint
        std::string endpoint = config.endpoint;
        
        // Remove trailing slash if present
        if (endpoint.back() == '/') {
            endpoint.pop_back();
        }
        
        // Build the API path
        // Format: /openai/deployments/{model}/chat/completions?api-version={version}
        std::string path = "/openai/deployments/" + config.model + 
                          "/chat/completions?api-version=" + config.api_version;
        
        // Create HTTPS client
        httplib::Client client(endpoint);
        client.set_connection_timeout(30, 0); // 30 seconds
        client.set_read_timeout(60, 0);       // 60 seconds
        
        // Build the request body
        json request_body;
        request_body["messages"] = json::array();
        
        // Add system message
        json system_msg;
        system_msg["role"] = "system";
        system_msg["content"] = build_system_prompt();
        request_body["messages"].push_back(system_msg);
        
        // Add user message with query and results
        json user_msg;
        user_msg["role"] = "user";
        user_msg["content"] = build_user_prompt(query, search_results);
        request_body["messages"].push_back(user_msg);
        
        // Set parameters
        request_body["temperature"] = 0.7;
        request_body["max_tokens"] = 1000;
        request_body["top_p"] = 0.95;
        request_body["frequency_penalty"] = 0;
        request_body["presence_penalty"] = 0;
        
        std::string body_str = request_body.dump();
        
        // Set headers
        httplib::Headers headers = {
            {"Content-Type", "application/json"},
            {"api-key", config.api_key}
        };
        
        std::cerr << "[azure_openai] Calling Azure OpenAI at " << endpoint << path << "\n";
        
        // Make the POST request
        auto res = client.Post(path, headers, body_str, "application/json");
        
        if (!res) {
            response_json["error"] = "Failed to connect to Azure OpenAI";
            response_json["success"] = false;
            std::cerr << "[azure_openai] Connection failed\n";
            return response_json;
        }
        
        if (res->status != 200) {
            response_json["error"] = "Azure OpenAI API error";
            response_json["status_code"] = res->status;
            response_json["details"] = res->body;
            response_json["success"] = false;
            std::cerr << "[azure_openai] API error: " << res->status << " - " << res->body << "\n";
            return response_json;
        }
        
        // Parse the response
        json api_response = json::parse(res->body);
        
        // Extract the AI overview from the response
        if (api_response.contains("choices") && 
            api_response["choices"].is_array() && 
            !api_response["choices"].empty()) {
            
            const auto& choice = api_response["choices"][0];
            if (choice.contains("message") && 
                choice["message"].contains("content")) {
                
                response_json["success"] = true;
                response_json["overview"] = choice["message"]["content"];
                response_json["model"] = config.model;
                
                // Include token usage if available
                if (api_response.contains("usage")) {
                    response_json["usage"] = api_response["usage"];
                }
                
                std::cerr << "[azure_openai] Successfully generated AI overview\n";
            } else {
                response_json["error"] = "Unexpected response structure";
                response_json["success"] = false;
            }
        } else {
            response_json["error"] = "No choices in response";
            response_json["success"] = false;
        }
        
    } catch (const std::exception& e) {
        response_json["error"] = std::string("Exception: ") + e.what();
        response_json["success"] = false;
        std::cerr << "[azure_openai] Exception: " << e.what() << "\n";
    }
    
    return response_json;
}

} // namespace cord19
