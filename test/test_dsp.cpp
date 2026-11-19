// Suite Rack — DSP tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include <random>

#include "dsp/engine.hpp"
#include "module_ids.hpp"
#include "params.hpp"
#include "modules.hpp"

using namespace rack;

namespace {
int fails = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}
double rms(const std::vector<float>& x, size_t a, size_t b) {
  double s = 0.0;
  for (size_t i = a; i < b; ++i) s += double(x[i]) * x[i];
  return std::sqrt(s / std::max<size_t>(1, b - a));
}
bool finite(const std::vector<float>& x) {
  for (float v : x)
    if (!std::isfinite(v)) return false;
  return true;
}
} // namespace

int main() {
  const float sr = 48000.0f;
  const int n = static_cast<int>(sr * 0.5f);
  std::vector<float> tone(n);
  for (int i = 0; i < n; ++i)
    tone[i] = 0.25f * std::sin(2.0f * 3.14159265f * 1000.0f * i / sr);

  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Off);
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i)
      err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "two empty slots are transparent");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Eq4K);
    c.slot[0].c[kCtlHpf] = 1.0f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    std::vector<float> low(n);
    for (int i = 0; i < n; ++i)
      low[i] = 0.25f * std::sin(2.0f * 3.14159265f * 100.0f * i / sr);
    auto l = low;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 1000, n) < rms(low, 1000, n) * 0.5,
          "EQ slot high-pass removes a 100 Hz tone at maximum HPF");
    check(finite(l), "EQ slot stays finite");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Comp76Blue);
    c.slot[0].c[kCtlThreshold] = 0.75f;
    c.slot[0].c[kCtlRatio] = 0.75f;
    c.slot[0].c[kCtlMakeup] = 0.40f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 2000, n) < rms(tone, 2000, n) * 0.95,
          "compressor slot reduces a loud tone");
    check(m.slotGrDb[0] > 0.5f, "compressor slot reports gain reduction");
    check(finite(l), "compressor slot stays finite");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::EqHearth);
    c.slot[0].c[kCtlHpf] = 1.0f;
    c.slot[0].bypass = true;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i)
      err = std::max(err, static_cast<double>(std::fabs(l[i] - tone[i])));
    check(err < 1e-6, "slot bypass is transparent");
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Eq4K);
    c.slot[0].gainDb = -6.0f;
    c.slot[1].type = static_cast<int>(ModuleId::Off);
    e.setControls(c);
    auto l = tone;
    Meters m;
    e.process(l.data(), l.data(), n, m);
    check(rms(l, 2000, n) < rms(tone, 2000, n) * 0.6,
          "per-slot gain trims the module output");
  }

  const int ids[] = {
        static_cast<int>(ModuleId::EqHearth),
        static_cast<int>(ModuleId::EqTilt),
        static_cast<int>(ModuleId::EqAir),
        static_cast<int>(ModuleId::Comp76Black),
        static_cast<int>(ModuleId::Comp2A),
        static_cast<int>(ModuleId::Comp3A),
        static_cast<int>(ModuleId::CompVCA),
        static_cast<int>(ModuleId::ColourValve),
        static_cast<int>(ModuleId::ColourConsole),
        static_cast<int>(ModuleId::ColourBus),
        static_cast<int>(ModuleId::ColourTape),
        static_cast<int>(ModuleId::ColourTransformer),
        static_cast<int>(ModuleId::DynGate),
        static_cast<int>(ModuleId::DynTransient),
        static_cast<int>(ModuleId::DynClip),
        static_cast<int>(ModuleId::DynDeEss),
        static_cast<int>(ModuleId::Density),
        static_cast<int>(ModuleId::BleedOut),
        static_cast<int>(ModuleId::Limiter),
        static_cast<int>(ModuleId::Image),
        static_cast<int>(ModuleId::Crush),
        static_cast<int>(ModuleId::Sub),
        static_cast<int>(ModuleId::Double),
        static_cast<int>(ModuleId::DynamicEq),
        static_cast<int>(ModuleId::Multiband),
        static_cast<int>(ModuleId::Rider),
        static_cast<int>(ModuleId::Cleanup),
        static_cast<int>(ModuleId::SpacePlate),
        static_cast<int>(ModuleId::SpaceRoom),
        static_cast<int>(ModuleId::SpaceHall),
        static_cast<int>(ModuleId::SpaceSpring),
        static_cast<int>(ModuleId::SpaceDigital),
        static_cast<int>(ModuleId::SpaceTape),
        static_cast<int>(ModuleId::SpaceAnalog),
        static_cast<int>(ModuleId::SpacePingPong),
        static_cast<int>(ModuleId::SpaceChorus),
        static_cast<int>(ModuleId::SpaceFlanger),
        static_cast<int>(ModuleId::SpacePhaser),
        static_cast<int>(ModuleId::SpaceDimension),
        static_cast<int>(ModuleId::HearthComp),
        static_cast<int>(ModuleId::Glue),
        static_cast<int>(ModuleId::CompXpress),
        static_cast<int>(ModuleId::CompDigital),
        static_cast<int>(ModuleId::CompMolot),
        static_cast<int>(ModuleId::CompRed),
        static_cast<int>(ModuleId::Reel),
        static_cast<int>(ModuleId::Ember),
        static_cast<int>(ModuleId::Tame),
        static_cast<int>(ModuleId::Eq510),
        static_cast<int>(ModuleId::Pultec),
        static_cast<int>(ModuleId::JcClassicLimiter),
        static_cast<int>(ModuleId::JcTapeHead),
        static_cast<int>(ModuleId::JcTubeDriver),
        static_cast<int>(ModuleId::JcDaTube),
        static_cast<int>(ModuleId::JcFattener),
        static_cast<int>(ModuleId::JcAC1),
        static_cast<int>(ModuleId::JcLMC1),
        static_cast<int>(ModuleId::JcModernLimiter),
        static_cast<int>(ModuleId::JcHyrax),
        static_cast<int>(ModuleId::JcBloom),
        static_cast<int>(ModuleId::JcRS124),
        static_cast<int>(ModuleId::JcCL1B),
        static_cast<int>(ModuleId::JcDuTape),
        static_cast<int>(ModuleId::JcVBL),
        static_cast<int>(ModuleId::JcL2),
        static_cast<int>(ModuleId::JcO3),
        static_cast<int>(ModuleId::HybridEq),
        static_cast<int>(ModuleId::CustomEq),
  };
  for (float rate : {48000.0f, 96000.0f}) {
    const int sweepN = rate > 48000.0f ? 24000 : n;
    std::vector<float> sweep(sweepN);
    for (int i = 0; i < sweepN; ++i) {
      sweep[i] =
          0.25f * std::sin(2.0f * 3.14159265f * 1000.0f * i / rate);
    }
    for (int id : ids) {
      Engine e;
      e.prepare(rate);
      Controls c;
      c.slot[0].type = id;
      for (int k = 0; k < kRackControlCount; ++k) {
        c.slot[0].c[k] = static_cast<float>(
            defaultControlValue(static_cast<ModuleId>(id), k));
      }
      if (moduleIsComp(id)) {
        c.slot[0].c[kCtlThreshold] = 0.80f;
        c.slot[0].c[kCtlRatio] = 0.75f;
        c.slot[0].c[kCtlMix] = 1.0f;
      }
      c.slot[1].type = static_cast<int>(ModuleId::Off);
      e.setControls(c);
      auto l = sweep;
      Meters m;
      e.process(l.data(), l.data(), sweepN, m);
      char label[96];
      std::snprintf(label, sizeof(label), "%s @ %.0f Hz", kModuleLabels[id],
                    static_cast<double>(rate));
      check(finite(l), label);
    }
  }
  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::BleedOut);
    c.slot[0].c[0] = 0.0f;   // Kick profile
    c.slot[0].c[1] = 0.90f;  // high threshold
    c.slot[0].c[2] = 0.70f;  // DeBleed
    c.slot[0].c[3] = 0.0f;   // ghost off
    e.setControls(c);
    check(e.latencySamples() > 0,
          "Bleed Out contributes lookahead latency");

    std::vector<float> quiet(n);
    for (int i = 0; i < n; ++i)
      quiet[i] = 0.01f * std::sin(2.0f * 3.14159265f * 1000.0f * i / sr);
    auto wet = quiet;
    Meters m;
    e.process(wet.data(), wet.data(), n, m);
    check(rms(wet, 4000, n) < rms(quiet, 4000, n) * 0.5,
          "Bleed Out gates a signal below threshold");
  }

  {
    Engine e;
    e.prepare(sr);
    Controls c;
    c.slot[0].type = static_cast<int>(ModuleId::Limiter);
    e.setControls(c);
    check(e.latencySamples() > 0,
          "Peak Limiter contributes lookahead latency");

    std::vector<float> loud(n);
    for (int i = 0; i < n; ++i)
      loud[i] = 1.5f * std::sin(2.0f * 3.14159265f * 1000.0f * i / sr);
    auto wet = loud;
    Meters m;
    e.process(wet.data(), wet.data(), n, m);
    double peak = 0.0;
    for (int i = 4000; i < n; ++i)
      peak = std::max(peak, static_cast<double>(std::fabs(wet[i])));
    check(m.slotGrDb[0] > 1.0f, "Peak Limiter reports gain reduction");
    check(peak < 0.95, "Peak Limiter holds the output below the ceiling");
  }

  {
    // Every module on its own: finite and audible at 48 and 96 kHz.
    bool allFinite = true;
    bool allAudible = true;
    for (float sr : {48000.0f, 96000.0f}) {
      Engine e;
      e.prepare(sr);
      const int n = static_cast<int>(sr * 0.12f);
      for (int id = 1; id < kModuleCount; ++id) {
        Shared s;
        initializeRack(s);
        for (int slot = 0; slot < kSlotCount; ++slot) {
          applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
        }
        applyModuleDefaults(s, 0, id, false);
        e.setControls(controlsFrom(s));
        e.reset();
        std::vector<float> l(n), r(n);
        for (int i = 0; i < n; ++i) {
          l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i / sr);
        }
        Meters m;
        e.process(l.data(), r.data(), n, m);
        for (float v : l) {
          if (!std::isfinite(v)) allFinite = false;
        }
        if (rms(l, static_cast<size_t>(n) / 4, n) < 1.0e-6) {
          allAudible = false;
          std::printf("module %d silent at %.0f\n", id, sr);
        }
      }
    }
    check(allFinite, "every module stays finite at 48/96 kHz");
    check(allAudible, "every module produces sound at 48/96 kHz");
  }
  {
    // Every module with all controls at 0 and 1.
    bool ok = true;
    for (float sr : {48000.0f, 96000.0f}) {
      Engine e;
      e.prepare(sr);
      for (int id = 1; id < kModuleCount; ++id) {
        for (double extreme : {0.0, 1.0}) {
          Shared s;
          initializeRack(s);
          for (int slot = 0; slot < kSlotCount; ++slot) {
            applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off),
                                false);
          }
          applyModuleDefaults(s, 0, id, false);
          for (int k = 0; k < kRackControlCount; ++k) {
            s.setFromHost(slotParam(0, kSlotOffsetC0 + k), extreme);
          }
          e.setControls(controlsFrom(s));
          e.reset();
          const int n = static_cast<int>(sr * 0.05f);
          std::vector<float> l(n), r(n);
          for (int i = 0; i < n; ++i) {
            l[i] = r[i] =
                0.25f * std::sin(2.0f * 3.14159265f * 220.0f * i / sr);
          }
          Meters m;
          e.process(l.data(), r.data(), n, m);
          for (float v : l) {
            if (!std::isfinite(v)) {
              ok = false;
              std::printf("extreme nonfinite module %d at %.0f\n", id, sr);
              break;
            }
          }
        }
      }
    }
    check(ok, "every module survives extreme controls at 48/96 kHz");
  }
  {
    // Every ordered pair of modules with default controls.
    Engine e;
    e.prepare(48000.0f);
    bool finitePairs = true;
    float worst = 0.0f;
    int worstA = 0, worstB = 0;
    const int n = 512;
    for (int a = 1; a < kModuleCount; ++a) {
      for (int b = 1; b < kModuleCount; ++b) {
        Shared s;
        initializeRack(s);
        for (int slot = 0; slot < kSlotCount; ++slot) {
          applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
        }
        applyModuleDefaults(s, 0, a, false);
        applyModuleDefaults(s, 1, b, false);
        e.setControls(controlsFrom(s));
        e.reset();
        std::vector<float> l(n), r(n);
        for (int i = 0; i < n; ++i) {
          l[i] = r[i] = 0.25f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                         48000.0f);
        }
        Meters m;
        e.process(l.data(), r.data(), n, m);
        for (float v : l) {
          if (!std::isfinite(v)) finitePairs = false;
          if (std::fabs(v) > worst) {
            worst = std::fabs(v);
            worstA = a;
            worstB = b;
          }
        }
      }
    }
    std::printf("rack pair worst peak %.2f at %d/%d\n", worst, worstA,
                worstB);
    check(finitePairs, "every ordered module pair stays finite");
    check(worst < 50.0f, "every ordered module pair stays bounded");
  }
  {
    // Random six-slot racks.
    std::mt19937 rng(0x5EED);
    std::uniform_real_distribution<float> u01(0.0f, 1.0f);
    Engine e;
    e.prepare(48000.0f);
    bool finiteBoards = true;
    float worst = 0.0f;
    const int n = 2048;
    for (int board = 0; board < 200; ++board) {
      Shared s;
      initializeRack(s);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        const int type = 1 + static_cast<int>(rng() % (kModuleCount - 1));
        applyModuleDefaults(s, slot, type, false);
        for (int k = 0; k < kRackControlCount; ++k) {
          s.setFromHost(slotParam(slot, kSlotOffsetC0 + k), u01(rng));
        }
        s.setFromHost(slotParam(slot, kSlotOffsetMix), 100.0 * u01(rng));
        s.setFromHost(slotParam(slot, kSlotOffsetGain),
                      -6.0 + 12.0 * u01(rng));
        s.setFromHost(slotParam(slot, kSlotOffsetBypass),
                      (board % 7 == 0 && (slot & 1)) ? 1.0 : 0.0);
      }
      e.setControls(controlsFrom(s));
      e.reset();
      std::vector<float> l(n), r(n);
      for (int i = 0; i < n; ++i) {
        const float env = i < 400 ? static_cast<float>(i) / 400.0f : 1.0f;
        l[i] = r[i] = 0.15f * env *
                      std::sin(2.0f * 3.14159265f * 220.0f * i / 48000.0f);
      }
      Meters m;
      e.process(l.data(), r.data(), n, m);
      for (float v : l) {
        if (!std::isfinite(v)) finiteBoards = false;
        if (std::fabs(v) > worst) worst = std::fabs(v);
      }
    }
    std::printf("random rack worst peak %.2f\n", worst);
    check(finiteBoards, "random six-slot racks stay finite");
    check(worst < 500.0f, "random six-slot racks stay bounded");
  }
  {
    // Bypass is dry and a compressor reports gain reduction.
    Engine e;
    e.prepare(48000.0f);
    Shared s;
    initializeRack(s);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(s, slot, static_cast<int>(ModuleId::Off), false);
    }
    applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Eq4K), false);
    s.setFromHost(slotParam(0, kSlotOffsetC0 + kCtlHpf), 1.0);
    s.setFromHost(slotParam(0, kSlotOffsetBypass), 1.0);
    e.setControls(controlsFrom(s));
    e.reset();
    const int n = 4096;
    std::vector<float> l(n), r(n);
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.2f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                    48000.0f);
    }
    Meters m;
    e.process(l.data(), r.data(), n, m);
    double err = 0.0;
    for (int i = 0; i < n; ++i) {
      err = std::max(err, static_cast<double>(std::fabs(
                              l[i] - 0.2f * std::sin(2.0f * 3.14159265f * 220.0f *
                                                     i / 48000.0f))));
    }
    check(err < 1e-5, "bypassed module is dry");

    Shared c;
    initializeRack(c);
    for (int slot = 0; slot < kSlotCount; ++slot) {
      applyModuleDefaults(c, slot, static_cast<int>(ModuleId::Off), false);
    }
    applyModuleDefaults(c, 0, static_cast<int>(ModuleId::Comp76Blue), false);
    for (int k = 0; k < kRackControlCount; ++k) {
      c.setFromHost(slotParam(0, kSlotOffsetC0 + k), 0.9);
    }
    e.setControls(controlsFrom(c));
    e.reset();
    for (int i = 0; i < n; ++i) {
      l[i] = r[i] = 0.4f * std::sin(2.0f * 3.14159265f * 220.0f * i /
                                    48000.0f);
    }
    e.process(l.data(), r.data(), n, m);
    check(m.slotGrDb[0] > 0.5f, "compressor reports gain reduction");
  }


  // External side-chain replaces the compressor detector.
  {
    auto run = [&](bool external, float inAmp, float scAmp) {
      Engine engine;
      engine.prepare(48000.0f);
      Controls c;
      c.power = true;
      c.slot[0].type = static_cast<int>(ModuleId::Comp76Blue);
      c.slot[0].bypass = false;
      c.externalSidechain = external;
      c.slot[0].mix = 1.0f;
      for (int i = 0; i < 12; ++i) c.slot[0].c[i] = 0.5f;
      c.slot[0].c[0] = 0.10f;  // threshold
      c.slot[0].c[1] = 0.70f;  // ratio
      c.slot[0].c[2] = 0.85f;  // fast attack
      c.slot[0].c[3] = 0.60f;
      engine.setControls(c);
      std::vector<float> l(4800), r(4800), sl(4800), sr(4800);
      for (int i = 0; i < 4800; ++i) {
        l[i] = r[i] = inAmp * std::sin(2.0f * 3.14159265f * 1000.0f * i / 48000.0f);
        sl[i] = sr[i] = scAmp * std::sin(2.0f * 3.14159265f * 1000.0f * i / 48000.0f);
      }
      Meters meters;
      float maxGr = 0.0f;
      for (int start = 0; start < 4800; start += 128) {
        const int n = std::min(128, 4800 - start);
        engine.process(l.data() + start, r.data() + start, n, meters,
                       sl.data() + start, sr.data() + start);
        maxGr = std::max(maxGr, meters.slotGrDb[0]);
      }
      return maxGr;
    };
    const float internal = run(false, 2.00f, 0.001f);
    const float quietSc = run(true, 2.00f, 0.001f);
    const float loudSc = run(true, 0.01f, 0.50f);
    check(internal > 3.0f, "compressor reduces a loud internal signal");
    check(quietSc < 0.5f,
          "quiet external side-chain leaves a loud signal uncompressed");
    check(loudSc > 3.0f,
          "loud external side-chain compresses a quiet signal");
  }


  // Meter module: transparent audio and the main measurements.
  {
    Engine engine;
    engine.prepare(48000.0f);
    Controls c;
    for (int slot = 0; slot < kSlotCount; ++slot) {
      c.slot[slot].type = static_cast<int>(ModuleId::Off);
    }
    c.slot[0].type = static_cast<int>(ModuleId::Meter);
    c.slot[0].c[0] = 0.0f;   // peak view
    c.slot[0].c[1] = 0.5f;   // standard scale
    c.slot[0].c[2] = 0.0f;   // hold off
    engine.setControls(c);
    Meters m;
    std::vector<float> l(256), r(256);
    double passthroughError = 0.0;
    for (int b = 0; b < 120; ++b) {
      for (int i = 0; i < 256; ++i) {
        const float t = static_cast<float>(b * 256 + i) / 48000.0f;
        l[i] = r[i] = 0.18f * std::sin(2.0f * 3.14159265f * 1000.0f * t);
      }
      std::vector<float> dryL = l;
      engine.process(l.data(), r.data(), 256, m);
      if (b > 100) {
        for (int i = 0; i < 256; ++i) {
          passthroughError = std::max(
              passthroughError,
              std::fabs(static_cast<double>(l[i] - dryL[i])));
        }
      }
    }
    check(passthroughError < 1.0e-6,
          "meter module is bit-transparent");
    check(std::fabs(m.slotMeter[0].peakLDb + 14.9f) < 0.5f,
          "meter peak reads the input level");
    check(std::fabs(m.slotMeter[0].holdLDb + 14.9f) < 0.5f,
          "meter hold follows the peak when hold is off");
    check(std::fabs(m.slotMeter[0].vu) < 1.2f,
          "meter VU is calibrated near 0 for -18 dBFS RMS");
    check(m.slotMeter[0].correlation > 0.99f,
          "meter correlation reads +1 for identical channels");
    check(m.slotMeter[0].lufsM > -20.0f && m.slotMeter[0].lufsM < -10.0f,
          "meter momentary loudness is populated");
    bool spectrum = false;
    for (int i = 0; i < kMeterSpectrumBins; ++i) {
      if (m.meterSpectrum[i] > -100.0f) spectrum = true;
    }
    check(spectrum, "meter spectrum is populated");
  }

  // Hold keeps the transient and reset clears it.
  {
    Engine engine;
    engine.prepare(48000.0f);
    Controls c;
    for (int slot = 0; slot < kSlotCount; ++slot) {
      c.slot[slot].type = static_cast<int>(ModuleId::Off);
    }
    c.slot[0].type = static_cast<int>(ModuleId::Meter);
    c.slot[0].c[2] = 1.0f;  // hold on
    engine.setControls(c);
    Meters m;
    std::vector<float> l(256), r(256);
    for (int i = 0; i < 256; ++i) {
      l[i] = r[i] = 0.5f * std::sin(2.0f * 3.14159265f * 1000.0f *
                                    static_cast<float>(i) / 48000.0f);
    }
    engine.process(l.data(), r.data(), 256, m);
    const float held = m.slotMeter[0].holdLDb;
    for (int b = 0; b < 20; ++b) {
      for (int i = 0; i < 256; ++i) {
        l[i] = r[i] = 0.01f * std::sin(2.0f * 3.14159265f * 1000.0f *
                                       static_cast<float>(i) / 48000.0f);
      }
      engine.process(l.data(), r.data(), 256, m);
    }
    check(held > -7.0f && std::fabs(m.slotMeter[0].holdLDb - held) < 0.1f,
          "meter hold keeps the earlier peak");
    engine.resetMeters();
    engine.process(l.data(), r.data(), 256, m);
    check(m.slotMeter[0].holdLDb < -30.0f,
          "meter reset clears the peak hold");
  }

  // A meter after a compressor reports that compressor's gain reduction.
  {
    Engine engine;
    engine.prepare(48000.0f);
    Controls c;
    for (int slot = 0; slot < kSlotCount; ++slot) {
      c.slot[slot].type = static_cast<int>(ModuleId::Off);
    }
    c.slot[0].type = static_cast<int>(ModuleId::Comp76Blue);
    for (int k = 0; k < 12; ++k) c.slot[0].c[k] = 0.5f;
    c.slot[0].c[0] = 0.10f;
    c.slot[0].c[1] = 0.70f;
    c.slot[0].c[2] = 0.85f;
    c.slot[1].type = static_cast<int>(ModuleId::Meter);
    engine.setControls(c);
    Meters m;
    std::vector<float> l(256), r(256);
    float maxGr = 0.0f;
    for (int b = 0; b < 40; ++b) {
      for (int i = 0; i < 256; ++i) {
        const float t = static_cast<float>(b * 256 + i) / 48000.0f;
        l[i] = r[i] = 0.8f * std::sin(2.0f * 3.14159265f * 220.0f * t);
      }
      engine.process(l.data(), r.data(), 256, m);
      maxGr = std::max(maxGr, m.slotMeter[1].grDb);
    }
    check(maxGr > 0.3f, "meter shows gain reduction from earlier slots");
  }

  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
