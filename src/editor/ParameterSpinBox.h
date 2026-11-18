#pragma once
#include <QDoubleSpinBox>
#include <QLineEdit>
#include "StudioWidgets.h"
#include <cmath>
#include <QEvent>
#include <algorithm>

// Integer UI units never quantize the document on refresh. Text that still equals the
// displayed rounding of the stored value maps back to that exact value, so off-grid legacy
// values stay intact when a field merely gains or loses focus; only a typed change rounds.
class ParameterSpinBox : public QDoubleSpinBox {
public:
    explicit ParameterSpinBox(QWidget* parent=nullptr) : QDoubleSpinBox(parent) {
        setDecimals(323); setKeyboardTracking(false); styleStudioNumericField(this);
        setProperty("displayScale",1.0);
        // Only untouched text keeps the exact value; clearing and retyping the shown integer is an
        // explicit edit and rounds (CodeRabbit review 2026-11-16). The typed text itself is kept, not a
        // flag: a refresh under QSignalBlocker replaces the text without any signal, and then the
        // field no longer shows what was typed (CodeRabbit review 2026-11-17).
        // The value at typing time detects a later refresh even when it shows the same text: keyboard
        // tracking is off, so only a refresh changes value() while text is typed (CodeRabbit review 2026-11-18).
        connect(lineEdit(),&QLineEdit::textEdited,this,[this](const QString& text){typed_=text;typedAt_=value();});
        connect(this,&QAbstractSpinBox::editingFinished,this,[this]{typed_.clear();});
    }
    void setDisplayUnit(double scale,const QString& unit) {
        scale_=scale; setProperty("displayScale",scale); setSuffix(unit);
        setSingleStep(1/scale); setValue(value());
    }
protected:
    bool event(QEvent* event) override {
        if(event->type()==QEvent::ToolTip) {
            if(!property("baseDescription").isValid())setProperty("baseDescription",toolTip());
            setToolTip(property("baseDescription").toString()+"\nExact value: "+QString::number(value()*scale_,'g',17)+suffix()+". The saved value is kept until you type a new one. Alt+R resets the default.");
        }
        return QDoubleSpinBox::event(event);
    }
    QString textFromValue(double number) const override {
        const double shown=number*scale_;
        return QString::number(qRound64(shown));
    }
    double valueFromText(const QString& text) const override {
        if((typed_.isEmpty() || text!=typed_ || value()!=typedAt_) && (text==textFromValue(value())+suffix() || text==prefix()+textFromValue(value())+suffix()))return value();
        QString number=text;number.remove(suffix());number.remove(prefix());
        bool ok=false;const double result=locale().toDouble(number.trimmed(),&ok);
        return ok ? result/scale_ : value();
    }
    QValidator::State validate(QString& text,int&) const override {
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
    QString typed_; // The field's text as last typed by the user, until editing finishes.
    double typedAt_=0; // value() when typed_ was set.
};
