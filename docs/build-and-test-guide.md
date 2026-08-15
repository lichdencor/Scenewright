# Build & Test Guide

Generic instructions for compiling the SKSE plugin and testing it in-game. Contributors on native Windows, Linux, or anything else should be able to follow this without needing any particular personal setup.

---

## 0. One-time per clone: activate shared git hooks

```sh
git config core.hooksPath .githooks
```

See `.githooks/README.md` for what the pre-commit hook actually checks.

---

## 1. Compile

CommonLibSSE-NG plugins need a real MSVC toolchain (vcpkg's `commonlibsse-ng` port is declared `"supports": "windows & x64"` — no supported cross-compile path today). Pick whichever of these fits:

**Option A — Don't compile locally at all.** `.github/workflows/build.yml` builds on a real `windows-latest` MSVC machine on every push and uploads the resulting `.dll`/`.pdb` as a downloadable artifact on the run. Good enough if you just want to test a change without setting up any Windows environment yourself.

**Option B — Native Windows.** Visual Studio 2022 (Community is fine) with the "Desktop development with C++" workload, plus `vcpkg` (clone it, run `bootstrap-vcpkg.bat`, set `VCPKG_ROOT`). Open the repo folder in VS2022 — CMake + vcpkg auto-configure on first open (slow the first time, it pulls CommonLibSSE-NG) — then build with F7. Output lands in `build/<preset>/`.

**Option C — A Windows VM/container of your own choosing** (VirtualBox, a KVM-backed Windows container like `dockur/windows`, a cloud Windows box, whatever you already have). Same steps as Option B once you're inside it. Nothing in this repo assumes a specific one.

---

## 2. Deploy the built DLL for testing

Land it at:
```
<Skyrim SE Data folder>/SKSE/Plugins/Scenewright.dll
```
If you use a mod manager (MO2, Vortex, Amethyst, Limo, etc.), give it its own mod entry/folder and deploy through the manager rather than copying straight into `Data/` by hand — most managers track their own deployed files and can silently overwrite or ignore anything placed there manually.

---

## 3. Run it

- **Native Windows:** launch `skse64_loader.exe` from the Skyrim SE install directory (or through your mod manager, which typically wraps this for you).
- **Linux via Proton:** any mod manager capable of launching SKSE under Proton works (MO2, Vortex, Amethyst, Limo, and others all solve this). Getting Steam to run `skse64_loader.exe` instead of the vanilla launcher is the part that trips people up on Linux — if your mod manager doesn't already handle it, that's the thing to solve first, independent of anything in this repo.

---

## 4. Verify the plugin actually loaded

SKSE and plugin logs land at the Windows-standard path:
```
Documents/My Games/Skyrim Special Edition/SKSE/
```
On native Windows that's literally `%USERPROFILE%\Documents\...`. Under Proton it's rooted inside whichever prefix ran the game:
```
<compatdata_root>/<Steam AppID>/pfx/drive_c/users/steamuser/Documents/My Games/Skyrim Special Edition/SKSE/
```
(Skyrim SE's AppID is `489830`, but a mod manager may run the game in its own separate prefix rather than that one — check which prefix actually got written to if unsure.)

What to look for:
- **Your plugin's own log file with its expected init lines present** = loaded successfully.
- **No log file for your plugin at all**, even though `skse64.log` exists = SKSE didn't load it — almost always an Address Library / SKSE / game-build version mismatch, not a bug in your code yet. Check that your plugin's targeted runtime version, SKSE's version, and the actual game build all match.
- **Crash with no log update** = check a `SKSE/Crash Logs/` subfolder for a [CrashLoggerSSE](https://github.com/alandtse/CrashLoggerSSE) dump if you have it installed (worth installing early, before you even start on camera code — cheap insurance).

---

## 5. Iterate

1. Build (§1).
2. Deploy (§2).
3. Relaunch (§3).
4. Check the log (§4).

Install [CrashLoggerSSE](https://github.com/alandtse/CrashLoggerSSE) in your test setup before writing any camera/fade code — Epic 1's fade-hold-across-save-load crash risk (`epic-1-camera-scene-engine.md` §2) and any camera vtable-detour mistakes are exactly the kind of native crash it's built to catch.

---

## Known open risks

- Whether DXVK (Proton's D3D→Vulkan translation layer) interacts badly with the camera/fade-menu (Scaleform/UI) hooks this project needs — no Skyrim-specific report found either way. Worth testing early with Milestone 0-2 (hello-world + fade) specifically, since it's cheap to catch now versus after the camera hook is built.
