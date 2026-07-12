# KOPDS App

A minimal Wt web application, built with CMake and vcpkg. It uses vcpkg's
Wt 4.13.3 port with the PostgreSQL Dbo backend enabled.

The supplied presets use vcpkg's dynamic-library triplets. Wt's static package
does not propagate every Pango/font dependency required by the final linker on
macOS, while the dynamic package keeps those implementation dependencies inside
the Wt libraries.

## Prerequisites

- CMake 3.25 or newer
- vcpkg, with `VCPKG_ROOT` set to its installation directory

Wt is GPL-2.0-only when installed from vcpkg. Ensure the application's
distribution complies with that license.

## Build on Apple Silicon macOS

Install vcpkg once if it is not already available:

```sh
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
```

Set `VCPKG_ROOT` in the shell where CMake is run. Use the path to your existing
vcpkg checkout if it is installed elsewhere.

```sh
export VCPKG_ROOT="$HOME/vcpkg"
test -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

The `test` command must succeed before configuring. Then run:

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64
```

## Build on Linux x64

```sh
cmake --preset linux-x64
cmake --build --preset linux-x64
```

## Run

```sh
./build/macos-arm64/kopds-app \
  --http-address 127.0.0.1 \
  --http-port 8080 \
  --docroot . \
  --resources-dir ./build/macos-arm64/vcpkg_installed/arm64-osx-dynamic/share/wt/resources
```

Then open <http://127.0.0.1:8080>. The application provides `/dashboard` and
`/system-status` routes through a responsive Bootstrap 5 shell.

The System Status page runs `select 1` when it is opened or refreshed. Database
failures are displayed as a degraded status and do not stop the application.
Without `DATABASE_URL`, PostgreSQL is shown as not configured.

## Run with PostgreSQL and Compose

Apply the schema once before starting the application:

```sh
docker compose up -d postgres
docker compose exec -T postgres psql -v ON_ERROR_STOP=1 -U kopds -d kopds \
  < db/migrations/001_initial.sql
```

```sh
docker compose up --build
```

Compose waits for PostgreSQL's `pg_isready` health check before starting the
application. Open <http://127.0.0.1:8080>; the page reports that PostgreSQL is
connected.

Stop the services with:

```sh
docker compose down
```

Add `--volumes` to the `down` command when you also want to delete the local
PostgreSQL data volume.

## Container CI

The GitHub Actions workflow builds two Linux image variants:

- `linux/amd64` for conventional Linux hosts
- `linux/arm64` for Docker Desktop on Apple Silicon macOS and ARM64 Linux

Docker images always use a Linux kernel; GitHub-hosted macOS runners cannot
produce a native macOS container image.

## Project structure

The application follows a bounded-context DDD structure:

```text
src/
├── bootstrap/                 # Composition root and Wt setup
├── shell/                     # Application layout and routing
├── shared/infrastructure/     # Shared database session factory
└── contexts/
    ├── membership/            # Member aggregate and CRUD
    ├── contribution_configuration/ # Contribution type aggregate and CRUD
    └── system_health/         # Framework-independent health checks
```

Future business capabilities should be added as sibling bounded contexts rather
than placed directly in the shell or infrastructure directories.

The current business contexts are:

- `membership`: member listing, generated `KOPDS-0001` numbers, editing,
  soft deletion, and restoration.
- `contribution_configuration`: contribution-type rules and inline localized
  labels, including the required `id-ID` translation.

All database tables contain `created_at`, `updated_at`, and nullable
`deleted_at`. PostgreSQL triggers maintain `updated_at`; application deletes set
`deleted_at` and never physically remove records.

## Verification checklist

Run the native or container build yourself, then verify:

- `/` redirects to `/dashboard`.
- Sidebar links update the URL and selected state.
- The sidebar becomes an off-canvas menu on narrow screens.
- `/system-status` reports healthy, unavailable, and unconfigured PostgreSQL.
- The Refresh button updates health without restarting the application.
- `/members` supports create, view, edit, search, pagination, delete, and restore.
- `/configuration/contribution-types` manages rules and inline translations.
- Created, updated, and deleted timestamps reflect database mutations.
- An unknown internal path displays the Not Found page.
