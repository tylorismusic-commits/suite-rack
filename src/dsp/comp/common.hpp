// Comp Suite model reused in Drum Suite — common.hpp
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Thin compatibility layer over hearth-dsp.  The shared DSP lives in
// <hdsp/...>; these aliases keep the model code and tests in namespace comp.
#pragma once

#include <hdsp/adaa.hpp>
#include <hdsp/db.hpp>
#include <hdsp/dynamics.hpp>
#include <hdsp/envelope.hpp>
#include <hdsp/sidechain.hpp>
#include <hdsp/smoother.hpp>
#include <hdsp/util.hpp>

namespace comp {

using hdsp::kPi;
using hdsp::clampf;
using hdsp::dbToGain;
using hdsp::gainToDb;
using hdsp::timeCoeff;
using hdsp::Smoothed;
using hdsp::OnePoleLP;
using hdsp::HighPass2;
using hdsp::Adaa;
using hdsp::logCosh;
using hdsp::AsymShaper;
using hdsp::ratioStepFrom;
using hdsp::ratioFromStep;
using hdsp::gainReductionDbSlope;
using hdsp::gainReductionDb;
using hdsp::Topology;
using hdsp::SidechainFrame;

} // namespace comp
