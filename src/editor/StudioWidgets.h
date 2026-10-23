#pragma once
#include "EffectRenderer.h"
#include <QWidget>
#include <QIcon>
#include <QColor>
#include <optional>
class QDoubleSpinBox;
class QVBoxLayout;
void applyStudioStyle(QWidget* window);
QWidget* sliderField(QDoubleSpinBox* value);
void syncSliderFields(QWidget* root);
QWidget* disclosure(QVBoxLayout* layout, const QString& title, QWidget* content);
std::optional<EffectKind> chooseEffect(QWidget* parent);
QIcon studioToolIcon(const QString& name);

class QLineEdit;
class QPushButton;
class ColorEditor : public QWidget {
    Q_OBJECT
public:
    explicit ColorEditor(QWidget* parent=nullptr);
    void setColor(QColor color);
    QColor color() const { return color_; }
signals:
    void colorChanged(QColor color);
private:
    QColor color_=Qt::white;
    QPushButton* picker_;
    QLineEdit* hex_;
};
class ColorListEditor : public QWidget {
    Q_OBJECT
public:
    ColorListEditor(int minimum,int maximum,QWidget* parent=nullptr);
    void setColors(const QVector<QColor>& colors);
    QVector<QColor> colors() const { return colors_; }
signals:
    void colorsChanged(const QVector<QColor>& colors);
private:
    void rebuild();
    QVector<QColor> colors_;
    int minimum_, maximum_;
    QWidget* rows_;
    QPushButton *add_, *remove_;
};
void installParameterTooltips(QWidget* root);
