# Repository Guidelines

## Project Structure & Module Organization
- `libs/scenario_fmu/` — main C++20 library. Public headers in `include/`, private headers in `include_private/`, sources in `src/`. Builds a shared library target `scenario`.
- `3rdParty/reference_fmu/` — git submodule with reference FMUs (read‑only; update via submodule, don’t edit directly).
- `test/` — reserved for repository tests.
- `build/` — CMake build output (ignored by git).

## Build, Test, and Development Commands
- Initialize submodules: `git submodule update --init --recursive`.
- Configure (Ninja, preset): `cmake --preset=vcpkg`.
- Build: `cmake --build build` (append `-j` to parallelize).

- Run tests: `ctest --test-dir build -V`.
- Inspect artifact (example): `objdump -TC build/libs/scenario_fmu/libscenario.so`.

## Coding Style & Naming Conventions
- Language: C++20. Indentation: 4 spaces, no tabs. Keep lines ≤100 chars.
- Filenames: snake_case for `.cpp`/`.hpp`. Public headers live in `include/`; internal in `include_private/`.
- Types/classes: `PascalCase`; functions/variables: `lower_snake_case`; constants: `kPascalCase`.
- Includes: local then project then system. Prefer `#pragma once` in headers.

## Testing Guidelines
- Framework: CTest; GoogleTest via vcpkg is recommended.
- Place tests in `test/` named `*_test.cpp`. Register with CMake (`add_executable`, `add_test`).
- Aim for coverage on public headers in `include/` and critical paths.

## Commit & Pull Request Guidelines
- Commits: concise, imperative, optionally scoped. Examples:
  - `build: add Ninja preset`
  - `fmu: implement FMI2 set/get`
- PRs: clear description, linked issues, build/test status, and relevant logs/screenshots. Keep changes focused; update docs when behavior changes.

## Security & Configuration Tips
- Don’t commit build outputs or local toolchain paths. Update submodules with `git submodule update --init --recursive`.
- Avoid modifying `3rdParty/reference_fmu/` directly; change via submodule updates or wrap in `libs/`.

