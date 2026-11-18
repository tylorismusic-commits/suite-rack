# Changelog

## 0.9.0-m2 — 2026-11-17

- Added Image: width, mono-maker, balance and an M/S monitor toggle.
- Added Crush: bit depth, sample-rate reduction, tone and mix.
- Image and Crush live in hearth-dsp and are consumed through the submodule.

## 0.8.2-m2 — 2026-11-17

- Replaced the category-based module dispatch with an explicit ModuleId switch.
- Added `-Werror=switch` to the engine-bearing targets, so adding a module
  without wiring its processing branch is now a compile error.

## 0.8.1-m2 — 2026-11-17

- Fixed the Peak Limiter being skipped by the rack engine dispatch.
- Added engine and panel regression tests for Peak Limiter gain reduction,
  ceiling behaviour, latency and module selection.
- Bleed Out and Peak Limiter now both run through the dynamics dispatch.

## 0.8.0-m2 — 2026-11-17

- Added the Peak Limiter module with Input, Ceiling, Release and Link.
- Bleed Out now has an automated GHOST ON toggle next to the Ghost knob.
- Fixed Bleed Out not being processed in the rack engine dispatch.
- Peak Limiter and Bleed Out contribute lookahead latency to the rack.
- Peak Limiter lives in hearth-dsp and is consumed through the submodule.

## 0.7.0-m2 — 2026-11-17

- Added the Bleed Out multiband de-bleed gate module with Profile, Threshold,
  DeBleed and Ghost.
- Added stepped control support to the module faceplates.
- Bleed Out contributes its lookahead latency to the rack's reported latency.
- Filled slots hide the CHOOSE hint.
- Bleed Out lives in hearth-dsp and is consumed through the submodule.

## 0.6.0-m2 — 2026-11-17

- Added the Density harmonic-density module from Mastering Suite.
- Density lives in hearth-dsp 0.4.0 and is consumed through the submodule.

## 0.5.1-m2 — 2026-11-17

- Moved knob labels and values further from the knob body for the larger text.
- Double-clicking a knob resets it to the active module's default value.
- Right-click text entry remains reserved for the full-catalog pass.

## 0.5.0-m2 — 2026-11-17

- Added Gate, Transient, Clip and De-Ess modules.
- Added the DYN browser category.
- Gain reduction is now a horizontal LED meter with 0/3/6/9/12 dB labels.
- Larger, more readable knob labels and values.
- Centered the rack subtitle in the header-to-modules gap.
- Matched the Comp Suite power button exactly.
- Moved the module DSP into hearth-dsp 0.4.0 and consume it as a git submodule.

## 0.4.0-m2 — 2026-11-17

- Added the five Patina colour modules: Valve, Console, Bus, Tape and
  Transformer.
- Single-band colour module with Drive, Character, Tone and Mix.
- Browser COLOUR category and a four-column module grid.
- Added colour modules to the DSP sweep.
- Browser scrolling for catalogs larger than sixteen entries.

## 0.3.0-m2 — 2026-11-17

- Module registry with nine modules: 4K EQ, Hearth EQ, Tilt, Air, 76 Blue,
  76 Black, 2A, 3A and VCA.
- Generic per-slot controls: type, bypass, mix, gain and twelve module-control
  slots. All controls are normalised 0–1 and mapped by the registry.
- Per-slot gain.
- Browser categories: ALL / EQ / COMP.
- Module-specific faceplate colours, labels, defaults and value formatting.
- State format v2 for the generic control layout; older M1 states are rejected.
- Updated DSP, panel, state and metadata tests for the registry.

## 0.2.0-m1 — 2026-11-17

- LV2 plugin and X11/Cairo UI, sharing the CLAP DSP and panel.
- Shared CLAP/LV2 state format with round-trip recall.
- State and automation mapping tests: parameter round-trip, bad/truncated state
  rejection, dirty-flag behaviour and slot/global control mapping.
- LV2 metadata generator and a TTL freshness test.
- CLAP state now uses the shared format.
- Bumped to 0.2.0-m1.

## Unreleased

- Suite Rack is now planned as the flagship “try all my stuff” rack: every
  included Suite family is represented by one or more compact 500-series
  modules, plus new mix-world extras.
- Added the 500-series control rule: one job per module, 3–8 primary controls,
  no deep pages.
- Planned module catalog expanded to EQ, colour, compression, Space, dynamics,
  utility, new extras and JClones ports.
- Common Sense EQ, Amp Suite, Bass Suite and the deep Repair Suite tools are
  explicitly out of scope for the rack.

## 0.1.0-m0 — 2026-11-17

- Six-slot virtual mix rack scaffold.
- Comp Suite-family header with power, preset bar, centered title and utility
  pills.
- Vertical 500-series-style module columns with per-module background and knob colours.
- Module browser with Off / EQ / Comp.
- Slot bypass, slot mix, per-slot compressor GR.
- EQ and Comp DSP modules with serial engine tests and panel tests.
- CLAP wrapper loads and processes in REAPER.
- UI preview renders the rack, compressor row and module browser.
