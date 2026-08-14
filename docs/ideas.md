# Technical Plan: Cinematic Scene & Community Hub Framework for Skyrim

**Status:** Pre-implementation / research phase
**Purpose:** Investigation and scoping document — not final architecture

---

## 1. Vision & Scope

**Problem being solved:** Skyrim's open world has no narrative memory. Companions are static followers, not reactive characters, and the main quest loses urgency almost immediately. This project trades macro "save the world" stakes for micro, character-driven intimacy (Dragon Age 2 / Mass Effect style), anchored by a small, reactive companion hub.

**Location:** A standalone custom worldspace/interior cell, narratively placed in the mountains south of Falkreath. Deliberately isolated from vanilla cities to avoid mod conflicts and "dependency hell."

**Design philosophy:** The hub is a "chapter break" — a curated stage away from open-world chaos, not a settlement-builder or city overhaul.

---

## 2. Spatial Design — "Haven-Lite" Hub

- **Great Hall / spine:** wide navmesh, double-width doors, minimal clutter in walking lanes — built specifically to prevent actor pathing collisions and AI traffic jams.
- **Off-spine clusters:** vertical zones for visual density without blocking movement.
  - Lower tier: refugee camp, courtyard campfire, forge stations.
  - Upper tier: private quarters for major custom followers (Inigo, Lucien, Kaidan, etc.).
- **Contextual NPC matrix:** a handful of static camp NPCs (commander, healer, chef) with dialogue arrays keyed to global state, not generic loops.

**Tooling:** Creation Kit — raw cell placement, navmesh painting, Preferred Path painting only. No gameplay logic lives here.

---

## 3. Narrative System — Demeanor State Machine

Rejects a linear approval-point bar in favor of a multi-axis state machine.

- **Civil War Vector:** Imperial/Stormcloak alignment flips global variables — changes camp population and companion commentary.
- **Cruelty vs. Mercy Axis:** sub-quest resolutions tagged with hidden moral keywords, feeding into a per-companion Demeanor State (Suspicious → Fearful/Respected/Friendly, etc.).

**Confrontation loop (core reactivity mechanic):**

1. **Challenge** — companion confronts player over a major action (e.g., becoming Vampire Lord, joining Dark Brotherhood).
2. **Defense** — player responds via text-only dialogue tree (Dragon Age: Origins style).
3. **Resolution** — if the defense logically satisfies the companion's internal rules, hostility drops to reluctant rationalization rather than binary approval/abandonment.

**Open question to investigate:** what data structure represents "internal logic" per companion — a simple keyword-weight table, or something more like a rules engine? This decision affects how much writing scales per companion.

---

## 4. Technical Architecture — Scene Lifecycle Engine

**Why native code:** Papyrus's threading model can't reliably hit frame-perfect fades or hook the AI update loop. This pushes the core orchestration into an SKSE C++ plugin.

**Stack:**

- C++ via CommonLibSSE-NG, built with CMake + vcpkg
- Open Animation Replacer (OAR) for conditional gesture trees
- SkyUI SDK for custom dialogue-tree UI
- Creation Kit for geometry only (see §2)

### Phase Pipeline

`Trigger Event → Pre-Stage → Execution → Interaction → Teardown`

**Phase 1 — Pre-Stage (init/warmup)**

- Atomic lock (`bIsSceneExecuting`) to prevent re-entrancy/race conditions.
- Native frame-perfect fade-to-black.
- Direct FormID-based actor placement (`MoveTo`) — bypasses Papyrus events entirely.
- Animation warmup (poses via OAR) applied while screen is black, to avoid visible model snapping.

**Phase 2 — Execution (guardrailed stage play)**

- AI Intercept Hook: while a scene is active, the plugin hooks Skyrim's AI update loop and suppresses pathfinding for background actors in the cell, so nothing can wander into frame.
- Real-time camera cuts computed from actor vectors, breaking the vanilla fixed third-person camera.

**Phase 3 — Interaction (mute-protagonist UI)**

- Over-the-shoulder camera framing.
- Custom text-based choice lists; each choice triggers the companion's next animation beat and audio line.

**Phase 4 — Teardown**

- Fade out, release Quest Alias wrappers, restore native companion AI packages, clear global state and memory, return actors to ambient sandboxing.
- Target: zero orphan scripts, zero save-corruption footprint.

---

## 5. Risk Register (things to investigate before committing to architecture)

| Risk | Why it matters | What to investigate |
|---|---|---|
| AI Intercept Hook fails to release on crash/quit-to-menu | Could leave background actors permanently frozen or corrupt saves | Whether CommonLibSSE-NG exposes safe hooks for game-exit/load events to force-unlock the scene state outside your own Phase 4 |
| Custom follower compatibility (Inigo, Lucien, Kaidan) | These mods have their own quest alias systems and internal state tracking; "restoring core AI" is easy to claim, hard to guarantee | Read each mod's public source/API docs (Inigo and Lucien both have documented alias structures) for exactly which aliases/packages your teardown must not disturb |
| Papyrus/native boundary | Even with a C++ core, you'll likely still need some Papyrus glue for quest data | Decide the minimal Papyrus surface early so it doesn't creep back into the hot path |
| Save-game bloat/corruption from custom worldspace | New worldspaces have historically been a common source of Skyrim save issues | Test iteratively with save/load cycles from MVP stage onward, not just at the end |
| Dialogue UI scope | SkyUI SDK customization for a DA:O-style tree is nontrivial | Prototype the UI in isolation before wiring it to Phase 3 |

---

## 6. MVP Definition

Deliberately thinner than the full pipeline, to surface the hardest bugs first:

1. Single blank custom interior cell.
2. One actor (not two) hooked into the native C++ loop.
3. Prove Phase 1 alone: frame-perfect fade → hard MoveTo → animation warmup, with zero stutter and correct state on screen-up.
4. Only then add a second actor, then Phase 4 teardown, then Phase 2/3 (AI hook + camera + dialogue UI).

**Exit criteria for MVP:** scene can run start-to-finish and the actor returns to normal ambient AI with no residual script/quest alias left active, verified across a save/load cycle.

---

## 7. Open Questions to Resolve During Investigation

- What's the actual API surface CommonLibSSE-NG exposes for hooking the AI update loop, and is it stable across Skyrim SE/AE game versions?
- Does the AI Intercept Hook need per-actor granularity, or is cell-wide suppression sufficient/safer?
- How much of the Demeanor State Machine's data (per-companion "internal logic") should live in native code vs. an external data file (JSON/SKSE co-save) for easier iteration without recompiling?
- What's the realistic writing/scripting load per companion for the Confrontation Loop, given N companions and M major narrative flags?
