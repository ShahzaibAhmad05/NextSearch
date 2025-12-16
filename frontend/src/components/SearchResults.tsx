// src/components/SearchResults.tsx
import React from "react";
import type { SearchResult } from "../types";

type Props = {
  results: SearchResult[];
};

export default function SearchResults({ results }: Props) {
  if (!results.length) {
    return <div style={{ marginTop: 18, color: "#666" }}>No results.</div>;
  }

  return (
    <div style={{ marginTop: 18, display: "grid", gap: 12 }}>
      {results.map((r) => (
        <div
          key={r.docId}
          style={{
            border: "1px solid #e5e5e5",
            borderRadius: 14,
            padding: 14,
            background: "white"
          }}
        >
          <div style={{ display: "flex", justifyContent: "space-between", gap: 12 }}>
            <div style={{ fontWeight: 700, fontSize: 16 }}>
              {r.url ? (
                <a href={r.url} target="_blank" rel="noreferrer" style={{ color: "inherit" }}>
                  {r.title || "(untitled)"}
                </a>
              ) : (
                <span>{r.title || "(untitled)"}</span>
              )}
            </div>
            <div style={{ fontSize: 12, color: "#666" }}>
              score: {r.score.toFixed(4)}
            </div>
          </div>

          <div style={{ marginTop: 6, fontSize: 13, color: "#555" }}>
            docId: {r.docId} • segment: <code>{r.segment}</code> • cord_uid:{" "}
            <code>{r.cord_uid}</code>
          </div>

          <div style={{ marginTop: 6, fontSize: 13, color: "#111" }}>
            json_relpath: <code>{r.json_relpath}</code>
          </div>
        </div>
      ))}
    </div>
  );
}
