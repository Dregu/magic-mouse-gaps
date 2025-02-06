# magic-mouse-gaps

Hyprland plugin to move mouse events from just outside a window (i.e. on gaps and borders) to just inside a nearby window, to click browser tabs without having to aim.

This is copied entirely from the [csgo-vulkan-fix](https://github.com/hyprwm/hyprland-plugins/tree/main/csgo-vulkan-fix), I did nothing really...

```
plugin {
    magic-mouse-gaps {
        # Exact initial window class or classes separated by space (default: firefox)
        class = firefox Cromite code-oss

        # Add some extra oomph if the window has some extra borders (default: 0)
        margin = 3
    }
}
```
