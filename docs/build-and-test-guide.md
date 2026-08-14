# Build & Test Guide: Compiling and Running the SKSE Plugin

**Setup this assumes:** compiling in **WinBoat** (`ghcr.io/dockur/windows`, a KVM Windows instance running as a Docker container on this Fedora machine — same one already used for Excel/Pandora Engine), testing on a separate Linux laptop that runs Skyrim SE well under Steam Proton-GE. Two machines, one loop — but the first hop (VM → this host) is now free, via WinBoat's shared folder.

**Confidence note:** the compile side (§1) is well-established, sourced in `epic-1-camera-scene-engine.md`. The Proton/Linux testing side (§2-4) is thinner ground — some of it is community-guide-confirmed, some is reasoned inference flagged explicitly below. Expect to verify a few specifics on your machine the first time through.

---

## 0. One-time per clone: activate shared git hooks

```sh
git config core.hooksPath .githooks
```

Do this on every machine that commits to this repo (VM included) — see `.githooks/README.md` for what the pre-commit hook actually checks.

## 1. Compile — two options

**Option A: GitHub Actions (no local Windows environment at all).** `.github/workflows/build.yml` already runs on a real `windows-latest` MSVC machine on every push, and uploads the built `.dll`/`.pdb` as a downloadable artifact. Loop: edit code here → commit/push → open the Actions run → download the `Scenewright-<sha>` artifact → send it to the test laptop (§2). Slower per-iteration (CI queue + full/incremental build time) but zero setup and nothing to babysit — the best option when you don't want to touch a Windows session at all. **Cross-compiling locally (e.g. clang-cl + MinGW, no Windows anywhere)** was considered and ruled out for now: CommonLibSSE-NG's vcpkg port explicitly declares `"supports": "windows & x64"`, none of the cross-toolchain pieces (`xwin`/Windows SDK sysroot, `lld-link`) are set up on this machine, and there's no confirmed precedent of anyone building it that way — real risk of costing more setup time than it saves versus the two options below.

**Option B: WinBoat (interactive, faster iteration).** Same toolchain as Pandora Engine, just running inside the WinBoat Windows session instead of a separate VM — better when you're actively iterating on camera/scene code and want a normal F7-and-check loop instead of waiting on CI each time.

Same toolchain as Pandora Engine — Visual Studio 2022 with the "Desktop development with C++" workload, CMake + vcpkg in manifest mode — just running inside the WinBoat Windows session instead of a separate VM.

1. In the WinBoat Windows session, confirm the shared folder is toggled on (Network → `host.lan` → your Linux home folder). If you haven't already, clone this repo directly onto that share (e.g. work out of `\\host.lan\lchavez\Documents\Personal\Scenewright` from inside Windows) so there's no separate copy/sync step at all — VS2022 opens and builds the project straight off the shared path.
2. Clone `vcpkg` (inside Windows, on a local Windows path is fine — no need to put the vcpkg tool itself on the share), run `bootstrap-vcpkg.bat`, set `VCPKG_ROOT`.
3. Open the project folder in VS2022 — CMake+vcpkg auto-configures on first open (slow the first time, it pulls CommonLibSSE-NG).
4. Build with F7 (or `Ctrl+Shift+B`). Since the project itself lives on `\\host.lan\...`, the output DLL (`build/debug/Scenewright.dll` or similar, per `CMakeLists.txt`) lands directly on this Fedora host's filesystem — no manual export step out of the Windows session needed.

---

## 2. Get the DLL from this host to the Linux test laptop

This is now the *only* remaining network hop (WinBoat → this host is already solved via the shared folder above). Pick whichever you already have working between this machine and the test laptop:

- **`rsync`/`scp` over the network**, or
- **Syncthing** for an always-on, no-manual-step sync.

Land the DLL at:
```
<Skyrim install>/Data/SKSE/Plugins/Scenewright.dll
```
(or inside your mod manager's mod folder — see §3).

A one-line watch script on this host (`inotifywait` on the build output → `rsync` to the test laptop) turns this into a near-automatic loop worth setting up once you're iterating quickly.

---

## 3. Get SKSE to actually launch under Proton-GE

**Already solved on your end** — you run this via **Amethyst**, which natively overrides `SkyrimSE.exe` with the SKSE loader with no extra setup, and you have MO2/Limo as known-working fallbacks. No launcher research needed here; use whichever of the three you already have configured. The rest of this guide assumes SKSE is loading successfully via your existing setup — only §4 (log path) needs a small adjustment depending on which mod manager's Proton prefix you're actually running under.

---

## 4. Verify the plugin actually loaded

SKSE and plugin logs land in the Windows-standard path, just rooted inside whichever Proton prefix actually ran the game:
```
<compatdata_root>/<AppID>/pfx/drive_c/users/steamuser/Documents/My Games/Skyrim Special Edition/SKSE/
```
Which `<AppID>` that is depends on whether Amethyst/MO2/Limo runs Skyrim in its own dedicated prefix or shares the plain `489830` Skyrim SE prefix — since this varies by mod manager and your specific config, the fastest way to find it is: `find ~/.local/share/Steam/steamapps/compatdata -ipath '*Skyrim Special Edition/SKSE*' -newer /tmp` right after a test run, which surfaces whichever prefix was just written to.

What to look for:
- **Your plugin's own log file with its expected init lines present** = loaded successfully.
- **No log file for your plugin at all**, even though `skse64.log` exists = SKSE didn't load it — almost always an Address Library / SKSE / game-build version mismatch, not a bug in your code yet. Check that your plugin's targeted runtime version, SKSE's version, and the actual game build all match.
- **Crash with no log update** = check a `SKSE/Crash Logs/` subfolder for a [CrashLoggerSSE](https://github.com/alandtse/CrashLoggerSSE) dump if you have it installed (worth installing early, before you even start on camera code — cheap insurance).

---

## 5. Iterate

Once the above loop is proven once:
1. Build in WinBoat (F7) — output lands on this host via the shared folder, no separate export step.
2. Sync step from §2 fires (manual rsync, or your watch script) to get it onto the test laptop.
3. Relaunch via whichever method you settled on in §3.
4. Check the log path from §4.

This is also exactly the moment to install [CrashLoggerSSE](https://github.com/alandtse/CrashLoggerSSE) into the test setup, before writing any camera/fade code — Epic 1's fade-hold-across-save-load crash risk (`epic-1-camera-scene-engine.md` §2) and any camera vtable-detour mistakes are exactly the kind of native crash it's built to catch.

---

## Known open risks in this workflow (flagged, not yet resolved)

- Whether Proton-GE truly auto-detects `skse64_loader.exe` without manual setup — unconfirmed, check GE's release notes directly.
- Whether DXVK (Proton's D3D→Vulkan translation layer) interacts badly with the camera/fade-menu (Scaleform/UI) hooks this project needs — no Skyrim-specific report found either way; this is worth testing early with Milestone 0-2 (hello-world + fade) specifically because it's cheap to catch now versus after the camera hook is built.
- Exact log path will differ slightly by Steam library location — verify once on your actual machine rather than trusting the path above literally.
