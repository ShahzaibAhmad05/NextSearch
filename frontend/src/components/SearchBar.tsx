// src/components/SearchBar.tsx
import React from "react";

type Props = {
  query: string;
  k: number;
  loading: boolean;
  onChangeQuery: (q: string) => void;
  onChangeK: (k: number) => void;
  onSubmit: () => void;
};

export default function SearchBar(props: Props) {
  const { query, k, loading, onChangeQuery, onChangeK, onSubmit } = props;

  return (
    <div style={{ display: "flex", gap: 12, alignItems: "center", flexWrap: "wrap" }}>
      <input
        value={query}
        onChange={(e) => onChangeQuery(e.target.value)}
        onKeyDown={(e) => {
          if (e.key === "Enter") onSubmit();
        }}
        placeholder="Search..."
        style={{
          flex: "1 1 420px",
          padding: "10px 12px",
          borderRadius: 10,
          border: "1px solid #ddd",
          outline: "none"
        }}
      />

      <label style={{ display: "flex", alignItems: "center", gap: 8 }}>
        <span style={{ fontSize: 14, color: "#555" }}>Top K</span>
        <input
          type="number"
          min={1}
          max={200}
          value={k}
          onChange={(e) => onChangeK(Number(e.target.value))}
          style={{
            width: 90,
            padding: "10px 12px",
            borderRadius: 10,
            border: "1px solid #ddd"
          }}
        />
      </label>

      <button
        onClick={onSubmit}
        disabled={loading || query.trim().length === 0}
        style={{
          padding: "10px 14px",
          borderRadius: 10,
          border: "1px solid #111",
          background: loading ? "#eee" : "#111",
          color: loading ? "#444" : "#fff",
          cursor: loading ? "not-allowed" : "pointer"
        }}
      >
        {loading ? "Searching..." : "Search"}
      </button>
    </div>
  );
}
