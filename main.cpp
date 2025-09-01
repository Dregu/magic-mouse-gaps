#define WLR_USE_UNSTABLE

#include <unistd.h>
#include <re2/re2.h>

#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/managers/SeatManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/desktop/LayerSurface.hpp>
#include "hyprland/src/helpers/memory/Memory.hpp"

inline HANDLE PHANDLE = nullptr;

// Methods
inline CFunctionHook* g_pMouseMotionHook = nullptr;
typedef void (*origMotion)(CSeatManager*, uint32_t, const Vector2D&);

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

void hkNotifyMotion(CSeatManager* thisptr, uint32_t time_msec, const Vector2D& local) {
    static auto* const PMARGIN = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:magic-mouse-gaps:margin")->getDataStaticPtr();
    static auto* const PCLASS  = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:magic-mouse-gaps:class")->getDataStaticPtr();
    static auto* const PEDGE   = (Hyprlang::STRING const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:magic-mouse-gaps:edge")->getDataStaticPtr();

    Vector2D           newCoords = local;
    Vector2D           surfacePos;
    Vector2D           surfaceCoords;
    bool               foundSurface = false;
    SP<CLayerSurface>  pFoundLayerSurface;
    const auto         PMONITOR = g_pInputManager->isLocked() && g_pCompositor->m_lastMonitor ? g_pCompositor->m_lastMonitor.lock() : g_pCompositor->getMonitorFromCursor();

    if (PMONITOR) {
        // forced above all
        if (!g_pInputManager->m_exclusiveLSes.empty()) {
            if (!foundSurface)
                foundSurface = g_pCompositor->vectorToLayerSurface(newCoords, &g_pInputManager->m_exclusiveLSes, &surfaceCoords, &pFoundLayerSurface);

            if (!foundSurface) {
                foundSurface = (*g_pInputManager->m_exclusiveLSes.begin())->m_surface->resource();
                surfacePos   = (*g_pInputManager->m_exclusiveLSes.begin())->m_realPosition->goal();
            }
        }

        if (!foundSurface)
            foundSurface = g_pCompositor->vectorToLayerPopupSurface(newCoords, PMONITOR, &surfaceCoords, &pFoundLayerSurface);

        // overlays are above fullscreen
        if (!foundSurface)
            foundSurface = g_pCompositor->vectorToLayerSurface(newCoords, &PMONITOR->m_layerSurfaceLayers[3], &surfaceCoords, &pFoundLayerSurface);

        // also IME popups
        if (!foundSurface) {
            auto popup = g_pInputManager->m_relay.popupFromCoords(newCoords);
            if (popup) {
                foundSurface = popup->getSurface();
                surfacePos   = popup->globalBox().pos();
            }
        }

        // also top layers
        if (!foundSurface)
            foundSurface = g_pCompositor->vectorToLayerSurface(newCoords, &PMONITOR->m_layerSurfaceLayers[2], &surfaceCoords, &pFoundLayerSurface);

        if (!foundSurface && !g_pCompositor->m_lastWindow.expired() && RE2::FullMatch(g_pCompositor->m_lastWindow->m_class, *PCLASS)) {
            if (strstr(*PEDGE, "l") && local.x < 0)
                newCoords.x = **PMARGIN;
            if (strstr(*PEDGE, "t") && local.y < 0)
                newCoords.y = **PMARGIN;
            if (strstr(*PEDGE, "r") && local.x > g_pCompositor->m_lastWindow->m_realSize->goal().x)
                newCoords.x = g_pCompositor->m_lastWindow->m_realSize->goal().x - **PMARGIN;
            if (strstr(*PEDGE, "b") && local.y > g_pCompositor->m_lastWindow->m_realSize->goal().y)
                newCoords.y = g_pCompositor->m_lastWindow->m_realSize->goal().y - **PMARGIN;
        }
    }

    (*(origMotion)g_pMouseMotionHook->m_original)(thisptr, time_msec, newCoords);
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    /*const std::string HASH = __hyprland_api_get_hash();

  if (HASH != GIT_COMMIT_HASH)
  {
      HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Failure in
  initialization: Version mismatch (headers ver is not equal to running hyprland
  ver)", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000); throw
  std::runtime_error("[magic-mouse-gaps] Version mismatch");
  }*/

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:magic-mouse-gaps:margin", Hyprlang::INT{0});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:magic-mouse-gaps:class", Hyprlang::STRING{"firefox"});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:magic-mouse-gaps:edge", Hyprlang::STRING{"t"});

    auto FNS = HyprlandAPI::findFunctionsByName(PHANDLE, "sendPointerMotion");
    for (auto& fn : FNS) {
        if (!fn.demangled.contains("CSeatManager"))
            continue;

        g_pMouseMotionHook = HyprlandAPI::createFunctionHook(PHANDLE, fn.address, (void*)::hkNotifyMotion);
        break;
    }

    bool success = g_pMouseMotionHook;
    if (!success) {
        HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Hook init failed", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[magic-mouse-gaps] Hook init failed");
    }

    success = success && g_pMouseMotionHook->hook();

    if (!success) {
        HyprlandAPI::addNotification(PHANDLE, "[magic-mouse-gaps] Hook failed", CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[magic-mouse-gaps] Hook failed");
    }

    return {"magic-mouse-gaps",
            "A plugin to move mouse events from gaps to nearby matching window "
            "(use firefox tabs from gaps)",
            "Dregu", "1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    ;
}
