// Suite Rack — panel interaction tests
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>

#include "factory_module_presets.hpp"
#include "module_ids.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "presets.hpp"
#include "ui/panel.hpp"
#include "user_presets.hpp"
#include "value_text.hpp"

using namespace rack;

namespace {
int fails = 0;
void check(bool ok, const char* what) {
  std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
  if (!ok) ++fails;
}

bool clickParam(Panel& panel, Shared& s, PanelState& state, uint32_t param) {
  for (const auto& r : panel.widgetRects()) {
    if (r.param != param) continue;
    const double x = r.x + r.w * 0.5;
    const double y = r.y + r.h * 0.5;
    panel.pointerDown(s, state, x, y, false);
    panel.pointerUp(s, state);
    return true;
  }
  return false;
}

bool clickLabel(Panel& panel, Shared& s, PanelState& state,
                const char* label) {
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != label) continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    panel.pointerUp(s, state);
    return true;
  }
  return false;
}
} // namespace

int main() {
  const std::filesystem::path configRoot =
      "/tmp/opencode/suite-rack-test-config";
  std::filesystem::remove_all(configRoot);
  setenv("XDG_CONFIG_HOME", configRoot.string().c_str(), 1);

  cairo_surface_t* surf =
      cairo_image_surface_create(CAIRO_FORMAT_RGB24, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  Shared s;
  initializeRack(s);
  Panel panel;
  PanelState state;
  panel.draw(cr, s, state);

  check(state.selectedSlot == 0, "starts on slot 1");
  check(s.get(P_SLOT0_TYPE) == static_cast<double>(ModuleId::Eq4K),
        "slot 1 starts as 4K EQ");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Comp76Blue),
        "slot 2 starts as 76 Blue");

  // Click the second column's module name to open its browser.
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp(s, state);
  check(state.selectedSlot == 1 && state.browserSlot == 1,
        "clicking the second module name opens its browser");
  panel.draw(cr, s, state);

  check(clickLabel(panel, s, state, "OFF"), "browser offers Off");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Off),
        "Off module clears the slot");

  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 250, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "COMP"), "browser category tab is clickable");
  check(state.browserCategory == 2, "COMP tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "VCA"), "filtered browser offers VCA");
  check(s.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::CompVCA),
        "VCA module selects into the slot");

  // Slot 3: COLOUR category and the Valve module.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 441, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "COLOUR"),
        "COLOUR category tab is clickable");
  check(state.browserCategory == 3, "COLOUR tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Valve"), "filtered browser offers Valve");
  check(s.get(P_SLOT2_TYPE) == static_cast<double>(ModuleId::ColourValve),
        "Valve module selects into slot 3");

  // Slot 4: DYN category and the Gate module.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 596, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "DYN"), "DYN category tab is clickable");
  check(state.browserCategory == 4, "DYN tab filters the browser");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Gate"), "filtered browser offers Gate");
  check(s.get(P_SLOT3_TYPE) == static_cast<double>(ModuleId::DynGate),
        "Gate module selects into slot 4");

  // Slot 5: Bleed Out and its stepped profile control.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 750, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "DYN"), "DYN tab works on slot 5");
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Bleed Out"),
        "filtered browser offers Bleed Out");
  check(s.get(P_SLOT4_TYPE) == static_cast<double>(ModuleId::BleedOut),
        "Bleed Out selects into slot 5");

  const uint32_t profileParam = slotParam(4, kSlotOffsetC0 + 0);
  s.set(profileParam, 0.0);
  panel.draw(cr, s, state);
  bool wheeled = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param != profileParam) continue;
    wheeled = panel.wheel(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, 1.0);
    break;
  }
  check(wheeled, "Bleed Out profile knob is wheelable");
  check(std::fabs(s.get(profileParam) - 1.0 / 7.0) < 1e-6,
        "stepped profile snaps to the next profile");

  const uint32_t ghostOnParam = slotParam(4, kSlotOffsetC0 + 4);
  check(s.get(ghostOnParam) == 1.0, "ghost starts enabled");
  check(clickParam(panel, s, state, ghostOnParam),
        "ghost toggle is clickable");
  check(s.get(ghostOnParam) == 0.0, "ghost toggle turns ghost off");

  // Slot 6: Peak Limiter.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 906, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Peak Limiter"),
        "filtered browser offers Peak Limiter");
  check(s.get(P_SLOT5_TYPE) == static_cast<double>(ModuleId::Limiter),
        "Peak Limiter selects into slot 6");
  panel.draw(cr, s, state);
  bool limiterControls = true;
  for (int k = 0; k < 4; ++k) {
    bool found = false;
    for (const auto& r : panel.widgetRects()) {
      if (r.param == slotParam(5, kSlotOffsetC0 + k)) found = true;
    }
    limiterControls = limiterControls && found;
  }
  check(limiterControls, "Peak Limiter exposes all four controls");

  // Bypass pill for slot 2.
  panel.draw(cr, s, state);
  check(clickParam(panel, s, state, P_SLOT1_BYPASS),
        "bypass pill is clickable");
  check(s.get(P_SLOT1_BYPASS) == 1.0, "bypass pill toggles the selected slot");

  // Global power.
  check(clickParam(panel, s, state, P_POWER), "power button is clickable");
  check(s.get(P_POWER) == 0.0, "power button toggles global power");

  // The generic slot controls exist for the active module.
  panel.draw(cr, s, state);
  bool gainFound = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param == slotParam(0, kSlotOffsetGain)) gainFound = true;
  }
  check(gainFound, "per-slot gain is present");

  // Double-clicking a knob resets it to the active module's default.
  const uint32_t lowParam = slotParam(0, kSlotOffsetC0 + kCtlLow);
  s.set(lowParam, 0.90);
  panel.draw(cr, s, state);
  check(clickParam(panel, s, state, lowParam), "EQ low knob is clickable");
  check(clickParam(panel, s, state, lowParam),
        "EQ low knob accepts a double-click");
  check(std::fabs(s.get(lowParam) - 0.60) < 1e-6,
        "double-click resets the control to its module default");

  // Right-click text entry on the same knob.
  s.set(lowParam, 0.90);
  panel.draw(cr, s, state);
  bool entered = false;
  for (const auto& r : panel.widgetRects()) {
    if (r.param != lowParam || !r.knob) continue;
    panel.beginTextEntry(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5);
    entered = true;
    break;
  }
  check(entered && state.textParam == static_cast<int>(lowParam),
        "right-click starts text entry on a knob");
  for (const char* c = "7.5"; *c; ++c) {
    panel.textKey(s, state, 0, *c);
  }
  panel.textKey(s, state, Panel::kTextEnter, 0);
  check(std::fabs(s.get(lowParam) - (7.5 / 20.0 + 0.5)) < 1e-3,
        "typed text sets the control value");
  check(state.textParam < 0, "text entry closes after Enter");

  panel.draw(cr, s, state);
  for (const auto& r : panel.widgetRects()) {
    if (r.param != lowParam || !r.knob) continue;
    panel.beginTextEntry(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5);
    break;
  }
  panel.textKey(s, state, Panel::kTextEscape, 0);
  check(state.textParam < 0, "Escape cancels text entry");

  // Browser search filters the module list.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 262, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "ALL"), "ALL tab reopens the full catalog");
  panel.draw(cr, s, state);
  bool searchHit = false;
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != "SEARCH") continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    searchHit = true;
    break;
  }
  check(searchHit && state.browserSearch, "browser search field starts editing");
  for (const char* c = "cl-1b"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  check(state.browserQuery == "cl-1b", "browser search records the query");
  panel.draw(cr, s, state);
  check(!clickLabel(panel, s, state, "Pultec"),
        "browser search hides non-matching modules");
  check(clickLabel(panel, s, state, "CL-1B"),
        "search result can be chosen");
  check(state.browserSlot < 0 && state.browserQuery.empty(),
        "choosing a result closes the browser and clears the query");

  // User preset saving through the header SAVE button.
  const uint32_t savedControl = slotParam(0, kSlotOffsetC0 + kCtlLow);
  s.set(savedControl, 0.33);
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 860, 34, false);
  panel.pointerUp(s, state);
  check(state.savePreset, "SAVE opens the preset name field");
  for (const char* c = "Test Rack"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  check(panel.presetCount() == kPresetCount + 1,
        "saving adds a user preset to the list");
  check(std::string(panel.presetName(kPresetCount)) == "Test Rack",
        "saved preset keeps its name");
  check(std::filesystem::exists(userPresetDir() / "Test Rack.suite-rack"),
        "saved preset file exists");
  s.set(savedControl, 0.77);
  panel.applyPreset(s, state, kPresetCount);
  check(std::fabs(s.get(savedControl) - 0.33) < 1e-6,
        "user preset recalls the saved rack");

  // Rack preset browser lists factory and user presets.
  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  check(state.presetOpen, "clicking the preset bar opens the preset browser");
  s.set(savedControl, 0.99);
  panel.pointerDown(s, state, 200, 100.0, false);
  panel.pointerUp(s, state);
  check(!state.presetOpen && state.presetIndex == 0,
        "preset browser row loads a factory rack");
  check(std::fabs(s.get(savedControl) - 0.60) < 1e-6,
        "factory rack restores its stored values");
  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  panel.pointerDown(s, state, 200,
                    100.0 + kPresetCount * 20.0 + 10.0, false);
  panel.pointerUp(s, state);
  check(state.presetIndex == kPresetCount,
        "preset browser loads a saved user rack");
  check(std::fabs(s.get(savedControl) - 0.33) < 1e-6,
        "user rack restores its stored values");

  // Rename and delete the current user rack preset.
  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  panel.pointerDown(s, state, 240, 356, false);
  panel.pointerUp(s, state);
  check(state.renamePreset, "rack preset browser offers rename");
  for (const char* c = "Renamed Rack"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  check(std::string(panel.presetName(kPresetCount)) == "Renamed Rack",
        "rack preset rename updates the name");
  check(std::filesystem::exists(userPresetDir() / "Renamed Rack.suite-rack") &&
            !std::filesystem::exists(userPresetDir() / "Test Rack.suite-rack"),
        "rack preset rename moves the file");

  panel.pointerDown(s, state, 237, 34, false);
  panel.pointerUp(s, state);
  panel.pointerDown(s, state, 320, 356, false);  // DELETE arms confirmation
  panel.pointerUp(s, state);
  check(state.deletePresetConfirm, "rack preset delete asks for confirmation");
  panel.pointerDown(s, state, 320, 356, false);  // CONFIRM
  panel.pointerUp(s, state);
  check(panel.presetCount() == kPresetCount &&
            !std::filesystem::exists(userPresetDir() /
                                     "Renamed Rack.suite-rack"),
        "rack preset delete removes the file");

  // 2A factory presets use the makeup/mix control indices.
  const FactoryModulePreset* twoA = nullptr;
  for (const auto& f : kFactoryModulePresets) {
    if (std::string(f.name) == "2A Vocal Glue") twoA = &f;
  }
  check(twoA && std::fabs(twoA->controls[4] - 0.60f) < 1e-5 &&
            std::fabs(twoA->controls[5] - 1.0f) < 1e-5,
        "2A factory preset addresses gain and mix correctly");

  // Right-click a module name to save a per-module preset.
  const uint32_t highParam = slotParam(0, kSlotOffsetC0 + kCtlHigh);
  s.set(savedControl, 0.42);
  s.set(highParam, 0.66);
  panel.draw(cr, s, state);
  panel.beginTextEntry(s, state, 130, 134);
  check(state.saveModule, "right-click on a module name saves a module preset");
  for (const char* c = "Crunch EQ"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  check(std::filesystem::exists(modulePresetDir() / "Crunch EQ.suite-module"),
        "module preset file exists");

  s.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::Off));
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 130, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "PRESETS"),
        "browser offers the preset category");
  panel.draw(cr, s, state);
  bool presetSearch = false;
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != "SEARCH") continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    presetSearch = true;
    break;
  }
  check(presetSearch, "preset category has a search field");
  for (const char* c = "Crunch EQ"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "Crunch EQ"),
        "saved module preset appears in the browser");
  check(s.get(P_SLOT0_TYPE) == static_cast<double>(ModuleId::Eq4K),
        "module preset restores the module type");
  check(std::fabs(s.get(savedControl) - 0.42) < 1e-6 &&
            std::fabs(s.get(highParam) - 0.66) < 1e-6,
        "module preset restores the module controls");

  // Right-click a user module preset for rename/delete actions.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 130, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "PRESETS"),
        "module preset tab reopens for management");
  panel.draw(cr, s, state);
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != "SEARCH") continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    break;
  }
  for (const char* c = "Crunch EQ"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  panel.draw(cr, s, state);
  bool rightClicked = false;
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != "Crunch EQ") continue;
    panel.beginTextEntry(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5);
    rightClicked = true;
    break;
  }
  check(rightClicked && state.modulePresetMenu >= 0,
        "right-click opens module preset actions");
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 584, 349, false);  // CANCEL
  panel.pointerUp(s, state);
  check(state.modulePresetMenu < 0, "module preset actions can be cancelled");

  const auto loadedModules = loadModulePresets(modulePresetDir());
  bool moduleManagementOk = false;
  for (const auto& p : loadedModules) {
    if (p.name != "Crunch EQ") continue;
    if (!renameModulePreset(modulePresetDir(), p.name, "Crunch Renamed",
                            p.type, p.controls)) {
      break;
    }
    moduleManagementOk =
        std::filesystem::exists(modulePresetDir() / "Crunch Renamed.suite-module") &&
        !std::filesystem::exists(modulePresetDir() / "Crunch EQ.suite-module");
    moduleManagementOk =
        moduleManagementOk &&
        deleteModulePreset(modulePresetDir(), "Crunch Renamed") &&
        !std::filesystem::exists(modulePresetDir() /
                                 "Crunch Renamed.suite-module");
    break;
  }
  check(moduleManagementOk, "module presets can be renamed and deleted");
  state.browserSlot = -1;
  state.browserPresetFilter = false;
  state.browserQuery.clear();
  state.browserSearch = false;
  state.modulePresetMenu = -1;
  panel.draw(cr, s, state);

  // A factory preset modelled on Comp Suite.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 130, 134, false);
  panel.pointerUp(s, state);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "PRESETS"),
        "preset category reopens for factory content");
  panel.draw(cr, s, state);
  for (const auto& r : panel.widgetRects()) {
    if (std::string(r.label) != "SEARCH") continue;
    panel.pointerDown(s, state, r.x + r.w * 0.5, r.y + r.h * 0.5, false);
    break;
  }
  for (const char* c = "Vocal Push"; *c; ++c) panel.textKey(s, state, 0, *c);
  panel.textKey(s, state, Panel::kTextEnter, 0);
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "76 Vocal Push"),
        "factory Comp Suite preset appears in the browser");
  check(s.get(P_SLOT0_TYPE) == static_cast<double>(ModuleId::Comp76Blue),
        "factory preset loads its module");
  check(std::fabs(s.get(slotParam(0, kSlotOffsetC0 + kCtlThreshold)) - 0.55f) <
            1e-5,
        "factory preset loads its control values");

  // Card PRESETS button focuses the browser on the loaded module.
  panel.draw(cr, s, state);
  check(clickLabel(panel, s, state, "PRESET"),
        "card PRESET button opens the preset browser");
  check(state.browserSlot == 0 && state.browserCategory == 6 &&
            state.browserPresetFilter,
        "card PRESET button filters to the loaded module");
  panel.draw(cr, s, state);
  check(!clickLabel(panel, s, state, "2A Vocal Glue"),
        "module preset filter hides other module families");
  check(clickLabel(panel, s, state, "76 Vocal Push"),
        "filtered module preset can still be chosen");

  // Typed values understand units and snap stepped controls.
  {
    Shared typed;
    initializeRack(typed);
    double v = 0.0;
    typed.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::Comp76Blue));
    check(parseParamValue(typed,
                          slotParam(0, kSlotOffsetC0 + kCtlRatio), "4:1",
                          &v) &&
              std::fabs(v - 0.375) < 1e-6,
          "typed ratio snaps to the nearest ratio step");
    typed.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::SpacePlate));
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 2), "0.1s",
                          &v) &&
              std::fabs(v - 0.5) < 1e-6,
          "typed pre-delay converts seconds to milliseconds");
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 1), "2200ms",
                          &v) &&
              std::fabs(v - (2.2 - 0.3) / 11.7) < 1e-3,
          "typed decay converts milliseconds to seconds");
    typed.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::JcL2));
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 4), "23.9",
                          &v) &&
              std::fabs(v - 1.0) < 1e-6,
          "typed quantize snaps to the nearest bit depth");
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 5), "type2",
                          &v) &&
              std::fabs(v - 1.0) < 1e-6,
          "typed dither fuzzily snaps to the closest mode");
    typed.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::JcRS124));
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 0), "610",
                          &v) &&
              std::fabs(v - 0.5) < 1e-6,
          "typed model prefix snaps to the matching model");
    typed.set(P_SLOT0_TYPE, static_cast<double>(ModuleId::HybridEq));
    check(parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 5), "12k",
                          &v) &&
              std::fabs(v - std::log(12000.0 / 2500.0) / std::log(6.4)) <
                  1e-3,
          "typed high frequency accepts k suffix");
    check(!parseParamValue(typed, slotParam(0, kSlotOffsetC0 + 5), "banana",
                           &v),
          "invalid typed value is rejected");
  }

  check(!panel.widgetRects().empty(), "panel reports widget geometry");
  bool sourcesOk = true;
  for (int id = 0; id < kModuleCount; ++id) {
    if (!moduleSource(id) || !*moduleSource(id)) sourcesOk = false;
  }
  check(sourcesOk, "every module has a source label");
  check(std::string(moduleSource(static_cast<int>(ModuleId::SpacePlate))) ==
            "Space Suite",
        "Space modules report Space Suite");
  check(std::string(moduleSource(static_cast<int>(ModuleId::Cleanup))) ==
            "Repair Suite",
        "Cleanup reports Repair Suite");

  bool descriptorsOk = true;
  for (int id = 1; id < kModuleCount; ++id) {
    const ModuleDesc* d = moduleDesc(id);
    if (!d->name || !*d->name || !d->description || !*d->description) {
      descriptorsOk = false;
    }
    if (d->controlCount < 3 || d->controlCount > 8) descriptorsOk = false;
    bool seen[12] = {};
    for (int k = 0; k < d->controlCount; ++k) {
      const ControlDesc& c = d->controls[k];
      if (c.param < 0 || c.param >= 12 || seen[c.param]) descriptorsOk = false;
      if (c.param >= 0 && c.param < 12) seen[c.param] = true;
      if (!c.label || !*c.label || !c.name || !*c.name) descriptorsOk = false;
      const double def = defaultControlValue(d->id, k);
      if (def < 0.0 || def > 1.0) descriptorsOk = false;
    }
  }
  check(descriptorsOk,
        "every module has 3-8 valid controls, labels and defaults");

  bool factoryPresetsOk = true;
  for (const auto& f : kFactoryModulePresets) {
    if (!validModule(static_cast<int>(f.type)) ||
        f.type == ModuleId::Off) {
      factoryPresetsOk = false;
    }
    for (float v : f.controls) {
      if (v < 0.0f || v > 1.0f) factoryPresetsOk = false;
    }
  }
  check(factoryPresetsOk, "factory module presets reference valid modules and values");

  bool rackPresetsOk = true;
  for (const auto& p : kPresets) {
    for (int i = 0; i < p.count; ++i) {
      const int param = p.values[i].param;
      if (param < 0 || param >= static_cast<int>(P_COUNT)) {
        rackPresetsOk = false;
        continue;
      }
      const ParamSpec& spec = kParamSpecs[param];
      if (p.values[i].value < spec.min - 1e-6 ||
          p.values[i].value > spec.max + 1e-6) {
        rackPresetsOk = false;
      }
    }
  }
  check(rackPresetsOk, "factory rack presets reference valid parameters");

  const ParamSpec& slot1Type = kParamSpecs[slotParam(0, kSlotOffsetType)];
  const ParamSpec& slot1Ctl = kParamSpecs[slotParam(0, kSlotOffsetC0)];
  check(std::string(slot1Type.module) == "Slot 1" &&
            std::string(slot1Type.name) == "Module",
        "automation names use one-based slot numbers");
  check(std::string(slot1Ctl.name) == "Module Control 1",
        "automation names generic module controls clearly");
  check(slot1Type.labels.count == kModuleCount &&
            std::string(slot1Type.labels.names[0]) == "Off" &&
            std::string(moduleName(static_cast<int>(ModuleId::Off))) == "Off",
        "module automation options include Off and every module");

  // Drag slot 1 onto slot 2 and check the rack reorders.
  Shared dragS;
  initializeRack(dragS);
  Panel dragPanel;
  PanelState dragState;
  dragPanel.draw(cr, dragS, dragState);
  dragPanel.pointerDown(dragS, dragState, 130, 134, false);
  dragPanel.pointerMove(dragS, dragState, 285, 134, false);
  check(dragS.get(P_SLOT0_TYPE) ==
            static_cast<double>(ModuleId::Comp76Blue),
        "slots reorder live while dragging");
  dragPanel.pointerUp(dragS, dragState);
  check(dragS.get(P_SLOT0_TYPE) ==
            static_cast<double>(ModuleId::Comp76Blue),
        "drag moves the source module into the target slot");
  check(dragS.get(P_SLOT1_TYPE) == static_cast<double>(ModuleId::Eq4K),
        "the target slot shifts behind the dragged module");
  check(dragState.selectedSlot == 1, "selection follows the dragged module");

  // About button opens and closes the module/source/control reference.
  panel.draw(cr, s, state);
  panel.pointerDown(s, state, 930, 34, false);
  panel.pointerUp(s, state);
  check(state.aboutOpen, "ABOUT button opens the reference");
  check(state.aboutScroll == 0, "ABOUT starts at the first page");
  panel.wheel(s, state, 500, 300, -1.0);
  check(state.aboutScroll == 1, "ABOUT wheel scrolls down");
  panel.pointerDown(s, state, 149, 617, false);
  panel.pointerUp(s, state);
  check(state.aboutScroll == 0, "ABOUT UP button scrolls back");
  panel.pointerDown(s, state, 860, 667, false);
  panel.pointerUp(s, state);
  check(!state.aboutOpen, "ABOUT close button closes the reference");

  // Every control formats to text and parses back without drift.
  {
    bool ok = true;
    Shared values;
    initializeRack(values);
    for (int id = 1; id < kModuleCount; ++id) {
      const ModuleDesc* m = moduleDesc(id);
      for (int slot = 0; slot < kSlotCount; ++slot) {
        applyModuleDefaults(values, slot, static_cast<int>(ModuleId::Off),
                            false);
      }
      applyModuleDefaults(values, 0, id, false);
      for (int k = 0; k < m->controlCount; ++k) {
        const uint32_t p = slotParam(0, kSlotOffsetC0 + m->controls[k].param);
        for (double v : {0.0, 0.25, 0.5, 0.75, 1.0}) {
          const std::string text = formatParamValue(values, p, v);
          double out = -1.0;
          if (!parseParamValue(values, p, text, &out)) {
            ok = false;
            std::printf("parse failed: %s / %s -> '%s'\n", m->name,
                        m->controls[k].name, text.c_str());
            continue;
          }
          const bool quantized =
              m->controls[k].toggled || m->controls[k].stepped ||
              m->controls[k].labels.count > 0;
          const bool displayQuantized =
              m->controls[k].format == ControlFormat::AttackMs ||
              m->controls[k].format == ControlFormat::ReleaseMs ||
              m->controls[k].format == ControlFormat::Ratio;
          const double tolerance = displayQuantized ? 0.13 : 0.04;
          if (!quantized && std::fabs(out - v) > tolerance) {
            ok = false;
            std::printf("round-trip drift: %s / %s %.3f -> '%s' -> %.3f\n",
                        m->name, m->controls[k].name, v, text.c_str(), out);
          }
        }
      }
    }
    check(ok, "every typed control value round-trips");
  }


  {
    Shared ab;
    PanelState st;
    Panel p;
    p.draw(cr, ab, st);
    ab.set(P_INPUT, 2.0);
    p.pointerDown(ab, st, 617, 34, false);
    p.pointerUp(ab, st);
    check(ab.abActiveSlot() == 1, "A/B pill switches to B");
    ab.set(P_INPUT, 6.0);
    p.pointerDown(ab, st, 617, 34, false);
    p.pointerUp(ab, st);
    check(ab.abActiveSlot() == 0 && std::fabs(ab.get(P_INPUT) - 2.0) < 1e-9,
          "A/B pill returns to A");
    p.pointerDown(ab, st, 668, 34, false);
    p.pointerUp(ab, st);
    p.pointerDown(ab, st, 617, 34, false);
    p.pointerUp(ab, st);
    check(ab.abActiveSlot() == 1 && std::fabs(ab.get(P_INPUT) - 2.0) < 1e-9,
          "COPY copies A into B");
  }


  {
    namespace fs = std::filesystem;
    const fs::path root = fs::temp_directory_path() / "rack-preset-export-test";
    fs::remove_all(root);
    fs::create_directories(root / "config");
    fs::create_directories(root / "home");
    const std::string oldXdg =
        std::getenv("XDG_CONFIG_HOME") ? std::getenv("XDG_CONFIG_HOME") : "";
    const std::string oldHome =
        std::getenv("HOME") ? std::getenv("HOME") : "";
    setenv("XDG_CONFIG_HOME", (root / "config").string().c_str(), 1);
    setenv("HOME", (root / "home").string().c_str(), 1);

    Shared s;
    PanelState st;
    Panel p;
    p.applyPreset(s, st, 0);
    p.exportPreset(s, st);
    const std::string name = sanitizePresetName(kPresets[0].name);
    const fs::path exported =
        userPresetExportDir() / (name + ".suite-rack");
    check(fs::exists(exported), "EXPORT writes a rack preset");
    check(p.importPresetFile(s, st, exported.string()),
          "IMPORT reads a rack preset");
    check(st.presetIndex >= kPresetCount, "import selects the user preset");

    if (!oldXdg.empty()) setenv("XDG_CONFIG_HOME", oldXdg.c_str(), 1);
    else unsetenv("XDG_CONFIG_HOME");
    if (!oldHome.empty()) setenv("HOME", oldHome.c_str(), 1);
    else unsetenv("HOME");
    fs::remove_all(root);
  }

  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("%d failure(s)\n", fails);
  return fails == 0 ? 0 : 1;
}
