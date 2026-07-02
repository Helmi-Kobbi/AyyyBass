# AyyyBass — North African Acid Groove Generator

**Design spec — v1**
Date: 2026-06-27
Status: brainstorm complete, pending implementation plan

---

## One line

A one-button instrument that instantly generates hypnotic North African basslines. Press the acid-house smiley → get a groove. Don't like it → press again. Like it → keep it. Zero programming.

## The soul

Trance music didn't start in Berlin — it came from the *lila* and the *zawiya* too. AyyyBass puts the acid-house smiley and the Gnawa *guembri* in the same box. It removes the decision fatigue and the pattern-programming chore, so a musician gets straight to a usable, hypnotic, culturally-rooted bassline. It is the instrument its creator wished existed.

## What it is / is not

**Is:** a generative groove instrument · a taste-filtered idea machine · a live MIDI instrument · a minimal, joyful, one-gesture interface.

**Is not:** a sequencer with a piano roll · a DAW replacement · a manual composition tool · a parameter-heavy synth controller.

## Core interaction loop

1. Press the big yellow smiley → the brain generates a bassline.
2. It plays immediately, locked over a simple, mutable canvas beat.
3. Press again → a fresh line. **Mutate** → keep this vibe, nudge it. **♥ Keep** → save to a recall slot.

The human's thumb is the whole judgement system: press-again = "no", keep = "yes".

## Musical design — the "North African acid" brain (v1)

- **Ternary-native feel from day one** — 6/8, 12/8, triplet subdivision. Not straight-16ths with swing bolted on. The timing model must support binary *and* ternary grids natively.
- **12-TET modes (v1)** — pentatonic (Gnawa-rooted, zero microtones) plus 12-tone maqam such as Hijaz, Kurd, Nahawand. Convincingly North African on any standard synth.
- **Slide is the expressive core** — the North African glissando; the exact thing that makes the TB-303 a natural fit for Arabic phrasing (cf. Acid Arab). Not "an acid effect" — *the* gesture.
- **Accent** — via velocity or CC.
- **Free pattern length** — odd lengths (5, 7, 9…) drift against the canvas → polymeter/phasing. A signature behavior.
- **Density** — sparse-and-hypnotic ↔ busy-and-rolling.
- **Long hypnotic cycles** — Gnawa patterns commonly breathe over 8–12 bars; the engine should embrace long evolving cycles, not just 1-bar loops.
- **The guembri ostinato is the bass model** — repetitive, percussive, hypnotic, ternary.

**Deferred (mechanism chosen):** true microtonal maqam (Bayati, Saba, Rast). The chosen path is using the **303's own slide to bend into the quarter-tones** (the move Acid Arab exploits), with MPE / MTS-ESP as the option for synths that support per-note tuning.

## Pattern data model

Per step: `note`, `velocity`, `accent` (bool/intensity), `slide` (bool), `rest`, `micro-timing offset`.
Per pattern: `length` (free), `subdivision` (binary/ternary), `scale/mode`, `density`.

Kept deliberately a touch more general than acid strictly needs — it is the long-lived contract shared by every future brain. Generation is **seed-based / deterministic** where possible, so a pattern has a stable number that can be recalled and shared.

## The canvas beat

A simple, *mutable* backing groove so the bassline is heard in context (and demos well). Either a 4/4 kick+hats or a North African groove (qraqeb shuffle / bendir / darbuka feel). It is a dumb canvas, not generated intelligence. Can be muted for "just bass" in a live rig.

## Architecture — body + swappable brain

- **Body (fixed):** the button, the canvas beat, sound (software) / MIDI out (hardware), favoriting, recall slots, the three params (scale · length · density), and the animated face.
- **Brain (swappable) = just the generation algorithm, a layer above the body.** Like a record on a deck — swap the record, the deck never changes. Given params + musical context, the brain answers `generate() → pattern` and `mutate(pattern) → pattern`. The body knows nothing about acid, North Africa, or any genre — it only asks for a pattern and plays it. All musical knowledge lives in the brain. v1 ships **one** brain: North African acid.
- **v1 eats its own dog food.** That single brain is built behind the *real* contract — the same slot a future third-party brain would use. This is the cheap insurance that makes everything below additive instead of a rewrite.
- **The pattern data model is the long-lived contract.** It must stay expressive enough for styles beyond acid, so we keep it a touch more general than acid strictly needs (see Pattern data model). Changing it later breaks every brain — so we get it right now. This is the *only* real architectural cost of planning for many brains, and it is small.
- **Many brains = a selectable library, one active at a time** (the styles menu: Acid, Detroit, Electro, Dub, Minimal, Berlin… alongside our North African brain). The active brain sets the personality shown on the OLED. Running several brains *simultaneously / layered* is a separate, much harder idea (they fight over the same bass voice) — deliberately out of scope, and the one thing here that genuinely *would* change the architecture, so it is not promised.
- **The extensibility ladder (honest):** (0) v1 — one brain behind the clean slot. (1) later — more brains added in code by developers writing to the contract. (2) harder — brains as loadable plugins with no rebuild (genuinely constrained on a microcontroller). (3) the dream — a *musician-friendly* authoring tool so non-coders make brains (a product of its own). Climbing rung 0 correctly in v1 keeps every higher rung reachable.
- **The contribute-your-own-style ecosystem lives in software/web first.** Sharing and running community brains is easy in software; on the ESP32, early reality is a *curated set flashed in*, not drag-and-drop plugins. Another reason software leads. *One body, infinite brains* is the destination; software is the on-ramp.
- **No plugin system / marketplace / authoring tool in v1.** We build one brain and the clean slot it sits behind — nothing more.
- **One portable core.** The engine is written once in a portable language and shared by the software prototype and the ESP32 firmware — never a Python-core / C++-firmware split. Leaning: a C++ (or Rust) core compiled to **wasm for a browser playground** (the play-in-days front-end) and later to **ESP32-S3** firmware. Exact language/build is the author's call, to be fixed in the implementation plan.

## "Good groove" philosophy

- The human is the fitness function. No objective "groove-quality oracle."
- Software does only **light garbage-rejection** (never hand the player silence, 16 identical notes, or a rhythmically dead line).
- Effort goes into **generator musicality**, not a scorer. **No LLM in the live path** (too slow, network-dependent, unproven).
- **Deferred:** "the box learns your taste over a session" (the ♥ button as training signal). Phase 2.

## UI identity — the smiley

- The acid-house smiley is the brand *and* the big yellow button (the star; very visible in a rack of black gear; great Superbooth eye-candy).
- **Each brain has its own personality face** (Acid, Detroit, Dub … each its own character). The OLED face reflects the *active* brain; the yellow acid smiley is both the v1 brain's face and the overall brand mark.
- The **OLED carries two glanceable views** you flip between: (1) the **personality face** emoting through states — idle → generating → got one → kept it → mutate (default, ambient); (2) a **pattern-shape glance** — the step blocks (you literally *see* the polymeter length sitting shorter than the 16-step beat), accent and slide dot-rows, and a pattern number. Plus the three params on the encoder.
- **Glanceable, never editable.** Reading the pattern is fine; the moment the screen invites step-by-step editing, it betrays the instrument.
- If generation is **seed-deterministic**, the pattern number becomes a *shareable groove address* — recall it, or share a seed with a friend or the community.
- Physical concept: big yellow arcade button · OLED · encoder · ♥ keep button · MIDI out · USB-C.

## MIDI / targets

Notes + velocity (accent) + slide (legato/overlap; pitch-bend fallback) + optional CC.

**User-selectable MIDI output / routing** — the player picks where notes go. This is a *live-output* feature of the bodies, not the file: the web playground via the Web MIDI API (a dropdown of output ports), and the hardware via DIN / USB-MIDI. (The MIDI-file export in Plan 1 is "routed" by the host DAW.)

Targets: Behringer **TD-3-MO** (Devil Fish-style mod — wider slide/accent/filter plus extra MIDI CCs we can drive for expression; **primary target**) · TD-3 & clones · Crave · Edge/Spice · Roland-style analog · DAW (Ableton/Bitwig) · Eurorack via MIDI-to-CV.

## Build order

1. **Software prototype first** — makes its own sound, wears this exact screen layout. Prove the grooves feel good within days.
2. **Port the same portable engine to ESP32-S3 hardware** — for Superbooth.

## Why it's unique (from competitive research)

- One-button acid generation exists ([Aciduino](https://github.com/midilab/aciduino)) — but 4/4 Western, a closed firmware app, not a forkable brain.
- Controlled-randomness / lock-the-good-one exists (Music Thing Turing Machine, Mutable Marbles/Grids) — 4/4 Western, no cultural voice.
- Generative groove software exists (Audiomodern Riffer/Playbeat, Magenta GrooVAE) — desktop, Western, not embeddable.
- Algorithmic hardware sequencers exist (Torso T-1) — Euclidean/Western, not one-button, not North African.
- Acid + North African fusion exists as *music* (Acid Arab) — as records, not as a generative *instrument*.
- **A generative box rooted in North African ternary grooves does not exist.** That is the wedge.

## v1 success metric

A press produces a bassline that feels intentional, is ternary/North African in character, is playable live immediately, is hypnotic (surprising but not chaotic), and makes you *not* want to edit it — often enough that you keep pressing with a grin.

## Deferred / out of scope for v1

Microtonal maqam · taste-learning · other brains / plugin system / marketplace · brain-authoring tool · multiple brains running simultaneously · drum/melody/modulation brains · pattern chaining / song mode.

## Open questions for the implementation plan

1. Exact engine language and build pipeline (core author's call; leaning C++/Rust → wasm playground → ESP32).
2. How the software prototype makes sound (built-in lightweight synth vs. driving an external/soft 303).
3. How many North African canvas-groove presets ship in v1.
4. The precise body↔brain interface signature (kept minimal, but written down so the first fork is possible).
