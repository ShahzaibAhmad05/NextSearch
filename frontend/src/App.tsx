// src/App.tsx
import React, { useCallback, useEffect, useMemo, useRef, useState } from "react";
import SearchBar from "./components/SearchBar";
import SearchResults from "./components/SearchResults";
import AddDocumentModal from "./components/AddDocumentModal";
import { search as apiSearch } from "./api";
import type { SearchResult } from "./types";

type SortBy =
  | "Relevancy"
  | "Publish Date (Newest)"
  | "Publish Date (Oldest)";

function publishTimeToMs(iso?: string) {
  if (!iso) return NaN;
  const t = Date.parse(iso);
  return Number.isNaN(t) ? NaN : t;
}

function clampK(v: number) {
  // Keep sane bounds; backend can still accept larger values if you raise this.
  const min = 1;
  const max = 1000;
  if (Number.isNaN(v)) return 100;
  return Math.min(max, Math.max(min, Math.trunc(v)));
}

export default function App() {
  const [query, setQuery] = useState("");
  const [k, setK] = useState(100);
  const [loading, setLoading] = useState(false);

  const [showAdvanced, setShowAdvanced] = useState(false);

  const [results, setResults] = useState<SearchResult[]>([]);
  const [error, setError] = useState<string | null>(null);
  const [found, setFound] = useState<number | undefined>();

  const [backendTotalMs, setBackendTotalMs] = useState<number | null>(null);
  const [hasSearched, setHasSearched] = useState(false);

  const [showAddModal, setShowAddModal] = useState(false);

  const [sortBy, setSortBy] = useState<SortBy>("Relevancy");
  const [showSort, setShowSort] = useState(false);

  const advancedWrapRef = useRef<HTMLDivElement | null>(null);
  const prevKRef = useRef(k);

  const onSubmit = useCallback(async () => {
    if (!query.trim()) return;

    setError(null);
    setLoading(true);

    try {
      const data = await apiSearch(query, k);
      setResults(data.results);
      setFound(data.found);
      setHasSearched(true);
      setBackendTotalMs(data.total_time_ms ?? null);
    } catch (e: any) {
      setError(e?.message ?? String(e));
      setResults([]);
      setHasSearched(true);
      setBackendTotalMs(null);
    } finally {
      setLoading(false);
    }
  }, [query, k]);

  // Close Advanced popover when clicking outside / pressing Escape
  useEffect(() => {
    if (!showAdvanced) return;

    function onDocMouseDown(e: MouseEvent) {
      const wrap = advancedWrapRef.current;
      if (!wrap) return;
      if (!wrap.contains(e.target as Node)) setShowAdvanced(false);
    }

    function onKeyDown(e: KeyboardEvent) {
      if (e.key === "Escape") setShowAdvanced(false);
    }

    document.addEventListener("mousedown", onDocMouseDown);
    document.addEventListener("keydown", onKeyDown);
    return () => {
      document.removeEventListener("mousedown", onDocMouseDown);
      document.removeEventListener("keydown", onKeyDown);
    };
  }, [showAdvanced]);

  // When k changes after a search has happened, automatically re-run the search (debounced).
  useEffect(() => {
    if (!hasSearched) {
      prevKRef.current = k;
      return;
    }
    if (prevKRef.current === k) return;

    prevKRef.current = k;
    if (!query.trim()) return;

    const t = window.setTimeout(() => {
      if (!loading) onSubmit();
    }, 350);

    return () => window.clearTimeout(t);
  }, [k, hasSearched, query, loading, onSubmit]);

  const status = useMemo(() => {
    if (!hasSearched) return "";
    if (loading) return "Searching…";
    if (error) return "Error fetching results";
    if (results.length === 0) return "No results found";

    const n = found ?? results.length;
    const parts: string[] = [`About ${n} result${n === 1 ? "" : "s"}`];

    if (backendTotalMs != null) parts.push(`(${backendTotalMs.toFixed(2)} ms)`);
    return parts.join(" ");
  }, [hasSearched, loading, error, results.length, backendTotalMs, found]);

  const sortedResults = useMemo(() => {
    const copy = [...results];

    if (sortBy !== "Relevancy") {
      copy.sort((a, b) => {
        const ta = publishTimeToMs(a.publish_time);
        const tb = publishTimeToMs(b.publish_time);

        const aBad = Number.isNaN(ta);
        const bBad = Number.isNaN(tb);
        if (aBad && bBad) return 0;
        if (aBad) return 1;
        if (bBad) return -1;

        return sortBy === "Publish Date (Newest)" ? tb - ta : ta - tb;
      });
    }

    return copy;
  }, [results, sortBy]);

  return (
    <div className="bg-light min-vh-100">
      {/* Top bar */}
      <nav className="navbar navbar-expand-lg bg-white border-bottom fixed-top">
        <div className="container" style={{ maxWidth: 980 }}>
          <a className="navbar-brand fw-bold" href="#">
          </a>
          <div className="d-flex gap-2">
            <button
              className="btn btn-outline-dark"
              onClick={() => setShowAddModal(true)}
            >
              Add Document
            </button>
          </div>
        </div>
      </nav>

      {/* ======================
          PRE-SEARCH (CENTERED)
          ====================== */}
      {!hasSearched && (
        <div
          className="d-flex align-items-center justify-content-center"
          style={{ minHeight: "100vh" }}
        >
          <div className="container" style={{ maxWidth: 1000 }}>
            <div className="text-center mb-4">
              <h1 className="display-4 fw-bold mb-2">NextSearch</h1>
              <div className="text-secondary">
                through 60k+ Cord19 research papers
              </div>
            </div>

            <div className="card-body">
              <SearchBar
                query={query}
                k={k}
                loading={loading}
                onChangeQuery={setQuery}
                onChangeK={setK}
                onSubmit={onSubmit}
              />
            </div>
          </div>
        </div>
      )}

      {/* ======================
          POST-SEARCH (NORMAL)
          ====================== */}
      {hasSearched && (
        <div className="pt-5">
          <div className="container pt-4" style={{ maxWidth: 980 }}>
            {/* Hero */}
            <div className="py-4">
              <h1 className="display-5 fw-bold mb-2">NextSearch</h1>
              <div className="text-secondary">
                through 60k+ Cord19 research papers
              </div>
            </div>

            {/* Search area */}
            <div
              className="bg-light py-3 sticky-top"
              style={{ top: 54, zIndex: 1020 }}
            >
              <div className="card shadow-sm">
                <div className="card-body">
                  <SearchBar
                    query={query}
                    k={k}
                    loading={loading}
                    onChangeQuery={setQuery}
                    onChangeK={setK}
                    onSubmit={onSubmit}
                  />

                  <div className="mt-3 d-flex flex-wrap gap-2 align-items-center">
                    {/* Sort dropdown */}
                    <div className="position-relative">
                      <button
                        className="btn btn-sm btn-outline-secondary dropdown-toggle"
                        type="button"
                        onClick={() => setShowSort((v) => !v)}
                      >
                        Sort by {sortBy}
                      </button>

                      {showSort && (
                        <div
                          className="dropdown-menu show"
                          style={{ position: "absolute" }}
                        >
                          {(
                            [
                              "Relevancy",
                              "Publish Date (Newest)",
                              "Publish Date (Oldest)",
                            ] as SortBy[]
                          ).map((opt) => (
                            <button
                              key={opt}
                              className={`dropdown-item ${
                                opt === sortBy ? "active" : ""
                              }`}
                              onClick={() => {
                                setSortBy(opt);
                                setShowSort(false);
                              }}
                              type="button"
                            >
                              {opt}
                            </button>
                          ))}
                        </div>
                      )}
                    </div>

                    {/* Advanced popover */}
                    <div className="position-relative" ref={advancedWrapRef}>
                      <button
                        type="button"
                        className="btn btn-sm btn-outline-secondary"
                        onClick={() => setShowAdvanced((v) => !v)}
                        aria-expanded={showAdvanced}
                        aria-haspopup="dialog"
                      >
                        Advanced
                      </button>

                      {showAdvanced && (
                        <div
                          className="dropdown-menu show p-3"
                          style={{
                            position: "absolute",
                            right: 0,
                            top: "calc(100% + 8px)",
                            width: 340,
                            borderRadius: 14,
                            boxShadow:
                              "0 12px 28px rgba(0,0,0,0.10), 0 2px 6px rgba(0,0,0,0.08)",
                          }}
                          role="dialog"
                          aria-label="Advanced search"
                        >
                          <div className="d-flex align-items-start justify-content-between gap-2">
                            <div>
                              <div className="fw-semibold">Advanced search</div>
                              <div className="small text-secondary">
                                Tune how many results are requested from the backend.
                              </div>
                            </div>

                            <button
                              type="button"
                              className="btn btn-sm btn-outline-secondary"
                              onClick={() => setShowAdvanced(false)}
                              aria-label="Close advanced search"
                            >
                              ×
                            </button>
                          </div>

                          {/* Moved here from the main plate */}
                          {status && (
                            <div className="mt-3 small text-secondary">
                              {status}
                            </div>
                          )}

                          <hr className="my-3" />

                          <div className="d-flex align-items-center justify-content-between">
                            <label className="form-label mb-0 fw-semibold">
                              No. of results (k)
                            </label>
                            <span className="small text-secondary">{k}</span>
                          </div>

                          <div className="mt-2">
                            <input
                              type="range"
                              className="form-range"
                              min={1}
                              max={100}
                              step={1}
                              value={k}
                              onChange={(e) => setK(clampK(Number(e.target.value)))}
                            />
                          </div>

                          <div className="mt-2 d-flex gap-2 align-items-center">
                            <input
                              type="number"
                              className="form-control form-control-sm"
                              value={k}
                              min={1}
                              max={100}
                              onChange={(e) => setK(clampK(Number(e.target.value)))}
                            />
                            <button
                              type="button"
                              className="btn btn-sm btn-outline-secondary"
                              onClick={() => setK(25)}
                            >
                              25
                            </button>
                            <button
                              type="button"
                              className="btn btn-sm btn-outline-secondary"
                              onClick={() => setK(50)}
                            >
                              50
                            </button>
                            <button
                              type="button"
                              className="btn btn-sm btn-outline-secondary"
                              onClick={() => setK(100)}
                            >
                              100
                            </button>
                          </div>

                          <div className="mt-2 small text-secondary">
                            Changing <span className="fw-semibold">k</span> re-runs the search automatically.
                          </div>
                        </div>
                      )}
                    </div>
                  </div>

                  {error && (
                    <div className="alert alert-danger mt-3 mb-0">
                      <div className="fw-semibold">{error}</div>
                    </div>
                  )}
                </div>
              </div>
            </div>

            {/* Results */}
            <div className="px-3 pb-3">
              <SearchResults results={sortedResults} />
            </div>
          </div>
        </div>
      )}

      <AddDocumentModal
        show={showAddModal}
        onClose={() => setShowAddModal(false)}
      />
    </div>
  );
}
