#include "ui/TrayApp.h"

#include <QApplication>
#include <QStyle>

TrayApp::TrayApp(QObject* parent)
    : QObject(parent)
{
    QAction* quitAction = menu_.addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &TrayApp::quitRequested);
    trayIcon_.setContextMenu(&menu_);
    // A real icon asset is a later concern; a standard platform icon is
    // enough for v1 so the tray entry is actually visible/clickable
    // (QSystemTrayIcon::show() with no icon set shows nothing on Windows).
    trayIcon_.setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
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
