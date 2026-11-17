// Comp Suite model reused in Drum Suite — controls.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstddef>

namespace comp {

// Model selector, in the order shown on the style tabs.
enum class Model : int {
  Blue76 = 0,   // 1176-style FET, blue-stripe voicing
  Black76 = 1,  // 1176-style FET, black-face voicing
  Opto2A = 2,   // T4-style optical leveler, 2A-style
  Opto3A = 3,   // T4-style optical leveler, 3A-style
  Xpress = 4,   // diode-bridge style compressor with distortion modes
  Vca = 5,      // SSL-style stereo bus VCA
  Clean = 6,    // clean digital compressor
  Molot = 7,    // character compressor with stereo modes and saturation
  Red = 8,      // British Class-A VCA, compression then limiting
  Easy = 9,     // drum-focused character compressor
  Count = 10
};

// Output calibration so the models sit at roughly the same level when the
// controls are in the same place.  Measured at noon settings
// (input/output/makeup 0.5, attack/release 0.5, 4:1) on broadband programme
// material and on a 100 Hz tone, then averaged so neither is more than about
// 2 dB out.  The 76s are the anchor at the level where their OUTPUT reaches a
// Tukan NC76 at noon; the other models are trimmed to them.  The output knob
// is a trim around this calibrated level and does not change the dynamics.
inline constexpr float kModelTrimDb[static_cast<std::size_t>(
    Model::Count)] = {
    12.0f, // Blue76: reference, matching the NC76 at noon
    12.0f, // Black76
    4.5f,  // Opto2A
    4.5f,  // Opto3A
    5.5f,  // Xpress
    4.0f,  // Vca
    0.0f,  // Clean
    0.0f,  // Molot
    4.5f,  // Red
    3.0f,  // Easy
};

// Every control is normalised to 0..1.  Each model maps the shared slots to
// its own physical controls, so the UI only shows the ones a model has.
struct Controls {
  int model = 0;
  bool enabled = true;      // power / compress on
  float input = 0.0f;       // Input gain (76, Xpress, Clean, Molot, Red, Easy)
  float threshold = 0.5f;   // threshold, or 2A/3A Peak Reduction
  float ratio = 0.5f;       // 76: 4/8/12/20/all, Xpress: ratio steps, VCA: 2..10, 2A: Limit switch
  float attack = 0.5f;      // 76 steps 1..7, Xpress/VCA continuous
  float release = 0.5f;
  float makeup = 0.5f;      // 2A/3A Gain, VCA/Clean/Molot/Red make-up
  float output = 0.5f;      // 76 and Xpress output, 2A output level switch
  float mix = 1.0f;         // parallel blend, applied once by the engine
  float sidechainHpf = 0.0f;
  float distortion = 0.0f;  // Xpress: Off / Dist 2 / Dist 3 / Clip
  bool limiterOn = false;   // output limiter, as in the reference suite
  float limitThreshold = 0.5f;
  bool autoMakeup = false;  // track average reduction and restore the level
  bool externalSidechain = false;  // detect from the side-chain inputs
  bool truePeak = false;           // limiter detects inter-sample peaks
  float link = 1.0f;               // VCA stereo link: 0 dual mono, 1 linked
  bool vcaAutoRelease = false;     // VCA two-stage automatic release
  float knee = 0.5f;               // Clean/Molot: 0..1
  float satDrive = 0.5f;           // Molot: saturator drive, 0..1
  float attackMode = 0.0f;         // Molot: 0 Alpha, 1 Sigma
  float stereoMode = 5.0f;         // Molot: 0 Mono .. 5 Stereo
  float beef = 0.35f;              // Easy: low end into the compressor
  float air = 0.2f;                // Easy: top end after the compressor
  float drumType = 2.0f;           // Easy: 0 Toms .. 5 Parallel
  float easyMode = 0.0f;           // Easy: 0 Squish, 1 Lift
  float easyChar = 0.0f;           // Easy: 0 Normal, 1 Extreme
  float satType = 1.0f;            // Easy: 0 Soft, 1 Tape, 2 Hard
  float oversample = 0.0f;         // 0 Off, 1 2x, 2 4x
};

// Metering published by the engine, in dB or normalised units.
struct Meters {
  float grDb = 0.0f;        // gain reduction, positive dB
  float outPeakDb = -120.0f;  // louder of the two output channels
  float outVuDb = -120.0f;    // averaged output for the VU needle
  float inPeakDb = -120.0f;   // louder of the two input channels
  // Per-channel levels, for stereo meters.
  float outPeakLDb = -120.0f;
  float outPeakRDb = -120.0f;
  float inPeakLDb = -120.0f;
  float inPeakRDb = -120.0f;
  // Detector source, for the transfer display: the side-chain input when the
  // host routed one and External is selected.
  float scPeakDb = -120.0f;
  bool scActive = false;
};

} // namespace comp
