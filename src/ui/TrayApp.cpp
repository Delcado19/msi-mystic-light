#include "ui/TrayApp.h"

TrayApp::TrayApp(QObject* parent)
    : QObject(parent)
{
    QAction* quitAction = menu_.addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &TrayApp::quitRequested);
    trayIcon_.setContextMenu(&menu_);
}

bool TrayApp::start()
{
    // No system tray available (e.g. headless environment) — nothing to show.
    if (!QSystemTrayIcon::isSystemTrayAvailable()) return false;
    trayIcon_.show();
    return true;
}

QMenu* TrayApp::contextMenu() const
{
    return const_cast<QMenu*>(&menu_);
}
