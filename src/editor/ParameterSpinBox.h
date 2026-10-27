#pragma once
#include <QDoubleSpinBox>
#include <cmath>
#include <QEvent>
#include <algorithm>

// Integer UI units never quantize the document on refresh. The approximation mark
// keeps off-grid legacy values intact when a field merely gains or loses focus.
class ParameterSpinBox : public QDoubleSpinBox {
public:
    explicit ParameterSpinBox(QWidget* parent=nullptr) : QDoubleSpinBox(parent) {
        setDecimals(323); setKeyboardTracking(false); setMinimumWidth(88);
        setProperty("displayScale",1.0);
    }
    void setDisplayUnit(double scale,const QString& unit) {
        scale_=scale; setProperty("displayScale",scale); setSuffix(unit);
        setSingleStep(1/scale); setValue(value());
    }
protected:
    bool event(QEvent* event) override {
        if(event->type()==QEvent::ToolTip) {
            if(!property("baseDescription").isValid())setProperty("baseDescription",toolTip());
            setToolTip(property("baseDescription").toString()+"\nExact value: "+QString::number(value()*scale_,'g',17)+suffix()+". An approximation mark means the saved value is preserved until edited. Alt+R resets the default.");
        }
        return QDoubleSpinBox::event(event);
    }
    QString textFromValue(double number) const override {
        const double shown=number*scale_;
        return (std::abs(shown-std::round(shown))>1e-7 ? QString::fromUtf8("\xE2\x89\x88 ") : QString{})+QString::number(qRound64(shown));
    }
    double valueFromText(const QString& text) const override {
        if(text.contains(QChar(0x2248)))return value();
        QString number=text;number.remove(suffix());number.remove(prefix());
        bool ok=false;const double result=locale().toDouble(number.trimmed(),&ok);
        return ok ? result/scale_ : value();
    }
    QValidator::State validate(QString& text,int&) const override {
        if(text.contains(QChar(0x2248)))return QValidator::Acceptable;
        QString number=text;number.remove(suffix());number.remove(prefix());number=number.trimmed();
        if(number.isEmpty() || number=="-" || number=="+")return QValidator::Intermediate;
        bool ok=false;const auto result=locale().toLongLong(number,&ok);
        if(!ok)return QValidator::Invalid;
        return result/scale_>=minimum() && result/scale_<=maximum() ? QValidator::Acceptable : QValidator::Intermediate;
    }
    void stepBy(int steps) override {
        setValue(std::clamp((std::round(value()*scale_)+steps)/scale_,minimum(),maximum()));
    }
private:
    double scale_=1;
};
