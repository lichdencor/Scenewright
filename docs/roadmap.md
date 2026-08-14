# Roadmap: Prioritized Work Items by Standalone Project

**Status:** Direction confirmed after `investigation-findings.md` — supersedes the phase ordering in `ideas.md` §6 (MVP Definition).

**Reprioritization rationale (from discussion):**
- The actual differentiator and hardest technical risk is the **camera/cinematics system**, not the dialogue or morality logic. It should be proven standalone, with zero dependency on dialogue, morality, or any custom follower.
- The Confrontation Loop and Demeanor State Machine can be proven with **dummy/mock NPCs built for this project**, not Inigo/Lucien/Kaidan. Vanilla-safe, author-controlled, no alias systems to fight.
- Missing voice acting for dummy NPCs is not a blocker: **Fuz Ro D'oh** (lets dialogue run without matching voice files, so subtitle-only text NPCs don't hang the game waiting on audio) and **vanilla-voice-file sentence mixing** (stitching existing NPC bark/line audio into new sentences) are both viable first-pass options before investing in full voice acting.
- The native **AI Intercept Hook is scrapped**. Dummy NPCs don't carry the complex alias/package stacks that made this risky in the first place, so suppression can be scoped down or deferred.
- **Custom follower (Inigo/Lucien/Kaidan) integration is scrapped from the active roadmap entirely and pinned to the backlog/icebox.** It is not a scheduled phase — it only gets picked up, if ever, after a working end-to-end build exists, and it shifts from "reverse-engineer their alias system" to "ask the mod authors for support once there's something to show them."

**Tracking:** this roadmap is mirrored in Trello (board "Scenewright"). Epic 1 (Camera & Scene Engine) is live in the "To do" list with its task checklist. The custom-follower AI work lives as a single card in a dedicated "Backlog" list, out of the active workflow.

---

## Priority order

| Rank | Project | Depends on | Why this order |
|---|---|---|---|
| **P0** | Cinematic Camera & Scene Engine | Nothing else | The core, unclaimed technical contribution — prove it works in total isolation first |
| **P1** | Dummy NPC Confrontation POC | P0 (reuses its fade/placement/scene plumbing) | Proves the Confrontation Loop mechanic is playable at all, on safe/disposable NPCs |
| **P1** | Demeanor State Machine | Can run in parallel with the row above, same dummy NPCs | Proves the multi-axis logic without waiting on P0 to fully finish |
| **P2** | AI Suppression / Package-Override layer | P0 + P1 working together in one scene | Only needed once a scene has more than one actor or a populated background cell |
| **Backlog** | Custom Follower Integration (Inigo, Lucien, Kaidan) | Not scheduled | Scrapped from the active roadmap, pinned as an icebox item — only revisited if a working end-to-end build exists and it's worth asking mod authors for collaboration |

---

## P0 — Cinematic Camera & Scene Engine *(top priority, standalone)*

| Task | Notes |
|---|---|
| Stand up minimal SKSE plugin skeleton (CommonLibSSE-NG + CMake + vcpkg) | No dialogue, no morality, no custom followers — camera only |
| Reverse-engineer / wrap `RE::FaderMenu` for native fade-to-black | CommonLibSSE-NG exposes it sparsely; confirm actual frame timing against the game's update loop |
| Implement native `MoveTo`-based actor placement for one dummy actor | Confirmed-safe API, low risk — do this first to get a target on screen |
| Prototype non-native camera cuts computed from actor vectors | Study SmoothCam, Dynamic Camera, and FreeCameraFramework source as reference implementations rather than starting from zero |
| Define the camera "shot vocabulary" for this project | e.g. over-the-shoulder, two-shot, reaction cut — decide what cinematographic language the engine needs to support before generalizing the code |
| Wire Trigger → Pre-Stage → Camera-Cut phases using Creation Kit's native Scene system for sequencing | Don't hand-roll phase/trigger logic in C++ — vanilla Scenes (Phases/Actions/aliases) already do this; C++ only handles what Scenes can't (placement precision, camera) |
| Exit criteria | A single dummy actor, in a blank cell, goes through fade → placement → at least two distinct non-vanilla camera angles → fade out, with zero stutter, verified across a save/load cycle |

---

## P1 — Dummy NPC Confrontation POC

| Task | Notes |
|---|---|
| Build 1–2 disposable custom NPCs in Creation Kit for this project | Not Inigo/Lucien/Kaidan — full author control, no third-party alias systems, safe to break |
| Design the confrontation dialogue tree data structure | Keyword-weight table first (simpler, faster to prototype) — defer "rules engine" complexity until this proves too limited |
| Build the text-only choice UI | Prototype both SkyUI SDK and SSE ImGui / SKSE Menu Framework against this dummy scene before committing to one |
| Solve the missing-voice-acting problem for the first trial | Try, in order of effort: (1) Fuz Ro D'oh to allow text-only lines with no audio at all, (2) sentence-mixed vanilla voice files stitched into approximate lines, (3) full custom voice acting — only if (1)/(2) prove insufficient |
| Wire one full Challenge → Defense → Resolution loop on a dummy NPC | Confirms the mechanic is fun/legible before any morality-axis complexity is added |
| Exit criteria | Player can trigger a confrontation with a dummy NPC, pick a text response, and see a resolution outcome — using P0's camera/scene plumbing — verified across a save/load cycle |

---

## P1 — Demeanor State Machine

| Task | Notes |
|---|---|
| Decide storage: native code vs external JSON/SKSE co-save | Favor external data first — faster iteration on companion "internal logic" without recompiling, per the open question in `ideas.md` §7 |
| Implement the two initial axes: Civil War Vector, Cruelty vs Mercy | Keep to two axes until the dummy NPC proves the interaction model is legible |
| Tie axis state into the P1 Confrontation POC's Resolution step | This is the actual integration point — confirm the "reluctant rationalization" graded outcome is achievable, not just binary |
| Exit criteria | A dummy NPC's confrontation outcome visibly changes based on prior recorded axis state, without recompiling the plugin to test a tuning change |

---

## P2 — AI Suppression / Package-Override layer

| Task | Notes |
|---|---|
| Implement a SexLab-style native package-override (not an AI-loop hook) | `AddPackageOverride`/`RemovePackageOverride` pattern, priority-numbered — proven at scale, avoids the undocumented AI-loop interception risk flagged in `investigation-findings.md` §2 |
| Scope suppression to the dummy actors in P0/P1's test cell only | Cell-wide background suppression is a later concern — don't solve it before there's a populated cell that needs it |
| Verify SKSE load-lifecycle messages (`kPreLoadGame`/`kPostLoadGame`/`kDataLoaded`/`kNewGame`) as the force-cleanup path | Directly inspect the SKSE messaging header for a distinct "quit to menu without loading" event — if none exists, design Phase 4's fallback around "no clean teardown before next load" as the trigger |
| Exit criteria | Two dummy actors in a scene, one suppressed via override while the other performs; override cleanly releases on scene end AND on a simulated crash/quit-to-menu, verified across a save/load cycle |

---

## Backlog — Custom Follower Integration (scrapped from active plan, icebox only)

| Task | Notes |
|---|---|
| Package a demo build showing P0–P2 working end-to-end on dummy NPCs | This is the artifact you bring to mod authors — not a design doc, a running scene |
| Reach out to Inigo, Lucien, and Kaidan's authors directly | All three currently warn explicitly against external AI takeover (see `investigation-findings.md` §4) — treat this as a collaboration ask, not a reverse-engineering target |
| Adopt each author's own documented recovery pattern as the fallback | e.g. Inigo's console `stopquest`/`startquest` on `InigoNpcChatMain` — ship this as a player-facing "if this glitches" fallback rather than promising guaranteed clean restoration |
| Exit criteria | Explicit sign-off or patch collaboration from at least one of the three authors before shipping any scene that uses their follower |
