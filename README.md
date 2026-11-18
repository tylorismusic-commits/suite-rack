# Suite Rack

**Status: 0.43.0-m2 — CLAP + LV2, seventy-one modules, full preset browsers, drag reorder, browser search and typed values.**
More factory module presets are next.

A virtual 500 series mix rack in the Suite visual language: six serial slots, each
hosting one compact module. Slot 1 starts as EQ, slot 2 as Comp, slots 3–6 are
empty. Modules are chosen from an in-panel browser.

Suite Rack is the flagship “try all my stuff” plugin: one insert that gives the
flavour of the whole Suite, plus new mix-world extras. Each module is limited to
a real 500 series control set — one job, a handful of knobs, no deep pages. The
standalone plugins remain the full versions.

> Better plugins exist. I just wanted to have a Suite Rack.

![Suite Rack — six-slot rack](docs/rack-eq.png)

![Suite Rack — compressor row](docs/rack-comp.png)

![Suite Rack — module browser](docs/rack-browser.png)

![Suite Rack — EQ category](docs/rack-browser-eq.png)

![Suite Rack — Space category](docs/rack-browser-space.png)

![Suite Rack — Bleed Out](docs/rack-bleedout.png)

![Suite Rack — Peak Limiter](docs/rack-limiter.png)

![Suite Rack — Image and Crush](docs/rack-image.png)

![Suite Rack — Filter and Motion](docs/rack-filter.png)

![Suite Rack — Sub and Double](docs/rack-sub.png)

![Suite Rack — Dynamic EQ and Multiband](docs/rack-dyn.png)

![Suite Rack — Rider and Cleanup](docs/rack-rider.png)

![Suite Rack — Space modules](docs/rack-space.png)

![Suite Rack — compressor family](docs/rack-comp-family.png)

![Suite Rack — Comp Suite models](docs/rack-comp-suite.png)

![Suite Rack — complete compressor family](docs/rack-comp-complete.png)

![Suite Rack — Reel tape](docs/rack-reel.png)

![Suite Rack — Ember finish](docs/rack-ember.png)

![Suite Rack — Tame](docs/rack-tame.png)

![Suite Rack — 510 and Pultec](docs/rack-eq-extra.png)

![Suite Rack — Classic Limiter](docs/rack-jclones.png)

![Suite Rack — JClones saturation](docs/rack-jclones-sat.png)

![Suite Rack — JClones AC-1](docs/rack-jclones-ac1.png)

![Suite Rack — JClones LMC-1](docs/rack-lmc1.png)

![Suite Rack — JClones Modern Limiter](docs/rack-jclones-limiter.png)

![Suite Rack — JClones Hyrax](docs/rack-hyrax.png)

![Suite Rack — JClones Bloom](docs/rack-bloom.png)

![Suite Rack — JClones RS-124](docs/rack-rs124.png)

![Suite Rack — JClones CL-1B](docs/rack-cl1b.png)

![Suite Rack — JClones DuTape](docs/rack-dutape.png)

![Suite Rack — JClones VBL](docs/rack-vbl.png)

![Suite Rack — JClones L2](docs/rack-l2.png)

![Suite Rack — JClones O3](docs/rack-o3.png)

![Suite Rack — right-click typed value](docs/rack-text-entry.png)

![Suite Rack — browser search](docs/rack-browser-search.png)

![Suite Rack — save user preset](docs/rack-save-preset.png)

![Suite Rack — factory and user module presets](docs/rack-browser-user.png)

![Suite Rack — module presets filtered from a module card](docs/rack-browser-module-presets.png)

![Suite Rack — rack preset browser](docs/rack-presets.png)

![Suite Rack — Hybrid EQ](docs/rack-hybrid-eq.png)

![Suite Rack — About](docs/rack-about.png)

## What goes in the rack

**From the Suite:**

- **Comp Suite** — Clean, Easy, 76Blue, 76Black, 2A, 3A, Xpress, VCA, Molot, Red
- **Patina** — Valve, Console, Bus, Tape, Transformer
- **Space Suite** — Plate, Room, Hall, Spring, Digital/Tape/Analog/Ping-Pong
  Delay, Chorus, Flanger, Phaser, Dimension
- **Vocal Suite** — De-Ess, Air
- **Drum Suite** — Transient, Clip, Bleed Out gate
- **Trowel** — Smoothing, Tame
- **Suite Channel** — 4K EQ, Gate/Ducker
- **Hearth** — Hearth EQ, Hearth Comp
- **Adhesive** — Glue, Reel
- **Brick / Mastering Suite** — Peak Limiter, Tilt, Density
- **Ember** — Ember finish
- **Repair Suite** — Cleanup (De-Click + De-Hum only)
- **JClones** — 16 neutral-named ports, including L2, Classic Limiter, Hyrax,
  O3, Modern Limiter, RS-124, VBL, CL-1B, LMC-1, Bloom, Fattener, AC-1,
  Tape Head, DuTape, Tube Driver and DaTube

**New extras:**

- **Image** — stereo width, mono-maker, balance, M/S
- **Dynamic EQ** — frequency-selective dynamics
- **Multiband** — 3-band compressor
- **Sub** — subharmonic synthesizer
- **Double** — doubler / microshift
- **Filter** — auto-filter / envelope filter
- **Crush** — bitcrusher / lo-fi
- **Motion** — tremolo / auto-pan / rotary
- **Rider** — auto-level / vocal rider

**Not in the rack:** Common Sense EQ, Amp Suite, Bass Suite, and the deep Repair
Suite tools (spectral editor, De-Clip, De-Reverb, De-Noise). Those do not fit
the 500-series format; Cleanup is the lightweight repair representative.

## Current 0.43.0-m2 slice

- 1040×720 panel with the Comp Suite-family header: power, preset bar, centered
  title, utility pills.
- Six vertical 500-series-style module columns, each with its own background and
  knob colour.
- Per-slot bypass, mix and gain.
- Drag a module name or empty card area to another slot to reorder the rack; modules shift live to fill the gap.
- Double-click a knob to reset it to the active module's default. Right-click a
  knob to type an exact value; Enter applies and Escape cancels.
- Stepped controls, used by the Bleed Out profile selector.
- Bleed Out has a separate automated **GHOST ON** button.
- Filled slots hide the **CHOOSE** hint so longer module names have room.
- **ABOUT** opens a reference table with each module's source, controls and attributions.
- Module browser with **ALL / EQ / COMP / COLOUR / DYN / SPACE** categories,
  scrolling and a live name/category search field.
- Every browser option shows its source Suite or "Suite Rack exclusive".
- The **SPACE** category includes reverbs, delays, modulation, Image and Double.
- Generic per-slot controls: twelve control slots plus type, bypass, mix and
  gain. Host automation follows the slot, not the module.
- **Available modules:** 4K EQ, Hearth EQ, Tilt, Air, 76 Blue, 76 Black, 2A,
  3A, VCA, Valve, Console, Bus, Tape, Transformer, Gate, Transient, Clip and
  De-Ess, Density, Bleed Out, Peak Limiter, Image, Crush, Filter, Motion, Sub,
  Double, Dynamic EQ, Multiband, Rider, Cleanup, four reverbs, four delays and
  four modulation modules, Hearth Comp, Glue, Xpress, Digital, Molot, Red,
  Reel, Ember, Tame, 510 EQ, Pultec, Classic Limiter, Tape Head, Tube Driver,
  DaTube, Fattener, AC-1, LMC-1, Modern Limiter, Hyrax, RS-124, CL-1B, DuTape,
  VBL, L2, O3, Bloom and Hybrid EQ.
- Serial engine and per-slot meters in place; every module is swept at 48 kHz
  and 96 kHz in the DSP tests.
- **CLAP and LV2 builds** load in REAPER with the six-column editor and process
  audio.
- **Shared state format** for CLAP and LV2; project save/reload recalls the
  rack settings in both formats.
- **State and automation mapping tests** cover round-trip recall, dirty-flag
  reporting and slot/global parameter mapping.
- A full rack preset browser opens from the preset bar or the header
  **PRESETS** button, listing eleven factory racks and saved user racks.
- The header keeps only PRESETS, SAVE and ABOUT, grouped on the right; the
  inactive 6 SLOTS and SERIAL pills were removed.
- The About page shows each module's source plus a short description of its
  character instead of a control list.
- User rack presets save to `$XDG_CONFIG_HOME/SuiteRack/presets` (or
  `~/.config/SuiteRack/presets`) and appear in the preset bar after the factory
  presets. The header **SAVE** button opens a name field; Enter writes the
  preset and Escape cancels.
- Long module preset names wrap inside their browser cards instead of running
  into neighbouring cards.
- Every loaded module card has a small **PRESET** button. It opens the
  browser's **PRESETS** tab filtered to that module, so changing module presets
  does not require hunting through the full list. The tab can also be opened
  normally to browse all module presets.
- Right-click a module name to save that module's type and controls as a user
  module preset. Saved presets appear in the browser's **PRESETS** tab and can
  be loaded into any slot.

## Architecture

```
Input → Slot 1 → Slot 2 → Slot 3 → Slot 4 → Slot 5 → Slot 6 → Output
```

Each slot owns a module instance and a 16-control parameter block. Module type
and module state are stored separately, so the host parameter count stays small
even with the full catalog. All module controls are normalised 0–1; the module
registry maps them to faceplate labels, display formats and DSP behaviour.
Automation follows the slot, not the module: if a slot changes module, its
control lanes are repurposed.

The module DSP itself lives in **hearth-dsp 0.4.0** under
`third_party/hearth-dsp`, pulled in as a git submodule. Suite Rack keeps only
the rack engine and the adapter that maps its slot controls onto the shared
typed module controls, so future plugins can use the same EQ, compressor,
colour, gate, transient, clip and de-esser stages.

## Build and preview

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/rack_ui_preview assets/fonts /tmp/suite-rack-preview
```

## Install

CLAP:

```sh
cp build/clap/SuiteRack.clap ~/.clap/
cp -r build/clap/SuiteRack.resources ~/.clap/
```

LV2:

```sh
cp -r build/lv2/suite-rack.lv2 ~/.lv2/
```

## Next

1. More factory module preset content.
2. Roll the same user-preset format into Comp Suite and Patina.
3. A final Trowel Smoothing decision; Tame is the current Trowel-derived module.

GPL-3.0-or-later.
