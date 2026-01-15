# Stats Persistence Test

## How to Test Persistence

### 1. Start the server:
```bash
cd C:\Users\shahz\Projects\nextsearch
.\build\api_server.exe your_index_path 8080
```

You should see:
```
[stats] No existing stats file found at: stats.json
```

### 2. Make some API calls to generate stats:

```bash
# Make searches
curl "http://localhost:8080/api/search?q=test&k=10"
curl "http://localhost:8080/api/search?q=test&k=10"  # Cache hit
curl "http://localhost:8080/api/search?q=another&k=10"
```

### 3. Check stats.json file:

The file `stats.json` will be automatically created in the root directory:

```json
{
  "total_searches": 3,
  "search_cache_hits": 1,
  "ai_overview_calls": 0,
  "ai_overview_cache_hits": 0,
  "ai_summary_calls": 0,
  "ai_summary_cache_hits": 0,
  "ai_api_calls_remaining": 1000,
  "last_updated": "2026-01-15T22:30:45Z"
}
```

### 4. Stop the server (Ctrl+C)

### 5. Restart the server:

```bash
.\build\api_server.exe your_index_path 8080
```

You should see:
```
[stats] Loaded stats from file:
  - Total searches: 3
  - AI API calls remaining: 1000
```

### 6. Make more API calls:

```bash
curl "http://localhost:8080/api/search?q=persistence&k=10"
```

### 7. Check stats.json again:

```json
{
  "total_searches": 4,  // Incremented from previous session!
  "search_cache_hits": 1,
  "ai_api_calls_remaining": 1000,
  "last_updated": "2026-01-15T22:35:12Z"
}
```

## Thread Safety Test

To test concurrent access:

```bash
# Run 10 parallel requests
for ($i=1; $i -le 10; $i++) {
    Start-Job -ScriptBlock {
        curl "http://localhost:8080/api/search?q=concurrent&k=10"
    }
}

# Wait for all jobs to complete
Get-Job | Wait-Job

# Check stats - should show exactly 10 searches (or 10 + cache hits)
curl "http://localhost:8080/api/stats" -H "Authorization: Bearer YOUR_TOKEN"
```

## Features

✅ **Automatic Persistence**: Stats are saved after every change
✅ **Thread-Safe**: Uses mutex for file I/O, atomic operations for counters
✅ **Survives Restarts**: All stats persist across server restarts
✅ **Real-time Updates**: The stats.json file is updated immediately
✅ **Timestamp Tracking**: Shows when stats were last updated

## Stats File Location

The `stats.json` file is created in the same directory where you run the server (usually the project root).

## Important Notes

1. **File I/O Performance**: Stats are saved after EVERY change. For extremely high-traffic scenarios, consider batching writes.

2. **Concurrent Safety**: 
   - Atomic counters prevent race conditions in memory
   - Mutex protects file I/O from concurrent writes
   - Safe for multiple simultaneous API calls

3. **AI API Calls Counter**:
   - Initially set from `AI_API_CALLS_LIMIT` in `.env`
   - Once set, persists in `stats.json`
   - To reset: delete `stats.json` and restart server
   - To manually adjust: edit `stats.json` and restart server

4. **Manual Editing**: You can manually edit `stats.json` while the server is stopped. Changes will be loaded on next startup.
