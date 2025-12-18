// src/components/AddDocumentModal.tsx
import React, { useMemo, useState } from "react";
import { addCordSlice as apiAddCordSlice } from "../api";

type Props = {
  show: boolean;
  onClose: () => void;
};

export default function AddDocumentModal({ show, onClose }: Props) {
  const [file, setFile] = useState<File | null>(null);
  const [loading, setLoading] = useState(false);
  const [err, setErr] = useState<string | null>(null);
  const [ok, setOk] = useState<string | null>(null);

  const fileLabel = useMemo(() => {
    if (!file) return "No file chosen";
    return `${file.name} (${Math.round(file.size / 1024)} KB)`;
  }, [file]);

  async function onSubmit(e: React.FormEvent) {
    e.preventDefault();
    setErr(null);
    setOk(null);

    if (!file) {
      setErr("Please choose a .zip file (CORD-19 cord_slice).");
      return;
    }
    if (!file.name.toLowerCase().endsWith(".zip")) {
      setErr("File must be a .zip.");
      return;
    }

    setLoading(true);
    try {
      const out = await apiAddCordSlice(file);
      const ms =
        out?.total_time_ms != null && typeof out.total_time_ms === "number"
          ? out.total_time_ms.toFixed(2)
          : String(out?.total_time_ms ?? "?");

      setOk(
        `Indexed ${out?.docs_indexed ?? "?"} docs into ${out?.segment ?? "?"} (reloaded=${
          out?.reloaded ?? "?"
        }) in ${ms}ms`
      );
      setFile(null);
    } catch (e: any) {
      setErr(e?.message ?? String(e));
    } finally {
      setLoading(false);
    }
  }

  if (!show) return null;

  // IMPORTANT:
  // This project uses Bootstrap. Bootstrap defines `.modal` and `.modal-backdrop`
  // with JS-driven behavior (e.g., `.modal { display:none; }` unless `.show`).
  // Using those names causes "black screen / invisible popup".
  // So we use adddoc-* class names to avoid collisions.
  return (
    <div className="adddoc-backdrop" role="dialog" aria-modal="true" aria-label="Add CORD Slice">
      <div className="adddoc-modal">
        <div className="adddoc-header">
          <h2 className="m-0 fs-5">Add CORD Slice</h2>
          <button className="btn btn-sm btn-outline-dark" type="button" onClick={onClose} disabled={loading}>
            ✕
          </button>
        </div>

        <form onSubmit={onSubmit}>
          <p className="mb-2">
            Upload a <b>CORD-19 slice zip</b> with this structure:
          </p>
          <pre className="adddoc-pre mb-3">
{`cord19_sliced
├─ document_parses/
│  ├─ pdf_json/
│  └─ pmc_json/
├─ COVID.DATA.LIC.AGMT.pdf
├─ json_schema.txt
├─ metadata.csv
└─ metadata.readme`}
          </pre>

          <label className="form-label fw-semibold">Zip file</label>
          <div className="d-flex gap-2 align-items-center">
            <input
              type="file"
              className="form-control"
              accept=".zip,application/zip"
              onChange={(e) => setFile(e.target.files?.[0] ?? null)}
              disabled={loading}
            />
            <span className="adddoc-filelabel">{fileLabel}</span>
          </div>

          {err && <div className="adddoc-alert adddoc-alert-error">{err}</div>}
          {ok && <div className="adddoc-alert adddoc-alert-ok">{ok}</div>}

          <div className="adddoc-footer">
            <button className="btn btn-outline-dark" type="button" onClick={onClose} disabled={loading}>
              Close
            </button>
            <button className="btn btn-dark" type="submit" disabled={loading || !file}>
              {loading ? "Indexing..." : "Upload & Index"}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}
