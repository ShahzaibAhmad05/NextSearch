"""
A script to slice 60k files from the original dataset of 
one million files.
"""

import pandas as pd

SOURCE = "metadata.csv"          # original big file
OUTPUT = "metadata_slice.csv"    # reduced file
CHUNK = 50_000                   # rows per chunk
LIMIT = 60_000                   # how many valid rows you want max

# toggle this depending on what you want
REQUIRE_URL = False  # True => only rows with abstract AND url

def row_filter(chunk: pd.DataFrame) -> pd.DataFrame:
    # must have abstract
    chunk = chunk.dropna(subset=["abstract"])

    if REQUIRE_URL:
        chunk = chunk.dropna(subset=["url"])

    return chunk

def main():
    count = 0
    header_written = False

    for chunk in pd.read_csv(SOURCE, chunksize=CHUNK):
        chunk = row_filter(chunk)

        if chunk.empty:
            continue

        # cut if we would exceed LIMIT
        remaining = LIMIT - count
        if remaining <= 0:
            break

        if len(chunk) > remaining:
            chunk = chunk.head(remaining)

        mode = "w" if not header_written else "a"
        chunk.to_csv(OUTPUT, index=False, mode=mode, header=not header_written)

        count += len(chunk)
        header_written = True

        if count >= LIMIT:
            break

    print(f"Done. Wrote {count} rows to {OUTPUT}")

if __name__ == "__main__":
    main()
