// Suite Rack — factory module presets based on the standalone Suite plugins
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <array>

#include "module_ids.hpp"

namespace rack {

struct FactoryModulePreset {
  const char* name;
  const char* source;
  ModuleId type;
  std::array<float, 12> controls;
};

// Controls follow each module's descriptor order; unspecified slots stay zero.
inline constexpr FactoryModulePreset kFactoryModulePresets[] = {
    // Comp Suite — 76 Blue
    {"76 Vocal Push", "Comp Suite", ModuleId::Comp76Blue,
     {0.55f, 0.375f, 0.35f, 0.45f, 0.55f, 1.0f, 0.10f}},
    {"76 Vocal Slam", "Comp Suite", ModuleId::Comp76Blue,
     {0.62f, 0.75f, 0.50f, 0.40f, 0.45f, 1.0f, 0.10f}},
    {"76 Vocal Gentle", "Comp Suite", ModuleId::Comp76Blue,
     {0.52f, 0.25f, 0.45f, 0.55f, 0.52f, 1.0f, 0.10f}},
    {"76 Kick Punch", "Comp Suite", ModuleId::Comp76Blue,
     {0.58f, 0.50f, 0.85f, 0.25f, 0.50f, 1.0f, 0.02f}},
    {"76 Snare Crack", "Comp Suite", ModuleId::Comp76Blue,
     {0.58f, 0.625f, 0.70f, 0.30f, 0.48f, 1.0f, 0.08f}},
    {"76 Drum Smash (par)", "Comp Suite", ModuleId::Comp76Blue,
     {0.60f, 0.75f, 0.60f, 0.30f, 0.42f, 0.35f, 0.05f}},
    {"76 Room Crush", "Comp Suite", ModuleId::Comp76Blue,
     {0.60f, 0.75f, 0.50f, 0.25f, 0.40f, 0.50f, 0.05f}},
    {"76 Bass DI", "Comp Suite", ModuleId::Comp76Blue,
     {0.55f, 0.25f, 0.55f, 0.55f, 0.50f, 1.0f, 0.02f}},
    {"76 Guitar Bite", "Comp Suite", ModuleId::Comp76Blue,
     {0.57f, 0.50f, 0.45f, 0.50f, 0.50f, 1.0f, 0.10f}},

    // Comp Suite — 76 Black
    {"76B Vocal Level", "Comp Suite", ModuleId::Comp76Black,
     {0.55f, 0.25f, 0.40f, 0.50f, 0.51f, 1.0f, 0.10f}},
    {"76B Bass Control", "Comp Suite", ModuleId::Comp76Black,
     {0.55f, 0.25f, 0.60f, 0.55f, 0.50f, 1.0f, 0.02f}},
    {"76B Drum Bus", "Comp Suite", ModuleId::Comp76Black,
     {0.55f, 0.25f, 0.75f, 0.35f, 0.52f, 0.85f, 0.05f}},
    {"76B Room Glue", "Comp Suite", ModuleId::Comp76Black,
     {0.55f, 0.50f, 0.55f, 0.30f, 0.40f, 0.45f, 0.05f}},

    // Comp Suite — 2A
    {"2A Vocal Glue", "Comp Suite", ModuleId::Comp2A,
     {0.365f, 0.0f, 0.0f, 0.0f, 0.60f, 1.0f}},
    {"2A Vocal Limit", "Comp Suite", ModuleId::Comp2A,
     {0.737f, 0.0f, 0.0f, 0.0f, 0.55f, 1.0f}},
    {"2A Bass Smooth", "Comp Suite", ModuleId::Comp2A,
     {0.475f, 0.0f, 0.0f, 0.0f, 0.65f, 1.0f}},
    {"2A Guitar Strum", "Comp Suite", ModuleId::Comp2A,
     {0.315f, 0.0f, 0.0f, 0.0f, 0.55f, 1.0f}},
    {"2A Room Level", "Comp Suite", ModuleId::Comp2A,
     {0.517f, 0.0f, 0.0f, 0.0f, 0.58f, 0.45f}},

    // Comp Suite — Clean / Xpress
    {"Clean Vocal Control", "Comp Suite", ModuleId::CompXpress,
     {0.50f, 0.25f, 0.35f, 0.50f, 0.55f, 1.0f, 0.05f, 0.0f}},
    {"Clean Drum Control", "Comp Suite", ModuleId::CompXpress,
     {0.50f, 0.375f, 0.20f, 0.35f, 0.55f, 1.0f, 0.05f, 0.0f}},
    {"Clean Mix Glue", "Comp Suite", ModuleId::CompXpress,
     {0.50f, 0.125f, 0.55f, 0.60f, 0.52f, 1.0f, 0.05f, 0.0f}},
    {"Clean Bass Control", "Comp Suite", ModuleId::CompXpress,
     {0.50f, 0.25f, 0.45f, 0.55f, 0.54f, 1.0f, 0.05f, 0.0f}},

    // Space Suite — reverbs
    {"Plate Vocal", "Space Suite", ModuleId::SpacePlate,
     {0.45f, 0.16239f, 0.10f, 0.55f, 0.65f, 0.28f}},
    {"Plate Drum Room", "Space Suite", ModuleId::SpacePlate,
     {0.35f, 0.07692f, 0.05f, 0.60f, 0.50f, 0.22f}},
    {"Hall Wide", "Space Suite", ModuleId::SpaceHall,
     {0.70f, 0.35897f, 0.175f, 0.45f, 0.80f, 0.30f}},
    {"Room Tight", "Space Suite", ModuleId::SpaceRoom,
     {0.40f, 0.05983f, 0.04f, 0.55f, 0.45f, 0.20f}},
    {"Spring Guitar", "Space Suite", ModuleId::SpaceSpring,
     {0.55f, 0.14530f, 0.00f, 0.50f, 0.60f, 0.30f}},

    // Space Suite — delays
    {"Slap Mono", "Space Suite", ModuleId::SpaceDigital,
     {0.04452f, 0.10f, 0.55f, 0.05f, 0.0f, 0.25f}},
    {"Tape Dotted 1/8", "Space Suite", ModuleId::SpaceTape,
     {0.18709f, 0.35f, 0.40f, 0.35f, 0.60f, 0.30f}},
    {"Analog Dark", "Space Suite", ModuleId::SpaceAnalog,
     {0.24962f, 0.40f, 0.30f, 0.20f, 0.50f, 0.30f}},
    {"Ping-Pong Wide", "Space Suite", ModuleId::SpacePingPong,
     {0.24962f, 0.45f, 0.55f, 0.15f, 1.00f, 0.35f}},

    // Space Suite — modulation
    {"Chorus Wide", "Space Suite", ModuleId::SpaceChorus,
     {0.3673f, 0.55f, 0.00f, 0.85f, 0.40f}},
    {"Flanger Jet", "Space Suite", ModuleId::SpaceFlanger,
     {0.2616f, 0.65f, 0.55f, 0.60f, 0.45f}},
    {"Phaser Slow", "Space Suite", ModuleId::SpacePhaser,
     {0.1652f, 0.70f, 0.35f, 0.70f, 0.40f}},
    {"Dimension Subtle", "Space Suite", ModuleId::SpaceDimension,
     {0.3038f, 0.40f, 0.00f, 0.80f, 0.35f}},

    // Trowel — hybrid EQ curves
    {"Vocal Air", "Trowel", ModuleId::HybridEq,
     {0.376f, 0.465f, 0.100f, 0.500f, 0.500f, 0.845f, 0.111f, 0.000f}},
    {"Drum Body", "Trowel", ModuleId::HybridEq,
     {0.000f, 0.268f, 0.167f, 0.400f, 0.550f, 0.627f, 0.056f, 0.000f}},
    {"Mix Polish", "Trowel", ModuleId::HybridEq,
     {0.200f, 0.465f, 0.033f, 0.500f, 0.520f, 0.747f, 0.083f, 0.050f}},

    // Bleed Out
    {"Kick Tight", "Bleed Out", ModuleId::BleedOut,
     {0.000f, 0.5333f, 0.75f, 0.50f, 0.0f}},
    {"Kick Body", "Bleed Out", ModuleId::BleedOut,
     {0.000f, 0.4333f, 0.55f, 0.50f, 0.0f}},
    {"Snare Tight", "Bleed Out", ModuleId::BleedOut,
     {0.142857f, 0.5000f, 0.85f, 0.50f, 0.0f}},
    {"Snare Natural", "Bleed Out", ModuleId::BleedOut,
     {0.142857f, 0.4000f, 0.55f, 0.50f, 0.0f}},
    {"Snare Ghost Notes", "Bleed Out", ModuleId::BleedOut,
     {0.142857f, 0.5333f, 0.80f, 0.75f, 1.0f}},
    {"Tom Room", "Bleed Out", ModuleId::BleedOut,
     {0.285714f, 0.4333f, 0.55f, 0.50f, 0.0f}},
    {"Hat Tight", "Bleed Out", ModuleId::BleedOut,
     {0.428571f, 0.4333f, 0.30f, 0.50f, 0.0f}},
    {"Ride Natural", "Bleed Out", ModuleId::BleedOut,
     {0.571429f, 0.3667f, 0.20f, 0.50f, 0.0f}},
    {"Room Glue", "Bleed Out", ModuleId::BleedOut,
     {0.714286f, 0.3667f, 0.15f, 0.50f, 0.0f}},
    {"OH Wide", "Bleed Out", ModuleId::BleedOut,
     {0.857143f, 0.3333f, 0.10f, 0.50f, 0.0f}},
    {"Bus Glue", "Bleed Out", ModuleId::BleedOut,
     {1.000f, 0.3000f, 0.00f, 0.50f, 0.0f}},

    // Trowel — Tame section
    {"Vocal - Air & Tame", "Trowel", ModuleId::Tame,
     {0.35f, 0.60f, 0.55f, 0.55f, 1.0f}},
    {"Drum Bus - Punch", "Trowel", ModuleId::Tame,
     {0.20f, 0.70f, 0.50f, 0.50f, 1.0f}},
    {"Mix Bus - Polish", "Trowel", ModuleId::Tame,
     {0.15f, 0.70f, 0.50f, 0.45f, 1.0f}},
    {"Broken Trowel", "Trowel", ModuleId::Tame,
     {1.00f, 0.50f, 0.80f, 0.80f, 1.0f}},

    // Ember
    {"Touch of Glow", "Ember", ModuleId::Ember, {0.18f}},
    {"Vocal Bus", "Ember", ModuleId::Ember, {0.35f}},
    {"Mix Bus Finish", "Ember", ModuleId::Ember, {0.48f}},
    {"Drum Bus Glue", "Ember", ModuleId::Ember, {0.58f}},
    {"Mastering Glow", "Ember", ModuleId::Ember, {0.72f}},
};

inline constexpr int kFactoryModulePresetCount =
    static_cast<int>(sizeof(kFactoryModulePresets) /
                     sizeof(kFactoryModulePresets[0]));

} // namespace rack
