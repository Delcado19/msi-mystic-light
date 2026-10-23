#include "HardwareDiscovery.h"
#include <QFile>
#include <QSettings>

BoardIdentity readBoardIdentity()
{
#ifdef Q_OS_WIN
    const QSettings bios("HKEY_LOCAL_MACHINE\\HARDWARE\\DESCRIPTION\\System\\BIOS", QSettings::NativeFormat);
    return {bios.value("BaseBoardManufacturer").toString().trimmed(),
            bios.value("BaseBoardProduct").toString().trimmed()};
#elif defined(Q_OS_LINUX)
    const auto read = [](const QString& path) {
        QFile file(path);
        return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.read(4096)).trimmed() : QString{};
    };
    return {read("/sys/class/dmi/id/board_vendor"), read("/sys/class/dmi/id/board_name")};
#else
    return {}; // Missing identity must not be guessed from a USB manufacturer.
#endif
}

DeviceIdentity deviceIdentity(const HidDescriptor& descriptor, const BoardIdentity& board)
{
    DeviceIdentity result;
    result.path = QString::fromUtf8(descriptor.path.data(), static_cast<qsizetype>(descriptor.path.size()));
    result.vendorId = descriptor.vendorId;
    result.productId = descriptor.productId;
    result.usagePage = descriptor.usagePage;
    result.usage = descriptor.usage;
    result.interfaceNumber = descriptor.interfaceNumber;
    result.boardManufacturer = board.manufacturer;
    result.boardModel = board.model;
    return result;
}

QVector<ProfileCandidate> findProfileCandidates(const std::vector<HidDescriptor>& devices,
    const BoardIdentity& board, const QVector<DeviceProfile>& profiles)
{
    QVector<ProfileCandidate> result;
    for (const auto& descriptor : devices) {
        if (descriptor.path.empty()) continue;
        const auto identity = deviceIdentity(descriptor, board);
        for (const auto& profile : profiles)
            if (profile.matches(identity)) result.append({identity, profile.id});
    }
    return result;
}
