// Suite Rack — renders the panel for visual checks
// SPDX-License-Identifier: GPL-3.0-or-later
#include <cairo.h>
#include <cstdio>
#include <string>
#include "modules.hpp"
#include "params.hpp"
#include "ui/panel.hpp"
using namespace rack;
int main(int argc, char** argv) {
  if (argc < 3) return 2;
  const std::string font = argv[1], out = argv[2];
  Shared s;
  initializeRack(s);
  Panel panel;
  panel.loadFont(font);
  PanelState state;
  cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, kPanelWidth, kPanelHeight);
  cairo_t* cr = cairo_create(surf);
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-eq.png").c_str());
  // Comp editor on slot 2.
  state.selectedSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-comp.png").c_str());
  state.browserSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-browser.png").c_str());
  state.browserCategory = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-browser-eq.png").c_str());
  state.browserCategory = 5;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-browser-space.png").c_str());
  state.browserCategory = 0;
  state.browserSlot = -1;
  state.selectedSlot = 4;
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::BleedOut));
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-bleedout.png").c_str());
  state.selectedSlot = 5;
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::Limiter));
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-limiter.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::Image));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Crush));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-image.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::Filter));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Motion));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-filter.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::Sub));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Double));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-sub.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::DynamicEq));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Multiband));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-dyn.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::Rider));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Cleanup));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-rider.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::SpacePlate));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::SpaceTape));
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::SpaceChorus));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::SpaceHall));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-space.png").c_str());
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::HearthComp));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Glue));
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::Comp2A));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::CompVCA));
  state.selectedSlot = 2;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-comp-family.png").c_str());
  applyModuleDefaults(s, 0, static_cast<int>(ModuleId::Comp76Blue));
  applyModuleDefaults(s, 1, static_cast<int>(ModuleId::CompXpress));
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::CompDigital));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::Comp3A));
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::Comp2A));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::CompVCA));
  state.selectedSlot = 1;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-comp-suite.png").c_str());
  applyModuleDefaults(s, 0, static_cast<int>(ModuleId::CompMolot));
  applyModuleDefaults(s, 1, static_cast<int>(ModuleId::CompRed));
  applyModuleDefaults(s, 2, static_cast<int>(ModuleId::Comp76Black));
  applyModuleDefaults(s, 3, static_cast<int>(ModuleId::CompXpress));
  applyModuleDefaults(s, 4, static_cast<int>(ModuleId::CompDigital));
  applyModuleDefaults(s, 5, static_cast<int>(ModuleId::CompVCA));
  state.selectedSlot = 0;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-comp-complete.png").c_str());
  state.aboutOpen = true;
  panel.draw(cr, s, state);
  cairo_surface_write_to_png(surf, (out + "/rack-about.png").c_str());
  cairo_destroy(cr);
  cairo_surface_destroy(surf);
  std::printf("wrote rack-eq.png and rack-comp.png\n");
  return 0;
}
