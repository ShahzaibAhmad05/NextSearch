// src/App.tsx
import React, { useState } from "react";
import "./App.css";
import SearchBar from "./components/SearchBar";
import SearchResults from "./components/SearchResults";
import { search as apiSearch } from "./api";
import type { SearchResult } from "./types";
import { addDocument as apiAddDocument } from "./api";

export default function App() {
  const [query, setQuery] = useState("");
  const [k, setK] = useState(25);
  const [loading, setLoading] = useState(false);
  const [results, setResults] = useState<SearchResult[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [backendSearchMs, setBackendSearchMs] = useState<number | null>(null);
  const [backendTotalMs, setBackendTotalMs] = useState<number | null>(null);

  async function onSubmit() {
    setError(null);
    setLoading(true);
    try {
      const data = await apiSearch(query, k);
      setResults(data.results);

      setBackendSearchMs(data.search_time_ms ?? null);
      setBackendTotalMs(data.total_time_ms ?? null);
    } catch (e: any) {
      setError(e?.message ?? String(e));
      setResults([]);
    } finally {
      setLoading(false);
    }
  }

  return (
    <div className="page">
      <div className="container">
        <header className="header">
          <div className="brand">NextSearch</div>
          <div className="sub">A Scalable Search Engine</div>
        </header>

        <button
          onClick={async () => {
            const cord_root = prompt("CORD_ROOT (absolute path on server)?") ?? "";
            const json_relpath = prompt("JSON_REL_PATH (relative path)?") ?? "";
            const cord_uid = prompt("CORD_UID?") ?? "";
            const title = prompt("TITLE?") ?? "";
            if (!cord_root || !json_relpath || !cord_uid || !title) return;

            try {
              const out = await apiAddDocument({ cord_root, json_relpath, cord_uid, title });
              alert(`Added: ${out.segment} (reloaded=${out.reloaded}) time=${out.total_time_ms?.toFixed?.(2)}ms`);
            } catch (e: any) {
              alert(e?.message ?? String(e));
            }
          }}
          style={{
            padding: "10px 14px",
            borderRadius: 10,
            border: "1px solid #111",
            background: "#fff",
            cursor: "pointer"
          }}
        >
          Add Document
        </button>

        <br />
        <br />

        <SearchBar
          query={query}
          k={k}
          loading={loading}
          onChangeQuery={setQuery}
          onChangeK={setK}
          onSubmit={onSubmit}
        />

        {(backendSearchMs != null || backendTotalMs != null) && (
          <div style={{ marginTop: 10, color: "#555", fontSize: 13 }}>
            {backendSearchMs != null && <>Search: {backendSearchMs.toFixed(2)} ms</>}
            {backendTotalMs != null && <> • Backend total: {backendTotalMs.toFixed(2)} ms</>}
          </div>
        )}

        {error ? (
          <div className="error">
            {error}
            <div className="hint">
              Make sure backend is running: <code>./api_server &lt;INDEX_DIR&gt; 8080</code>
            </div>
          </div>
        ) : null}

        <SearchResults results={results} />
      </div>
    </div>
  );
}
