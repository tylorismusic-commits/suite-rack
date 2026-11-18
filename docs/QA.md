# Suite Rack — interaction and performance audit

## Coverage

| Check | Result |
|---|---|
| 72 modules, 48/96 kHz | finite and audible |
| Every module, controls at 0 and 1 | finite |
| 5,184 ordered module pairs | finite; worst peak 1.11 |
| 200 random six-slot racks | finite; worst peak 1.89 |
| State round-trip, every module | all 12 controls match |
| Typed-value round-trip, every control | passes |
| LV2 wrapper, all 72 modules, tone | finite |
| LV2 wrapper, all 72 modules, silence | zero except DuTape tape noise at ~-75 dB |
| Bypass | dry |
| Compressor GR | reported |

## Performance

- Engine prepare: ~10 ms, ~18 MB RSS at 48 kHz.
- Heavy six-module rack: ~7% of one core.
- Re-sending controls every block adds no measurable overhead; modules only
  copy/validate values, so no revision cache is needed.

## Fixed in this pass

- LV2 manifest `doap:` prefix (lilv/strict hosts).
- Clip mode names and stepped behaviour.
- De-Ess and Crush kHz parsing.
- Preset list gap click and footer spacing.
- CLAP mid-session latency notification.

## Notes

- DuTape models tape noise by design; it is the only module with a nonzero
  idle output.
- Bypass is immediate; time-based modules do not have tails in this design.
