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
./build/macos-arm64/kopds-app --http-address 127.0.0.1 --http-port 8080
```

Then open <http://127.0.0.1:8080>.

When `DATABASE_URL` is set, the application runs `select 1` before starting the
HTTP server and exits if PostgreSQL cannot be reached. Without `DATABASE_URL`,
local non-container runs continue with the database status shown as not
configured.

## Run with PostgreSQL and Compose

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
