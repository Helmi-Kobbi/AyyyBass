<div align="center">

<img src="docs/assets/ayyybass-smiley.svg" width="160" alt="AyyyBass" />

# AyyyBass

**One button. Press it → a hypnotic North African acid bassline.**

Don't like it? Press again. Like it? Keep it. Zero programming.

[![Play it live](https://img.shields.io/badge/▶%20play%20it-live-FFD21E?style=for-the-badge)](https://helmi-kobbi.github.io/AyyyBass/)
[![License: MIT](https://img.shields.io/badge/license-MIT-2f6db3?style=for-the-badge)](LICENSE)

*Gnawa guembri meets the 303 squelch — from the lila to the rave.* &nbsp; 🇹🇳 🇩🇿 🇲🇦

</div>

---

## ▶ Try it now — nothing to install

### **[helmi-kobbi.github.io/AyyyBass](https://helmi-kobbi.github.io/AyyyBass/)**

Runs in your browser, makes its own sound. Press the smiley. That's the whole manual.

## What it is

AyyyBass is a **one-button generative bassline instrument** with a North African acid soul: ternary (6/8, 12/8) hypnotic, guembri-rooted basslines voiced through the acid-303 squelch and slide. The acid-house smiley — here wearing a Tunisian *chechia* — is both the brand and the button.

It removes the decision fatigue and the pattern-programming chore, so you get straight to a usable, hypnotic, culturally-rooted line. It's the instrument its maker wished existed.

## Why it's different

- **Ternary from the ground up** — 6/8 and 12/8, not 4/4 with swing bolted on. The Maghreb groove, not another techno box.
- **Slide is the soul** — the glissando that lets a 303 sing in Arabic phrasing.
- **Polymeter by design** — free odd lengths (5, 7, 9…) drift against the beat and hypnotise.
- **You are the taste** — press-again = no, keep = yes. No AI black box, no cloud, instant.
- **One body, infinite brains** — the generator is a swappable *brain*. Fork it for your own style or culture.

## Controls

scale (pentatonic·Gnawa / hijaz / kurd / nahawand) · key · octave · length · density · tempo · ternary + drums toggles · **generate** · **mutate** · **♥ keep**

## How it's built

| Folder | What lives there |
|---|---|
| `core/` | the **brain** — a portable, dependency-free C++ engine (generate + mutate), destined for the browser and the ESP32 *unchanged* |
| `platform/midi/` | renders a pattern to a Standard MIDI File |
| `apps/cli/` | command-line generator → `.mid` |
| `playground/` | the live browser instrument (a faithful JS twin of the brain + a Web Audio 303 voice) |

### Build & run the engine

```bash
make test          # build + run the unit tests (needs clang or g++)
make ayyybass      # build the CLI
./ayyybass --seed 527 --scale hijaz --length 7 --density 0.55 --bpm 130 -o out.mid
```

The **seed is the pattern number** — `527` always sounds like `527`, so you can share a groove as a number.

## Roadmap

- ✅ v1 brain + MIDI export + live browser playground
- ⏳ tune the brain by ear — the "gives you chills" pass
- ◻ wasm playground running the real C++ brain
- ◻ ESP32-S3 hardware: big yellow button, OLED, encoder, MIDI out — for Superbooth
- ◻ more brains (styles) · taste-learning · microtonal maqam via the 303 slide-bend

## Contributing

Open and collaborative — **fellow musicians welcome.** Play the [live demo](https://helmi-kobbi.github.io/AyyyBass/), then:

- A groove you love, an idea, a bug → open an **Issue**.
- Want your own style or culture as a brain? Fork, write to the brain contract, open a **Pull Request**. *One body, infinite brains.*

The full vision and architecture live in the [design spec](docs/superpowers/specs/2026-06-27-ayyybass-design.md).

## License

[MIT](LICENSE) © 2026 Helmi Kobbi. Built in the open, with AI as a co-pilot.
