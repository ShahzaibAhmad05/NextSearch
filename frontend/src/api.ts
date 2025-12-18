import type { SearchResponse, SuggestResponse } from "./types";

/**
 * Use same-origin API path in dev via Vite proxy:
 *   /api/search, /api/add_document
 *
 * This avoids CORS & mixed-content issues entirely.
 */
const BASE = (import.meta.env.VITE_API_BASE as string | undefined) ?? "/api";

function isFetchNetworkError(err: unknown): boolean {
  return err instanceof TypeError && /failed to fetch/i.test(err.message);
}

async function safeJson(res: Response): Promise<any> {
  const ct = res.headers.get("content-type") ?? "";
  if (ct.includes("application/json")) return res.json();
  const txt = await res.text().catch(() => "");
  try {
    return JSON.parse(txt);
  } catch {
    return txt;
  }
}

export async function search(query: string, k: number): Promise<SearchResponse> {
  const url = new URL(`${BASE}/search`, window.location.origin);
  url.searchParams.set("q", query);
  url.searchParams.set("k", String(k));

  const res = await fetch(url.toString(), {
    method: "GET",
    headers: { Accept: "application/json" },
  });

  if (!res.ok) {
    const text = await res.text().catch(() => "");
    throw new Error(`Search failed (${res.status}): ${text}`);
  }

  return (await res.json()) as SearchResponse;
}

export async function suggest(
  query: string,
  k: number,
  signal?: AbortSignal
): Promise<SuggestResponse> {
  const url = new URL(`${BASE}/suggest`, window.location.origin);
  url.searchParams.set("q", query);
  url.searchParams.set("k", String(k));

  const res = await fetch(url.toString(), {
    method: "GET",
    headers: { Accept: "application/json" },
    signal,
  });

  if (!res.ok) {
    const text = await res.text().catch(() => "");
    throw new Error(`Suggest failed (${res.status}): ${text}`);
  }

  return (await res.json()) as SuggestResponse;
}

export async function addCordSlice(cordSliceZip: File, signal?: AbortSignal): Promise<any> {
  const fd = new FormData();
  fd.append("cord_slice", cordSliceZip, cordSliceZip.name);

  try {
    const res = await fetch(new URL(`${BASE}/add_document`, window.location.origin).toString(), {
      method: "POST",
      headers: { Accept: "application/json" },
      body: fd,
      signal,
    });

    if (!res.ok) {
      const payload = await safeJson(res);
      const msg =
        typeof payload === "string"
          ? payload
          : payload?.error
          ? `${payload.error}${payload.details ? `: ${payload.details}` : ""}`
          : JSON.stringify(payload);
      throw new Error(`Add document failed (${res.status}): ${msg}`);
    }

    return await res.json();
  } catch (err) {
    if (isFetchNetworkError(err)) {
      throw new Error(
        "Failed to fetch /add_document. With /api proxy, this usually means the Vite proxy couldn't reach the backend.\n" +
          "Check backend is running on 127.0.0.1:8080 and that /health works."
      );
    }
    throw err;
  }
}
