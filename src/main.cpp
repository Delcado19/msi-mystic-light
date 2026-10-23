#include <QApplication>
#include <algorithm>
#include <stdexcept>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include "modules/ModuleRegistry.h"
#include "components/ComponentCatalog.h"
#include "discovery/HardwareDiscovery.h"
#include "startup/ConflictDialog.h"
#include "editor/LightingWindow.h"
#include "output/HardwareOutput.h"
#include <QDockWidget>
#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QStyle>
#include <QLockFile>
#include "editor/AppSession.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("Delcado");
    app.setApplicationName("MSI Mystic Light Standalone");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption({"module-dir", "Load trusted local driver modules from this directory.", "directory",
        QCoreApplication::applicationDirPath() + "/modules"});
    parser.addOption({"profile-dir", "Read device profiles from this directory.", "directory",
        QCoreApplication::applicationDirPath() + "/profiles"});
    parser.addOption({"component-dir", "Read data-only component definitions from this directory.", "directory",
        QCoreApplication::applicationDirPath() + "/components"});
    parser.addOption({"scan", "List HID endpoints and matching board profiles without opening devices."});
    parser.addOption({"inspect", "Validate local catalogs only; do not show the startup conflict dialog."});
    parser.addOption({"editor", "Open the editor without the startup conflict dialog; live output still requires review."});
    parser.process(app);
    if (!parser.isSet("inspect") && !parser.isSet("scan") && !parser.isSet("editor")) {
        SystemConflictProcesses processes;
        ConflictDialog conflicts(processes);
        if (conflicts.exec() != QDialog::Accepted) return 0;
    }
    ModuleRegistry modules;
    bool failed = false;
    for (const auto& error : modules.loadDirectory(parser.value("module-dir"))) {
        qWarning().noquote() << error;
        failed = true;
    }
    qInfo().noquote() << "Loaded driver modules:" << modules.moduleIds().join(", ");
    QVector<DeviceProfile> availableProfiles;
    const QDir profiles(parser.value("profile-dir"));
    if (!profiles.exists()) { qWarning() << "Profile directory does not exist"; failed = true; }
    for (const auto& file : profiles.entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
        try {
            const auto profile = DeviceProfile::load(file.absoluteFilePath());
            QString error;
            if (!modules.acceptsProfile(profile, &error)) { qWarning().noquote() << file.fileName() << error; failed = true; }
            else {
                if (std::any_of(availableProfiles.cbegin(), availableProfiles.cend(),
                    [&](const DeviceProfile& existing) { return existing.id == profile.id; }))
                    throw std::invalid_argument("Duplicate device profile ID");
                availableProfiles.append(profile);
                qInfo().noquote() << "Available profile:" << profile.name;
            }
        } catch (const std::exception& error) {
            qWarning().noquote() << file.fileName() << error.what();
            failed = true;
        }
    }
    ComponentCatalog catalog;
    try {
        catalog = loadComponentCatalog(parser.value("component-dir"));
        for (const auto& component : catalog)
            qInfo().noquote() << "Available component:" << component.id << component.name;
    } catch (const std::exception& error) {
        qWarning().noquote() << error.what();
        failed = true;
    }
    if (parser.isSet("scan") && !failed) {
        try {
            const auto board = readBoardIdentity();
            qInfo().noquote() << "System board:" << board.manufacturer << board.model;
            if (board.manufacturer.isEmpty() || board.model.isEmpty())
                qWarning() << "Board identity incomplete; required board selectors will not match.";
            const auto devices = HidApiDevice::enumerate();
            for (const auto& descriptor : devices) {
                qInfo().noquote() << QString("HID %1:%2 interface=%3 usagePage=%4 usage=%5")
                    .arg(descriptor.vendorId, 4, 16, QLatin1Char('0'))
                    .arg(descriptor.productId, 4, 16, QLatin1Char('0'))
                    .arg(descriptor.interfaceNumber).arg(descriptor.usagePage).arg(descriptor.usage)
                    << QString::fromStdWString(descriptor.product);
                qInfo().noquote() << "  Path:" << QString::fromUtf8(descriptor.path.c_str());
            }
            const auto candidates = findProfileCandidates(devices, board, availableProfiles);
            for (const auto& candidate : candidates)
                qInfo().noquote() << "Profile candidate:" << candidate.profileId << candidate.device.path;
            qInfo() << "Matching profile/endpoint pairs:" << candidates.size();
            if (candidates.size() > 1)
                qWarning() << "Multiple candidates; explicit endpoint/profile selection is required.";
        } catch (const std::exception& error) {
            qWarning().noquote() << error.what();
            failed = true;
        }
    }
    if (failed) return 1;
    if (parser.isSet("inspect") || parser.isSet("scan")) return 0;
    // --editor skips only the startup conflict dialog; every live output start has its own gate.
    const auto sessionDirectory=QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    if(!QDir().mkpath(sessionDirectory)) { QMessageBox::warning(nullptr,"Startup failed","Cannot create session directory"); return 1; }
    QLockFile instance(sessionDirectory+"/application.lock"); instance.setStaleLockTime(0);
    if(!instance.tryLock()) {
        if(instance.error()==QLockFile::LockFailedError) QMessageBox::information(nullptr,"Already running","Another instance owns the lighting session. Open it from the tray.");
        else QMessageBox::warning(nullptr,"Startup failed","Cannot lock the session directory");
        return 1;
    }
    QSettings settings;
    LightingWindow window(catalog, [availableProfiles] {
        return findProfileCandidates(HidApiDevice::enumerate(), readBoardIdentity(), availableProfiles);
    }, nullptr, availableProfiles);
    auto* outputDock=new QDockWidget("Hardware output",&window);
    outputDock->setObjectName("hardwareOutputDock");
    auto* outputPanel=new OutputPanel(window,[availableProfiles,directory=parser.value("module-dir")](QWidget* parent,const QVector<OutputFrame>& frames) {
        return authorizeHardwareOutput(parent,availableProfiles,frames,directory);
    },outputDock);
    outputDock->setWidget(outputPanel);
    window.addDockWidget(Qt::BottomDockWidgetArea,outputDock);
    outputDock->setVisible(false);
    auto* view=window.menuBar()->addMenu("View"); view->addAction(outputDock->toggleViewAction());
    QSystemTrayIcon tray(window.style()->standardIcon(QStyle::SP_ComputerIcon),&window);
    tray.setToolTip("MSI Mystic Light");
    auto* trayMenu=new QMenu(&window);
    auto show=[&window] { window.showNormal(); window.raise(); window.activateWindow(); };
    QObject::connect(trayMenu->addAction("Show editor"),&QAction::triggered,&window,show);
    QObject::connect(trayMenu->addAction("Stop output"),&QAction::triggered,outputPanel,&OutputPanel::stopOutput);
    QObject::connect(trayMenu->addAction("Quit"),&QAction::triggered,&window,&QWidget::close);
    tray.setContextMenu(trayMenu);
    QObject::connect(&tray,&QSystemTrayIcon::activated,&window,[show](QSystemTrayIcon::ActivationReason reason) { if(reason==QSystemTrayIcon::DoubleClick) show(); });
    if(QSystemTrayIcon::isSystemTrayAvailable()) {
        tray.show(); QObject::connect(view->addAction("Hide to tray"),&QAction::triggered,&window,&QWidget::hide);
    }
    AppSession session(window,settings,QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/recovery.lighting.json");
    session.restore([&window] {
        return QMessageBox::question(&window,"Recover unsaved project","Recover the unsaved project from the previous session?",
            QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes;
    });
    window.show();
    return app.exec();
}
