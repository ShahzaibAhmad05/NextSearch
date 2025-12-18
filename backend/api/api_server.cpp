#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "api_add_document.hpp"
#include "api_engine.hpp"
#include "api_http.hpp"
#include "third_party/httplib.h"

using cord19::Engine;
using cord19::json;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: api_server <INDEX_DIR> [port]\n"
                  << "Example: api_server ./index 8080\n";
        return 1;
    }

    Engine engine;
    engine.index_dir = std::filesystem::path(argv[1]);

    int port = 8080;
    if (argc >= 3) port = std::stoi(argv[2]);

    if (!engine.reload()) {
        std::cerr << "Failed to load index segments from: " << engine.index_dir << "\n";
        return 1;
    }

    httplib::Server svr;

    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        std::cerr << "[http] " << req.method << " " << req.path << " -> " << res.status << "\n";
    });

    svr.set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        try { if (ep) std::rethrow_exception(ep); }
        catch (const std::exception& e) {
            std::cerr << "[exception] " << req.method << " " << req.path << " : " << e.what() << "\n";
        }
        res.status = 500;
        res.set_content(R"({"error":"internal server error"})", "application/json");
    });

    svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        std::cerr << "[error] " << req.method << " " << req.path << " -> " << res.status << "\n";
    });

    // CORS preflight handler (OPTIONS) for all routes
    svr.Options(R"(.*)", [](const httplib::Request& req, httplib::Response& res) {
        cord19::enable_cors(res);

        // Echo requested headers if present (some browsers send a wider set than our default).
        if (req.has_header("Access-Control-Request-Headers")) {
            res.set_header("Access-Control-Allow-Headers",
                           req.get_header_value("Access-Control-Request-Headers"));
        }

        // Echo requested method if present (defensive; we already allow GET/POST/OPTIONS).
        if (req.has_header("Access-Control-Request-Method")) {
            res.set_header("Access-Control-Allow-Methods",
                           req.get_header_value("Access-Control-Request-Method") + std::string(", OPTIONS"));
        }

        res.status = 204;
    });

    svr.Get("/health", [&](const httplib::Request&, httplib::Response& res) {
        cord19::enable_cors(res);
        json j;
        j["ok"] = true;
        j["segments"] = (int)engine.segments.size();
        res.set_content(j.dump(2), "application/json");
    });

    svr.Get("/search", [&](const httplib::Request& req, httplib::Response& res) {
        cord19::enable_cors(res);

        using clock = std::chrono::steady_clock;
        auto total_t0 = clock::now();

        if (!req.has_param("q")) {
            res.status = 400;
            res.set_content(R"({"error":"missing q param"})", "application/json");
            return;
        }
        std::string q = req.get_param_value("q");
        int k = 10;
        if (req.has_param("k")) k = std::stoi(req.get_param_value("k"));

        auto search_t0 = clock::now();
        auto j = engine.search(q, k);
        auto search_t1 = clock::now();

        double search_ms = std::chrono::duration<double, std::milli>(search_t1 - search_t0).count();
        j["search_time_ms"] = search_ms;

        auto total_t1 = clock::now();
        double total_ms = std::chrono::duration<double, std::milli>(total_t1 - total_t0).count();
        j["total_time_ms"] = total_ms;

        std::cerr << "[search] q=\"" << q << "\" k=" << k << " search=" << search_ms << "ms total="
                  << total_ms << "ms\n";

        res.set_content(j.dump(2), "application/json");
    });

    svr.Get("/suggest", [&](const httplib::Request& req, httplib::Response& res) {
        cord19::enable_cors(res);

        if (!req.has_param("q")) {
            res.status = 400;
            res.set_content(R"({\"error\":\"missing q param\"})", "application/json");
            return;
        }
        std::string q = req.get_param_value("q");

        int k = 5;
        if (req.has_param("k")) k = std::stoi(req.get_param_value("k"));

        auto j = engine.suggest(q, k);
        res.set_content(j.dump(2), "application/json");
    });

    svr.Post("/add_document",
             [&](const httplib::Request& req, httplib::Response& res, const httplib::ContentReader& cr) {
                 cord19::handle_add_document(engine, req, res, cr);
             });

    svr.Post("/reload", [&](const httplib::Request&, httplib::Response& res) {
        cord19::enable_cors(res);
        bool ok = engine.reload();
        json j;
        j["reloaded"] = ok;
        j["segments"] = (int)engine.segments.size();
        res.set_content(j.dump(2), "application/json");
    });

    std::cout << "API running on http://127.0.0.1:" << port << "\n";
    std::cout << "Try: /search?q=mycoplasma+pneumonia&k=10\n";
    svr.listen("0.0.0.0", port);
    return 0;
}