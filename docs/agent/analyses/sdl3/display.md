# Display (SDL3)

Subject markdown for the `sak::sdl3::display` object.

- Source object: `include/sak/sdl3/display.hpp`.
- Source of truth: SDL3 headers `SDL_video.h` and `SDL_events.h` (all objects `\since SDL 3.2.0` unless noted).
- Scope: canonical `Display` objects only; legacy / `_renamed_` / substituted aliases are excluded (e.g. `SDL_GetDisplayForPoint_renamed_SDL_GetDisplayForPoint`).
- Every display-query function is restricted to the main thread.

## Handle type

- `SDL_DisplayID` — `typedef Uint32`.
  - Unique ID for a display for the time it is connected to the system; never reused for the lifetime of the application.
  - A display that is disconnected and reconnected gets a new ID.
  - `0` is an invalid ID.

## Enumeration / lifecycle

- `SDL_GetDisplays(int *count) -> SDL_DisplayID *` — list of currently connected displays.
  - `count`: filled with the number of displays; may be `NULL`.
  - Returns a `0`-terminated array of instance IDs, or `NULL` on failure; free with `SDL_free()`.
  - Returned heap array → RAII wrapper opportunity.
- `SDL_GetPrimaryDisplay(void) -> SDL_DisplayID` — primary display; `0` on failure.

## Identity / properties

- `SDL_GetDisplayName(SDL_DisplayID displayID) -> const char *` — display name in UTF-8; `NULL` on failure.
- `SDL_GetDisplayProperties(SDL_DisplayID displayID) -> SDL_PropertiesID` — read-only display properties; `0` on failure.
  - Backend-independent: `SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN` (HDR headroom above SDR white point; informational).
  - KMS/DRM: `SDL_PROP_DISPLAY_KMSDRM_PANEL_ORIENTATION_NUMBER` (panel orientation in degrees clockwise; a hint only).
  - Wayland: `SDL_PROP_DISPLAY_WAYLAND_WL_OUTPUT_POINTER` (`wl_output`).
  - Windows: `SDL_PROP_DISPLAY_WINDOWS_HMONITOR_POINTER` (`HMONITOR`).

## Geometry / scaling

- `SDL_GetDisplayBounds(SDL_DisplayID displayID, SDL_Rect *rect) -> bool` — desktop area of a display in screen coordinates; primary often at `(0,0)`, but placement depends on monitor layout.
- `SDL_GetDisplayUsableBounds(SDL_DisplayID displayID, SDL_Rect *rect) -> bool` — usable desktop area (bounds minus system-reserved areas, e.g. macOS menu bar and dock). Fullscreen windows bypass these unusable areas; good guideline for non-fullscreen windows.
- `SDL_GetDisplayContentScale(SDL_DisplayID displayID) -> float` — expected content scale from display DPI (e.g. `2.0` = 200%); `0.0f` on failure.
  - After window creation prefer `SDL_GetWindowDisplayScale()` per window; it may differ from the base display value (high-DPI / multi-monitor).

## Orientation

- `SDL_GetNaturalDisplayOrientation(SDL_DisplayID displayID) -> SDL_DisplayOrientation` — orientation when the display is unrotated.
- `SDL_GetCurrentDisplayOrientation(SDL_DisplayID displayID) -> SDL_DisplayOrientation` — current orientation.
- Return `SDL_ORIENTATION_UNKNOWN` if not available.
- `SDL_DisplayOrientation` enum: `UNKNOWN`, `LANDSCAPE` (right side up), `LANDSCAPE_FLIPPED` (left side up), `PORTRAIT`, `PORTRAIT_FLIPPED` (upside down).

## Locating a display

- `SDL_GetDisplayForPoint(const SDL_Point *point) -> SDL_DisplayID` — display containing the point; `0` on failure.
- `SDL_GetDisplayForRect(const SDL_Rect *rect) -> SDL_DisplayID` — display entirely containing the rect, or closest to its center; `0` on failure.
- `SDL_GetDisplayForWindow(SDL_Window *window) -> SDL_DisplayID` — display containing the center of the window; `0` on failure.
  - Cross-object lookup; already used by `display.hpp` for construction from a window.

## Display modes

- `SDL_DisplayMode` struct fields:
  - `SDL_DisplayID displayID` — display the mode is associated with.
  - `SDL_PixelFormat format` — pixel format.
  - `int w`, `int h` — width, height.
  - `float pixel_density` — scale converting size to pixels (e.g. 1920×1080 mode at 2.0 → 3840×2160 pixels).
  - `float refresh_rate` — refresh rate, or `0.0f` for unspecified.
  - `int refresh_rate_numerator`, `int refresh_rate_denominator` — precise refresh rate, `0` when unspecified.
  - `SDL_DisplayModeData *internal` — opaque, private.
- `SDL_DisplayModeData` — opaque internal display mode data.
- `SDL_GetDesktopDisplayMode(SDL_DisplayID displayID) -> const SDL_DisplayMode *` — desktop mode; differs from current when SDL is fullscreen and changed resolution (returns the previous native mode).
- `SDL_GetCurrentDisplayMode(SDL_DisplayID displayID) -> const SDL_DisplayMode *` — current mode; differs from desktop when fullscreen and changed resolution (returns the current mode).
- `SDL_GetFullscreenDisplayModes(SDL_DisplayID displayID, int *count) -> SDL_DisplayMode **` — NULL-terminated array of fullscreen modes; free with `SDL_free()`.
  - Sorted by: width desc, height desc, bits per pixel desc, packed pixel layout desc, refresh rate desc, pixel density asc.
  - Returned heap array → RAII wrapper opportunity.
- `SDL_GetClosestFullscreenDisplayMode(SDL_DisplayID displayID, int w, int h, float refresh_rate, bool include_high_density_modes, SDL_DisplayMode *closest) -> bool`
  - Fills `closest` scanning size first, format second, then refresh rate; returns `false` if every mode is too small.
  - `refresh_rate` `0.0f` → desktop refresh rate; `include_high_density_modes` includes high-density modes in the search.

## Display events

- `SDL_DisplayEvent` struct (event.display.*) — display state change event data:
  - `SDL_EventType type` — one of `SDL_EVENT_DISPLAY_*`.
  - `Uint32 reserved`.
  - `Uint64 timestamp` — nanoseconds, via `SDL_GetTicksNS()`.
  - `SDL_DisplayID displayID` — the associated display.
  - `Sint32 data1`, `Sint32 data2` — event-dependent data.
- `SDL_EventType` display event values:
  - `SDL_EVENT_DISPLAY_ORIENTATION` (`0x151`) — orientation changed to `data1`.
  - `SDL_EVENT_DISPLAY_ADDED` — display added to the system.
  - `SDL_EVENT_DISPLAY_REMOVED` — display removed from the system.
  - `SDL_EVENT_DISPLAY_MOVED` — display changed position.
  - `SDL_EVENT_DISPLAY_DESKTOP_MODE_CHANGED` — display changed desktop mode.
  - `SDL_EVENT_DISPLAY_CURRENT_MODE_CHANGED` — display changed current mode.
  - `SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED` — display changed content scale.
  - `SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED` — display changed usable bounds.
  - `SDL_EVENT_DISPLAY_FIRST` = `SDL_EVENT_DISPLAY_ORIENTATION`; `SDL_EVENT_DISPLAY_LAST` = `SDL_EVENT_DISPLAY_USABLE_BOUNDS_CHANGED`.
