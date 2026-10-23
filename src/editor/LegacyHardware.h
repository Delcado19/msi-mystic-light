#pragma once
#include "LightingDocument.h"
#include "discovery/HardwareDiscovery.h"
// Import only saved geometry/wire order, binding it to freshly discovered exact endpoints.
// Original settings and effect recipes remain untouched; this never starts output.
QStringList importLegacyHardware(LightingDocument& document,const QString& file,const QVector<ProfileCandidate>& candidates);
