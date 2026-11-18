# Suite Rack

**Status: M0 scaffold with a working CLAP build.** LV2 wrapper, rack state recall and host automation are next.

A virtual mix rack in the Suite visual language: six serial slots, each hosting
an EQ or Comp module. Slot 1 starts as EQ, slot 2 as Comp, slots 3–6 are empty.
Modules are chosen from an in-panel browser.

Planned additions include Hearth EQ, 4K EQ (Suite Channel), Patina preamps, the
Comp Suite models, Space reverb/delay/module options, Bleed Out and Trowel
modules, plus neutral-named ports of the MIT JClones algorithms.

> Better plugins exist. I just wanted to have a Suite Rack.

![Suite Rack — six-slot rack](docs/rack-eq.png)

![Suite Rack — compressor row](docs/rack-comp.png)

![Suite Rack — module browser](docs/rack-browser.png)

## Current M0 slice

- 1040×720 panel with the Comp Suite-family header: power, preset bar, centered
  title, utility pills.
- Six vertical 500-series-style module columns, each with its own background and knob colour.
- Per-row bypass and slot mix.
- Module browser: **Off / EQ / Comp**.
- EQ module: four bands + HPF/LPF.
- Comp module: 76 / 2A / VCA with threshold, ratio, attack, release, makeup,
  mix and sidechain HPF.
- Serial engine and per-slot meters in place; DSP and panel tests pass.
- **CLAP build loads in REAPER**, opens the six-column editor and processes audio.
- LV2 wrapper and full rack state recall are next.

## Architecture

```
Input → Slot 1 → Slot 2 → Slot 3 → Slot 4 → Slot 5 → Slot 6 → Output
```

Each slot owns an EQ and Comp instance and its own parameter block. Slots are
pre-allocated, so changing a slot from EQ to Comp does not move host automation
IDs.

## Build and preview

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/rack_ui_preview assets/fonts /tmp/suite-rack-preview
```

## Install CLAP

```sh
cp build/clap/SuiteRack.clap ~/.clap/
cp -r build/clap/SuiteRack.resources ~/.clap/
```

## Next

1. LV2 wrapper and state recall.
2. Six-slot parameter serialization and host automation tests.
3. Module registry for Patina, Space, Bleed Out, Limiter.
4. Drag/reorder, per-slot gain, module presets.
5. Rack presets and user preset saving.

GPL-3.0-or-later.
