# Investigation Findings: Prior Art, Feasibility, and Tooling

**Status:** Research complete — feeds into `ideas.md` §5 (Risk Register) and §7 (Open Questions)
**Purpose:** Answers "has this been done, is there a better way, are we pointed the right direction" before committing to architecture.

---

## 1. Is the idea already explored?

**Verdict: essentially unexplored as an integrated system.** Every individual ingredient has a partial precedent, but nothing combines them.

| Ingredient | Closest existing analog | Gap |
|---|---|---|
| Companions reacting to player choices | **Inigo** (context banter, e.g. Dark Brotherhood armor triggers dialogue), **Lucien** (pushback on snarky/mean dialogue choices — see [review](https://skyrim.annathepiper.org/2024/10/30/mod-review-lucien/)), **Kaidan** (quest-aware, romance pacing shifts) | All are line-level flavor/banter branches, not a rules-engine that computes a graded outcome from a major decision |
| Multi-axis morality state | [**Skyrim Reputation**](https://www.nexusmods.com/skyrimspecialedition/mods/22374) (weighted good/evil score, single scalar with tiers), [**Wintersun**](https://www.nexusmods.com/skyrimspecialedition/mods/22506) (per-deity favor, multiple bars) | Reputation is one scalar, not orthogonal axes; Wintersun's bars don't interact or feed companion logic |
| Confrontation → persuasion → graded outcome | None found | No mod produces a third outcome ("reluctant rationalization") between approval and abandonment |
| Pure text (unvoiced) DA:O-style dialogue tree | UI reskins only (Convenient Dialogue UI, Smart Talk, Dynamic Dialogue Replacer) manage Bethesda's *voiced* menu | Nothing replaces the voiced menu with an authored branching argument tree. Closest adjacent tech is **SkyrimNet/Mantella/CHIM** ([GitHub](https://github.com/MinLL/SkyrimNet-GamePlugin)) — LLM-driven emergent conversation with trust/rapport tracking, but that's generative, the opposite of an authored deterministic tree |
| Companion hub / dedicated home base | [**Hall of Companions**](https://www.nexusmods.com/skyrimspecialedition/mods/154469), *My Home Is Your Home*, AFT/NFF "home base" features | Solves housing logistics only — static geography, zero state-machine or confrontation logic |

**Conclusion:** the *Confrontation Loop* and the *multi-axis Demeanor State Machine* are the genuinely novel parts of this project. The rest (hub location, dialogue UI, having Inigo/Lucien/Kaidan in a shared space) is well-trodden ground. This is worth building — it isn't a reinvention.

---

## 2. Technical feasibility (CommonLibSSE-NG)

**Verdict: feasible in principle, risky in specifics.** Every primitive the plan needs has *some* shipped precedent, but several are stitched from reverse-engineered internals rather than clean supported APIs.

| Plan requirement | Status | Evidence |
|---|---|---|
| Native FormID-based actor placement (`MoveTo`) | ✅ Confirmed real, documented | `RE::TESObjectREFR::MoveTo()` in [CommonLibSSE-NG docs](https://ng.commonlib.dev/classRE_1_1TESObjectREFR.html) |
| Native camera cuts from actor vectors | ✅ Precedented at scale | [SmoothCam](https://github.com/mwilsnd/SkyrimSE-SmoothCam), [Dynamic Camera](https://www.nexusmods.com/skyrimspecialedition/mods/45409), [FreeCameraFramework](https://github.com/staalo18/FreeCameraFramework) |
| Native frame-perfect fade-to-black | ⚠️ Partially confirmed | `Game.FadeOutGame()` is a real native call backed by `RE::FaderMenu`, but CommonLibSSE-NG exposes it only sparsely (no clean "StartFade" C++ API) — doable but requires reverse-engineering, and no mod's claimed **"frame-perfect"** guarantee could be verified either way |
| AI update-loop hook to suppress background pathing | ⚠️ Capability class real, exact shape aspirational | `RE::AIProcess` is documented and read/writable ([docs](https://ng.commonlib.dev/classRE_1_1AIProcess.html)), and mods like **Puppeteer** ([Nexus](https://www.nexusmods.com/skyrimspecialedition/mods/165086)) and **Puppeteer Master** ([Nexus](https://www.nexusmods.com/skyrim/mods/10870)) prove AI manipulation is possible — but via signature-scanned trampoline hooks on specific internal functions, not an official "AI loop" callback. There is no clean interception point as described in the plan. |
| Crash/exit-safe force-unlock of the scene state | ⚠️ Half-answered — **this is the single biggest open risk** | `SKSE::GetMessagingInterface()` exposes real, documented `kPreLoadGame`/`kPostLoadGame`/`kDataLoaded`/`kNewGame` messages that can force-cleanup on next load. **Not confirmed:** any distinct "player quit to main menu without loading" event separate from the load-transition messages. This needs direct inspection of the SKSE `Interfaces.h` message enum, not just search results, before the architecture is finalized. |
| Custom text-choice UI | ✅ Multiple real options, not just SkyUI SDK | [SkyUI SDK](https://github.com/schlangster/skyui) still works, but **SSE ImGui** ([GitHub](https://github.com/ryobg/sse-imgui)) and **SKSE Menu Framework** ([Nexus](https://www.nexusmods.com/skyrimspecialedition/mods/120352)) are real, current alternatives — arguably a more natural fit than SkyUI's Scaleform/Flash+Papyrus glue for a plugin that's already living in C++ |

**Save corruption:** general Skyrim save bloat/corruption is well documented but consistently attributed to Papyrus script/reference accumulation, not custom worldspaces or AI hooks specifically — no war stories found matching "AI hook left dangling corrupted my save." Absence of evidence, not evidence of safety.

**Action item:** before locking the architecture, directly inspect the SKSE messaging interface header for an exit/quit-to-menu event distinct from load transitions. If none exists, Phase 4 needs an explicit fallback (e.g., treat "no clean teardown ran before the next load" as the trigger, and accept that a hard crash without any subsequent load leaves the hook stuck until relaunch).

---

## 3. Scene/cutscene framework precedent

**Verdict: don't build this from scratch — borrow two existing, proven models.**

- **Creation Kit's own Scene system** (Quest → Scenes tab) already gives Phases (sequential, condition-gated), simultaneous per-actor Actions within a phase, package actions that override other packages, and alias-scoped actors where a scene auto-pauses if a participant is already in another scene ([CK wiki](https://ck.uesp.net/wiki/Scenes_Tab)). Vanilla quests use this for exactly this kind of staged mini-cutscene. **It falls short at:** no native background-actor suppression outside scene-aliased actors, no fine actor-alignment/offset system, and thin camera control (basic Action Camera markers only).
- **SexLab Framework** solved the harder version of this problem at massive scale (thousands of dependent mods, a decade-plus in production):
  - It's hybrid — Papyrus orchestration (`SexLabFramework.psc`, `sslThreadController.psc`) delegates the actual alignment math and package handling to native SKSE DLLs. Tellingly, `RealignActors()`/`ResetPositions()` in the current Papyrus layer are **empty stubs** — the real work moved to native code, with Papyrus just firing lifecycle events.
  - AI suppression isn't a loop hook or radius trick — it's a **native package-override stack**: `ActorUtil.AddPackageOverride(actor, package, priority, flags)`, with matching `RemovePackageOverride`/`ClearPackageOverride`. A priority-numbered override beats quest/faction packages and persists cleanly across saves by design ([source](https://github.com/SLP-Community/SexLab/blob/main/scripts/Source/ActorUtil.psc)). Fallout 4's **Native Animation Framework** ([GitHub](https://github.com/Deweh/Native-Animation-Framework)) generalizes the same idea fully natively — direct confirmation this is the accepted pattern.
  - Teardown (`EndAnimation(bool Quickly)`, `UnregisterForAllKeys()`) is explicit anti-orphan bookkeeping. Documented failure modes are runtime glitches (frozen actors fixable via `stopquest`, "helicopter legs" post-scene) — **not** save corruption, even at massive scale and over a decade.

**Takeaway:** borrow the CK Scene model (Phases/Actions/aliases) for orchestration structure, and borrow SexLab's **native package-override pattern** for AI suppression instead of a bespoke AI-loop hook — it's the proven-at-scale version of the same idea and sidesteps the undocumented-interception-point risk in §2. Native C++ is justified specifically for placement precision and override-priority guarantees; it is *not* justified for phase sequencing or dialogue, which vanilla Scenes already handle for free.

---

## 4. Custom follower compatibility (Inigo, Lucien, Kaidan)

**Verdict: a known landmine, not a solved problem.** This is the highest-severity finding — it directly threatens the "restore native companion AI packages" promise in Phase 4.

- **Inigo**: no SKSE API, but explicit repeated documentation: *"Do not try to control Inigo with options from other mods. You will break his brain!"* — naming UFO, AFT, EFF, NFF specifically as forbidden. Does document a manual recovery path (`sv` console command + `stopquest`/`startquest` on `InigoNpcChatMain`, plus an in-dialogue "refresh aliases" option).
- **Lucien**: documented as running "its own custom follower system," explicitly incompatible with EFF and AFT. No published alias/AI API.
- **Kaidan**: bluntest of the three — [FAQ](https://kaidanmod.com/faqs-and-compatibility/): *"Can I add Kaidan to a Follower Framework? Absolutely NOT – this will 100% break his AI."*
- **NFF, EFF, AFT, UFO all explicitly blacklist these three followers** rather than managing them — a utility mod ("SPID – NFF Add Ignore Token to CustomAI Followers") exists solely to make NFF *ignore* them.
- Documented real-world precedent of exactly this failure mode: Sofia has a known bug where a stray "guard AI package" gets attached and stops her following; Kaidan has documented AI lockups after unrelated scripted events. The underlying mechanism (external package pushed onto the follower's alias, native quest never regains authority) is exactly what this project's Phase 1/4 would be doing.
- The one credible precedent for safely staging a named custom follower into a scripted event (**Khajiit Will Follow** × Vigilant/Forgotten City patches) only worked because the follower's own author co-built the patch.

**Recommended path:** design the Confrontation Loop to be **follower-agnostic**, not hard-dependent on Inigo/Lucien/Kaidan's internal alias systems:
1. Detect "is this actor currently a PlayerFollower" via the vanilla follower faction/alias — don't touch each mod's custom alias namespace.
2. Use the SexLab-style **native package-override** (§3) stacked *on top of* the base-game follower alias, not a replacement of it — never write to or clear an Inigo/Lucien/Kaidan-namespaced alias directly.
3. Simply stop forcing the override on teardown and let each mod's own high-priority AI resume on its own schedule.
4. Treat compatibility with these three as **best-effort, may glitch, user can console-recover** — consider shipping Inigo's own documented recovery command pattern as a player-facing fallback rather than promising guaranteed clean restoration.

---

## 5. Tools inventory — needed vs. exists in the community

| Need | Exists today? | Tool / library | Notes |
|---|---|---|---|
| C++ SKSE plugin scaffolding | ✅ Yes | CommonLibSSE-NG + CMake + vcpkg | Standard, actively maintained toolchain |
| Reverse-engineered game structures (AIProcess, TESObjectREFR, FaderMenu, PlayerCamera) | ✅ Yes | [CommonLibSSE-NG](https://ng.commonlib.dev/) | Documented but incomplete in places (fade menu, AI loop) — expect to extend it |
| Address stability across game updates | ✅ Yes | Address Library for SKSE Plugins | Mandatory dependency; SexLab itself pins to it |
| Native actor placement | ✅ Yes | `RE::TESObjectREFR::MoveTo` | Ready to use as-is |
| Native camera control | ✅ Yes (as reference code) | SmoothCam, Dynamic Camera, FreeCameraFramework (open source) | Don't reimplement — read/borrow their camera-cut approach |
| Native AI suppression during a scene | ✅ Yes — **use this instead of a custom AI hook** | SexLab's `ActorUtil.AddPackageOverride` pattern / Fallout 4's Native Animation Framework as a fully-native reference implementation | Directly reusable pattern; avoids inventing an undocumented interception point |
| Conditional gesture/animation trees | ✅ Yes | Open Animation Replacer (OAR) | Already in the plan, confirmed actively maintained |
| Custom in-game dialogue UI | ✅ Yes — pick one | SkyUI SDK (Scaleform/Flash), **or** SSE ImGui, **or** SKSE Menu Framework | For a C++-native plugin, ImGui/Menu Framework avoids the Scaleform+Papyrus glue layer entirely — worth prototyping both before committing |
| Phase/trigger orchestration scaffolding | ✅ Yes — free, built into the game | Creation Kit **Scenes** (Quest → Scenes tab: Phases, Actions, aliases) | Use this for structure/sequencing; only drop to native code for the pieces vanilla can't do (placement precision, override priority, camera) |
| Save/load lifecycle hooks for cleanup | ✅ Partially | `SKSE::GetMessagingInterface()` — `kPreLoadGame`, `kPostLoadGame`, `kDataLoaded`, `kNewGame` | Confirmed real; **missing piece:** a distinct "quit without loading" event — needs direct header inspection (§2 action item) |
| Follower AI takeover/handback (generic) | ⚠️ Exists but excludes exactly the followers this project needs | NFF, EFF, AFT, UFO | All four explicitly blacklist Inigo/Lucien/Kaidan — don't depend on them for this project's specific followers; build the narrower package-override approach instead (§4) |
| Follower AI takeover (Inigo/Lucien/Kaidan specifically) | ❌ Does not exist | — | No mod, framework, or published API solves this; treat as the project's own R&D, scoped down via the follower-agnostic approach in §4 |
| Multi-axis morality/reputation engine | ❌ Does not exist as multi-axis | Skyrim Reputation (single scalar), Wintersun (multiple non-interacting scalars) | Nothing to reuse here — this is genuinely new work, as intended |
| Confrontation/persuasion dialogue tree with graded outcomes | ❌ Does not exist | — | Also genuinely new work — this is the project's actual contribution |

---

## 6. Overall direction check

- **Keep:** the core insight (multi-axis Demeanor State + Confrontation Loop) is real, unclaimed territory — worth building.
- **Change:** replace the bespoke "AI Intercept Hook" concept with SexLab's proven native package-override pattern (§3) — same effect, far less architectural risk, and it sidesteps depending on an undocumented AI-loop interception point.
- **Change:** don't hand-roll Phase/Trigger orchestration in C++ — use Creation Kit's native Scene system for structure and only drop into C++ for placement precision, override priority, and camera control, which is where vanilla genuinely falls short.
- **De-risk before committing:** verify directly against the SKSE messaging header whether a "quit to menu" event exists distinct from load-transition messages (§2) — this determines whether Phase 4's "zero orphan state" guarantee is achievable as stated or needs a fallback design.
- **Rescope the follower promise:** stop promising guaranteed clean AI restoration for Inigo/Lucien/Kaidan. Design generically against "is this actor a follower," ship a console-recovery fallback, and document compatibility as best-effort — this is the one place the original plan was overconfident against explicit, repeated warnings from the follower mods' own authors.
