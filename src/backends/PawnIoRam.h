// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "EneRamProtocol.h"
#include <memory>

class PawnIoRam {
public:
    PawnIoRam();
    ~PawnIoRam();
    PawnIoRam(const PawnIoRam&)=delete;
    PawnIoRam& operator=(const PawnIoRam&)=delete;
    static bool runtimeAvailable(QString* error=nullptr);
    bool open(int address);
    bool pushColors(const QVector<RgbColor>& colors);
    QString error() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
// Active probing selects ENE read registers, but never changes colors or remaps devices.
// Passive callers only obtain identities verified by an earlier GUI scan.
QVector<int> scanPawnIoRam(bool activeProbe);
QString pawnIoRamDiscoveryError();
