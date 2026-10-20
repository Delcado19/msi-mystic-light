#include "ui/TrayApp.h"

#include <QActionGroup>
#include <QApplication>
#include <QStyle>

TrayApp::TrayApp(QObject* parent)
    : QObject(parent)
{
    QAction* openStudioAction = menu_.addAction("Open Lighting Studio");
    connect(openStudioAction, &QAction::triggered, this, &TrayApp::openStudioRequested);

    profileMenu_ = menu_.addMenu("Profile");

    menu_.addSeparator();

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

void TrayApp::setAvailableProfiles(const std::vector<std::string>& profileNames, const std::string& activeProfileName)
{
    // Clear and rebuild rather than diffing against the previous contents:
    // callers (main.cpp) invoke this any time Settings.profiles changes, and
    // a stale/renamed/removed entry must not survive into the new list.
    //
    // Actions are owned by an exclusive QActionGroup rather than being plain
    // checkable QActions: with no group, checking one entry does not
    // uncheck the others, so clicking a non-active profile would leave two
    // entries checked. The old group is deleted via deleteLater(), not
    // immediately: the expected caller wiring is profileSelected -> switch
    // profile -> setAvailableProfiles, which re-enters this function from
    // inside a QAction::triggered handler owned by the very group being
    // replaced; deleting it synchronously would destroy that action while
    // QMenu's activation path is still using it.
    profileMenu_->clear();
    if (profileGroup_) profileGroup_->deleteLater();
    profileGroup_ = new QActionGroup(this);
    profileGroup_->setExclusive(true);
    for (const std::string& name : profileNames) {
        QAction* action = profileGroup_->addAction(QString::fromStdString(name));
        action->setCheckable(true);
        action->setChecked(name == activeProfileName);
        connect(action, &QAction::triggered, this, [this, name]() { emit profileSelected(name); });
    }
    profileMenu_->addActions(profileGroup_->actions());
}
