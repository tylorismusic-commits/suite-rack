// Suite Rack — CLAP plugin
// SPDX-License-Identifier: GPL-3.0-or-later

#include <clap/all.h>
#include <clap/ext/draft/gain-adjustment-metering.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <cairo-xlib.h>
#include <dlfcn.h>
#include <sys/time.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <condition_variable>
#include <cstdarg>
#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

#include "dsp/engine.hpp"
#include "modules.hpp"
#include "params.hpp"
#include "state.hpp"
#include "value_text.hpp"
#include "version.hpp"
#include "ui/geometry.hpp"
#include "ui/panel.hpp"
#include "ui/xdnd.hpp"

namespace rack {
namespace {

// Set RACK_DEBUG=1 to trace the editor thread to /tmp/suiterack-ui.log.
bool debugOn() {
  static const bool on = std::getenv("RACK_DEBUG") != nullptr;
  return on;
}

void dbg(const char* fmt, ...) {
  if (!debugOn()) {
    return;
  }
  FILE* f = std::fopen("/tmp/suiterack-ui.log", "a");
  if (!f) {
    return;
  }
  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(f, fmt, ap);
  va_end(ap);
  std::fputc('\n', f);
  std::fclose(f);
}

constexpr const char* kPluginId = "com.tylorismusic.suiterack";
constexpr const char* kPluginName = "Suite Rack";
constexpr const char* kVendor = "tylorismusic";

std::string bundleResourceDir() {
  Dl_info info{};
  if (dladdr(reinterpret_cast<void*>(&bundleResourceDir), &info) && info.dli_fname) {
    std::string path = info.dli_fname;
    const size_t slash = path.rfind('/');
    if (slash != std::string::npos) {
      path = path.substr(0, slash);
    }
    return path + "/SuiteRack.resources";
  }
  return "SuiteRack.resources";
}

// ---------------------------------------------------------------------------
// X11 editor.  Owns the window and the Cairo surface, and runs its own
// thread so the host's audio and main threads are never blocked on drawing.
// ---------------------------------------------------------------------------

class Editor {
public:
  explicit Editor(Shared& shared) : shared_(shared) {}

  // Called when the user changes a control, so the host can ask for a
  // process call and pick up the new values.
  std::function<void()> requestFlush;

  double scale() const { return scale_.load(std::memory_order_relaxed); }
  UiSize size() const { return sizeForScale(scale()); }

  // Host thread: record the request only.  All Xlib calls happen on the
  // editor thread, because Xlib is not thread-safe and calling
  // XInitThreads() after the host has used Xlib is undefined behaviour
  // (that made the editor fail to load in REAPER).
  void resize(double scale) {
    scale_.store(clampScale(scale), std::memory_order_relaxed);
    resizePending_.store(true, std::memory_order_relaxed);
  }

  bool open(void* parent) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (thread_.joinable()) {
      return opened_;
    }
    parentWindow_ =
        parent ? static_cast<Window>(reinterpret_cast<uintptr_t>(parent)) : 0;
    opened_ = false;
    openFailed_ = false;
    quitting_ = false;
    thread_ = std::thread([this] { run(); });
    cv_.wait_for(lock, std::chrono::seconds(3),
                 [this] { return opened_ || openFailed_; });
    return opened_;
  }

  void close() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      quitting_ = true;
    }
    if (thread_.joinable()) {
      thread_.join();
    }
    std::lock_guard<std::mutex> lock(mutex_);
    opened_ = false;
  }

  ~Editor() { close(); }

private:
  void run() {
    dbg("thread start");
    Display* dpy = XOpenDisplay(nullptr);
    dbg("display=%p", static_cast<void*>(dpy));
    if (!dpy) {
      std::lock_guard<std::mutex> lock(mutex_);
      openFailed_ = true;
      cv_.notify_all();
      return;
    }
    const int screen = DefaultScreen(dpy);
    const Window parent = parentWindow_ ? parentWindow_ : RootWindow(dpy, screen);
    UiSize current = size();

    XSetWindowAttributes attrs{};
    // Button1MotionMask is essential: PointerMotionMask only reports motion
    // with no button held, so dragging a knob would never deliver events.
    attrs.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask |
                       PointerMotionMask | Button1MotionMask | ButtonMotionMask |
                       KeyPressMask;
    const Window win =
        XCreateWindow(dpy, parent, 0, 0, current.width, current.height, 0,
                      CopyFromParent, InputOutput, CopyFromParent, CWEventMask,
                      &attrs);

    xdnd_.init(dpy, win, [this](const std::string& path) {
      panel_.importPresetFile(shared_, state_, path);
    });
    const UiSize minSize = sizeForScale(kMinUiScale);
    const UiSize maxSize = sizeForScale(kMaxUiScale);
    XSizeHints hints{};
    hints.flags = PAspect | PMinSize | PMaxSize;
    hints.min_aspect.x = hints.max_aspect.x = kPanelWidth;
    hints.min_aspect.y = hints.max_aspect.y = kPanelHeight;
    hints.min_width = static_cast<int>(minSize.width);
    hints.min_height = static_cast<int>(minSize.height);
    hints.max_width = static_cast<int>(maxSize.width);
    hints.max_height = static_cast<int>(maxSize.height);
    XSetWMNormalHints(dpy, win, &hints);
    XStoreName(dpy, win, kPluginName);
    XMapRaised(dpy, win);
    XFlush(dpy);

    panel_.loadFont(bundleResourceDir());
    {
      std::lock_guard<std::mutex> lock(mutex_);
      display_ = dpy;
      window_ = win;
      opened_ = true;
      cv_.notify_all();
    }

    // Use the window's own visual and depth.  Hosts often give the plugin a
    // parent with a non-default visual (ARGB), and cairo refuses to draw on a
    // surface whose visual does not match the drawable.
    XWindowAttributes wa{};
    XGetWindowAttributes(dpy, win, &wa);
    dbg("window=%lu depth=%d visual=%p size=%dx%d", win, wa.depth,
        static_cast<void*>(wa.visual), current.width, current.height);
    cairo_surface_t* windowSurface =
        cairo_xlib_surface_create(dpy, win, wa.visual, current.width, current.height);
    dbg("xlib surface status=%s",
        cairo_status_to_string(cairo_surface_status(windowSurface)));
    const cairo_format_t imageFormat =
        wa.depth == 32 ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
    cairo_surface_t* backing = nullptr;
    cairo_t* cr = nullptr;

    auto rebuildBacking = [&]() {
      if (cr) {
        cairo_destroy(cr);
        cr = nullptr;
      }
      if (backing) {
        cairo_surface_destroy(backing);
        backing = nullptr;
      }
      backing = cairo_image_surface_create(imageFormat, current.width,
                                           current.height);
      cr = cairo_create(backing);
    };
    rebuildBacking();

    while (true) {
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (quitting_) {
          break;
        }
      }
      const UiSize want = size();
      if (want.width != current.width || want.height != current.height) {
        current = want;
        XResizeWindow(dpy, win, current.width, current.height);
        cairo_xlib_surface_set_size(windowSurface, current.width, current.height);
        rebuildBacking();
      }
      while (XPending(dpy) > 0) {
        XEvent ev;
        XNextEvent(dpy, &ev);
        handleEvent(ev);
      }
      panel_.tick(shared_, state_);
      cairo_save(cr);
      cairo_scale(cr, scale(), scale());
      panel_.draw(cr, shared_, state_);
      cairo_restore(cr);
      cairo_surface_flush(backing);
      cairo_t* xcr = cairo_create(windowSurface);
      cairo_set_source_surface(xcr, backing, 0, 0);
      cairo_paint(xcr);
      cairo_destroy(xcr);
      // Required: cairo's Xlib backend buffers the composite, and without
      // this flush nothing reaches the server (the window stays black).
      cairo_surface_flush(windowSurface);
      XFlush(dpy);
      static int frames = 0;
      if (debugOn() && (frames++ % 120) == 0) {
        XWindowAttributes ga;
        XGetWindowAttributes(dpy, win, &ga);
        dbg("frame %d geom=%dx%d mapped=%d", frames, ga.width, ga.height,
            ga.map_state == IsViewable);
      }
      if (requestFlush && shared_.anyDirty()) {
        requestFlush();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

    if (cr) {
      cairo_destroy(cr);
    }
    if (backing) {
      cairo_surface_destroy(backing);
    }
    if (windowSurface) {
      cairo_surface_destroy(windowSurface);
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      display_ = nullptr;
      window_ = 0;
    }
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
  }

  // Event coordinates are physical pixels; the panel works in logical units.
  void handleEvent(const XEvent& ev) {
    if (xdnd_.handle(ev)) {
      return;
    }
    const double s = scale();
    switch (ev.type) {
      case ButtonPress: {
        if (display_ && window_) {
          XSetInputFocus(display_, window_, RevertToParent, CurrentTime);
          XFlush(display_);
        }
        const bool fine = (ev.xbutton.state & ShiftMask) != 0;
        const double x = ev.xbutton.x / s;
        const double y = ev.xbutton.y / s;
        if (ev.xbutton.button == Button4) {
          panel_.wheel(shared_, state_, x, y, 1.0);
        } else if (ev.xbutton.button == Button5) {
          panel_.wheel(shared_, state_, x, y, -1.0);
        } else if (ev.xbutton.button == Button1) {
          panel_.pointerDown(shared_, state_, x, y, fine);
        } else if (ev.xbutton.button == Button3) {
          panel_.beginTextEntry(shared_, state_, x, y);
        }
        break;
      }
      case ButtonRelease:
        panel_.pointerUp(shared_, state_);
        break;
      case KeyPress: {
        char buf[8] = {};
        KeySym sym = 0;
        XKeyEvent key = ev.xkey;
        const int n = XLookupString(&key, buf, sizeof(buf), &sym, nullptr);
        if (sym == XK_Return || sym == XK_KP_Enter) {
          panel_.textKey(shared_, state_, Panel::kTextEnter, 0);
        } else if (sym == XK_Escape) {
          panel_.textKey(shared_, state_, Panel::kTextEscape, 0);
        } else if (sym == XK_BackSpace) {
          panel_.textKey(shared_, state_, Panel::kTextBackspace, 0);
        } else if (n > 0) {
          panel_.textKey(shared_, state_, 0, buf[0]);
        }
        break;
      }
      case MotionNotify: {
        const bool fine = (ev.xmotion.state & ShiftMask) != 0;
        panel_.pointerMove(shared_, state_, ev.xmotion.x / s, ev.xmotion.y / s,
                           fine);
        break;
      }
      default:
        break;
    }
  }

  Shared& shared_;
  PanelState state_;
  Panel panel_;
  XdndDropTarget xdnd_;
  std::atomic<double> scale_{1.0};
  std::atomic<bool> resizePending_{false};
  std::mutex mutex_;
  std::condition_variable cv_;
  Display* display_ = nullptr;
  Window window_ = 0;
  Window parentWindow_ = 0;
  bool opened_ = false;
  bool openFailed_ = false;
  bool quitting_ = false;
  std::thread thread_;
};

// ---------------------------------------------------------------------------
// Plugin
// ---------------------------------------------------------------------------

struct Plugin {
  clap_plugin_t plugin{};
  const clap_host_t* host = nullptr;
  Shared shared;
  Engine engine;
  Editor editor{shared};
  double sampleRate = 48000.0;
  Meters meters;
  // The host caches parameter flags, so a model change is announced with a
  // rescan.  rescan() is main-thread only, so audio-thread changes queue a
  // main-thread callback.
  std::atomic<uint32_t> reportedLatency{0xffffffffu};
  std::atomic<bool> rescanQueued{false};
  std::atomic<bool> latencyQueued{false};
  std::atomic<bool> active{false};
  int reportedTypes[kSlotCount] = {-1, -1, -1, -1, -1, -1};
  float reportedGr[kSlotCount] = {-1, -1, -1, -1, -1, -1};
};

const char* const kFeatures[] = {CLAP_PLUGIN_FEATURE_AUDIO_EFFECT,
                                 CLAP_PLUGIN_FEATURE_GATE,
                                 CLAP_PLUGIN_FEATURE_STEREO, nullptr};

const clap_plugin_descriptor_t kDescriptor = {
    CLAP_VERSION,
    kPluginId,
    kPluginName,
    kVendor,
    "https://github.com/tylorismusic-commits/suite-rack",
    "",
    "",
    kVersion,
    "Virtual mix rack with six serial slots and EQ/Comp modules.",
    kFeatures,
};

void writeName(char* dst, size_t size, const char* src) {
  std::strncpy(dst, src, size - 1);
  dst[size - 1] = '\0';
}

Plugin* self(const clap_plugin_t* p) { return static_cast<Plugin*>(p->plugin_data); }

// --- Parameters -------------------------------------------------------------

constexpr uint32_t kClapGrBase = P_COUNT;
constexpr uint32_t kClapParamCount = P_COUNT + kSlotCount;

uint32_t paramsCount(const clap_plugin_t*) { return kClapParamCount; }

bool paramsInfo(const clap_plugin_t* p, uint32_t index,
                clap_param_info_t* info) {
  if (index >= kClapParamCount) {
    return false;
  }
  std::memset(info, 0, sizeof(*info));
  info->id = index;

  if (index >= kClapGrBase) {
    const int slot = static_cast<int>(index - kClapGrBase) + 1;
    char name[32];
    std::snprintf(name, sizeof(name), "Slot %d GR", slot);
    writeName(info->name, sizeof(info->name), name);
    writeName(info->module, sizeof(info->module), "Meter");
    info->flags = CLAP_PARAM_IS_READONLY;
    info->min_value = 0.0;
    info->max_value = 60.0;
    info->default_value = 0.0;
    return true;
  }

  const ParamSpec& s = kParamSpecs[index];
  info->flags = CLAP_PARAM_IS_AUTOMATABLE;
  if (s.stepped) info->flags |= CLAP_PARAM_IS_STEPPED;
  if (s.hidden) info->flags |= CLAP_PARAM_IS_HIDDEN;
  // Power is a normal parameter.  It must NOT be flagged CLAP_PARAM_IS_BYPASS:
  // hosts then treat it with inverted semantics (0 = not bypassed) and write 0
  // at load, which would switch the effect off.  Host FX bypass is
  // separate and handled by the host itself.
  writeName(info->name, sizeof(info->name), s.name);
  writeName(info->module, sizeof(info->module), s.module);
  info->min_value = s.min;
  info->max_value = s.max;
  info->default_value = s.def;

  if (index >= P_SLOT0_TYPE) {
    const int slot =
        static_cast<int>((index - P_SLOT0_TYPE) / kSlotStride);
    const int offset =
        static_cast<int>((index - P_SLOT0_TYPE) % kSlotStride);
    char slotName[32];
    std::snprintf(slotName, sizeof(slotName), "Slot %d", slot + 1);
    if (offset == kSlotOffsetType) {
      writeName(info->name, sizeof(info->name), "Module Type");
      writeName(info->module, sizeof(info->module), slotName);
      info->flags |= CLAP_PARAM_IS_STEPPED;
    } else if (offset >= kSlotOffsetC0) {
      const int control = offset - kSlotOffsetC0;
      const int type = static_cast<int>(
          self(p)->shared.get(slotParam(slot, kSlotOffsetType)) + 0.5);
      const ModuleDesc* module = moduleDesc(type);
      const ControlDesc* desc = controlFor(*module, control);
      char moduleName[96];
      if (desc) {
        std::snprintf(moduleName, sizeof(moduleName), "Slot %d · %s",
                      slot + 1, module->name);
        writeName(info->name, sizeof(info->name), desc->name);
        info->flags &= ~CLAP_PARAM_IS_HIDDEN;
        if (desc->stepped || desc->toggled) {
          info->flags |= CLAP_PARAM_IS_STEPPED;
        }
        info->default_value = defaultControlValue(module->id, control);
      } else {
        char unused[40];
        std::snprintf(unused, sizeof(unused), "Unused Control %d", control + 1);
        std::snprintf(moduleName, sizeof(moduleName), "Slot %d · %s",
                      slot + 1, module->name);
        writeName(info->name, sizeof(info->name), unused);
        info->flags |= CLAP_PARAM_IS_HIDDEN;
      }
      writeName(info->module, sizeof(info->module), moduleName);
    } else {
      writeName(info->module, sizeof(info->module), slotName);
    }
  }
  return true;
}

bool paramsValue(const clap_plugin_t* p, clap_id id, double* value) {
  if (id < P_COUNT) {
    *value = self(p)->shared.get(id);
    return true;
  }
  if (id >= kClapGrBase && id < kClapParamCount) {
    *value = self(p)->shared.slotGrDb[id - kClapGrBase].load(
        std::memory_order_relaxed);
    return true;
  }
  return false;
}

namespace {

} // namespace

// Hosts show these strings in their generic parameter lists.  Without them
// every control reads as a bare number ("0.50") and you cannot tell what you
// are selecting.
bool paramsValueToText(const clap_plugin_t* p, clap_id id, double value,
                       char* out, uint32_t size) {
  if (id >= kClapGrBase && id < kClapParamCount) {
    std::snprintf(out, size, "%.1f dB", value);
    return true;
  }
  if (id >= P_COUNT) return false;
  const std::string text = formatParamValue(self(p)->shared, id, value);
  std::snprintf(out, size, "%s", text.c_str());
  return true;
}

bool paramsTextToValue(const clap_plugin_t* p, clap_id id, const char* text,
                       double* value) {
  if (!text || id >= P_COUNT) {
    return false;
  }
  return parseParamValue(self(p)->shared, id, text, value);
}

void applyEvents(Plugin* s, const clap_input_events_t* in) {
  const uint32_t count = in->size(in);
  for (uint32_t i = 0; i < count; ++i) {
    const clap_event_header_t* hdr = in->get(in, i);
    if (hdr->space_id != CLAP_CORE_EVENT_SPACE_ID) {
      continue;
    }
    if (hdr->type == CLAP_EVENT_PARAM_VALUE) {
      const auto* pv = reinterpret_cast<const clap_event_param_value_t*>(hdr);
      // A pending UI change wins over the host's cached value: hosts that
      // resend their whole parameter set each block would otherwise undo a
      // preset (or a knob move) before we could send it back.
      if (pv->param_id < P_COUNT && !s->shared.isDirty(pv->param_id)) {
        s->shared.setFromHost(pv->param_id, pv->value);
      }
    }
  }
}

// Sends the values the editor changed back to the host, so they can be
// automated and recorded.
void emitDirty(Plugin* s, const clap_output_events_t* out) {
  if (!out) {
    return;
  }
  for (uint32_t p = 0; p < P_COUNT; ++p) {
    if (!s->shared.takeDirty(p)) {
      continue;
    }
    clap_event_param_value_t ev{};
    ev.header.size = sizeof(ev);
    ev.header.time = 0;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.header.type = CLAP_EVENT_PARAM_VALUE;
    ev.header.flags = 0;
    ev.param_id = p;
    ev.note_id = -1;
    ev.port_index = -1;
    ev.channel = -1;
    ev.key = -1;
    ev.value = s->shared.get(p);
    out->try_push(out, &ev.header);
  }
}

void emitGr(Plugin* s, const clap_output_events_t* out) {
  if (!out) return;
  for (int i = 0; i < kSlotCount; ++i) {
    const float gr = s->meters.slotGrDb[i];
    if (std::fabs(gr - s->reportedGr[i]) < 0.05f) continue;
    s->reportedGr[i] = gr;
    clap_event_param_value_t ev{};
    ev.header.size = sizeof(ev);
    ev.header.time = 0;
    ev.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
    ev.header.type = CLAP_EVENT_PARAM_VALUE;
    ev.param_id = kClapGrBase + static_cast<uint32_t>(i);
    ev.note_id = -1;
    ev.port_index = -1;
    ev.channel = -1;
    ev.key = -1;
    ev.value = gr;
    out->try_push(out, &ev.header);
  }
}

void paramsFlush(const clap_plugin_t* p, const clap_input_events_t* in,
                 const clap_output_events_t* out) {
  Plugin* s = self(p);
  if (in) {
    applyEvents(s, in);
  }
  emitDirty(s, out);
}

const clap_plugin_params_t kParamsExt = {
    paramsCount, paramsInfo, paramsValue, paramsValueToText, paramsTextToValue,
    paramsFlush,
};

// --- State ------------------------------------------------------------------

bool stateSave(const clap_plugin_t* p, const clap_ostream_t* stream) {
  Plugin* s = self(p);
  const std::vector<uint8_t> blob =
      serializeState(s->shared, s->editor.scale());
  const char* data = reinterpret_cast<const char*>(blob.data());
  size_t size = blob.size();
  while (size > 0) {
    const int64_t n = stream->write(stream, data, size);
    if (n <= 0) return false;
    data += n;
    size -= static_cast<size_t>(n);
  }
  return true;
}

bool stateLoad(const clap_plugin_t* p, const clap_istream_t* stream) {
  Plugin* s = self(p);
  std::vector<uint8_t> blob;
  char buffer[4096];
  while (true) {
    const int64_t n = stream->read(stream, buffer, sizeof(buffer));
    if (n < 0) return false;
    if (n == 0) break;
    blob.insert(blob.end(), buffer, buffer + n);
    if (blob.size() > 1024 * 1024) return false;
  }
  double scale = 0.0;
  if (!deserializeState(blob.data(), blob.size(), s->shared, &scale)) {
    return false;
  }
  if (scale > 0.0) {
    s->editor.resize(scale);
  }
  return true;
}

const clap_plugin_state_t kStateExt = {stateSave, stateLoad};

// --- Note ports -------------------------------------------------------------

uint32_t notePortsCount(const clap_plugin_t*, bool isInput) {
  return isInput ? 1 : 0;
}

bool notePortsInfo(const clap_plugin_t*, uint32_t index, bool isInput,
                   clap_note_port_info_t* info) {
  if (!isInput || index != 0) return false;
  std::memset(info, 0, sizeof(*info));
  info->id = 0;
  info->supported_dialects = CLAP_NOTE_DIALECT_CLAP | CLAP_NOTE_DIALECT_MIDI;
  info->preferred_dialect = CLAP_NOTE_DIALECT_CLAP;
  writeName(info->name, sizeof(info->name), "MIDI In");
  return true;
}

const clap_plugin_note_ports_t kNotePortsExt = {notePortsCount, notePortsInfo};

// --- Audio ports ------------------------------------------------------------

uint32_t audioPortsCount(const clap_plugin_t*, bool isInput) {
  return isInput ? 1 : 1;
}

bool audioPortsInfo(const clap_plugin_t*, uint32_t index, bool isInput,
                    clap_audio_port_info_t* info) {
  std::memset(info, 0, sizeof(*info));
  info->channel_count = 2;
  info->port_type = CLAP_PORT_STEREO;
  info->in_place_pair = CLAP_INVALID_ID;

  if (isInput) {
    if (index != 0) {
      return false;
    }
    info->id = 0;
    writeName(info->name, sizeof(info->name), "Input");
    info->flags = CLAP_AUDIO_PORT_IS_MAIN;
    return true;
  }
  if (index != 0) {
    return false;
  }
  info->id = 1;
  writeName(info->name, sizeof(info->name), "Output");
  info->flags = CLAP_AUDIO_PORT_IS_MAIN;
  return true;
}

const clap_plugin_audio_ports_t kAudioPortsExt = {audioPortsCount, audioPortsInfo};

// --- GUI --------------------------------------------------------------------

bool guiIsApiSupported(const clap_plugin_t*, const char* api, bool isFloating) {
  return !isFloating && api && std::strcmp(api, CLAP_WINDOW_API_X11) == 0;
}

bool guiGetPreferredApi(const clap_plugin_t*, const char** api, bool* isFloating) {
  *api = CLAP_WINDOW_API_X11;
  *isFloating = false;
  return true;
}

bool guiCreate(const clap_plugin_t*, const char*, bool) { return true; }

void guiDestroy(const clap_plugin_t* p) { self(p)->editor.close(); }

// X11 embedding uses physical pixels, so the host does not set a scale.
bool guiSetScale(const clap_plugin_t*, double) { return false; }

bool guiGetSize(const clap_plugin_t* p, uint32_t* width, uint32_t* height) {
  const UiSize s = self(p)->editor.size();
  *width = s.width;
  *height = s.height;
  return true;
}

bool guiCanResize(const clap_plugin_t*) { return true; }

bool guiGetResizeHints(const clap_plugin_t*, clap_gui_resize_hints_t* hints) {
  hints->can_resize_horizontally = true;
  hints->can_resize_vertically = true;
  hints->preserve_aspect_ratio = true;
  hints->aspect_ratio_width = kPanelWidth;
  hints->aspect_ratio_height = kPanelHeight;
  return true;
}

// Returns the nearest valid size; the editor always keeps 1040:720.
bool guiAdjustSize(const clap_plugin_t*, uint32_t* width, uint32_t* height) {
  const UiSize s = snapSize(*width, *height);
  *width = s.width;
  *height = s.height;
  return true;
}

bool guiSetSize(const clap_plugin_t* p, uint32_t width, uint32_t height) {
  self(p)->editor.resize(scaleForRequest(width, height));
  return true;
}

bool guiSetParent(const clap_plugin_t* p, const clap_window_t* window) {
  return self(p)->editor.open(window->x11 ? reinterpret_cast<void*>(
                                                static_cast<uintptr_t>(window->x11))
                                          : nullptr);
}

bool guiSetTransient(const clap_plugin_t*, const clap_window_t*) { return false; }

void guiSuggestTitle(const clap_plugin_t*, const char*) {}

bool guiShow(const clap_plugin_t*) { return true; }

bool guiHide(const clap_plugin_t*) { return true; }

const clap_plugin_gui_t kGuiExt = {
    guiIsApiSupported, guiGetPreferredApi, guiCreate, guiDestroy, guiSetScale,
    guiGetSize, guiCanResize, guiGetResizeHints, guiAdjustSize, guiSetSize,
    guiSetParent, guiSetTransient, guiSuggestTitle, guiShow, guiHide,
};

uint32_t latencyGet(const clap_plugin_t* p) {
  return static_cast<uint32_t>(self(p)->engine.latencySamples());
}

const clap_plugin_latency_t kLatencyExt = {latencyGet};

// --- Plugin lifecycle -------------------------------------------------------

bool pluginInit(const clap_plugin_t*) { return true; }

void pluginDestroy(const clap_plugin_t* p) {
  delete self(p);
}

bool pluginActivate(const clap_plugin_t* p, double sampleRate, uint32_t,
                    uint32_t) {
  Plugin* s = self(p);
  s->sampleRate = sampleRate;
  s->engine.prepare(static_cast<float>(sampleRate));
  s->engine.setControls(controlsFrom(s->shared));
  for (int i = 0; i < kSlotCount; ++i) {
    s->reportedTypes[i] = static_cast<int>(
        s->shared.get(slotParam(i, kSlotOffsetType)) + 0.5);
  }
  s->active.store(true, std::memory_order_relaxed);
  // REAPER only picks up a new latency when the plugin calls changed(), so do
  // that here (being-activated is the one legal moment) - but only when the
  // value actually changed, so a host that reacts by restarting cannot loop.
  const uint32_t latencyNow = s->engine.latencySamples();
  s->latencyQueued.store(false, std::memory_order_relaxed);
  if (latencyNow != s->reportedLatency.exchange(latencyNow)) {
    if (s->host && s->host->get_extension) {
      const auto* latency = static_cast<const clap_host_latency_t*>(
          s->host->get_extension(s->host, CLAP_EXT_LATENCY));
      if (latency && latency->changed) {
        latency->changed(s->host);
      }
    }
  }
  return true;
}

void pluginDeactivate(const clap_plugin_t* p) {
  self(p)->active.store(false, std::memory_order_relaxed);
}

bool pluginStartProcessing(const clap_plugin_t*) { return true; }

void pluginStopProcessing(const clap_plugin_t*) {}

void pluginReset(const clap_plugin_t* p) {
  self(p)->engine.reset();
}

clap_process_status pluginProcess(const clap_plugin_t* p, const clap_process_t* proc) {
  Plugin* s = self(p);
  const uint32_t frames = proc->frames_count;
  if (proc->in_events) {
    applyEvents(s, proc->in_events);
  }
  emitDirty(s, proc->out_events);

  if (proc->audio_inputs_count == 0 || proc->audio_outputs_count == 0) {
    return CLAP_PROCESS_CONTINUE;
  }
  // Be tolerant of mono or partially connected buffers.
  const clap_audio_buffer_t& inBuf = proc->audio_inputs[0];
  const clap_audio_buffer_t& outBuf = proc->audio_outputs[0];
  if (!outBuf.data32 || !inBuf.data32 || !outBuf.data32[0] || !inBuf.data32[0]) {
    return CLAP_PROCESS_CONTINUE;
  }
  float* left = outBuf.data32[0];
  float* right = outBuf.channel_count > 1 && outBuf.data32[1] ? outBuf.data32[1] : left;
  const float* inL = inBuf.data32[0];
  const float* inR = inBuf.channel_count > 1 && inBuf.data32[1] ? inBuf.data32[1] : inL;
  if (left != inL) {
    std::memcpy(left, inL, frames * sizeof(float));
  }
  if (right != inR) {
    std::memcpy(right, inR, frames * sizeof(float));
  }

  s->engine.setControls(controlsFrom(s->shared));

  const uint32_t latencyNow = static_cast<uint32_t>(s->engine.latencySamples());
  if (latencyNow != s->reportedLatency.load(std::memory_order_relaxed) &&
      !s->latencyQueued.exchange(true) && s->host &&
      s->host->request_callback) {
    s->host->request_callback(s->host);
  }

  bool typesChanged = false;
  for (int i = 0; i < kSlotCount; ++i) {
    const int type = static_cast<int>(
        s->shared.get(slotParam(i, kSlotOffsetType)) + 0.5);
    if (type != s->reportedTypes[i]) {
      s->reportedTypes[i] = type;
      typesChanged = true;
    }
  }
  if (typesChanged && s->host && s->host->request_callback &&
      !s->rescanQueued.exchange(true)) {
    s->host->request_callback(s->host);
  }

  s->engine.process(left, right, static_cast<int>(frames), s->meters);
  emitGr(s, proc->out_events);

  s->shared.inPeakDb.store(s->meters.inPeakDb, std::memory_order_relaxed);
  s->shared.outPeakDb.store(s->meters.outPeakDb, std::memory_order_relaxed);
  s->shared.inPeakLDb.store(s->meters.inPeakLDb, std::memory_order_relaxed);
  s->shared.inPeakRDb.store(s->meters.inPeakRDb, std::memory_order_relaxed);
  s->shared.outPeakLDb.store(s->meters.outPeakLDb, std::memory_order_relaxed);
  s->shared.outPeakRDb.store(s->meters.outPeakRDb, std::memory_order_relaxed);
  for (int i = 0; i < kSlotCount; ++i)
    s->shared.slotGrDb[i].store(s->meters.slotGrDb[i],
                                std::memory_order_relaxed);
  return CLAP_PROCESS_CONTINUE;
}

double gainAdjustmentGet(const clap_plugin_t* p) {
  const Plugin* s = self(p);
  double gr = 0.0;
  for (int i = 0; i < kSlotCount; ++i) {
    gr = std::max(gr, static_cast<double>(s->meters.slotGrDb[i]));
  }
  return -gr;
}

const clap_plugin_gain_adjustment_metering_t kGainAdjustmentMeteringExt = {
    gainAdjustmentGet,
};

const void* pluginGetExtension(const clap_plugin_t*, const char* id) {
  if (std::strcmp(id, CLAP_EXT_PARAMS) == 0) {
    return &kParamsExt;
  }
  if (std::strcmp(id, CLAP_EXT_STATE) == 0) {
    return &kStateExt;
  }
  if (std::strcmp(id, CLAP_EXT_NOTE_PORTS) == 0) {
    return &kNotePortsExt;
  }
  if (std::strcmp(id, CLAP_EXT_AUDIO_PORTS) == 0) {
    return &kAudioPortsExt;
  }
  if (std::strcmp(id, CLAP_EXT_GUI) == 0) {
    return &kGuiExt;
  }
  if (std::strcmp(id, CLAP_EXT_LATENCY) == 0) {
    return &kLatencyExt;
  }
  if (std::strcmp(id, CLAP_EXT_GAIN_ADJUSTMENT_METERING) == 0) {
    return &kGainAdjustmentMeteringExt;
  }
  return nullptr;
}

void pluginOnMainThread(const clap_plugin_t* p) {
  Plugin* s = self(p);
  if (s->latencyQueued.exchange(false)) {
    if (s->host && s->host->get_extension) {
      const auto* latency = static_cast<const clap_host_latency_t*>(
          s->host->get_extension(s->host, CLAP_EXT_LATENCY));
      if (latency && latency->changed) latency->changed(s->host);
    }
    s->reportedLatency.store(
        static_cast<uint32_t>(s->engine.latencySamples()),
        std::memory_order_relaxed);
  }
  if (!s->rescanQueued.exchange(false)) return;
  if (!s->host || !s->host->get_extension) return;
  const auto* params = static_cast<const clap_host_params_t*>(
      s->host->get_extension(s->host, CLAP_EXT_PARAMS));
  if (params && params->rescan) {
    params->rescan(s->host, CLAP_PARAM_RESCAN_ALL);
  }
}

Plugin* createPlugin(const clap_host_t* host) {
  Plugin* s = new Plugin();
  s->host = host;
  s->plugin.desc = &kDescriptor;
  s->plugin.plugin_data = s;
  s->plugin.init = pluginInit;
  s->plugin.destroy = pluginDestroy;
  s->plugin.activate = pluginActivate;
  s->plugin.deactivate = pluginDeactivate;
  s->plugin.start_processing = pluginStartProcessing;
  s->plugin.stop_processing = pluginStopProcessing;
  s->plugin.reset = pluginReset;
  s->plugin.process = pluginProcess;
  s->plugin.get_extension = pluginGetExtension;
  s->plugin.on_main_thread = pluginOnMainThread;
  s->engine.prepare(48000.0f);
  initializeRack(s->shared);
  // Latency may only change while the plugin is being activated, so the
  // oversampling switch is applied on the host's restart, not mid-block.
  s->editor.requestFlush = [s] {
    if (s->host && s->host->request_process) {
      s->host->request_process(s->host);
    }
  };
  return s;
}

// --- Factory and entry ------------------------------------------------------

uint32_t factoryCount(const clap_plugin_factory_t*) { return 1; }

const clap_plugin_descriptor_t* factoryDescriptor(const clap_plugin_factory_t*, uint32_t index) {
  return index == 0 ? &kDescriptor : nullptr;
}

const clap_plugin_t* factoryCreate(const clap_plugin_factory_t*, const clap_host_t* host,
                                   const char* id) {
  if (std::strcmp(id, kPluginId) != 0) {
    return nullptr;
  }
  return &createPlugin(host)->plugin;
}

const clap_plugin_factory_t kFactory = {factoryCount, factoryDescriptor, factoryCreate};

bool entryInit(const char*) {
  // Do not call XInitThreads() here: the host has already used Xlib by the
  // time plugins are loaded, and initialising threading afterwards is
  // undefined behaviour (it stopped the editor loading in REAPER).  The
  // editor instead confines every Xlib call to its own thread.
  return true;
}

void entryDeinit() {}

const void* entryGetFactory(const char* factoryId) {
  return std::strcmp(factoryId, CLAP_PLUGIN_FACTORY_ID) == 0 ? &kFactory : nullptr;
}

} // namespace
} // namespace rack

extern "C" CLAP_EXPORT const clap_plugin_entry_t clap_entry = {
    CLAP_VERSION,
    rack::entryInit,
    rack::entryDeinit,
    rack::entryGetFactory,
};
