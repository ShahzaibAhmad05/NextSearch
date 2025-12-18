#include "api_http.hpp"

namespace cord19 {

void enable_cors(httplib::Response& res) {
    // Keep this permissive for local dev. If you deploy publicly, scope Allow-Origin.
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");

    // Browsers may preflight multipart/form-data POSTs and ask for multiple headers.
    // Being explicit here avoids "Failed to fetch" caused by CORS preflight rejection.
    res.set_header("Access-Control-Allow-Headers",
                   "Content-Type, Accept, Origin, X-Requested-With, Authorization");

    // Helps the browser cache preflight results.
    res.set_header("Access-Control-Max-Age", "600");
}

} // namespace cord19