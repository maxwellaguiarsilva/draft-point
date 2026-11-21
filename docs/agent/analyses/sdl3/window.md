# Window (SDL3)

Subject markdown for the `sak::sdl3::window` object.

- Source object: `include/sak/sdl3/window.hpp`.
- Source of truth: SDL3 headers `SDL_video.h` and `SDL_events.h` (all objects `\since SDL 3.2.0` unless noted).
- Scope: canonical `Window` objects only; `_renamed_` / `_deprecated_use_` aliases, all `SDL_GL_*` / `SDL_EGL_*` context functions, all `SDL_Render*` / render-related symbols, display lookups, and OS-native / EGL / Wayland external handles are excluded.
- Every function here is restricted to the main thread unless noted.
- All `bool` setters return `true` on success / `false` on failure; call `SDL_GetError()` for more information.
- Prefix `SDL_` is declared here and omitted from the free-function names below; opaque `SDL_*` type names keep their prefix.

## Handle types

- `SDL_Window` — `typedef struct SDL_Window` — opaque handle to a window.
- `SDL_WindowID` — numeric ID; the value designating "no window / failure" is reserved.
  - `SDL_WindowEvent` references windows by this ID.

## Window flags / position sentinels

- `SDL_WindowFlags` — mask of read/write window state; some immutable after `CreateWindow()`, some app-changeable, some altered by user/system.
  - Creating with `SDL_WINDOW_RESIZABLE` constrains the window to compositor-recommended dimensions within usable desktop space.
  - Members share prefix `SDL_WINDOW_`; grouped by the `window::flag` categories below:
    - window state that can be both requested and reported:
      - `FULLSCREEN`
      - `HIDDEN` — needs `ShowWindow()` to become visible.
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
- Position sentinels used for the `x`/`y` of `SetWindowPosition`; for the display variants the `X` argument is an `SDL_DisplayID`:
  - `UNDEFINED` — always uses the primary display.
  - `UNDEFINED_DISPLAY(X)`
  - `ISUNDEFINED(X)`
  - `CENTERED` — center on the primary display.
  - `CENTERED_DISPLAY(X)`
  - `ISCENTERED(X)`

## Supporting enums / callback

- `SDL_FlashOperation` enum — flash mode for `FlashWindow`; members share prefix `SDL_FLASH_`:
  - `CANCEL`
  - `BRIEFLY`
  - `UNTIL_FOCUSED`
- `SDL_ProgressState` enum — taskbar-icon progress-bar state `\since SDL 3.4.0`; members share prefix `SDL_PROGRESS_STATE_`:
  - `INVALID` — check `SDL_GetError()`.
  - `NONE` — no progress bar shown.
  - `INDETERMINATE`
  - `NORMAL`
  - `PAUSED`
  - `ERROR`
- `SDL_HitTestResult` enum — return values from an `SDL_HitTest` callback; members share prefix `SDL_HITTEST_`:
  - `NORMAL`
  - `DRAGGABLE`
  - `RESIZE_TOPLEFT`
  - `RESIZE_TOP`
  - `RESIZE_TOPRIGHT`
  - `RESIZE_RIGHT`
  - `RESIZE_BOTTOMRIGHT`
  - `RESIZE_BOTTOM`
  - `RESIZE_BOTTOMLEFT`
  - `RESIZE_LEFT`
  - The `RESIZE_*` members designate each region of the window as a resizing border.
- `SDL_HitTest(SDL_Window *win, const SDL_Point *area, void *data) -> SDL_HitTestResult` — `typedef SDL_HitTestResult (SDLCALL *)`.
  - `win`: the window hit-testing was set on.
  - `area`: the point to hit-test.
  - `data`: the `callback_data` passed to `SetWindowHitTest()`.

## Creation & destruction

- `CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags) -> SDL_Window *` — `NULL` on failure.
  - Size is a request; actual size may differ per window-manager policy; be prepared for any size.
  - Shown unless `SDL_WINDOW_HIDDEN`; may then be shown via `ShowWindow()`.
  - `flags` may be empty or any `SDL_WindowFlags` OR'd together.
  - `SDL_WINDOW_OPENGL`/`VULKAN` trigger the corresponding LoadLibrary; UnloadLibrary is called by `DestroyWindow()`.
  - Creating with `SDL_WINDOW_VULKAN`/`SDL_WINDOW_METAL` fails if there is no working driver/platform support.
  - Created object → RAII wrapper opportunity (owned by caller).
- `CreatePopupWindow(SDL_Window *parent, int offset_x, int offset_y, int w, int h, SDL_WindowFlags flags) -> SDL_Window *` — `NULL` on failure.
  - `flags` **must** contain `SDL_WINDOW_TOOLTIP` or `SDL_WINDOW_POPUP_MENU`.
  - `SDL_WINDOW_MINIMIZED`/`MAXIMIZED`/`FULLSCREEN`/`BORDERLESS` ignored; `SDL_WINDOW_UTILITY`/`MODAL` cause failure.
  - `parent` must be non-null.
  - A popup cannot be minimized/maximized/fullscreen/raised/flashed/made modal/grab input.
  - Implicitly undecorated and off the taskbar/alt-tab.
  - Destroying/hiding the parent recursively destroys/hides popup children.
  - Created object → RAII wrapper opportunity (owned by caller).
- `CreateWindowWithProperties(SDL_PropertiesID props) -> SDL_Window *` — `NULL` on failure.
  - Backend-independent properties share prefix `SDL_PROP_WINDOW_CREATE_`:
    - `ALWAYS_ON_TOP_BOOLEAN`
    - `BORDERLESS_BOOLEAN`
    - `CONSTRAIN_POPUP_BOOLEAN`
    - `EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN`
    - `FOCUSABLE_BOOLEAN`
    - `FULLSCREEN_BOOLEAN`
    - `HEIGHT_NUMBER`
    - `HIDDEN_BOOLEAN`
    - `HIGH_PIXEL_DENSITY_BOOLEAN`
    - `MAXIMIZED_BOOLEAN`
    - `MENU_BOOLEAN`
    - `METAL_BOOLEAN`
    - `MINIMIZED_BOOLEAN`
    - `MODAL_BOOLEAN`
    - `MOUSE_GRABBED_BOOLEAN`
    - `OPENGL_BOOLEAN`
    - `PARENT_POINTER`
    - `RESIZABLE_BOOLEAN`
    - `TITLE_STRING`
    - `TRANSPARENT_BOOLEAN`
    - `TOOLTIP_BOOLEAN`
    - `UTILITY_BOOLEAN`
    - `VULKAN_BOOLEAN`
    - `WIDTH_NUMBER`
    - `X_NUMBER`
    - `Y_NUMBER` — accepts `SDL_WINDOWPOS_CENTERED`/`UNDEFINED`; defaults to `UNDEFINED`.
  - Wayland additions share prefix `SDL_PROP_WINDOW_CREATE_`:
    - `WAYLAND_SURFACE_ROLE_CUSTOM_BOOLEAN`
    - `WAYLAND_CREATE_EGL_WINDOW_BOOLEAN`
    - `WAYLAND_WL_SURFACE_POINTER`
  - Emscripten additions share prefix `SDL_PROP_WINDOW_CREATE_`:
    - `EMSCRIPTEN_CANVAS_ID_STRING`
    - `EMSCRIPTEN_KEYBOARD_ELEMENT_STRING`
  - Implicitly shown if the `hidden` property is not set.
  - Tooltip/menu properties create popup windows.
  - Created object → RAII wrapper opportunity (owned by caller).
- `DestroyWindow(SDL_Window *window) -> void`
  - Child (popup) windows are recursively destroyed.
  - On some platforms the visible window may only be removed when the event loop is pumped again, even though the handle is invalid right after this call.

## Enumeration & identity lookup

- `GetWindows(int *count) -> SDL_Window **` — NULL-terminated list of valid windows; `NULL` on failure.
  - `count`: filled with the number of windows; may be `NULL`.
  - Single allocation; free with `SDL_free()`.
  - Returned heap array → RAII wrapper opportunity.
- `GetWindowID(SDL_Window *window) -> SDL_WindowID` — the ID used by `SDL_WindowEvent` to map events to window objects.
- `GetWindowFromID(SDL_WindowID id) -> SDL_Window *` — window for a stored ID; `NULL` if it doesn't exist.
- `GetWindowFromEvent(const SDL_Event *event) -> SDL_Window *` — window associated with an event containing a `windowID`; `NULL` if there is none.
  - Safe to call from any thread.
- `GetWindowParent(SDL_Window *window) -> SDL_Window *` — `NULL` if it has none.

## State, visibility & fullscreen

- `ShowWindow(SDL_Window *window) -> bool`
- `HideWindow(SDL_Window *window) -> bool`
- `RaiseWindow(SDL_Window *window) -> bool`
  - Subject to window-manager policy; may fail to steal focus.
  - On success emits `SDL_EVENT_WINDOW_FOCUS_GAINED` and sets `SDL_WINDOW_INPUT_FOCUS`.
- `MaximizeWindow(SDL_Window *window) -> bool`
  - Non-resizable windows (no `SDL_WINDOW_RESIZABLE`) can't be maximized (no effect).
  - Asynchronous on some systems; emits `SDL_EVENT_WINDOW_MAXIMIZED`.
- `MinimizeWindow(SDL_Window *window) -> bool`
  - No direct effect on a fullscreen window; may alter the state returned to on leaving fullscreen.
  - Emits `SDL_EVENT_WINDOW_MINIMIZED`.
- `RestoreWindow(SDL_Window *window) -> bool`
  - Emits `SDL_EVENT_WINDOW_RESTORED`.
- `SyncWindow(SDL_Window *window) -> bool` — block until any pending window state is finalized.
  - Acts as a sync barrier on asynchronous systems; guaranteed finite time but may block for window-state animations.
  - Does nothing where changes are immediate.
  - Returns `false` if it timed out before reaching the requested state.
- `SetWindowFullscreen(SDL_Window *window, bool fullscreen) -> bool` — default is borderless desktop mode unless an exclusive mode is set via `SetWindowFullscreenMode()`.
  - Emits `SDL_EVENT_WINDOW_ENTER_FULLSCREEN`/`LEAVE_FULLSCREEN`; can be denied.
- `SetWindowFullscreenMode(SDL_Window *window, const SDL_DisplayMode *mode) -> bool`
  - `mode`: `NULL` for borderless desktop mode, or an exclusive mode from `GetFullscreenDisplayModes()`.
  - Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`/`PIXEL_SIZE_CHANGED` on change.
- `GetWindowFullscreenMode(SDL_Window *window) -> const SDL_DisplayMode *` — `NULL` for borderless desktop mode.
- `FlashWindow(SDL_Window *window, SDL_FlashOperation operation) -> bool`
- `SetWindowFocusable(SDL_Window *window, bool focusable) -> bool`
- `SetWindowParent(SDL_Window *window, SDL_Window *parent) -> bool` — `NULL` unparents.
  - Reparents if already a child.
  - Fails if the window is modal (cancel via `SetWindowModal`) or is a popup.
  - Setting a sibling/descendant as parent is undefined behavior.
- `SetWindowModal(SDL_Window *window, bool modal) -> bool` — the window must currently be a child to enable.

## Geometry, size, position & aspect

- `SetWindowSize(SDL_Window *window, int w, int h) -> bool` — operates on the client area.
  - `w`/`h` must be positive.
  - No effect on fullscreen/maximized windows.
  - Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`.
- `GetWindowSize(SDL_Window *window, int *w, int *h) -> bool` — client-area size in window coordinates.
  - May differ from pixels on high-DPI.
  - Out-params may be `NULL`.
- `GetWindowSizeInPixels(SDL_Window *window, int *w, int *h) -> bool` — client-area size in pixels (drawable size).
  - Out-params may be `NULL`.
- `SetWindowMinimumSize(SDL_Window *window, int min_w, int min_h) -> bool` — a zero dimension means no limit.
- `GetWindowMinimumSize(SDL_Window *window, int *w, int *h) -> bool`
  - Out-params may be `NULL`.
- `SetWindowMaximumSize(SDL_Window *window, int max_w, int max_h) -> bool` — a zero dimension means no limit.
- `GetWindowMaximumSize(SDL_Window *window, int *w, int *h) -> bool`
  - Out-params may be `NULL`.
- `GetWindowBordersSize(SDL_Window *window, int *top, int *left, int *bottom, int *right) -> bool` — decoration border sizes around the client area.
  - On failure the values are set to borderless-sized if non-`NULL`.
  - Fails before the display server decorates the window (e.g. right after creation) and if unsupported.
  - Out-params may be `NULL`.
- `SetWindowPosition(SDL_Window *window, int x, int y) -> bool` — `x`/`y` may be `SDL_WINDOWPOS_CENTERED` or `SDL_WINDOWPOS_UNDEFINED`.
  - No effect on exclusive-fullscreen/maximized windows.
  - Asynchronous on some systems; emits `SDL_EVENT_WINDOW_MOVED`; can be constrained or denied.
- `GetWindowPosition(SDL_Window *window, int *x, int *y) -> bool` — position as last reported by the system.
  - Out-params may be `NULL`.
- `SetWindowAspectRatio(SDL_Window *window, float min_aspect, float max_aspect) -> bool` — limits are width/height; a zero limit means no limit.
  - Deferred while fixed-size (maximized/fullscreen).
  - Asynchronous on some systems; emits `SDL_EVENT_WINDOW_RESIZED`.
- `GetWindowAspectRatio(SDL_Window *window, float *min_aspect, float *max_aspect) -> bool`
  - Out-params may be `NULL`.
- `GetWindowSafeArea(SDL_Window *window, SDL_Rect *rect) -> bool` — client area safe for interactable content.
  - Accounts for on-screen controls, curved edges, camera notches, TV overscan.
  - Still render elsewhere but avoid important/interactable content there.

## Decoration & appearance

- `SetWindowBordered(SDL_Window *window, bool bordered) -> bool` — add/remove `SDL_WINDOW_BORDERLESS`.
  - No-op if the border already matches.
  - Cannot change a fullscreen window's border.
- `SetWindowResizable(SDL_Window *window, bool resizable) -> bool` — add/remove `SDL_WINDOW_RESIZABLE`.
  - No-op if the state matches.
  - Cannot change a fullscreen window's resizable state.
- `SetWindowAlwaysOnTop(SDL_Window *window, bool on_top) -> bool` — add/remove `SDL_WINDOW_ALWAYS_ON_TOP`.
  - Brings the window to the front and keeps it above.
- `SetWindowFillDocument(SDL_Window *window, bool fill) -> bool` — add/remove `SDL_WINDOW_FILL_DOCUMENT` (Emscripten only).
  - When enabled the canvas fills the entire document and covers everything else; minimum size and aspect constraints may not work as expected.

## Pixel / DPI info

- `GetWindowPixelDensity(SDL_Window *window) -> float` — ratio of pixel size to window size; the value for an unknown/failed state is reserved (check `SDL_GetError()`).
- `GetWindowDisplayScale(SDL_Window *window) -> float` — content display scale relative to the window's pixel size; reserved value on failure.
  - Combination of pixel density and display content scale; the expected scale for content.
  - Updated when the scale setting changes or the window moves display.
- `GetWindowPixelFormat(SDL_Window *window) -> SDL_PixelFormat` — pixel format of the window; a sentinel on failure.
- `GetWindowICCProfile(SDL_Window *window, size_t *size) -> void *` — raw ICC profile for the current screen; `NULL` on failure.
  - `size`: filled with the profile size.
  - Free with `SDL_free()`.

## Properties

- `GetWindowProperties(SDL_Window *window) -> SDL_PropertiesID` — read-only window properties; reserved value on failure.
  - SDL-provided properties share prefix `SDL_PROP_WINDOW_`:
    - `SHAPE_POINTER` — the shape surface.
    - `HDR_ENABLED_BOOLEAN` — HDR headroom above SDR white point.
    - `SDR_WHITE_LEVEL_FLOAT`
    - `HDR_HEADROOM_FLOAT`
  - The HDR properties change dynamically and emit `SDL_EVENT_WINDOW_HDR_STATE_CHANGED`.
  - Windows properties share prefix `SDL_PROP_WINDOW_WIN32_`:
    - `HWND_POINTER`
    - `HDC_POINTER`
    - `INSTANCE_POINTER`
  - X11 properties share prefix `SDL_PROP_WINDOW_X11_`:
    - `DISPLAY_POINTER`
    - `SCREEN_NUMBER`
    - `WINDOW_NUMBER`
  - Wayland `xdg_*` objects do not persist across show/hide; query them each time the window is shown.
- `GetWindowFlags(SDL_Window *window) -> SDL_WindowFlags` — mask of the current flags.

## Title / icon / shape / opacity

- `SetWindowTitle(SDL_Window *window, const char *title) -> bool` — UTF-8.
- `GetWindowTitle(SDL_Window *window) -> const char *` — UTF-8; empty if there is no title.
- `SetWindowIcon(SDL_Window *window, SDL_Surface *icon) -> bool`
  - Alternate representations added via `AddSurfaceAlternateImage()` are used for high-DPI (100%-scale content plus up/downscaled alternatives).
- `SetWindowShape(SDL_Window *window, SDL_Surface *shape) -> bool` — shape is the alpha channel of a transparent window; `NULL` removes it.
  - Fully transparent areas are also mouse-transparent.
  - Shape is copied internally (safe to free after).
  - Requires creation with `SDL_WINDOW_TRANSPARENT`; expensive, call sparingly.
- `SetWindowOpacity(SDL_Window *window, float opacity) -> bool` — clamped to the unit range.
  - Returns `false` if unsupported on the platform.
- `GetWindowOpacity(SDL_Window *window) -> float` — unit range; reserved value on failure.
  - Returns an unclamped-opacity sentinel without error if transparency isn't supported.

## Grabs & mouse input

- `SetWindowKeyboardGrab(SDL_Window *window, bool grabbed) -> bool`
  - Keyboard grab captures system shortcuts like Alt+Tab / Meta; primarily for specialized apps (VNC, VM frontends), not normal games.
  - Enabling while another window is grabbed transfers the grab.
- `GetWindowKeyboardGrab(SDL_Window *window) -> bool`
- `SetWindowMouseGrab(SDL_Window *window, bool grabbed) -> bool` — confines the cursor to the window.
- `GetWindowMouseGrab(SDL_Window *window) -> bool`
- `GetGrabbedWindow(void) -> SDL_Window *` — `NULL` if no window holds an input grab.
- `SetWindowMouseRect(SDL_Window *window, const SDL_Rect *rect) -> bool` — confines the cursor to `rect` (window-relative) while the window has focus; `NULL` destroys the barrier.
  - Does not grab the cursor, only restricts its area.
- `GetWindowMouseRect(SDL_Window *window) -> const SDL_Rect *` — `NULL` if no confinement rectangle.
- `ShowWindowSystemMenu(SDL_Window *window, int x, int y) -> bool` — `x`/`y` are relative to the client origin.
  - On unsupported platforms/desktops this does nothing.
- `SetWindowHitTest(SDL_Window *window, SDL_HitTest callback, void *callback_data) -> bool`
  - Enables custom drag/resize regions (e.g. borderless windows).
  - Mouse input in special regions may be consumed by the OS.
  - `callback`: `NULL` disables (default off).
  - Unsupported platforms return `false` unconditionally.
  - Callback can fire at any time; keep it efficient.

## Window surfaces

- `WindowHasSurface(SDL_Window *window) -> bool`
- `GetWindowSurface(SDL_Window *window) -> SDL_Surface *` — a new optimal-format one is created if necessary; `NULL` on failure.
  - Freed when the window is destroyed — **do not free** (RAII: non-owning).
  - Invalidated on resize; call again after a resize.
  - Not combinable with 3D/render APIs.
- `SetWindowSurfaceVSync(SDL_Window *window, int vsync) -> bool`
  - vsync interval: values per vertical refresh, or an "adaptive" sentinel; default is disabled.
  - Not every value is supported; check the return.
- `GetWindowSurfaceVSync(SDL_Window *window, int *vsync) -> bool`
- `UpdateWindowSurface(SDL_Window *window) -> bool` — copy the window surface to the screen (equivalent to SDL 1.2 `Flip`).
- `UpdateWindowSurfaceRects(SDL_Window *window, const SDL_Rect *rects, int numrects) -> bool` — copy areas of the surface to the screen (equivalent to `UpdateRects`).
  - Updates at least the given rects, but may update more (optimization only).
- `DestroyWindowSurface(SDL_Window *window) -> bool`

## Misc

- `FlashWindow` — see State, visibility & fullscreen (flash).
- `SetWindowProgressState(SDL_Window *window, SDL_ProgressState state) -> bool` — a "none" state hides the bar. `\since SDL 3.4.0`.
- `GetWindowProgressState(SDL_Window *window) -> SDL_ProgressState` `\since SDL 3.4.0`.
- `SetWindowProgressValue(SDL_Window *window, float value) -> bool` — clamped to the unit range. `\since SDL 3.4.0`.
- `GetWindowProgressValue(SDL_Window *window) -> float` — unit range. `\since SDL 3.4.0`.
- Hit-test (`SDL_HitTest` / `SetWindowHitTest`) — see Grabs & mouse input.
- System menu (`ShowWindowSystemMenu`) — see Grabs & mouse input.

## Window events

- `SDL_WindowEvent` struct (event.window.*) — window state change event data:
  - `SDL_EventType type` — one of the window event values below.
  - `Uint64 timestamp` — nanoseconds, via `GetTicksNS()`.
  - `SDL_WindowID windowID` — the associated window.
  - `Sint32 data1` — event-dependent data.
  - `Sint32 data2` — event-dependent data.
- Window event values share prefix `SDL_EVENT_WINDOW_`; grouped below by the payload they carry in `data1`/`data2`:
  - without a `data1`/`data2` payload — pure notifications; `data1`/`data2` unused:
    - `SHOWN`
    - `HIDDEN`
    - `METAL_VIEW_RESIZED` — pixel size of an associated Metal view changed.
    - `MINIMIZED`
    - `MAXIMIZED`
    - `RESTORED`
    - `OCCLUDED`
    - `ENTER_FULLSCREEN`
    - `LEAVE_FULLSCREEN`
    - `MOUSE_ENTER` — gained mouse focus.
    - `MOUSE_LEAVE` — lost mouse focus.
    - `FOCUS_GAINED` — gained keyboard focus.
    - `FOCUS_LOST` — lost keyboard focus.
    - `CLOSE_REQUESTED` — the window manager requests close.
    - `HIT_TEST` — a hit test that wasn't the normal region.
    - `ICCPROF_CHANGED` — the display's ICC profile changed.
    - `DISPLAY_SCALE_CHANGED`
    - `SAFE_AREA_CHANGED`
    - `DESTROYED` — the window with the associated ID is being/has been destroyed; the handle stays valid only while handled in an event watcher.
    - `HDR_STATE_CHANGED` — the window's HDR properties changed.
  - with a `data1` payload only — `data1` carries an event-dependent value; `data2` unused:
    - `EXPOSED` — window exposed, should be redrawn (redrawable directly from event watchers); `data1` distinguishes a live-resize expose from other exposes.
    - `DISPLAY_CHANGED` — moved to display `data1`.
  - with `data1` and `data2` payload — `data1`×`data2` carry a geometry value (coordinates or size):
    - `MOVED` — moved to `data1`, `data2`.
    - `RESIZED` — resized to `data1`x`data2`.
    - `PIXEL_SIZE_CHANGED` — pixel size changed to `data1`x`data2`.
