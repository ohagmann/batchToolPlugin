#include "windows_fix.h"  // CRITICAL: Qt 6.8+ fix - MUST be FIRST
/**
 * @file BatchToolPlugin.cpp
 * @brief BRX-Einstiegspunkt: Befehle BATCHTOOL und BT, Hauptfenster oeffnen
 */

// WICHTIG: Platform header MUSS zuerst kommen!
#ifdef __linux__
#include "brx_platform_linux.h"
#else
#include "brx_platform_windows.h"
#endif

// Dann erst die anderen BRX headers
#include "aced.h"
#include "AcRx/AcRxDynamicLinker.h"
#include "BatchToolPlugin.h"
#include "../ui/MainWindow.h"

#include <QApplication>

#ifndef PLUGIN_VERSION
#define PLUGIN_VERSION "unbekannt"
#endif

namespace {

bool g_isInitialized = false;
QApplication* g_qApp = nullptr;
BatchProcessing::MainWindow* g_mainWindow = nullptr;

/// Befehl BATCHTOOL bzw. BT: Fenster anlegen oder nach vorn holen
void batchToolCommand() {
    if (!g_isInitialized) {
        acutPrintf(_T("\nbatchTool: Plugin nicht korrekt initialisiert.\n"));
        return;
    }
    if (!g_mainWindow) {
        g_mainWindow = new BatchProcessing::MainWindow();
    }
    // Thema bei jedem Oeffnen neu anwenden: schaltet der Anwender BricsCAD
    // zwischenzeitlich zwischen hell und dunkel um, folgt das Fenster.
    g_mainWindow->applyTheme();
    g_mainWindow->show();
    g_mainWindow->raise();
    g_mainWindow->activateWindow();
    if (g_qApp) g_qApp->processEvents();
}

void registerCommands() {
    // Langform und Kurzform fuehren auf denselben Befehl. Die Kurzform ist
    // ein echter Befehl und braucht keinen Eintrag in der default.pgp.
    acedRegCmds->addCommand(_T("BATCHTOOL_CMDS"), _T("BATCHTOOL"), _T("BATCHTOOL"),
                            ACRX_CMD_MODAL, batchToolCommand);
    acedRegCmds->addCommand(_T("BATCHTOOL_CMDS"), _T("BT"), _T("BT"),
                            ACRX_CMD_MODAL, batchToolCommand);
}

void unregisterCommands() {
    acedRegCmds->removeGroup(_T("BATCHTOOL_CMDS"));
}

}  // namespace

// ============================================================================
// BRX Entry Point
// ============================================================================

extern "C" BRX_EXPORT
AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* pAppId)
{
    switch (msg) {
        case AcRx::kInitAppMsg:
            acrxDynamicLinker->unlockApplication(pAppId);
            acrxDynamicLinker->registerAppMDIAware(pAppId);

            if (!g_qApp) {
                if (QCoreApplication::instance()) {
                    // Der Prozess hat schon eine Qt-Anwendung: BricsCAD selbst
                    // (Linux) oder ein zuvor geladenes Qt-Plugin wie openCirt.
                    // Eine zweite Instanz ist nicht zulaessig, die vorhandene
                    // wird mitbenutzt.
                    g_qApp = qobject_cast<QApplication*>(QCoreApplication::instance());
                    if (!g_qApp) {
                        acutPrintf(_T("\nbatchTool: vorhandene Qt-Instanz ist keine QApplication - Oberflaeche nicht verfuegbar.\n"));
                        return AcRx::kRetError;
                    }
                } else {
                    static int argc = 1;
                    static char* argv[] = { (char*)"batchTool", nullptr };
                    g_qApp = new QApplication(argc, argv);
                    g_qApp->setApplicationName("BatchProcessing");
                    g_qApp->setOrganizationName("openCirt");
                }
            }

            registerCommands();
            acutPrintf(_T("\nbatchTool %hs geladen. Befehl: BATCHTOOL (Kurzform BT)\n"), PLUGIN_VERSION);
            g_isInitialized = true;
            break;

        case AcRx::kUnloadAppMsg:
            if (g_mainWindow) {
                g_mainWindow->close();
                delete g_mainWindow;
                g_mainWindow = nullptr;
            }
            unregisterCommands();
            // Die QApplication bleibt stehen, auch wenn dieses Plugin sie
            // angelegt hat (Windows): ein zweites Qt-Plugin (openCirt) kann
            // sie weiter benutzen, und BricsCAD raeumt beim Beenden auf.
            g_qApp = nullptr;
            g_isInitialized = false;
            acutPrintf(_T("\nbatchTool entladen.\n"));
            break;

        default:
            break;
    }
    return AcRx::kRetOK;
}

// acrxGetApiVersion wird von drx_entrypoint bereitgestellt - hier NICHT definieren.
