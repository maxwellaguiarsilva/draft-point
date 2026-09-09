# Display (SDL3)

Subject markdown for the `sak::sdl3::display` object.

- Source object: `include/sak/sdl3/display.hpp`.
- Source of truth: SDL3 headers `SDL_video.h` and `SDL_events.h` (all objects `\since SDL 3.2.0` unless noted).
- Scope: canonical `Display` objects only; legacy / `_renamed_` / substituted aliases are excluded (e.g. `SDL_GetDisplayForPoint_renamed_SDL_GetDisplayForPoint`).
- Every display-query function is restricted to the main thread.
- Prefix `SDL_` is declared here and omitted from the free-function names below; opaque `SDL_*` type names keep their prefix.

## Handle type

- `SDL_DisplayID` — numeric ID; the value designating an invalid display is reserved.
  - Unique ID for a display for the time it is connected to the system; never reused for the lifetime of the application.
  - A display that is disconnected and reconnected gets a new ID.

## Enumeration / lifecycle

- `GetDisplays(int *count) -> SDL_DisplayID *` — `NULL` on failure.
  - `count`: filled with the number of displays; may be `NULL`.
  - Terminates with an invalid-ID sentinel; free with `SDL_free()`.
  - Returned heap array → RAII wrapper opportunity.
- `GetPrimaryDisplay(void) -> SDL_DisplayID` — reserved value on failure.

## Identity / properties

- `GetDisplayName(SDL_DisplayID displayID) -> const char *` — UTF-8; `NULL` on failure.
- `GetDisplayProperties(SDL_DisplayID displayID) -> SDL_PropertiesID` — reserved value on failure.
  - Backend-independent properties share prefix `SDL_PROP_DISPLAY_`:
    - `HDR_ENABLED_BOOLEAN` — HDR headroom above SDR white point.
  - KMS/DRM properties share prefix `SDL_PROP_DISPLAY_KMSDRM_`:
    - `PANEL_ORIENTATION_NUMBER` — panel orientation in degrees clockwise (hint only).
  - Wayland properties share prefix `SDL_PROP_DISPLAY_WAYLAND_`:
    - `WL_OUTPUT_POINTER`
  - Windows properties share prefix `SDL_PROP_DISPLAY_WINDOWS_`:
    - `HMONITOR_POINTER`

## Geometry / scaling

- `GetDisplayBounds(SDL_DisplayID displayID, SDL_Rect *rect) -> bool` — desktop area in screen coordinates; primary often at client origin, but placement depends on monitor layout.
- `GetDisplayUsableBounds(SDL_DisplayID displayID, SDL_Rect *rect) -> bool` — bounds minus system-reserved areas (e.g. macOS menu bar and dock).
  - Fullscreen windows bypass these unusable areas; guideline for non-fullscreen windows.
- `GetDisplayContentScale(SDL_DisplayID displayID) -> float` — expected content scale from display DPI; reserved value on failure.
  - After window creation prefer `GetWindowDisplayScale()` per window; it may differ from the base display value.

## Orientation

- `GetNaturalDisplayOrientation(SDL_DisplayID displayID) -> SDL_DisplayOrientation` — orientation when the display is unrotated; returns `SDL_ORIENTATION_UNKNOWN` if unavailable.
- `GetCurrentDisplayOrientation(SDL_DisplayID displayID) -> SDL_DisplayOrientation` — returns `SDL_ORIENTATION_UNKNOWN` if unavailable.
- `SDL_DisplayOrientation` enum — members share prefix `SDL_ORIENTATION_`:
  - `UNKNOWN`
  - `LANDSCAPE` — right side up.
  - `LANDSCAPE_FLIPPED` — left side up.
  - `PORTRAIT`
  - `PORTRAIT_FLIPPED` — upside down.

## Locating a display

- `GetDisplayForPoint(const SDL_Point *point) -> SDL_DisplayID` — reserved value on failure.
- `GetDisplayForRect(const SDL_Rect *rect) -> SDL_DisplayID` — selects display entirely containing the rect, or closest to its center; reserved value on failure.
- `GetDisplayForWindow(SDL_Window *window) -> SDL_DisplayID` — selects display containing the center of the window; reserved value on failure.
  - Cross-object lookup; used by `display.hpp` for construction from a window.

## Display modes

- `SDL_DisplayMode` struct fields:
  - `SDL_DisplayID displayID`
  - `SDL_PixelFormat format`
  - `int w`
  - `int h`
  - `float pixel_density` — scale converting size to pixels.
  - `float refresh_rate` — unspecified sentinel indicates desktop rate.
  - `int refresh_rate_numerator` — unspecified sentinel indicates desktop rate.
  - `int refresh_rate_denominator` — unspecified sentinel indicates desktop rate.
  - `SDL_DisplayModeData *internal`
- `SDL_DisplayModeData`
- `GetDesktopDisplayMode(SDL_DisplayID displayID) -> const SDL_DisplayMode *` — returns previous native mode when fullscreen changed resolution.
- `GetCurrentDisplayMode(SDL_DisplayID displayID) -> const SDL_DisplayMode *` — returns current mode when fullscreen changed resolution.
- `GetFullscreenDisplayModes(SDL_DisplayID displayID, int *count) -> SDL_DisplayMode **` — NULL-terminated array; free with `SDL_free()`.
  - Sort priority:
    - width descending
    - height descending
    - bits per pixel descending
    - packed pixel layout descending
    - refresh rate descending
    - pixel density ascending
  - Returned heap array → RAII wrapper opportunity.
- `GetClosestFullscreenDisplayMode(SDL_DisplayID displayID, int w, int h, float refresh_rate, bool include_high_density_modes, SDL_DisplayMode *closest) -> bool`
  - Fills `closest` scanning size first, format second, then refresh rate; returns `false` if every mode is too small.
  - Unspecified `refresh_rate` uses desktop refresh rate; `include_high_density_modes` includes high-density modes.

## Display events

- `SDL_DisplayEvent` struct (event.display.*) — display state change event data:
  - `SDL_EventType type` — one of the display event values below.
  - `Uint64 timestamp` — nanoseconds, via `GetTicksNS()`.
  - `SDL_DisplayID displayID` — the associated display.
  - `Sint32 data1` — event-dependent data.
  - `Sint32 data2` — event-dependent data.
- Display event values share prefix `SDL_EVENT_DISPLAY_`; grouped below by the payload they carry in `data1`/`data2`:
  - with a `data1` payload only — `data1` carries an event-dependent value; `data2` unused:
    - `ORIENTATION` — orientation changed to `data1`.
  - without a `data1`/`data2` payload — pure notifications; `data1`/`data2` unused:
    - `ADDED`
    - `REMOVED`
    - `MOVED`
    - `DESKTOP_MODE_CHANGED`
    - `CURRENT_MODE_CHANGED`
    - `CONTENT_SCALE_CHANGED`
    - `USABLE_BOUNDS_CHANGED`
