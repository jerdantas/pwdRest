# pwdClient — Qt Desktop Client for pwdRest

## Overview
`pwdClient` is a Qt 6 desktop application that connects to the `pwdServer` REST API to manage site credentials (site → username/password). It provides a fast, keyboard‑friendly UI with autocomplete and common operations: create/update entries, fetch/copy credentials, and delete entries.

The client authenticates against the server and then performs subsequent requests using a JWT provided by the server.

## Features
- Login flow that authenticates with the server and obtains a JWT.
- Site search with autocomplete (case‑insensitive) to quickly locate entries.
- View and copy username/password to the clipboard.
- Create new entries and update existing ones via a dialog.
- Delete entries with confirmation and immediate list refresh.
- Keyboard‑centric interactions:
  - Enter in the site field triggers the primary action (same as pressing OK).
  - Esc in the site field acts as Close.
  - Double‑clicking a site in the list triggers the primary action.
- Clear error reporting with message boxes for network/database issues and not‑found cases.

## UI At a Glance
- Login dialog prompts for credentials and establishes a session (JWT kept in memory).
- Main window:
  - Site input (`siteEdit`) with auto‑complete from the server‑provided list.
  - Sites list that supports single‑click selection and double‑click activation.
  - Buttons for OK (fetch/show), New, Update, Delete, and Close.
  - Username/password fields with copy‑to‑clipboard actions.

## How It Works (High Level)
- After login, the client fetches and maintains a cached list of site names to drive the completer.
- Core operations map to server REST endpoints (JSON over HTTP):
  - get(site) → fetch username/password
  - set(site, username, password) → create or update
  - del(site) → delete
  - list() → refresh site names for autocomplete
- On success, the local UI is updated; on errors, the client shows a descriptive message.

## Build & Run
### Prerequisites
- Qt 6.10.1 (Widgets)
- OpenSSL
- CMake 3.14+

These are the same core dependencies used across the project. Third‑party HTTP/JSON/JWT libraries are fetched in the top‑level CMake via FetchContent.

### Build
Use the provided convenience script from the repository root:

```bash
./build_client.sh
```

This creates a `build` directory and compiles the client target.

### Run
1. Start `pwdServer` first (ensure it has access to its SQLite database and correct `JWT_SECRET`).
2. Launch the client binary from the build output directory.
3. Log in with your server account; then search or manage site entries.

## Configuration Notes
- The client communicates with the running `pwdServer` instance and uses JWT for authentication.
- The server stores its SQLite DB at `~/.local/share/pwd/rempasswd.db` (see server docs). Ensure the server is reachable from the client machine.

## Keyboard & UX Details
- Site field focus auto‑selects all text for quick replacement.
- Pressing Enter in the site field triggers the OK action (fetch).
- Pressing Esc in the site field triggers Close.
- Double‑clicking a site in the list also triggers OK.
- Autocomplete is case‑insensitive and updates whenever the site list is refreshed.

## Error Handling
- Network and server errors are surfaced via message boxes.
- Not‑found cases display helpful suggestions including the known site list.
- Critical errors (e.g., database issues reported by the server) are shown as critical alerts.

## Security Considerations
- JWT is kept in memory for the session; restart the client to clear it.
- Copying passwords places sensitive data on the system clipboard—use with care.
- Ensure the server is configured with a strong `JWT_SECRET` and is served over TLS in production setups.

## Testing & Development Tips
- Follow the project’s Developer Guide for dependency versions and build tips.
- For end‑to‑end checks, run the server with the `-p` flag to observe requests/responses.
- When developing new UI actions, keep keyboard flows consistent (Enter/Esc, double‑click) and update the site completer after data‑changing operations.

## Folder Structure
- `pwdClient/` — client sources (Qt Widgets):
  - `client_main.cpp` — application entry point
  - `MainWindow.*` — main UI window logic
  - `LoginDialog.*` — user login dialog
  - `NewEntryDialog.*` — create/update entry dialog
  - `PasswordClient.*` — thin HTTP/JSON client used by the UI

## Known Limitations
- Requires a reachable and compatible `pwdServer` instance.
- Clipboard operations depend on the desktop environment policies.
