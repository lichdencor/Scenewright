# Branching & CI Strategy

**Principle: shift left.** Push as much verification as possible into pre-commit (instant, local) before CI (minutes, remote) before manual testing (slowest, needs a real game). Each layer only catches what the layer before it can't.

---

## Branches

- **`dev`** — where work actually happens. Direct pushes allowed. Runs only the fast job (`build.yml`: plain compile) so the feedback loop stays tight.
- **`main`** — protected. No direct pushes; only updated via merged pull requests (branch protection configured via GitHub, see below). By the time anything lands on `main`, it already passed the full required check suite as part of its PR.

## What runs where

| Trigger | Workflow | Jobs | Why |
|---|---|---|---|
| `push` to `dev` | `build.yml` | `build` (plain compile, `windows-latest`) | Fastest possible signal for the tight local loop — nothing slower belongs here. |
| `pull_request` (any) | `build.yml` | `build` | Re-confirms the PR head still compiles. |
| `pull_request` (any) | `analysis.yml` | `static-analysis` (MSVC `/analyze`, scoped to our own `src/`), `dynamic-analysis` (AddressSanitizer build) | Slower, more thorough — acceptable cost once per PR, not once per commit. |
| local `git commit` | `.githooks/pre-commit` | clang-format, JSON syntax, incremental compile, clang-tidy | Shift-left layer — catches formatting/lint/compile issues before they ever reach CI, on whichever machine has the toolchain to check them (skips gracefully elsewhere; see `.githooks/README.md`). |

`main` has no push-triggered workflow of its own — anything that lands there already went through the full PR suite above. Re-running everything again post-merge would be redundant.

## Honesty check on `dynamic-analysis`

The AddressSanitizer job builds today but can't catch anything meaningful yet — there's no test harness, and CI has no way to launch Skyrim/SKSE to actually exercise the plugin (see `docs/build-and-test-guide.md`). It's in place now so that once real test code exists, turning it into an actual gate is a config change, not a rewrite. Manual in-game testing remains the only way to verify real behavior until then.

## Branch protection

Configured on GitHub: `main` requires a pull request before merging (no direct pushes), admin included. Required status checks weren't pinned at setup time — GitHub only lists a check as selectable once it has run at least once — so after the first PR runs, go to Settings → Branches → main and add `build`, `static-analysis`, and `dynamic-analysis` as required checks.
