// src/components/AddDocumentModal.tsx
import React, { useState } from "react";
import { addDocument as apiAddDocument } from "../api";

type Props = {
  show: boolean;
  onClose: () => void;
};

export default function AddDocumentModal({ show, onClose }: Props) {
  const [cord_root, setCordRoot] = useState("");
  const [json_relpath, setJsonRelpath] = useState("");
  const [cord_uid, setCordUid] = useState("");
  const [title, setTitle] = useState("");
  const [loading, setLoading] = useState(false);
  const [err, setErr] = useState<string | null>(null);
  const [ok, setOk] = useState<string | null>(null);

  if (!show) return null;

  async function onSubmit(e: React.FormEvent) {
    e.preventDefault();
    setErr(null);
    setOk(null);

    if (!cord_root || !json_relpath || !cord_uid || !title) {
      setErr("All fields are required.");
      return;
    }

    setLoading(true);
    try {
      const out = await apiAddDocument({ cord_root, json_relpath, cord_uid, title });
      setOk(`Added: ${out.segment} (reloaded=${out.reloaded}) time=${out.total_time_ms?.toFixed?.(2)}ms`);
    } catch (e: any) {
      setErr(e?.message ?? String(e));
    } finally {
      setLoading(false);
    }
  }

  return (
    <>
      {/* Backdrop */}
      <div className="modal-backdrop fade show" />

      {/* Modal */}
      <div className="modal fade show" style={{ display: "block" }} role="dialog" aria-modal="true">
        <div className="modal-dialog modal-lg modal-dialog-centered" role="document">
          <div className="modal-content">
            <div className="modal-header">
              <h5 className="modal-title">Add Document</h5>
              <button type="button" className="btn-close" aria-label="Close" onClick={onClose} />
            </div>

            <form onSubmit={onSubmit}>
              <div className="modal-body">
                <div className="row g-3">
                  <div className="col-12">
                    <label className="form-label">CORD_ROOT (absolute path on server)</label>
                    <input className="form-control" value={cord_root} onChange={(e) => setCordRoot(e.target.value)} />
                  </div>

                  <div className="col-12">
                    <label className="form-label">JSON_REL_PATH (relative path)</label>
                    <input className="form-control" value={json_relpath} onChange={(e) => setJsonRelpath(e.target.value)} />
                  </div>

                  <div className="col-md-6">
                    <label className="form-label">CORD_UID</label>
                    <input className="form-control" value={cord_uid} onChange={(e) => setCordUid(e.target.value)} />
                  </div>

                  <div className="col-md-6">
                    <label className="form-label">TITLE</label>
                    <input className="form-control" value={title} onChange={(e) => setTitle(e.target.value)} />
                  </div>
                </div>

                {err ? <div className="alert alert-danger mt-3 mb-0">{err}</div> : null}
                {ok ? <div className="alert alert-success mt-3 mb-0">{ok}</div> : null}

                <div className="text-secondary small mt-3">
                  Backend must be running (example): <code>./api_server &lt;INDEX_DIR&gt; 8080</code>
                </div>
              </div>

              <div className="modal-footer">
                <button type="button" className="btn btn-outline-secondary" onClick={onClose} disabled={loading}>
                  Close
                </button>
                <button type="submit" className="btn btn-dark" disabled={loading}>
                  {loading ? "Adding..." : "Add"}
                </button>
              </div>
            </form>
          </div>
        </div>
      </div>
    </>
  );
}
