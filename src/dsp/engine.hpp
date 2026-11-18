// Suite Rack — serial rack engine
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cmath>

#include <hdsp/db.hpp>
#include <hdsp/modules/modules.hpp>
#include <hdsp/smoother.hpp>

#include "controls.hpp"
#include "module_ids.hpp"
#include "module_map.hpp"

namespace rack {

class Engine {
public:
  void prepare(float sampleRate) noexcept {
    sr_ = sampleRate > 0.0f ? sampleRate : 48000.0f;
    for (auto& eq : eq_) eq.prepare(sr_);
    for (auto& comp : comp_) comp.prepare(sr_);
    for (auto& colour : colour_) colour.prepare(sr_);
    for (auto& gate : gate_) gate.prepare(sr_);
    for (auto& transient : transient_) transient.prepare(sr_);
    for (auto& clip : clip_) clip.prepare(sr_);
    for (auto& deess : deess_) deess.prepare(sr_);
    for (auto& density : density_) density.prepare(sr_);
    for (auto& bleed : bleedOut_) bleed.prepare(sr_);
    for (auto& limiter : limiter_) limiter.prepare(sr_);
    for (auto& image : image_) image.prepare(sr_);
    for (auto& crush : crush_) crush.prepare(sr_);
    for (auto& filter : filter_) filter.prepare(sr_);
    for (auto& motion : motion_) motion.prepare(sr_);
    for (auto& sub : sub_) sub.prepare(sr_);
    for (auto& doubler : double_) doubler.prepare(sr_);
    for (auto& dynEq : dynamicEq_) dynEq.prepare(sr_);
    for (auto& multiband : multiband_) multiband.prepare(sr_);
    for (auto& rider : rider_) rider.prepare(sr_);
    for (auto& reel : reel_) reel.prepare(sr_);
    for (auto& ember : ember_) ember.prepare(sr_);
    for (auto& tame : tame_) tame.prepare(sr_);
    for (auto& eq510 : eq510_) eq510.prepare(sr_);
    for (auto& pultec : pultec_) pultec.prepare(sr_);
    for (auto& classic : classicLimiter_) classic.prepare(sr_);
    for (auto& tape : tapeHead_) tape.prepare(sr_);
    for (auto& tube : tubeDriver_) tube.prepare(sr_);
    for (auto& da : daTube_) da.prepare(sr_);
    for (auto& fat : fattener_) fat.prepare(sr_);
    for (auto& ac1 : ac1_) ac1.prepare(sr_);
    for (auto& lmc : lmc1_) lmc.prepare(sr_);
    for (auto& modern : modernLimiter_) modern.prepare(sr_);
    for (auto& hyrax : hyrax_) hyrax.prepare(sr_);
    for (auto& bloom : bloom_) bloom.prepare(sr_);
    for (auto& rs : rs124_) rs.prepare(sr_);
    for (auto& cl : cl1b_) cl.prepare(sr_);
    for (auto& tape : duTape_) tape.prepare(sr_);
    for (auto& vbl : vbl_) vbl.prepare(sr_);
    for (auto& cleanup : cleanup_) cleanup.prepare(sr_);
    for (auto& reverb : spaceReverb_) reverb.prepare(sr_);
    for (auto& delay : spaceDelay_) delay.prepare(sr_);
    for (auto& mod : spaceMod_) mod.prepare(sr_);
    input_.setTime(15.0f, sr_);
    output_.setTime(15.0f, sr_);
    power_.setTime(20.0f, sr_);
    for (auto& gain : slotGain_) gain.setTime(15.0f, sr_);
    input_.reset(1.0f);
    output_.reset(1.0f);
    power_.reset(1.0f);
    for (auto& gain : slotGain_) gain.reset(1.0f);
    reset();
  }

  void setControls(const Controls& c) noexcept {
    controls_ = c;
    for (int i = 0; i < kSlotCount; ++i) {
      eq_[i].setControls(eqControlsFrom(c.slot[i]));
      comp_[i].setControls(compControlsFrom(c.slot[i]));
      colour_[i].setControls(colourControlsFrom(c.slot[i]));
      gate_[i].setControls(gateControlsFrom(c.slot[i]));
      transient_[i].setControls(transientControlsFrom(c.slot[i]));
      clip_[i].setControls(clipControlsFrom(c.slot[i]));
      deess_[i].setControls(deEssControlsFrom(c.slot[i]));
      density_[i].setControls(densityControlsFrom(c.slot[i]));
      bleedOut_[i].setControls(bleedOutControlsFrom(c.slot[i]));
      limiter_[i].setControls(limiterControlsFrom(c.slot[i]));
      image_[i].setControls(imageControlsFrom(c.slot[i]));
      crush_[i].setControls(crushControlsFrom(c.slot[i]));
      filter_[i].setControls(filterControlsFrom(c.slot[i]));
      motion_[i].setControls(motionControlsFrom(c.slot[i]));
      sub_[i].setControls(subControlsFrom(c.slot[i]));
      double_[i].setControls(doubleControlsFrom(c.slot[i]));
      dynamicEq_[i].setControls(dynamicEqControlsFrom(c.slot[i]));
      multiband_[i].setControls(multibandControlsFrom(c.slot[i]));
      rider_[i].setControls(riderControlsFrom(c.slot[i]));
      reel_[i].setControls(reelControlsFrom(c.slot[i]));
      ember_[i].setControls(emberControlsFrom(c.slot[i]));
      tame_[i].setControls(tameControlsFrom(c.slot[i]));
      eq510_[i].setControls(eq510ControlsFrom(c.slot[i]));
      pultec_[i].setControls(pultecControlsFrom(c.slot[i]));
      classicLimiter_[i].setControls(classicLimiterControlsFrom(c.slot[i]));
      tapeHead_[i].setControls(tapeHeadControlsFrom(c.slot[i]));
      tubeDriver_[i].setControls(tubeDriverControlsFrom(c.slot[i]));
      daTube_[i].setControls(daTubeControlsFrom(c.slot[i]));
      fattener_[i].setControls(fattenerControlsFrom(c.slot[i]));
      ac1_[i].setControls(ac1ControlsFrom(c.slot[i]));
      lmc1_[i].setControls(lmc1ControlsFrom(c.slot[i]));
      modernLimiter_[i].setControls(modernLimiterControlsFrom(c.slot[i]));
      hyrax_[i].setControls(hyraxControlsFrom(c.slot[i]));
      bloom_[i].setControls(bloomControlsFrom(c.slot[i]));
      rs124_[i].setControls(rs124ControlsFrom(c.slot[i]));
      cl1b_[i].setControls(cl1bControlsFrom(c.slot[i]));
      duTape_[i].setControls(duTapeControlsFrom(c.slot[i]));
      vbl_[i].setControls(vblControlsFrom(c.slot[i]));
      cleanup_[i].setControls(cleanupControlsFrom(c.slot[i]));
      spaceReverb_[i].setControls(
          spaceReverbControlsFrom(c.slot[i], spaceReverbMode(c.slot[i].type)));
      spaceDelay_[i].setControls(
          spaceDelayControlsFrom(c.slot[i], spaceDelayMode(c.slot[i].type)));
      spaceMod_[i].setControls(
          spaceModControlsFrom(c.slot[i], spaceModMode(c.slot[i].type)));
      slotGain_[i].setTarget(hdsp::dbToGain(c.slot[i].gainDb));
    }
    input_.setTarget(hdsp::dbToGain(c.inputDb));
    output_.setTarget(hdsp::dbToGain(c.outputDb));
    power_.setTarget(c.power ? 1.0f : 0.0f);
  }

  void process(float* left, float* right, int frames, Meters& meters) noexcept {
    float inL = 0.0f, inR = 0.0f, outL = 0.0f, outR = 0.0f;
    float slotGr[kSlotCount] = {};
    for (int i = 0; i < frames; ++i) {
      const float rawL = std::isfinite(left[i]) ? left[i] : 0.0f;
      const float rawR = std::isfinite(right[i]) ? right[i] : 0.0f;
      const float gainIn = input_.next();
      const float dryL = rawL * gainIn;
      const float dryR = rawR * gainIn;
      float l = dryL, r = dryR;
      for (int s = 0; s < kSlotCount; ++s) {
        const SlotControls& c = controls_.slot[s];
        if (c.bypass) continue;
        const float sl = l, sr = r;
        float gr = 0.0f;
        // Explicit switch: with -Werror=switch, adding a ModuleId without
        // handling it here is a compile error. The previous category helpers
        // let Bleed Out and the Peak Limiter fall through silently.
        switch (static_cast<ModuleId>(c.type)) {
          case ModuleId::Off:
            continue;
          case ModuleId::Eq4K:
          case ModuleId::EqHearth:
          case ModuleId::EqTilt:
          case ModuleId::EqAir:
            eq_[s].process(l, r);
            break;
          case ModuleId::Comp76Blue:
          case ModuleId::Comp76Black:
          case ModuleId::Comp2A:
          case ModuleId::Comp3A:
          case ModuleId::CompVCA:
          case ModuleId::HearthComp:
          case ModuleId::Glue:
          case ModuleId::CompXpress:
          case ModuleId::CompDigital:
          case ModuleId::CompMolot:
          case ModuleId::CompRed:
            gr = comp_[s].process(l, r);
            break;
          case ModuleId::Reel:
            reel_[s].process(l, r);
            break;
          case ModuleId::Ember:
            ember_[s].process(l, r);
            gr = ember_[s].lastGrDb();
            break;
          case ModuleId::Tame:
            tame_[s].process(l, r);
            break;
          case ModuleId::Eq510:
            eq510_[s].process(l, r);
            break;
          case ModuleId::Pultec:
            pultec_[s].process(l, r);
            break;
          case ModuleId::JcClassicLimiter:
            classicLimiter_[s].process(l, r);
            gr = classicLimiter_[s].lastGrDb();
            break;
          case ModuleId::JcTapeHead:
            tapeHead_[s].process(l, r);
            break;
          case ModuleId::JcTubeDriver:
            tubeDriver_[s].process(l, r);
            break;
          case ModuleId::JcDaTube:
            daTube_[s].process(l, r);
            break;
          case ModuleId::JcFattener:
            fattener_[s].process(l, r);
            break;
          case ModuleId::JcAC1:
            ac1_[s].process(l, r);
            gr = ac1_[s].lastGrDb();
            break;
          case ModuleId::JcLMC1:
            lmc1_[s].process(l, r);
            gr = lmc1_[s].lastGrDb();
            break;
          case ModuleId::JcModernLimiter:
            modernLimiter_[s].process(l, r);
            gr = modernLimiter_[s].lastGrDb();
            break;
          case ModuleId::JcHyrax:
            hyrax_[s].process(l, r);
            gr = hyrax_[s].lastGrDb();
            break;
          case ModuleId::JcBloom:
            bloom_[s].process(l, r);
            break;
          case ModuleId::JcRS124:
            rs124_[s].process(l, r);
            gr = rs124_[s].lastGrDb();
            break;
          case ModuleId::JcCL1B:
            cl1b_[s].process(l, r);
            gr = cl1b_[s].lastGrDb();
            break;
          case ModuleId::JcDuTape:
            duTape_[s].process(l, r);
            break;
          case ModuleId::JcVBL:
            vbl_[s].process(l, r);
            gr = vbl_[s].lastGrDb();
            break;
          case ModuleId::ColourValve:
          case ModuleId::ColourConsole:
          case ModuleId::ColourBus:
          case ModuleId::ColourTape:
          case ModuleId::ColourTransformer:
            colour_[s].process(l, r);
            break;
          case ModuleId::DynGate:
            gate_[s].process(l, r);
            gr = gate_[s].lastGrDb();
            break;
          case ModuleId::DynTransient:
            transient_[s].process(l, r);
            break;
          case ModuleId::DynClip:
            clip_[s].process(l, r);
            break;
          case ModuleId::DynDeEss:
            deess_[s].process(l, r);
            gr = deess_[s].lastGrDb();
            break;
          case ModuleId::Density:
            density_[s].process(l, r);
            break;
          case ModuleId::BleedOut:
            bleedOut_[s].process(l, r);
            gr = bleedOut_[s].lastGrDb();
            break;
          case ModuleId::Limiter:
            limiter_[s].process(l, r);
            gr = limiter_[s].lastGrDb();
            break;
          case ModuleId::Image:
            image_[s].process(l, r);
            break;
          case ModuleId::Crush:
            crush_[s].process(l, r);
            break;
          case ModuleId::Filter:
            filter_[s].process(l, r);
            break;
          case ModuleId::Motion:
            motion_[s].process(l, r);
            break;
          case ModuleId::Sub:
            sub_[s].process(l, r);
            break;
          case ModuleId::Double:
            double_[s].process(l, r);
            break;
          case ModuleId::DynamicEq:
            dynamicEq_[s].process(l, r);
            gr = dynamicEq_[s].lastGrDb();
            break;
          case ModuleId::Multiband:
            multiband_[s].process(l, r);
            gr = multiband_[s].lastGrDb();
            break;
          case ModuleId::Rider:
            rider_[s].process(l, r);
            gr = rider_[s].lastGrDb();
            break;
          case ModuleId::Cleanup:
            cleanup_[s].process(l, r);
            break;
          case ModuleId::SpacePlate:
          case ModuleId::SpaceRoom:
          case ModuleId::SpaceHall:
          case ModuleId::SpaceSpring:
            spaceReverb_[s].process(l, r);
            break;
          case ModuleId::SpaceDigital:
          case ModuleId::SpaceTape:
          case ModuleId::SpaceAnalog:
          case ModuleId::SpacePingPong:
            spaceDelay_[s].process(l, r);
            break;
          case ModuleId::SpaceChorus:
          case ModuleId::SpaceFlanger:
          case ModuleId::SpacePhaser:
          case ModuleId::SpaceDimension:
            spaceMod_[s].process(l, r);
            break;
          case ModuleId::Count:
            continue;
        }
        const float mix = std::clamp(c.mix, 0.0f, 1.0f);
        l = sl + (l - sl) * mix;
        r = sr + (r - sr) * mix;
        const float gain = slotGain_[s].next();
        l *= gain;
        r *= gain;
        slotGr[s] = std::max(slotGr[s], gr);
      }
      const float active = power_.next();
      l = dryL + active * (l - dryL);
      r = dryR + active * (r - dryR);
      const float gainOut = output_.next();
      l *= gainOut;
      r *= gainOut;
      left[i] = std::isfinite(l) ? l : 0.0f;
      right[i] = std::isfinite(r) ? r : 0.0f;
      inL = std::max(inL, std::fabs(dryL));
      inR = std::max(inR, std::fabs(dryR));
      outL = std::max(outL, std::fabs(left[i]));
      outR = std::max(outR, std::fabs(right[i]));
    }
    meters.inPeakLDb = hdsp::gainToDb(inL);
    meters.inPeakRDb = hdsp::gainToDb(inR);
    meters.inPeakDb = std::max(meters.inPeakLDb, meters.inPeakRDb);
    meters.outPeakLDb = hdsp::gainToDb(outL);
    meters.outPeakRDb = hdsp::gainToDb(outR);
    meters.outPeakDb = std::max(meters.outPeakLDb, meters.outPeakRDb);
    for (int s = 0; s < kSlotCount; ++s) meters.slotGrDb[s] = slotGr[s];
  }

  void reset() noexcept {
    for (auto& eq : eq_) eq.reset();
    for (auto& comp : comp_) comp.reset();
    for (auto& colour : colour_) colour.reset();
    for (auto& gate : gate_) gate.reset();
    for (auto& transient : transient_) transient.reset();
    for (auto& clip : clip_) clip.reset();
    for (auto& deess : deess_) deess.reset();
    for (auto& density : density_) density.reset();
    for (auto& bleed : bleedOut_) bleed.reset();
    for (auto& limiter : limiter_) limiter.reset();
    for (auto& image : image_) image.reset();
    for (auto& crush : crush_) crush.reset();
    for (auto& filter : filter_) filter.reset();
    for (auto& motion : motion_) motion.reset();
    for (auto& sub : sub_) sub.reset();
    for (auto& doubler : double_) doubler.reset();
    for (auto& dynEq : dynamicEq_) dynEq.reset();
    for (auto& multiband : multiband_) multiband.reset();
    for (auto& rider : rider_) rider.reset();
    for (auto& reel : reel_) reel.reset();
    for (auto& ember : ember_) ember.reset();
    for (auto& tame : tame_) tame.reset();
    for (auto& eq510 : eq510_) eq510.reset();
    for (auto& pultec : pultec_) pultec.reset();
    for (auto& classic : classicLimiter_) classic.reset();
    for (auto& tape : tapeHead_) tape.reset();
    for (auto& tube : tubeDriver_) tube.reset();
    for (auto& da : daTube_) da.reset();
    for (auto& fat : fattener_) fat.reset();
    for (auto& ac1 : ac1_) ac1.reset();
    for (auto& lmc : lmc1_) lmc.reset();
    for (auto& modern : modernLimiter_) modern.reset();
    for (auto& hyrax : hyrax_) hyrax.reset();
    for (auto& bloom : bloom_) bloom.reset();
    for (auto& rs : rs124_) rs.reset();
    for (auto& cl : cl1b_) cl.reset();
    for (auto& tape : duTape_) tape.reset();
    for (auto& vbl : vbl_) vbl.reset();
    for (auto& cleanup : cleanup_) cleanup.reset();
    for (auto& reverb : spaceReverb_) reverb.reset();
    for (auto& delay : spaceDelay_) delay.reset();
    for (auto& mod : spaceMod_) mod.reset();
    input_.snap();
    output_.snap();
    power_.snap();
    for (auto& gain : slotGain_) gain.snap();
  }

  int latencySamples() const noexcept {
    int latency = 0;
    for (int i = 0; i < kSlotCount; ++i) {
      const SlotControls& slot = controls_.slot[i];
      if (slot.bypass) continue;
      if (slot.type == static_cast<int>(ModuleId::BleedOut)) {
        latency += bleedOut_[i].latencySamples();
      } else if (slot.type == static_cast<int>(ModuleId::Limiter)) {
        latency += limiter_[i].latencySamples();
      } else if (slot.type == static_cast<int>(ModuleId::Reel)) {
        latency += reel_[i].latencySamples();
      } else if (slot.type == static_cast<int>(ModuleId::JcClassicLimiter)) {
        latency += classicLimiter_[i].latencySamples();
      } else if (slot.type == static_cast<int>(ModuleId::JcModernLimiter)) {
        latency += modernLimiter_[i].latencySamples();
      } else if (slot.type == static_cast<int>(ModuleId::JcHyrax)) {
        latency += hyrax_[i].latencySamples();
      }
    }
    return latency;
  }

private:
  float sr_ = 48000.0f;
  Controls controls_;
  hdsp::modules::Eq eq_[kSlotCount];
  hdsp::modules::Comp comp_[kSlotCount];
  hdsp::modules::Colour colour_[kSlotCount];
  hdsp::modules::Gate gate_[kSlotCount];
  hdsp::modules::Transient transient_[kSlotCount];
  hdsp::modules::Clip clip_[kSlotCount];
  hdsp::modules::DeEss deess_[kSlotCount];
  hdsp::modules::Density density_[kSlotCount];
  hdsp::modules::BleedOut bleedOut_[kSlotCount];
  hdsp::modules::Limiter limiter_[kSlotCount];
  hdsp::modules::Image image_[kSlotCount];
  hdsp::modules::Crush crush_[kSlotCount];
  hdsp::modules::Filter filter_[kSlotCount];
  hdsp::modules::Motion motion_[kSlotCount];
  hdsp::modules::Sub sub_[kSlotCount];
  hdsp::modules::Double double_[kSlotCount];
  hdsp::modules::DynamicEq dynamicEq_[kSlotCount];
  hdsp::modules::Multiband multiband_[kSlotCount];
  hdsp::modules::Rider rider_[kSlotCount];
  hdsp::modules::Reel reel_[kSlotCount];
  hdsp::modules::Ember ember_[kSlotCount];
  hdsp::modules::Tame tame_[kSlotCount];
  hdsp::modules::Eq510 eq510_[kSlotCount];
  hdsp::modules::Pultec pultec_[kSlotCount];
  hdsp::modules::ClassicLimiter classicLimiter_[kSlotCount];
  hdsp::modules::TapeHead tapeHead_[kSlotCount];
  hdsp::modules::TubeDriver tubeDriver_[kSlotCount];
  hdsp::modules::DaTube daTube_[kSlotCount];
  hdsp::modules::Fattener fattener_[kSlotCount];
  hdsp::modules::AC1 ac1_[kSlotCount];
  hdsp::modules::LMC1 lmc1_[kSlotCount];
  hdsp::modules::ModernLimiter modernLimiter_[kSlotCount];
  hdsp::modules::Hyrax hyrax_[kSlotCount];
  hdsp::modules::Bloom bloom_[kSlotCount];
  hdsp::modules::RS124 rs124_[kSlotCount];
  hdsp::modules::CL1B cl1b_[kSlotCount];
  hdsp::modules::DuTape duTape_[kSlotCount];
  hdsp::modules::VBL vbl_[kSlotCount];
  hdsp::modules::Cleanup cleanup_[kSlotCount];
  hdsp::modules::SpaceReverb spaceReverb_[kSlotCount];
  hdsp::modules::SpaceDelay spaceDelay_[kSlotCount];
  hdsp::modules::SpaceMod spaceMod_[kSlotCount];
  hdsp::Smoothed input_, output_, power_;
  hdsp::Smoothed slotGain_[kSlotCount];
};

} // namespace rack
