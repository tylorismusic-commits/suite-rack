# Third-party notices

Suite Rack is GPL-3.0-or-later. It contains or derives from the following work.

## JClones JSFX

`src/dsp/molot.hpp` is a C++ translation of the Molot-family JSFX from
[JClones/JSFXClones](https://github.com/JClones/JSFXClones).

```
MIT License

Copyright (c) 2026 JClones

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

The 2A/3A optical side-chain structure was informed by the same project's
`JClones_CA2A.jsfx` (MIT), with the DSP reimplemented against the published
behaviour.

Additional JClones ports are planned for Suite Rack using neutral module names
and no original vendor trade dress. The MIT notice above applies to those ports.

## Fonts

`assets/fonts/Jost-ExtraBold.ttf` is Jost, licensed under the SIL Open Font
License 1.1. The licence text ships beside it as `assets/fonts/OFL.txt`.

## CLAP

`third_party/clap` contains the CLAP headers, MIT licensed.

## hearth-dsp

The shared DSP modules under `third_party/hearth-dsp` (ADAA saturation,
biquads, envelopes, true-peak detection, oversampling and the ratio ladder) are
part of hearth-dsp, GPL-3.0-or-later, extracted from this project. See
`third_party/hearth-dsp/THIRD_PARTY_NOTICES.md` for algorithm provenance.
