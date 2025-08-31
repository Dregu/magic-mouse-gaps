# magic-mouse-gaps

Hyprland plugin to move mouse events from just outside a tiled window (i.e. on gaps and borders) to just inside the window, to click browser tabs without having to aim inside the window.

```
plugin {
    magic-mouse-gaps {
        # Full match RE2 regex for window class to enable on, .* for any (default: firefox)
        class = firefox|Chromium|codium

        # Add some extra oomph if the window has some extra borders (default: 0)
        margin = 3

        # Window edges to enable on, any combination of t/b/l/r (default: t)
        edge = tlr
    }
}
```
