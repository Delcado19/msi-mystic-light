#pragma once
#include "EffectRenderer.h"
#include <QWidget>
#include <QIcon>
#include <optional>
class QDoubleSpinBox;
class QVBoxLayout;
void applyStudioStyle(QWidget* window);
QWidget* sliderField(QDoubleSpinBox* value);
void syncSliderFields(QWidget* root);
QWidget* disclosure(QVBoxLayout* layout, const QString& title, QWidget* content);
std::optional<EffectKind> chooseEffect(QWidget* parent);
QIcon studioToolIcon(const QString& name);
