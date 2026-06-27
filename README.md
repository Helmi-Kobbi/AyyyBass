# AyyyBass

> One button. Press it → a hypnotic North African acid bassline. Don't like it → press again. Like it → keep it. Zero programming.

**Status:** design complete · implementation plan in progress
**Build order:** software prototype first → ESP32-S3 hardware (Superbooth)

## What it is

A one-button generative bassline instrument with a **"Maghrebi acid"** soul — ternary (6/8, 12/8) hypnotic, guembri-rooted basslines voiced through the acid-303 squelch and slide. The acid-house smiley is both the brand and the button.

It is built as a **body + swappable brain**: the body (button, beat, sound/MIDI, screen) never changes; the *brain* is just the generation algorithm, swappable like a record on a deck. v1 ships one brain (Maghrebi acid); the clean body↔brain contract is the open-source seed so other musicians can later fork brains for their own styles. *One body, infinite brains.*

## What it is not

Not a sequencer with a piano roll. Not a DAW replacement. Not a parameter-heavy synth controller.

## Roadmap (high level)

- **v1 software** — playable web "button" that proves the grooves feel good (one Maghrebi-acid brain).
- **v1 hardware** — ESP32-S3 box (big yellow button, OLED, encoder, MIDI out) for Superbooth.
- **later** — more brains (styles), taste-learning, microtonal maqam, a musician-friendly brain-authoring framework.

## Repo layout

- `docs/superpowers/specs/` — design specs (source of truth)
- `docs/superpowers/plans/` — implementation plans
- engine / web playground / firmware land here once the plan is set

## Design spec

[docs/superpowers/specs/2026-06-27-ayyybass-design.md](docs/superpowers/specs/2026-06-27-ayyybass-design.md)

## License

TBD — open source intended.
