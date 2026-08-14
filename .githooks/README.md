# Shared git hooks

Hooks live here (checked into the repo) instead of `.git/hooks/` so the same
checks run on every clone — the Windows build VM and this Linux dev machine
alike. Git doesn't use this path by default; activate it once per clone:

```sh
git config core.hooksPath .githooks
```

(Works the same on the Windows VM via Git for Windows' bundled Git Bash —
no separate `.bat`/PowerShell version needed.)

## `pre-commit`

- **clang-format check** on staged `.cpp`/`.h` files — blocks the commit if a
  file isn't formatted per `.clang-format`. Skips with a warning if
  `clang-format` isn't installed.
- **JSON syntax check** on staged `vcpkg.json` / `vcpkg-configuration.json` /
  `CMakePresets.json` changes.
- **Incremental compile check** — only runs if `build/debug/` is already
  configured (`cmake --preset debug`, see `docs/build-and-test-guide.md`),
  since a full configure-from-scratch pulls and builds CommonLibSSE-NG via
  vcpkg and is too slow for a commit hook. On any machine without that
  configured build dir (or without `cmake` at all — e.g. this Linux dev
  machine, which has no MSVC toolchain), it skips with a message instead of
  silently pretending to check something it can't. `.github/workflows/build.yml`
  is the build gate that always runs regardless of which machine committed.
- **clang-tidy** on staged `.cpp` files (config in `.clang-tidy`, scoped to
  our own `src/` — CommonLibSSE-NG's headers aren't ours to fix) — same
  gating as the compile check: needs `build/debug/compile_commands.json` and
  `clang-tidy` on PATH, skips elsewhere. `.github/workflows/analysis.yml` runs
  the authoritative static-analysis pass (MSVC `/analyze`) on every PR
  regardless of what ran locally.

This is the shift-left half of the CI strategy — see `docs/ci-and-branching.md`
for the full picture (what runs on `dev` pushes vs. PRs, and why `main` only
moves via merged PRs).

Bypass any hook for one commit with `git commit --no-verify` — use sparingly.
