# Epic 1: Cinematic Camera & Scene Engine — Implementation Evidence & Build Order

**Status:** Ready to start. This document turns the Trello checklist for Epic 1 into a concrete, evidence-backed build order.
**Sources:** open source SKSE/CommonLibSSE-NG plugins, CK Wiki, skyrim.dev, and direct GitHub source reads (SmoothCam, FreeCameraFramework, PapyrusExtenderSSE, osf-animation, others — linked inline).

---

## 0. What this buys us

Four separate unknowns from `investigation-findings.md` are now resolved with actual function names, offsets, and reference source to copy from — not just "this is probably possible." Nothing below is invented; every claim is either confirmed from a real repo/doc (marked ✅) or explicitly flagged as inferred (marked ⚠️).

---

## 1. Milestone 0 — Toolchain & Hello World

**Fork:** [`SkyrimDev/HelloWorld-using-CommonLibSSE-NG`](https://github.com/SkyrimDev/HelloWorld-using-CommonLibSSE-NG) ✅ — simplest working plugin, CMake + `CMakePresets.json` + vcpkg manifest mode, targets SE/AE/GOG/VR through one dependency. Featured in the current [UESP SKSE plugin dev guide](https://skyrimck.uesp.net/wiki/Getting_Started_with_SKSE_Plugin_Development) (updated 2024-11-23).

Steps:
1. Install Visual Studio 2022 (Community) with the "Desktop development with C++" workload.
2. Clone `vcpkg` anywhere, run `bootstrap-vcpkg.bat`, set `VCPKG_ROOT`.
3. Set `SKYRIM_FOLDER` / `SKYRIM_MODS_FOLDER` env vars to point at the game/MO2 mods path.
4. Clone the template, open the folder in VS2022, let CMake+vcpkg auto-configure (first run is slow — it pulls CommonLibSSE-NG).
5. Build (F7), confirm the DLL lands in the mods folder.
6. Launch via `skse64_loader.exe`, enable the plugin, load a save, confirm a log line / console message appears.

Once this works, read (don't yet imitate) [`mwilsnd/SkyrimSE-SmoothCam`](https://github.com/mwilsnd/SkyrimSE-SmoothCam) and [`alandtse/CrashLoggerSSE`](https://github.com/alandtse/CrashLoggerSSE) as structural references before writing camera code.

Secondary template worth a skim once past hello-world: [`epinter/skse-clibng-template`](https://github.com/epinter/skse-clibng-template) (auto-deploy/launch hooks). Skip [`libxse/commonlibsse-ng-template`](https://github.com/libxse/commonlibsse-ng-template) despite it being the most actively maintained — it's XMake-based, not CMake, and breaks compatibility with the CMake-based tutorials/tooling everything else here assumes.

---

## 2. Native fade-to-black

✅ **Confirmed pattern (from 6+ independent open source plugins): call the engine function directly by relocation offset. Do not go through the Papyrus VM.** No precedent found anywhere of a plugin dispatching `Game.FadeOutGame` via the VM — every real plugin bypasses Papyrus entirely.

- Papyrus signature (for reference only): `FadeOutGame(bool abFadingOut, bool abBlackFade, float afSecsBeforeFade, float afFadeDuration)` — [CK Wiki](https://ck.uesp.net/wiki/FadeOutGame_-_Game)
- Real underlying engine signature, confirmed identically across [ArranzCNL/SkyrimSE-Plugin-Template](https://github.com/ArranzCNL/SkyrimSE-Plugin-Template), [Exit-9B/CustomSkills](https://github.com/Exit-9B/CustomSkills), [doodlum/skyrim-capture-warmer](https://github.com/doodlum/skyrim-capture-warmer), [tiltedphoques/TiltedEvolution](https://github.com/tiltedphoques/TiltedEvolution):
  ```cpp
  void FadeOutGame(bool aFadingOut, bool aBlackFade, float aFadeDuration, bool aHoldFade, float aSecondsToFade);
  ```
  at Address Library ID **52847 (SE)** / **51909 (AE)**. **CommonLibSSE-NG does not ship this offset — define it yourself**, same as every plugin above does (e.g. `RE::Offset::FadeOutGame = REL::ID(52847)`).
- `RE::FaderMenu` itself exposes no fade-control API in CommonLibSSE-NG ([ng.commonlib.dev](https://ng.commonlib.dev/class_r_e_1_1_fader_menu.html)) — it's a Scaleform menu reacting to `UIMessage`/`UIMessageQueue`, confirming direct-call is the only practical route.

**⚠️ Confirmed crash pitfall** (direct source comment in [ozooma10/osf-animation `FadeService.cpp`](https://github.com/ozooma10/osf-animation/blob/main/src/UI/FadeService.cpp), which solves nearly this exact "Pre-Stage fade" problem): **holding `aHoldFade=true` across a save/load boundary crashes the game.** Any hold must be deadline-bounded and force-released before a load can proceed. That plugin also runtime-verifies the function's byte prologue before calling it, since the offset can silently shift across game builds.

**Recommended verification approach:** don't assume fade completion from a fixed timer — check `RE::UI::GetSingleton()->IsMenuOpen(RE::FaderMenu::MENU_NAME)` before proceeding to actor placement, to avoid a placement-during-fade visual pop (the race the original plan flagged).

---

## 3. Native camera cuts

✅ **Confirmed from source: don't write a custom `RE::TESCameraState` subclass. Vtable-detour the existing state's `Update`, call the original, then overwrite its position/rotation fields.** No shipped mod was found pushing a genuinely new camera-state class into `PlayerCamera::cameraStates[]`; `TESCameraState` is subclassable in principle, but every real mod studied mutates the engine's existing state objects instead — inferred reason: `PlayerCamera` and other engine code index/`static_cast` those fixed-size slots by enum id, and nobody has documented safely bypassing that.

**Reference implementation to copy from: [SmoothCam](https://github.com/mwilsnd/SkyrimSE-SmoothCam)** —
- `source/hooks.cpp`: builds a `PolymorphicVTableDetour<RE::TESCameraState, 13>`, detours vfunc slot 3 (`Update`) and slot `0xF` (`HandleLookInput`) directly on the live state objects in `cameraStates[]`.
- `source/camera.cpp`: `Camera::SetPosition` calls the original vanilla `Update` first, then `reinterpret_cast`s the current state to `RE::ThirdPersonState*` and writes `translation` plus the `NiCamera`/camera-root node transforms directly.
- Notably already has a `ThirdpersonDialogueState : public BaseThird` internally (`camera_states/thirdperson/thirdperson_dialogue.h`) — the closest existing precedent to a "conversation camera" mode, worth reading directly.

**For hard cuts specifically (not smooth follow-cam):** copy [FreeCameraFramework](https://github.com/staalo18/FreeCameraFramework)'s interpolation switch — `InterpolationMode{kNone, kLinear, kCubicHermite}`, where `kNone` writes the raw target position/rotation with no easing (an instant snap = a cut). Its `TimelineManager::StopPlayback` is also a clean model for exit/restore: call the real exit function, restore saved FOV/rotation flags, and never allocate a custom state object, so cleanup has nothing exotic to undo.

**⚠️ Open decision — standalone hook vs. soft-depend on SmoothCam:**
- SmoothCam's own README states plainly: *"SmoothCam is going to have issues with any other mod that tries to position the third-person camera."* It exposes a `SmoothCamAPI.h` handoff contract (`RequestCameraControl`/`ReleaseCameraControl`) specifically so other plugins don't fight it — and the closest real conceptual precedent, **Cinematic Conversation Camera** (Nexus #187087, closed-source), reportedly *requires* SmoothCam and borrows the camera through it rather than hooking independently.
- **Recommendation: implement our own standalone vtable-detour for the MVP**, matching this project's stated design philosophy in `ideas.md` §1 ("deliberately isolated... to avoid mod conflicts and dependency hell"). Revisit SmoothCam-as-soft-dependency later only if standalone detouring proves to fight other popular camera mods in practice.

---

## 4. Creation Kit Scene → native hand-off

✅ **Confirmed mechanics** (CK Wiki [Scenes Tab](https://ck.uesp.net/wiki/Scenes_Tab), corroborated by a real decompiled example in [Rukan/Grimy-Skyrim-Papyrus-Source](https://github.com/Rukan/Grimy-Skyrim-Papyrus-Source)):
- A Scene = ordered Phases. Each Phase has a **Start fragment** and a **Completion fragment**; the engine will not advance to the next Phase until the Completion fragment finishes running. These compile into sequentially-numbered `Fragment_0()`, `Fragment_1()`, … functions on the quest's autogenerated script — not literally named `OnPhaseBegin`.
- A separate native `Scene` script object (`GetActorAlias`, `IsComplete`, `ForceStart`/`Stop`) can also drive/poll scene state from outside the fragment if needed.

✅ **Confirmed native-Papyrus registration pattern**, standard SKSE idiom, shipped in [PapyrusExtenderSSE](https://github.com/powerof3/PapyrusExtenderSSE) (374 native functions) and documented at [skyrim.dev/skse/native-papyrus-functions](https://skyrim.dev/skse/native-papyrus-functions):
```cpp
static void BeginCameraCut(RE::StaticFunctionTag*, RE::Actor* akA, RE::Actor* akB, std::int32_t aiPhase) { /* ... */ }

bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
    vm->RegisterFunction("BeginCameraCut", "SW_CameraAPI", BeginCameraCut);
    vm->RegisterFunction("EndCameraCut", "SW_CameraAPI", EndCameraCut);
    vm->RegisterFunction("IsCameraCutActive", "SW_CameraAPI", IsCameraCutActive);
    return true;
}
```
```papyrus
ScriptName SW_CameraAPI
Function BeginCameraCut(Actor akA, Actor akB, int aiPhase) Global Native
Function EndCameraCut() Global Native
Bool Function IsCameraCutActive() Global Native
```

✅ **Confirmed structural boundary:** Quests, Scenes, Phases, Actions, Actor Aliases, and dialogue Topics can only be authored in CK/Papyrus — there's no way to define these from pure C++, only to read/manipulate already-authored instances. Everything downstream of "phase X started" (placement, camera math, AI suppression) can and should be native.

✅ **Confirmed reason native must own timing, not react to Papyrus events:** Papyrus runs on its own time-budgeted thread (`fUpdateBudgetMS`), not per-frame with the renderer — even `Utility.Wait(0.0)` isn't guaranteed to return on the next render frame. A Papyrus fragment firing at phase-begin cannot be trusted for frame-accurate camera timing.

**Recommended design:** Papyrus/Scene layer does the absolute minimum — define the Scene/Phases/Aliases in CK, and in each Phase's Start fragment call one thin native trigger (`SW_CameraAPI.BeginCameraCut(akA, akB, iPhase)`). No waits, no polling loops in Papyrus. The Phase's Completion fragment can gate advance on `SW_CameraAPI.IsCameraCutActive()` returning false. Everything else — fade, placement, camera math — lives entirely in native code, timed against the game's own update/render hooks.

---

## 5. Build order (maps to the Trello checklist)

1. **Milestone 0** — Hello World plugin loads, logs a message. *(toolchain proven)*
2. **Native Papyrus API skeleton** — register `BeginCameraCut`/`EndCameraCut`/`IsCameraCutActive` as no-ops that just log; wire a throwaway CK quest/Scene with one Phase to call them, confirm the Papyrus→native call actually fires.
3. **Native fade-to-black** — implement the direct relocation call (§2), trigger it from `BeginCameraCut` for now, confirm timing and that a save/load during a held fade doesn't crash (test this explicitly — it's the confirmed pitfall).
4. **Native actor placement** — `MoveTo` a dummy actor during the held-fade window; confirm no visible pop by gating on `FaderMenu` closed-state rather than a fixed timer.
5. **Native camera hook** — implement the SmoothCam-style vtable detour on the active `TESCameraState`, write a hard-cut position/rotation (FreeCameraFramework's `kNone` mode) computed from the dummy actor's vectors; implement clean restore on `EndCameraCut`.
6. **Wire it all through a real CK Scene** — Start fragment calls `BeginCameraCut`, Completion fragment gates on `IsCameraCutActive()`.
7. **Exit criteria (per Trello):** single dummy actor, blank cell, fade → placement → 2+ distinct non-vanilla camera angles → fade out, zero stutter, verified across a save/load cycle.

---

## 6. Risks carried forward

- Fade-hold-across-save-load crash (§2) — must be tested explicitly, not just coded around.
- Address Library offsets (`FadeOutGame`, camera vtable layout) can silently shift across game builds — the osf-animation plugin's practice of runtime-verifying the function prologue before calling it is worth adopting.
- Standalone camera detouring may eventually conflict with other camera mods a player has installed (SmoothCam's own README flags this as a near-certainty for any mod touching third-person positioning) — acceptable for MVP per this project's isolation-first philosophy, revisit if it becomes a real compatibility complaint.
