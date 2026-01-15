#pragma once

#include "api_engine.hpp"
#include "third_party/httplib.h"

namespace cord19 {

// /add_document: Upload a CORD-19 "cord_slice" zip and build a new segment.
// Expects multipart/form-data with a single file field named "cord_slice" (a .zip).
// The zip should contain (either at root or inside a single top-level folder):
//   document_parses/{pdf_json,pmc_json}/
//   metadata.csv
// plus other CORD-19 slice artifacts.
//
// Index construction is done without holding the engine mutex, so /search remains usable.
// A brief engine.reload() happens at the end to make the new segment visible.
void handle_add_document(Engine& engine,
                         const httplib::Request& req,
                         httplib::Response& res,
                         const httplib::ContentReader& content_reader);

} // namespace cord19