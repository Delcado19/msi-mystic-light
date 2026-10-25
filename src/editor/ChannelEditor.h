#pragma once
#include "LightingDocument.h"
#include <QDialog>

// Stages configuration only; accepting never touches a transport.
class ChannelEditor : public QDialog {
public:
    ChannelEditor(const LightingDocument& document, ChannelAssignment initial, QWidget* parent = nullptr);
    ChannelAssignment assignment() const;
private:
    ChannelAssignment initial_;
    bool fixedComponent_=false;
};
