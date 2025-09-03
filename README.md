# magic-mouse-gaps

Hyprland plugin to move mouse events from just outside a tiled window (i.e. on gaps and borders) to just inside the window, to click browser tabs without having to aim inside the window.

```
plugin {
    magic-mouse-gaps {
        # Full match RE2 regex for window class to enable on, .* for any (default: firefox)
        class = firefox|Chromium|codium

        # Add some extra margin to clicks if the window has some internal margin above the tabs (default: 0)
        margin = 3

        # Window edges to enable on, any combination of t/b/l/r (default: t)
        edge = tlr

        # Max size of the extra area outside a window to catch mouse events in, 0 to disable (default: 32)
        size = 32
    }
}
```
