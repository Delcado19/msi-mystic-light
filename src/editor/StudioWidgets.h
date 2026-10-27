#pragma once
#include "EffectRenderer.h"
#include <QWidget>
#include <QIcon>
#include <QColor>
class QDoubleSpinBox;
class QVBoxLayout;
class QSlider;
void applyStudioStyle(QWidget* window);
QWidget* studioBanner(QWidget* parent,int height);
QWidget* sliderField(QDoubleSpinBox* value);
void syncSliderFields(QWidget* root);
QWidget* disclosure(QVBoxLayout* layout, const QString& title, QWidget* content);
bool studioMotionEnabled();
QIcon studioToolIcon(const QString& name);

class QLabel;
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
    void gestureStarted();
    void gestureFinished(bool accept);
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
    QVector<double> positions() const { return positions_; }
    void setStops(const QVector<QColor>& colors,const QVector<double>& positions);
    void setGradientMode() { gradient_=true; }
    void addStop(double position);
    void removeStop();
signals:
    void colorsChanged(const QVector<QColor>& colors);
    void stopsChanged();
    void gestureStarted();
    void gestureFinished();
private:
    void rebuild();
    QVector<QColor> colors_;
    int minimum_, maximum_;
    QVector<double> positions_;
    int selected_=0;
    bool gradient_=false;
    QWidget* rows_;
    ColorEditor* selectedColor_;
    QVector<QSlider*> rgb_;
    QVector<QLabel*> rgbValues_;
    QPushButton *add_, *remove_;
};
void installParameterTooltips(QWidget* root);

void openStudioColorPicker(ColorEditor* editor,bool mouseInitiated);
