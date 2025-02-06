#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>

inline HANDLE PHANDLE = nullptr;

// Methods
inline CFunctionHook *g_pMouseMotionHook = nullptr;
typedef void (*origMotion)(CSeatManager *, uint32_t, const Vector2D &);

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION()
{
    return HYPRLAND_API_VERSION;
}

bool hasClass(std::string const &s, std::string const &name)
{
    auto const x = s.find(name);
    return x != std::string::npos && (!x || (x && !isalpha(s[x - 1]))) && x <= s.size() - name.size() && !isalpha(s[x + name.size()]);
}

void hkNotifyMotion(CSeatManager *thisptr, uint32_t time_msec, const Vector2D &local)
{
    static auto *const PMARGIN = (Hyprlang::INT *const *)HyprlandAPI::getConfigValue(PHANDLE, "plugin:magic-mouse-gaps:margin")->getDataStaticPtr();
    static auto *const PCLASS = (Hyprlang::STRING const *)HyprlandAPI::getConfigValue(PHANDLE, "plugin:magic-mouse-gaps:class")->getDataStaticPtr();

    Vector2D newCoords = local;

    if (!g_pCompositor->m_pLastWindow.expired() && hasClass(*PCLASS, g_pCompositor->m_pLastWindow->m_szInitialClass) && g_pCompositor->m_pLastMonitor)
    {
        if (local.x < 0)
            newCoords.x = **PMARGIN;
        if (local.y < 0)
            newCoords.y = **PMARGIN;
    }

    (*(origMotion)g_pMouseMotionHook->m_pOriginal)(thisptr, time_msec, newCoords);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle)
{
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH)
    {
        HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[vkfix] Version mismatch");
    }

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:magic-mouse-gaps:margin", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:magic-mouse-gaps:class", Hyprlang::STRING{"firefox"});

    auto FNS = HyprlandAPI::findFunctionsByName(PHANDLE, "sendPointerMotion");
    for (auto &fn : FNS)
    {
        if (!fn.demangled.contains("CSeatManager"))
            continue;

        g_pMouseMotionHook = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void *)::hkNotifyMotion);
        break;
    }

    bool success = g_pMouseMotionHook;
    if (!success)
    {
        HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Hook init failed", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[magic-mouse-gaps] Hook init failed");
    }

    success = success && g_pMouseMotionHook->hook();

    if (!success)
    {
        HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Hook failed", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[magic-mouse-gaps] Hook failed");
    }

    return {"magic-mouse-gaps", "A plugin to move mouse events from gaps to nearby matching window (use firefox tabs from gaps)", "Dregu", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT()
{
    ;
}
