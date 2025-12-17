"""
Script used to slice a subset from the HUGE cord19 dataset 

Example run:

python slice_cord19.py --in_root D:\cord19 --out_root D:\cord19_sliced --n 5000 --prefer either --require_body

"""


import argparse
import csv
import random
import shutil
from pathlib import Path

def safe_copy(src: Path, dst: Path):
    dst.parent.mkdir(parents=True, exist_ok=True)
    if not dst.exists():
        shutil.copy2(src, dst)

def parse_semicolon_paths(s: str):
    # CORD-19 uses ";" separated lists in pdf_json_files / pmc_json_files
    if not s:
        return []
    parts = [p.strip() for p in s.split(";")]
    return [p for p in parts if p]

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--in_root", required=True, help=r"e.g. D:\cord19")
    ap.add_argument("--out_root", required=True, help=r"e.g. D:\cord19_sliced")
    ap.add_argument("--n", type=int, default=2000, help="how many rows/docs to keep")
    ap.add_argument("--seed", type=int, default=1337)
    ap.add_argument("--prefer", choices=["pmc", "pdf", "either"], default="either",
                    help="prefer pmc_json if available, or pdf_json, or either")
    ap.add_argument("--require_body", action="store_true",
                    help="only keep docs that have at least one json file path")
    args = ap.parse_args()

    in_root = Path(args.in_root)
    out_root = Path(args.out_root)
    in_meta = in_root / "metadata.csv"
    out_meta = out_root / "metadata.csv"

    if not in_meta.exists():
        raise FileNotFoundError(f"metadata.csv not found at: {in_meta}")

    out_root.mkdir(parents=True, exist_ok=True)

    # Read all metadata rows
    with in_meta.open("r", encoding="utf-8", newline="") as f:
        reader = csv.DictReader(f)
        rows = list(reader)
        fieldnames = reader.fieldnames

    random.seed(args.seed)
    random.shuffle(rows)

    kept = []
    copied_files = 0

    for row in rows:
        pdfs = parse_semicolon_paths(row.get("pdf_json_files", ""))
        pmcs = parse_semicolon_paths(row.get("pmc_json_files", ""))

        if args.require_body and not (pdfs or pmcs):
            continue

        # Choose files based on preference (but we still copy whatever exists)
        if args.prefer == "pmc" and not pmcs:
            continue
        if args.prefer == "pdf" and not pdfs:
            continue

        # Copy referenced json files (keep same relative path)
        for rel in pdfs + pmcs:
            src = in_root / rel
            if src.exists():
                dst = out_root / rel
                safe_copy(src, dst)
                copied_files += 1

        kept.append(row)
        if len(kept) >= args.n:
            break

    if not kept:
        raise RuntimeError("No rows were kept. Try removing --require_body or changing --prefer.")

    # Write sliced metadata.csv (same header)
    with out_meta.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(kept)

    # Optionally copy schema/readme files for convenience
    for extra in ["metadata.readme", "json_schema.txt", "COVID.DATA.LIC.AGMT.pdf"]:
        src = in_root / extra
        if src.exists():
            safe_copy(src, out_root / extra)

    print(f"Kept rows: {len(kept)}")
    print(f"Copied JSON files: {copied_files}")
    print(f"Output: {out_root}")

if __name__ == "__main__":
    main()
