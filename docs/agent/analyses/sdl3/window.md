# Window (SDL3)

Subject markdown for the `sak::sdl3::window` object.

- Source object: `include/sak/sdl3/window.hpp`.
- Source of truth: SDL3 headers `SDL_video.h` and `SDL_events.h` (all objects `\since SDL 3.2.0` unless noted).
- Scope: canonical `Window` objects only; `_renamed_` / `_deprecated_use_` aliases, all `SDL_GL_*` / `SDL_EGL_*` context functions, all `SDL_Render*` / render-related symbols, display lookups, and OS-native / EGL / Wayland external handles are excluded.
- Every function here is restricted to the main thread unless noted.
- All `bool` setters return `true` on success / `false` on failure; call `SDL_GetError()` for more information.

## Handle types

- `SDL_Window` — `typedef struct SDL_Window` — opaque handle to a window.
- `SDL_WindowID` — `typedef Uint32`.
  - Unique ID for a window; `0` is an invalid ID.
  - `SDL_WindowEvent` references windows by this ID.

## Window flags / position sentinels

- `SDL_WindowFlags` — `typedef Uint64`. Mix of read/write window state; some immutable after `SDL_CreateWindow()`, some app-changeable, some altered by user/system.
  - Creating with `SDL_WINDOW_RESIZABLE` constrains the window to compositor-recommended dimensions within usable desktop space.
  - `SDL_WINDOW_*` — grouped by the `window::flag` categories below:
    - window state that can be both requested and reported:
      - `FULLSCREEN`
      - `HIDDEN` — not mapped/shown; needs `SDL_ShowWindow()` to become visible.
      - `MINIMIZED`
      - `MAXIMIZED`
      - `OCCLUDED`
      - `BORDERLESS`
      - `RESIZABLE`
      - `ALWAYS_ON_TOP`
      - `TRANSPARENT`
      - `FILL_DOCUMENT` — Emscripten only `\since SDL 3.4.0`.
    - focus and modality:
      - `INPUT_FOCUS`
      - `MOUSE_FOCUS`
      - `NOT_FOCUSABLE`
      - `MODAL`
      - `EXTERNAL` — not created by SDL.
    - pointer and keyboard capture:
      - `MOUSE_GRABBED`
      - `KEYBOARD_GRABBED`
      - `MOUSE_CAPTURE` — unrelated to `MOUSE_GRABBED`.
      - `MOUSE_RELATIVE_MODE`
    - system window roles:
      - `UTILITY` — not shown in the taskbar/window list.
      - `TOOLTIP` — no mouse/keyboard focus; requires a parent window.
      - `POPUP_MENU` — requires a parent window.
    - graphics api selected at creation:
      - `OPENGL`
      - `VULKAN`
      - `METAL`
    - pixel density:
      - `HIGH_PIXEL_DENSITY`
- Position sentinels (`SDL_SetWindowPosition` x/y):
  - `SDL_WINDOWPOS_UNDEFINED_MASK` (`0x1FFF0000u`) — magic value used through the helpers below.
  - `SDL_WINDOWPOS_UNDEFINED_DISPLAY(X)` — don't care about position; `X` is the `SDL_DisplayID`.
  - `SDL_WINDOWPOS_UNDEFINED` — don't care about position/display; always uses the primary display.
  - `SDL_WINDOWPOS_ISUNDEFINED(X)` — tests if a position is marked "undefined" (`(X & 0xFFFF0000) == SDL_WINDOWPOS_UNDEFINED_MASK`).
  - `SDL_WINDOWPOS_CENTERED_MASK` (`0x2FFF0000u`) — magic value used through the helpers below.
  - `SDL_WINDOWPOS_CENTERED_DISPLAY(X)` — center the window; `X` is the `SDL_DisplayID`.
  - `SDL_WINDOWPOS_CENTERED` — center on the primary display.
  - `SDL_WINDOWPOS_ISCENTERED(X)` — tests if a position is marked "centered" (`(X & 0xFFFF0000) == SDL_WINDOWPOS_CENTERED_MASK`).

## Supporting enums / callback

- `SDL_FlashOperation` enum — flash mode for `SDL_FlashWindow`:
  - `SDL_FLASH_CANCEL` — cancel any flash state.
  - `SDL_FLASH_BRIEFLY` — flash briefly to get attention.
  - `SDL_FLASH_UNTIL_FOCUSED` — flash until the window gains focus.
- `SDL_ProgressState` enum — progress-bar state for the taskbar icon `\since SDL 3.4.0`:
  - `SDL_PROGRESS_STATE_INVALID` (`-1`) — error state; check `SDL_GetError()`.
  - `SDL_PROGRESS_STATE_NONE` — no progress bar shown.
  - `SDL_PROGRESS_STATE_INDETERMINATE` — indeterminate progress.
  - `SDL_PROGRESS_STATE_NORMAL` — normal progress.
  - `SDL_PROGRESS_STATE_PAUSED` — paused progress.
  - `SDL_PROGRESS_STATE_ERROR` — error progress.
- `SDL_HitTestResult` enum — return values from an `SDL_HitTest` callback:
  - `SDL_HITTEST_NORMAL`, `SDL_HITTEST_DRAGGABLE`, `SDL_HITTEST_RESIZE_TOPLEFT`, `SDL_HITTEST_RESIZE_TOP`, `SDL_HITTEST_RESIZE_TOPRIGHT`, `SDL_HITTEST_RESIZE_RIGHT`, `SDL_HITTEST_RESIZE_BOTTOMRIGHT`, `SDL_HITTEST_RESIZE_BOTTOM`, `SDL_HITTEST_RESIZE_BOTTOMLEFT`, `SDL_HITTEST_RESIZE_LEFT` — each region of the window treated as draggable or as a resizing border.
- `SDL_HitTest(SDL_Window *win, const SDL_Point *area, void *data) -> SDL_HitTestResult` — `typedef SDL_HitTestResult (SDLCALL *)`.
  - `win`: window hit-testing was set on; `area`: point to hit-test; `data`: the `callback_data` passed to `SDL_SetWindowHitTest()`.

## Creation & destruction

- `SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags) -> SDL_Window *` — create a window; `NULL` on failure.
  - Size is a request; actual size may differ per window manager policy; be prepared for any size.
  - Window is shown unless `SDL_WINDOW_HIDDEN`; may then be shown via `SDL_ShowWindow()`.
  - `flags` `0` or any `SDL_WindowFlags` OR'd together.
  - `SDL_WINDOW_OPENGL`/`VULKAN` trigger the corresponding LoadLibrary; UnloadLibrary is called by `SDL_DestroyWindow()`. Creating with `SDL_WINDOW_VULKAN`/`SDL_WINDOW_METAL` fails if no working driver/platform support.
  - Created object → RAII wrapper opportunity (owned by caller).
- `SDL_CreatePopupWindow(SDL_Window *parent, int offset_x, int offset_y, int w, int h, SDL_WindowFlags flags) -> SDL_Window *` — create a child popup window; `NULL` on failure.
  - `flags` **must** contain `SDL_WINDOW_TOOLTIP` or `SDL_WINDOW_POPUP_MENU`.
  - `SDL_WINDOW_MINIMIZED`/`MAXIMIZED`/`FULLSCREEN`/`BORDERLESS` ignored; `SDL_WINDOW_UTILITY`/`MODAL` cause failure.
  - `parent` must be non-null; a popup cannot be minimized/maximized/fullscreen/raised/flashed/made modal/grab input. Implicitly undecorated, off taskbar/alt-tab.
  - Destroying/hiding the parent recursively destroys/hides popup children.
  - Created object → RAII wrapper opportunity (owned by caller).
- `SDL_CreateWindowWithProperties(SDL_PropertiesID props) -> SDL_Window *` — create a window from properties; `NULL` on failure.
  - Backend-independent properties: `SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN`, `_BORDERLESS_BOOLEAN`, `_CONSTRAIN_POPUP_BOOLEAN`, `_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN`, `_FOCUSABLE_BOOLEAN`, `_FULLSCREEN_BOOLEAN`, `_HEIGHT_NUMBER`, `_HIDDEN_BOOLEAN`, `_HIGH_PIXEL_DENSITY_BOOLEAN`, `_MAXIMIZED_BOOLEAN`, `_MENU_BOOLEAN`, `_METAL_BOOLEAN`, `_MINIMIZED_BOOLEAN`, `_MODAL_BOOLEAN`, `_MOUSE_GRABBED_BOOLEAN`, `_OPENGL_BOOLEAN`, `_PARENT_POINTER`, `_RESIZABLE_BOOLEAN`, `_TITLE_STRING`, `_TRANSPARENT_BOOLEAN`, `_TOOLTIP_BOOLEAN`, `_UTILITY_BOOLEAN`, `_VULKAN_BOOLEAN`, `_WIDTH_NUMBER`, `_X_NUMBER` / `_Y_NUMBER` (`SDL_WINDOWPOS_CENTERED`/`UNDEFINED`, default `UNDEFINED`).
  - Wayland additions: `SDL_PROP_WINDOW_CREATE_WAYLAND_SURFACE_ROLE_CUSTOM_BOOLEAN`, `_WAYLAND_CREATE_EGL_WINDOW_BOOLEAN`, `_WAYLAND_WL_SURFACE_POINTER`.
  - Emscripten additions: `SDL_PROP_WINDOW_CREATE_EMSCRIPTEN_CANVAS_ID_STRING`, `_EMSCRIPTEN_KEYBOARD_ELEMENT_STRING`.
  - Implicitly shown if the `hidden` property is not set; tooltip/menu properties create popup windows.
  - Created object → RAII wrapper opportunity (owned by caller).
- `SDL_DestroyWindow(SDL_Window *window) -> void` — destroy a window.
  - Child (popup) windows are recursively destroyed.
  - On some platforms the visible window may only be removed when the event loop is pumped again, even though the handle is invalid right after this call.

## Enumeration & identity lookup

- `SDL_GetWindows(int *count) -> SDL_Window **` — NULL-terminated list of valid windows; `NULL` on failure.
  - `count`: filled with the number of windows; may be `NULL`.
  - Single allocation; free with `SDL_free()`.
  - Returned heap array → RAII wrapper opportunity.
- `SDL_GetWindowID(SDL_Window *window) -> SDL_WindowID` — numeric ID; `0` on failure. The ID is what `SDL_WindowEvent` references to map events to window objects.
- `SDL_GetWindowFromID(SDL_WindowID id) -> SDL_Window *` — window for a stored ID; `NULL` if it doesn't exist.
- `SDL_GetWindowFromEvent(const SDL_Event *event) -> SDL_Window *` — window associated with an event (one containing a `windowID`); `NULL` if there is none.
  - Safe to call from any thread.
- `SDL_GetWindowParent(SDL_Window *window) -> SDL_Window *` — parent of a window; `NULL` if it has none.

## State, visibility & fullscreen

- `SDL_ShowWindow(SDL_Window *window) -> bool` — show a window.
- `SDL_HideWindow(SDL_Window *window) -> bool` — hide a window (clears `SDL_WINDOW_HIDDEN` semantics).
- `SDL_RaiseWindow(SDL_Window *window) -> bool` — request to raise above others and gain input focus.
  - Subject to window-manager policy; may fail to steal focus. On success emits `SDL_EVENT_WINDOW_FOCUS_GAINED` and sets `SDL_WINDOW_INPUT_FOCUS`.
- `SDL_MaximizeWindow(SDL_Window *window) -> bool` — request to be made as large as possible.
  - Non-resizable windows (no `SDL_WINDOW_RESIZABLE`) can't be maximized (no effect). Asynchronous on some systems; emits `SDL_EVENT_WINDOW_MAXIMIZED`.
- `SDL_MinimizeWindow(SDL_Window *window) -> bool` — request to minimize to iconic.
  - No direct effect on a fullscreen window; may alter the state returned to on leaving fullscreen. Emits `SDL_EVENT_WINDOW_MINIMIZED`.
- `SDL_RestoreWindow(SDL_Window *window) -> bool` — request to restore a minimized/maximized window to normal size and position.
  - Emits `SDL_EVENT_WINDOW_RESTORED`.
- `SDL_SyncWindow(SDL_Window *window) -> bool` — block until any pending window state is finalized.
  - Acts as a sync barrier on asynchronous systems; guaranteed finite time but may block for window-state animations. Does nothing where changes are immediate. Returns `false` if it timed out before the requested state.
- `SDL_SetWindowFullscreen(SDL_Window *window, bool fullscreen) -> bool` — request fullscreen (`true`) or windowed (`false`) state.
  - Default fullscreen uses borderless desktop mode unless an exclusive mode is set via `SDL_SetWindowFullscreenMode()`. Emits `SDL_EVENT_WINDOW_ENTER_FULLSCREEN`/`LEAVE_FULLSCREEN`; can be denied.
- `SDL_SetWindowFullscreenMode(SDL_Window *window, const SDL_DisplayMode *mode) -> bool` — set the display mode used while visible and fullscreen.
  - `mode` `NULL` for borderless desktop mode, or an exclusive mode from `SDL_GetFullscreenDisplayModes()`. Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`/`PIXEL_SIZE_CHANGED` on change.
- `SDL_GetWindowFullscreenMode(SDL_Window *window) -> const SDL_DisplayMode *` — current exclusive fullscreen mode; `NULL` for borderless desktop mode.
- `SDL_FlashWindow(SDL_Window *window, SDL_FlashOperation operation) -> bool` — request the window demand attention; operation is a `SDL_FlashOperation`.
- `SDL_SetWindowFocusable(SDL_Window *window, bool focusable) -> bool` — set whether the window may have input focus.
- `SDL_SetWindowParent(SDL_Window *window, SDL_Window *parent) -> bool` — set a window as child of `parent`; `NULL` unparents.
  - Reparents if already a child. Fails if the window is modal (cancel via `SDL_SetWindowModal`) or is a popup. Setting a sibling/descendant parent is undefined behavior.
- `SDL_SetWindowModal(SDL_Window *window, bool modal) -> bool` — toggle modal state; window must currently be a child to enable.

## Geometry, size, position & aspect

- `SDL_SetWindowSize(SDL_Window *window, int w, int h) -> bool` — request client-area size.
  - `w`/`h` must be `> 0`; no effect on fullscreen/maximized windows. Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`.
- `SDL_GetWindowSize(SDL_Window *window, int *w, int *h) -> bool` — client-area size in window coordinates (may differ from pixels on high-DPI); out-params may be `NULL`.
- `SDL_GetWindowSizeInPixels(SDL_Window *window, int *w, int *h) -> bool` — client-area size in pixels (drawable size); out-params may be `NULL`.
- `SDL_SetWindowMinimumSize(SDL_Window *window, int min_w, int min_h) -> bool` — minimum client size; `0` = no limit.
- `SDL_GetWindowMinimumSize(SDL_Window *window, int *w, int *h) -> bool` — minimum client size; out-params may be `NULL`.
- `SDL_SetWindowMaximumSize(SDL_Window *window, int max_w, int max_h) -> bool` — maximum client size; `0` = no limit.
- `SDL_GetWindowMaximumSize(SDL_Window *window, int *w, int *h) -> bool` — maximum client size; out-params may be `NULL`.
- `SDL_GetWindowBordersSize(SDL_Window *window, int *top, int *left, int *bottom, int *right) -> bool` — decoration border sizes around the client area.
  - On failure the values are set to `0,0,0,0` (as if borderless) if non-`NULL`; fails before the display server decorates the window (e.g. right after creation) and if unsupported. NULL out-params permitted.
- `SDL_SetWindowPosition(SDL_Window *window, int x, int y) -> bool` — request window position; `x`/`y` may be `SDL_WINDOWPOS_CENTERED` or `SDL_WINDOWPOS_UNDEFINED`.
  - No effect on exclusive-fullscreen/maximized windows. Asynchronous on some systems; emits `SDL_EVENT_WINDOW_MOVED`. Request can be constrained or denied.
- `SDL_GetWindowPosition(SDL_Window *window, int *x, int *y) -> bool` — current position as last reported by the system; out-params may be `NULL`.
- `SDL_SetWindowAspectRatio(SDL_Window *window, float min_aspect, float max_aspect) -> bool` — request aspect-ratio limits (width/height, e.g. 2560x1600 → 1.6).
  - `min_aspect`/`max_aspect` `0.0f` = no limit; deferred while fixed-size (maximized/fullscreen). Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`.
- `SDL_GetWindowAspectRatio(SDL_Window *window, float *min_aspect, float *max_aspect) -> bool` — current aspect-ratio limits; out-params may be `NULL`.
- `SDL_GetWindowSafeArea(SDL_Window *window, SDL_Rect *rect) -> bool` — client area safe for interactable content.
  - Accounts for on-screen controls, curved edges, camera notches, TV overscan; still render elsewhere but avoid important/interactable content there.

## Decoration & appearance

- `SDL_SetWindowBordered(SDL_Window *window, bool bordered) -> bool` — add/remove `SDL_WINDOW_BORDERLESS`.
  - No-op if border already matches; cannot change a fullscreen window's border.
- `SDL_SetWindowResizable(SDL_Window *window, bool resizable) -> bool` — add/remove `SDL_WINDOW_RESIZABLE`.
  - No-op if state matches; cannot change a fullscreen window's resizable state.
- `SDL_SetWindowAlwaysOnTop(SDL_Window *window, bool on_top) -> bool` — add/remove `SDL_WINDOW_ALWAYS_ON_TOP`; brings window to front and keeps it above.
- `SDL_SetWindowFillDocument(SDL_Window *window, bool fill) -> bool` — add/remove `SDL_WINDOW_FILL_DOCUMENT` (Emscripten only).
  - When enabled the canvas fills the entire document and covers everything else; minimum size and aspect constraints may not work as expected.

## Pixel / DPI info

- `SDL_GetWindowPixelDensity(SDL_Window *window) -> float` — ratio of pixel size to window size (e.g. 1920x1080 window with 3840x2160 back buffer → `2.0`); `0.0f` on failure.
- `SDL_GetWindowDisplayScale(SDL_Window *window) -> float` — content display scale relative to the window's pixel size; `0.0f` on failure.
  - Combination of pixel density and display content scale; the expected scale for content. Updated when the scale setting changes or the window moves display.
- `SDL_GetWindowPixelFormat(SDL_Window *window) -> SDL_PixelFormat` — pixel format of the window; `SDL_PIXELFORMAT_UNKNOWN` on failure.
- `SDL_GetWindowICCProfile(SDL_Window *window, size_t *size) -> void *` — raw ICC profile for the current screen; `NULL` on failure.
  - `size`: filled with the profile size.
  - Free with `SDL_free()`.

## Properties

- `SDL_GetWindowProperties(SDL_Window *window) -> SDL_PropertiesID` — read-only window properties; `0` on failure.
  - SDL-provided: `SDL_PROP_WINDOW_SHAPE_POINTER` (shape surface), `SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN` (HDR headroom above SDR white point), `SDL_PROP_WINDOW_SDR_WHITE_LEVEL_FLOAT`, `SDL_PROP_WINDOW_HDR_HEADROOM_FLOAT` — the HDR ones change dynamically and emit `SDL_EVENT_WINDOW_HDR_STATE_CHANGED`.
  - Windows: `SDL_PROP_WINDOW_WIN32_HWND_POINTER`, `_HDC_POINTER`, `_INSTANCE_POINTER`.
  - X11: `SDL_PROP_WINDOW_X11_DISPLAY_POINTER`, `_SCREEN_NUMBER`, `_WINDOW_NUMBER`.
  - Wayland `xdg_*` objects do not persist across show/hide; query them each time the window is shown.
- `SDL_GetWindowFlags(SDL_Window *window) -> SDL_WindowFlags` — mask of the current flags.

## Title / icon / shape / opacity

- `SDL_SetWindowTitle(SDL_Window *window, const char *title) -> bool` — set the title (UTF-8).
- `SDL_GetWindowTitle(SDL_Window *window) -> const char *` — the title (UTF-8); `""` if there is no title.
- `SDL_SetWindowIcon(SDL_Window *window, SDL_Surface *icon) -> bool` — set the window icon from a surface.
  - Alternate representations added via `SDL_AddSurfaceAlternateImage()` are used for high-DPI (100%-scale content + up/downscaled alternatives).
- `SDL_SetWindowShape(SDL_Window *window, SDL_Surface *shape) -> bool` — set the shape (alpha channel) of a transparent window; `NULL` removes the shape.
  - Fully transparent areas are also mouse-transparent. Shape is copied internally (safe to free after). Requires creation with `SDL_WINDOW_TRANSPARENT`; expensive, call sparingly.
- `SDL_SetWindowOpacity(SDL_Window *window, float opacity) -> bool` — set opacity, clamped to `[0.0f, 1.0f]`.
  - Returns `false` if unsupported on the platform.
- `SDL_GetWindowOpacity(SDL_Window *window) -> float` — current opacity `[0.0f, 1.0f]`; `-1.0f` on failure.
  - Returns `1.0f` without error if transparency isn't supported.

## Grabs & mouse input

- `SDL_SetWindowKeyboardGrab(SDL_Window *window, bool grabbed) -> bool` — set keyboard grab.
  - Keyboard grab captures system shortcuts like Alt+Tab / Meta; primarily for specialized apps (VNC, VM frontends), not normal games. Enabling while another window is grabbed transfers the grab.
- `SDL_GetWindowKeyboardGrab(SDL_Window *window) -> bool` — whether keyboard is grabbed.
- `SDL_SetWindowMouseGrab(SDL_Window *window, bool grabbed) -> bool` — set mouse grab (confines the cursor to the window).
- `SDL_GetWindowMouseGrab(SDL_Window *window) -> bool` — whether mouse is grabbed.
- `SDL_GetGrabbedWindow(void) -> SDL_Window *` — window currently holding an input grab; `NULL` otherwise.
- `SDL_SetWindowMouseRect(SDL_Window *window, const SDL_Rect *rect) -> bool` — confine cursor to `rect` (window-relative) when the window has focus; `NULL` destroys the barrier.
  - Does not grab the cursor, only restricts its area.
- `SDL_GetWindowMouseRect(SDL_Window *window) -> const SDL_Rect *` — current confinement rectangle; `NULL` if none.
- `SDL_ShowWindowSystemMenu(SDL_Window *window, int x, int y) -> bool` — display the system-level window menu at `x`/`y` relative to the client origin.
  - On unsupported platforms/desktops this does nothing.
- `SDL_SetWindowHitTest(SDL_Window *window, SDL_HitTest callback, void *callback_data) -> bool` — set a hit-test callback designating special window regions.
  - Enables custom drag/resize regions (e.g. borderless windows). Mouse input in special regions may be consumed by the OS. `callback` `NULL` disables (default off). Unsupported platforms return `false` unconditionally. Callback can fire at any time; keep it efficient.

## Window surfaces

- `SDL_WindowHasSurface(SDL_Window *window) -> bool` — whether the window has an associated surface.
- `SDL_GetWindowSurface(SDL_Window *window) -> SDL_Surface *` — the window's SDL surface; a new optimal-format one is created if necessary; `NULL` on failure.
  - Freed when the window is destroyed — **do not free** (RAII: non-owning). Invalidated on resize; call again after a resize. Not combinable with 3D/render APIs.
- `SDL_SetWindowSurfaceVSync(SDL_Window *window, int vsync) -> bool` — toggle surface vsync.
  - Defaults to `SDL_WINDOW_SURFACE_VSYNC_DISABLED` (`0`); `vsync` `1`/`2`/... per vertical refresh, `SDL_WINDOW_SURFACE_VSYNC_ADAPTIVE` (`-1`) for adaptive. Not every value is supported; check the return.
- `SDL_GetWindowSurfaceVSync(SDL_Window *window, int *vsync) -> bool` — current vsync interval.
- `SDL_UpdateWindowSurface(SDL_Window *window) -> bool` — copy the window surface to the screen (equivalent to SDL 1.2 `SDL_Flip`).
- `SDL_UpdateWindowSurfaceRects(SDL_Window *window, const SDL_Rect *rects, int numrects) -> bool` — copy areas of the surface to the screen (equivalent to `SDL_UpdateRects`).
  - Updates at least the given rects, but may update more (optimization only).
- `SDL_DestroyWindowSurface(SDL_Window *window) -> bool` — destroy the window's surface.

## Misc

- `SDL_FlashWindow` — see State/visibility (flash).
- `SDL_SetWindowProgressState(SDL_Window *window, SDL_ProgressState state) -> bool` — set taskbar-icon progress state; `SDL_PROGRESS_STATE_NONE` hides the bar. `\since SDL 3.4.0`.
- `SDL_GetWindowProgressState(SDL_Window *window) -> SDL_ProgressState` — current progress state; `SDL_PROGRESS_STATE_INVALID` on failure. `\since SDL 3.4.0`.
- `SDL_SetWindowProgressValue(SDL_Window *window, float value) -> bool` — set progress value, clamped to `[0.0f, 1.0f]`. `\since SDL 3.4.0`.
- `SDL_GetWindowProgressValue(SDL_Window *window) -> float` — current progress value `[0.0f, 1.0f]`; `-1.0f` on failure. `\since SDL 3.4.0`.
- Hit-test (`SDL_HitTest` / `SDL_SetWindowHitTest`) — see Grabs & mouse input.
- System menu (`SDL_ShowWindowSystemMenu`) — see Grabs & mouse input.

## Window events

- `SDL_WindowEvent` struct (event.window.*) — window state change event data:
  - `SDL_EventType type` — one of `SDL_EVENT_WINDOW_*`.
  - `Uint32 reserved`.
  - `Uint64 timestamp` — nanoseconds, via `SDL_GetTicksNS()`.
  - `SDL_WindowID windowID` — the associated window.
  - `Sint32 data1`, `Sint32 data2` — event-dependent data.
- `SDL_EventType` window event values — each increments by one from `SDL_EVENT_WINDOW_SHOWN` (`0x202`); grouped below by the payload they carry in `data1`/`data2`:
  - `SDL_EVENT_WINDOW_FIRST` = `SDL_EVENT_WINDOW_SHOWN`; `SDL_EVENT_WINDOW_LAST` = `SDL_EVENT_WINDOW_HDR_STATE_CHANGED`.
  - without a `data1`/`data2` payload — pure notifications; `data1`/`data2` unused:
    - `SDL_EVENT_WINDOW_SHOWN` (`0x202`) — window shown.
    - `SDL_EVENT_WINDOW_HIDDEN` — window hidden.
    - `SDL_EVENT_WINDOW_METAL_VIEW_RESIZED` — pixel size of an associated Metal view changed.
    - `SDL_EVENT_WINDOW_MINIMIZED` — minimized.
    - `SDL_EVENT_WINDOW_MAXIMIZED` — maximized.
    - `SDL_EVENT_WINDOW_RESTORED` — restored to normal size and position.
    - `SDL_EVENT_WINDOW_MOUSE_ENTER` — gained mouse focus.
    - `SDL_EVENT_WINDOW_MOUSE_LEAVE` — lost mouse focus.
    - `SDL_EVENT_WINDOW_FOCUS_GAINED` — gained keyboard focus.
    - `SDL_EVENT_WINDOW_FOCUS_LOST` — lost keyboard focus.
    - `SDL_EVENT_WINDOW_CLOSE_REQUESTED` — the window manager requests close.
    - `SDL_EVENT_WINDOW_HIT_TEST` — a hit test that wasn't `SDL_HITTEST_NORMAL`.
    - `SDL_EVENT_WINDOW_ICCPROF_CHANGED` — the display's ICC profile changed.
    - `SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED` — window display scale changed.
    - `SDL_EVENT_WINDOW_SAFE_AREA_CHANGED` — window safe area changed.
    - `SDL_EVENT_WINDOW_OCCLUDED` — window occluded.
    - `SDL_EVENT_WINDOW_ENTER_FULLSCREEN` — entered fullscreen.
    - `SDL_EVENT_WINDOW_LEAVE_FULLSCREEN` — left fullscreen.
    - `SDL_EVENT_WINDOW_DESTROYED` — window with the associated ID is being/has been destroyed; the handle stays valid only while handled in an event watcher.
    - `SDL_EVENT_WINDOW_HDR_STATE_CHANGED` — window HDR properties changed.
  - with a `data1` payload only — `data1` carries an event-dependent value; `data2` unused:
    - `SDL_EVENT_WINDOW_EXPOSED` — window exposed, should be redrawn (redrawable directly from event watchers); `data1` `1` for live-resize expose, `0` otherwise.
    - `SDL_EVENT_WINDOW_DISPLAY_CHANGED` — moved to display `data1`.
  - with `data1` and `data2` payload — `data1`×`data2` carry a geometry value (coordinates or size):
    - `SDL_EVENT_WINDOW_MOVED` — moved to `data1`, `data2`.
    - `SDL_EVENT_WINDOW_RESIZED` — resized to `data1`x`data2`.
    - `SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED` — pixel size changed to `data1`x`data2`.
