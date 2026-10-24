#pragma once
#include <QSettings>
#include <QPainter>
#include <QPointF>
#include <cmath>

// UI-only preference: project coordinates remain exact until the user performs a gesture.
struct SnapGrid {
    bool enabled = true;
    double spacing = 10;
    static SnapGrid load() {
        QSettings settings;
        const auto value=settings.value("editor/gridSpacing",10).toDouble();
        return {settings.value("editor/snapToGrid",true).toBool(),
                std::isfinite(value) && value>=1 && value<=100 ? value : 10};
    }
    void save() const {
        QSettings settings;
        settings.setValue("editor/snapToGrid",enabled);
        settings.setValue("editor/gridSpacing",spacing);
    }
    double snap(double value) const {
        if(!enabled || !std::isfinite(value) || !std::isfinite(spacing) || spacing<=0) return value;
        // Symmetric half-away-from-zero rounding also works outside the original positive canvas.
        const double result=std::round(value/spacing)*spacing;
        return std::isfinite(result) ? result : value;
    }
    QPointF snap(QPointF value) const { return {snap(value.x()),snap(value.y())}; }
    void paint(QPainter& painter,QRectF visible,double zoom) const {
        if(!enabled || !visible.isValid() || !std::isfinite(zoom) || zoom<=0 || !std::isfinite(spacing) || spacing<=0) return;
        // Hide intermediate lines at small zoom; snapping always uses the original world spacing.
        const double step=spacing*std::max(1.0,std::ceil(12/(spacing*zoom)));
        painter.save();painter.setPen(QPen(QColor(147,191,255,65),1/zoom));
        int lines=0; // Bound work even when huge imported coordinates cannot represent the next grid line.
        for(double x=std::ceil(visible.left()/step)*step;x<=visible.right() && lines++<2048;x+=step)
            painter.drawLine(QPointF(x,visible.top()),QPointF(x,visible.bottom()));
        lines=0;
        for(double y=std::ceil(visible.top()/step)*step;y<=visible.bottom() && lines++<2048;y+=step)
            painter.drawLine(QPointF(visible.left(),y),QPointF(visible.right(),y));
        painter.restore();
    }
};
