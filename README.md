# magic-mouse-gaps

Hyprland plugin to move mouse clicks from just outside a window (i.e. on gaps and borders) to just inside a the window, to click browser tabs without having to aim.

This is copied entirely from the [csgo-vulkan-fix](https://github.com/hyprwm/hyprland-plugins/tree/main/csgo-vulkan-fix), I did nothing really...

```
plugin {
    magic-mouse-gaps {
        # Full match RE2 regex for window class to enable on, .* for any (default: firefox)
        class = firefox|Chromium|codium

        # Add some extra oomph if the window has some extra borders (default: 0)
        margin = 3
    }
}
```
