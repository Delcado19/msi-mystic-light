#include "EffectRenderer.h"
#include "LightingDocument.h"
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace {
constexpr double pi = 3.14159265358979323846;
QPainterPath polygon(int vertices, double inner = 0)
{
    QPainterPath path;
    const int count = inner > 0 ? vertices * 2 : vertices;
    for (int i = 0; i < count; ++i) {
        const double angle = 2 * pi * i / count - pi / 2;
        const double radius = inner > 0 && i % 2 ? inner : 0.5;
        const QPointF point(radius * std::cos(angle), radius * std::sin(angle));
        if (i == 0) path.moveTo(point); else path.lineTo(point);
    }
    path.closeSubpath();
    return path;
}
QRgb interpolate(QRgb from, QRgb to, double t)
{
    // Interpolate premultiplied channels, avoiding color halos at transparent stops.
    return qRgba(qRound(qRed(from) + (qRed(to) - qRed(from)) * t),
        qRound(qGreen(from) + (qGreen(to) - qGreen(from)) * t),
        qRound(qBlue(from) + (qBlue(to) - qBlue(from)) * t),
        qRound(qAlpha(from) + (qAlpha(to) - qAlpha(from)) * t));
}
}

const QVector<EffectDefinition>& effectDefinitions()
{
    static const QVector<EffectDefinition> definitions{
        {EffectKind::Paint, "paint", "Paint layer"},
        {EffectKind::StaticColor, "static", "Static Color"},
        {EffectKind::LinearGradient, "linear", "Linear Gradient"},
        {EffectKind::RadialGradient, "radial", "Radial Gradient"},
        {EffectKind::ConicalGradient, "conical", "Conical Gradient"},
        {EffectKind::Stripes, "stripes", "Stripes"}, {EffectKind::Waves, "waves", "Waves"},
        {EffectKind::Rectangle, "rectangle", "Rectangle"}, {EffectKind::Circle, "circle", "Circle"}, {EffectKind::Ring, "ring", "Ring"},
        {EffectKind::Star, "star", "Star"}, {EffectKind::Heart, "heart", "Heart"},
        {EffectKind::Triangle, "triangle", "Triangle"}, {EffectKind::Hexagon, "hexagon", "Hexagon"},
        {EffectKind::Diamond, "diamond", "Diamond"}, {EffectKind::Cross, "cross", "Cross"},
        {EffectKind::Moon, "moon", "Moon"}, {EffectKind::Image, "image", "Image"},
        {EffectKind::Rain, "rain", "Particle Rain"}, {EffectKind::Orbit, "orbit", "Particle Orbit"},
        {EffectKind::Fireflies, "fireflies", "Fireflies"}, {EffectKind::Burst, "burst", "Particle Burst"}
    };
    return definitions;
}
const EffectDefinition& effectDefinition(EffectKind kind)
{
    for (const auto& definition : effectDefinitions()) if (definition.kind == kind) return definition;
    throw std::invalid_argument("Unknown effect kind");
}
const EffectDefinition& effectDefinition(const QString& id)
{
    for (const auto& definition : effectDefinitions()) if (definition.id == id) return definition;
    throw std::invalid_argument("Unknown effect type");
}
bool isShape(EffectKind kind) { return (kind>=EffectKind::Circle && kind<=EffectKind::Moon) || kind==EffectKind::Rectangle; }

bool isGradient(EffectKind kind) { return kind >= EffectKind::LinearGradient && kind <= EffectKind::Waves; }

bool isParticle(EffectKind kind) { return kind >= EffectKind::Rain && kind <= EffectKind::Burst; }
const QVector<EffectParameter>& advancedParameters()
{
    static const QVector<EffectParameter> parameters{
        {"driftX", "Horizontal speed", &EffectLayer::driftX, -500, 500},
        {"driftY", "Vertical speed", &EffectLayer::driftY, -500, 500},
        {"zoom", "Size change %", &EffectLayer::zoom, 0, 90},
        {"warp", "Wave distortion", &EffectLayer::warp, 0, 100},
        {"breathe", "Breathe rate", &EffectLayer::breathe, 0, 10},
        {"trails", "Trail duration", &EffectLayer::trails, 0, 2},
        {"cycleRate", "Palette loops / sec", &EffectLayer::cycleRate, 0, 10},
        {"particleCount", "Particle count", &EffectLayer::particleCount, 1, 400},
        {"particleSeed", "Random pattern", &EffectLayer::particleSeed, 0, 1000000},
        {"particleSpeed", "Movement speed", &EffectLayer::particleSpeed, 0, 500},
        {"particleSize", "Particle radius", &EffectLayer::particleSize, 1, 40}
    };
    return parameters;
}

void validateEffect(const EffectLayer& layer)
{
    effectDefinition(layer.effect);
    if(layer.gradientFill) {
        if(!isShape(layer.effect)) throw std::invalid_argument("Gradient fill requires a shape");
        for(const auto point:{layer.gradientStart,layer.gradientEnd})
            if(!std::isfinite(point.x()) || !std::isfinite(point.y()) || std::abs(point.x())>20000 || std::abs(point.y())>20000) throw std::invalid_argument("Invalid gradient fill endpoint");
    }
    if(layer.effect==EffectKind::Paint && !layer.image.isNull() && layer.image.size()!=LightingDocument::canvasSize())
        throw std::invalid_argument("Paint sources must match the canvas size");
    if(!layer.gradientColors.isEmpty() && (layer.gradientColors.size()<2 || layer.gradientColors.size()>16))
        throw std::invalid_argument("Gradients require 2..16 colors");
    if(!layer.gradientPositions.isEmpty()) {
        if(layer.gradientPositions.size()!=(layer.gradientColors.isEmpty() ? 2 : layer.gradientColors.size())) throw std::invalid_argument("Gradient stop count mismatch");
        double before=-1;
        for(double position:layer.gradientPositions) {
            if(!std::isfinite(position) || position<0 || position>1 || position<=before) throw std::invalid_argument("Gradient stops must increase within 0..1");
            before=position;
        }
    }
    for(const auto& color:layer.gradientColors) if(!color.isValid()) throw std::invalid_argument("Invalid gradient color");
    for (const auto& parameter : advancedParameters()) {
        const double value = layer.*(parameter.member);
        if (!std::isfinite(value) || value < parameter.minimum || value > parameter.maximum)
            throw std::invalid_argument("Invalid advanced effect parameter");
    }
    if (std::floor(layer.particleCount) != layer.particleCount || std::floor(layer.particleSeed) != layer.particleSeed ||
        layer.colorCycle.size() < 2 || layer.colorCycle.size() > 16)
        throw std::invalid_argument("Invalid particle count/seed or color cycle");
    for (const auto& color : layer.colorCycle) if (!color.isValid()) throw std::invalid_argument("Invalid cycle color");
    const auto crop = layer.crop;
    if (!std::isfinite(crop.x()) || !std::isfinite(crop.y()) || !std::isfinite(crop.width()) || !std::isfinite(crop.height()) ||
        crop.x() < 0 || crop.y() < 0 || crop.width() < 0.001 || crop.height() < 0.001 ||
        crop.right() > 1.0000001 || crop.bottom() > 1.0000001)
        throw std::invalid_argument("Invalid normalized image crop");
    if (!std::isfinite(layer.spin) || std::abs(layer.spin) > 360 ||
        !std::isfinite(layer.pulse) || layer.pulse < 0 || layer.pulse > 10 ||
        !std::isfinite(layer.hueShift) || std::abs(layer.hueShift) > 360)
        throw std::invalid_argument("Invalid animation parameters");
    if (!layer.image.isNull() && (layer.image.width() > 4096 || layer.image.height() > 4096))
        throw std::invalid_argument("Image exceeds 4096 pixels per side");
    if (!layer.color.isValid() || !layer.secondaryColor.isValid() ||
        !std::isfinite(layer.center.x()) || !std::isfinite(layer.center.y()) ||
        std::abs(layer.center.x()) > 10000 || std::abs(layer.center.y()) > 10000 ||
        !std::isfinite(layer.extent.width()) || !std::isfinite(layer.extent.height()) ||
        layer.extent.width() < 1 || layer.extent.width() > 10000 ||
        layer.extent.height() < 1 || layer.extent.height() > 10000 ||
        !std::isfinite(layer.rotation) || layer.rotation < -360 || layer.rotation > 360 ||
        layer.repetitions < 1 || layer.repetitions > 64)
        throw std::invalid_argument("Invalid effect parameters");
}

static QImage renderClippedBaseEffect(const EffectLayer& layer, double seconds)
{
    QImage image(LightingDocument::canvasSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    if (layer.effect == EffectKind::StaticColor) { image.fill(layer.color); return image; }
    if (layer.effect == EffectKind::Image || layer.effect == EffectKind::Paint) {
        if (layer.image.isNull()) return image;
        QPainter painter(&image);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.translate(layer.center);
        painter.rotate(layer.rotation);
        const QRectF frame(-layer.extent.width()/2, -layer.extent.height()/2,
            layer.extent.width(), layer.extent.height());
        painter.setClipRect(frame);
        const QRectF source(layer.crop.x()*layer.image.width(), layer.crop.y()*layer.image.height(),
            layer.crop.width()*layer.image.width(), layer.crop.height()*layer.image.height());
        const QSizeF size = source.size().scaled(layer.extent,
            layer.imageFill ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio);
        painter.drawImage(QRectF(QPointF(-size.width()/2, -size.height()/2), size), layer.image, source);
        return image;
    }
    if (isGradient(layer.effect)) {
        const double angle = layer.rotation * pi / 180;
        const double cosine = std::cos(angle), sine = std::sin(angle);
        const auto colors=layer.gradientColors.isEmpty() ? QVector<QColor>{layer.color,layer.secondaryColor} : layer.gradientColors;
        QVector<QRgb> stops; for(const auto& color:colors) stops.append(qPremultiply(color.rgba()));
        const bool travelling=layer.driftX!=0 || layer.driftY!=0;
        auto positions=layer.gradientPositions;
        if(travelling && (layer.effect==EffectKind::LinearGradient || layer.effect==EffectKind::RadialGradient)) {
            // Like the original paletteAt: reserve a closing segment from the last color back to the first.
            if(positions.isEmpty()) for(int i=0;i<stops.size();++i) positions.append(i/double(stops.size()-1));
            const double span=(stops.size()-1)/double(stops.size());
            for(auto& position:positions) position*=span;
            positions.append(1); stops.append(stops.first());
        }

        for (int y = 0; y < image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                // Clamp stationary gradients at their endpoints; wrapping local coordinates creates an internal seam.
                const double dx = x+0.5-layer.center.x()-layer.driftX*seconds, dy = y+0.5-layer.center.y()-layer.driftY*seconds;
                const double u = (dx * cosine + dy * sine) / layer.extent.width();
                const double v = (-dx * sine + dy * cosine) / layer.extent.height();
                double t = u + 0.5;
                switch (layer.effect) {
                case EffectKind::RadialGradient: t = 2 * std::hypot(u, v); break;
                case EffectKind::ConicalGradient:
                    t = std::atan2(v, u) / (2 * pi); if (t < 0) t += 1; break;
                case EffectKind::Stripes: {
                    const double phase = t * layer.repetitions;
                    t = std::min(int((phase-std::floor(phase))*stops.size()),int(stops.size()-1))/double(stops.size()-1); break;
                }
                case EffectKind::Waves: t = (1 - std::cos(2 * pi * t * layer.repetitions)) / 2; break;
                default: break;
                }
                if(travelling && (layer.effect==EffectKind::LinearGradient || layer.effect==EffectKind::RadialGradient)) t-=std::floor(t);
                if(positions.isEmpty()) {
                    const double position=std::clamp(t,0.0,1.0)*(stops.size()-1);
                    const int stop=std::min(int(position),int(stops.size()-2));
                    row[x]=interpolate(stops[stop],stops[stop+1],position-stop);
                } else {
                    int right=int(std::upper_bound(positions.begin(),positions.end(),t)-positions.begin());
                    if(right==0) row[x]=stops.first();
                    else if(right==stops.size()) row[x]=stops.last();
                    else row[x]=interpolate(stops[right-1],stops[right],(t-positions[right-1])/(positions[right]-positions[right-1]));
                }
            }
        }
        return image;
    }
    if (isParticle(layer.effect)) {
        QPainter painter(&image); painter.setRenderHint(QPainter::Antialiasing);
        QTransform placement; placement.translate(layer.center.x(),layer.center.y()); placement.rotate(layer.rotation); placement.scale(layer.extent.width(),layer.extent.height());
        painter.setTransform(placement); painter.setPen(Qt::NoPen);
        QRandomGenerator random(static_cast<quint32>(layer.particleSeed));
        const auto unit = [&] { return random.generateDouble(); };
        const auto wrap = [](double value) { return value - std::floor(value); };
        for (int i = 0; i < static_cast<int>(layer.particleCount); ++i) {
            const double a = unit(), b = unit(), c = unit();
            const double phase = seconds * layer.particleSpeed / 200;
            QPointF point;
            switch (layer.effect) {
            case EffectKind::Rain: point = {a - 0.5, wrap(b + phase) - 0.5}; break;
            case EffectKind::Orbit: {
                const double angle = 2*pi*(a + phase*(0.2 + b));
                point = {0.45*std::sqrt(c)*std::cos(angle), 0.45*std::sqrt(c)*std::sin(angle)}; break;
            }
            case EffectKind::Fireflies:
                point = {0.35*std::sin(2*pi*(a + phase*(0.1+c))), 0.35*std::sin(2*pi*(b + phase*0.17))}; break;
            default: {
                const double age = wrap(b + phase), angle = 2*pi*a;
                point = {0.5*age*std::cos(angle), 0.5*age*std::sin(angle)}; break;
            }
            }
            painter.setBrush(QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(layer.color.rgba()),
                qPremultiply(layer.secondaryColor.rgba()), c))));
            painter.drawEllipse(point, layer.particleSize/layer.extent.width(), layer.particleSize/layer.extent.height());
        }
        return image;
    }
    QPainterPath path;
    switch (layer.effect) {
    case EffectKind::Rectangle: path.addRect(QRectF(-0.5,-0.5,1,1)); break;
    case EffectKind::Circle: path.addEllipse(QRectF(-0.5, -0.5, 1, 1)); break;
    case EffectKind::Ring:
        path.addEllipse(QRectF(-0.5, -0.5, 1, 1));
        path.addEllipse(QRectF(-0.325, -0.325, 0.65, 0.65)); break;
    case EffectKind::Star: path = polygon(5, 0.22); break;
    case EffectKind::Heart:
        path.moveTo(0, 0.5);
        path.cubicTo(-0.9, -0.1, -0.35, -0.8, 0, -0.25);
        path.cubicTo(0.35, -0.8, 0.9, -0.1, 0, 0.5); path.closeSubpath(); break;
    case EffectKind::Triangle: path = polygon(3); break;
    case EffectKind::Hexagon: path = polygon(6); break;
    case EffectKind::Diamond: path = polygon(4); break;
    case EffectKind::Cross: {
        QPainterPath horizontal, vertical;
        horizontal.addRect(-0.5, -0.15, 1, 0.3); vertical.addRect(-0.15, -0.5, 0.3, 1);
        path = horizontal.united(vertical); break;
    }
    case EffectKind::Moon: {
        QPainterPath disc, cutout;
        disc.addEllipse(QRectF(-0.5, -0.5, 1, 1)); cutout.addEllipse(QRectF(-0.15, -0.6, 1, 1));
        path = disc.subtracted(cutout); break;
    }
    default: throw std::invalid_argument("Effect is not a shape");
    }
    QTransform transform;
    transform.translate(layer.center.x(), layer.center.y());
    transform.rotate(layer.rotation);
    transform.scale(layer.extent.width(), layer.extent.height());
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    if(layer.gradientFill) {
        QLinearGradient gradient(transform.map(layer.gradientStart-QPointF(.5,.5)),transform.map(layer.gradientEnd-QPointF(.5,.5)));
        const auto colors=layer.gradientColors.isEmpty() ? QVector<QColor>{layer.color,layer.secondaryColor} : layer.gradientColors;
        for(int i=0;i<colors.size();++i) gradient.setColorAt(layer.gradientPositions.value(i,i/double(colors.size()-1)),colors[i]);
        painter.fillPath(transform.map(path),gradient);
    } else painter.fillPath(transform.map(path), layer.color);
    return image;
}

static double periodic(double value,double size) { return value-size*std::floor(value/size); }
static QImage renderBaseEffect(const EffectLayer& source,double seconds)
{
    auto layer=source; layer.center={periodic(layer.center.x(),640),periodic(layer.center.y(),400)};
    if(layer.effect==EffectKind::StaticColor || isGradient(layer.effect)) return renderClippedBaseEffect(layer,seconds);
    QImage result(LightingDocument::canvasSize(),QImage::Format_ARGB32_Premultiplied); result.fill(Qt::transparent);
    QTransform geometry; geometry.translate(layer.center.x(),layer.center.y()); geometry.rotate(layer.rotation); geometry.scale(layer.extent.width(),layer.extent.height());
    const auto bounds=geometry.mapRect(QRectF(-0.5,-0.5,1,1)).adjusted(-layer.particleSize,-layer.particleSize,layer.particleSize,layer.particleSize);
    QPainter painter(&result);
    for(int y=-1;y<=1;++y) for(int x=-1;x<=1;++x) {
        const QPointF offset(x*640,y*400);
        if(!bounds.translated(offset).intersects(QRectF(result.rect()))) continue;
        auto tile=layer; tile.center+=offset; painter.drawImage(0,0,renderClippedBaseEffect(tile,seconds));
    }
    return result;
}

QTransform effectTransform(const EffectLayer& layer, double seconds)
{
    const double scale = 1 + layer.zoom/100 * std::sin(2*pi*seconds);
    QTransform transform;
    transform.translate(periodic(layer.center.x() + layer.driftX*seconds,640),periodic(layer.center.y() + layer.driftY*seconds,400));
    transform.rotate(std::remainder(layer.rotation + layer.spin*seconds, 360.0));
    transform.scale(layer.extent.width()*scale, layer.extent.height()*scale);
    return transform;
}
QImage transformedMask(const EffectLayer& layer,const QTransform& transform)
{
    QImage image(LightingDocument::canvasSize(),QImage::Format_Grayscale8); image.fill(0);
    bool invertible=false; const auto inverse=transform.inverted(&invertible); if(!invertible) return image;
    // Inverse sampling wraps before clipping and avoids overlapping black mask tiles erasing one another.
    for(int y=0;y<image.height();++y) {
        auto* row=image.scanLine(y);
        for(int x=0;x<image.width();++x) {
            const auto point=inverse.map(QPointF(x+0.5,y+0.5))-QPointF(0.5,0.5);
            const double sx=periodic(point.x(),640),sy=periodic(point.y(),400);
            const int left=int(sx),top=int(sy),right=(left+1)%640,bottom=(top+1)%400;
            const double fx=sx-left,fy=sy-top;
            const auto* first=layer.mask.constScanLine(top); const auto* second=layer.mask.constScanLine(bottom);
            row[x]=uchar(qRound((first[left]*(1-fx)+first[right]*fx)*(1-fy)+(second[left]*(1-fx)+second[right]*fx)*fy));
        }
    }
    return image;
}

static QImage warpRaster(const QImage& source, double amplitude, double seconds, QColor background)
{
    QImage result(source.size(),QImage::Format_ARGB32_Premultiplied); result.fill(background);
    QPainter painter(&result);
    for (int y=0;y<result.height();++y) {
        const int shift=qRound(amplitude*std::sin(2*pi*(y/100.0+seconds)));
        for(int tile=-1;tile<=1;++tile) painter.drawImage(QPoint(shift+tile*source.width(),y),source,QRect(0,y,source.width(),1));
    }
    return result;
}
QImage renderMask(const EffectLayer& layer, double seconds)
{
    if (!layer.maskLinked) return layer.mask;
    auto mask=transformedMask(layer,effectTransform(layer).inverted()*effectTransform(layer,seconds));
    if (layer.warp>0) mask=warpRaster(mask,layer.warp,seconds,Qt::black).convertToFormat(QImage::Format_Grayscale8);
    return mask;
}

static QImage renderAtTime(const EffectLayer& layer, double seconds)
{
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 86400)
        throw std::invalid_argument("Preview time must be between 0 and 86400 seconds");
    auto evaluated = layer;
    // Evaluate from absolute time, without accumulating drift or editing undoable state.
    const double scale = 1 + layer.zoom/100 * std::sin(2*pi*seconds);
    if(!isGradient(layer.effect)) evaluated.center += QPointF(layer.driftX*seconds, layer.driftY*seconds);
    evaluated.extent *= scale;
    evaluated.zoom = 0;
    if(!isGradient(layer.effect)) { evaluated.driftX = 0; evaluated.driftY = 0; }
    // Evaluated geometry may be off-canvas or exceed editable bounds; only stored inputs are validated.
    evaluated.rotation = std::remainder(layer.rotation + layer.spin * seconds, 360.0);
    if (layer.cycleRate > 0) {
        const double position = seconds*layer.cycleRate*layer.colorCycle.size();
        const auto index = static_cast<int>(std::floor(std::fmod(position, double(layer.colorCycle.size()))));
        const auto next = (index+1) % layer.colorCycle.size();
        evaluated.color = QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(layer.colorCycle[index].rgba()),
            qPremultiply(layer.colorCycle[next].rgba()), position - std::floor(position))));
        if(isGradient(layer.effect) || layer.gradientFill) {
            const int count=layer.gradientColors.isEmpty() ? 2 : layer.gradientColors.size(); evaluated.gradientColors.clear();
            for(int i=0;i<count;++i) {
                const double phase=position+i*double(layer.colorCycle.size()-1)/(count-1);
                const int a=int(std::floor(std::fmod(phase,double(layer.colorCycle.size())))),b=(a+1)%layer.colorCycle.size();
                evaluated.gradientColors.append(QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(layer.colorCycle[a].rgba()),qPremultiply(layer.colorCycle[b].rgba()),phase-std::floor(phase)))));
            }
        }
    }
    if (layer.hueShift != 0 && layer.effect != EffectKind::Image) {
        auto shift = [&](QColor color) {
            float h, s, v, a; color.getHsvF(&h, &s, &v, &a);
            if (h < 0) return color; // Achromatic colors stay achromatic.
            h = std::fmod(h + layer.hueShift * seconds / 360.0, 1.0);
            return QColor::fromHsvF(h < 0 ? h + 1 : h, s, v, a);
        };
        evaluated.color = shift(evaluated.color); evaluated.secondaryColor = shift(layer.secondaryColor);
        for(auto& color:evaluated.gradientColors) color=shift(color);

    }
    auto image = renderBaseEffect(evaluated, seconds);
    if(layer.effect==EffectKind::Paint && layer.cycleRate>0) {
        QPainter painter(&image); painter.setCompositionMode(QPainter::CompositionMode_SourceIn); painter.fillRect(image.rect(),evaluated.color);
    }
    if (layer.warp > 0) image=warpRaster(image,layer.warp,seconds,Qt::transparent);
    if (layer.effect == EffectKind::Image && layer.hueShift != 0) {
        for (int y=0; y<image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x=0; x<image.width(); ++x) {
                QColor color = QColor::fromRgba(qUnpremultiply(row[x]));
                float h,s,v,a; color.getHsvF(&h,&s,&v,&a);
                if (h >= 0) {
                    double hue = std::fmod(h+layer.hueShift*seconds/360,1.0);
                    color = QColor::fromHsvF(hue<0 ? hue+1 : hue,s,v,a);
                }
                row[x] = qPremultiply(color.rgba());
            }
        }
    }
    if (layer.pulse > 0 || layer.breathe > 0) {
        double coverage = layer.pulse > 0 ? (1 + std::cos(2*pi*layer.pulse*seconds))/2 : 1;
        if (layer.breathe > 0) coverage *= std::exp(2*(std::cos(2*pi*layer.breathe*seconds)-1));
        for (int y = 0; y < image.height(); ++y) {
            auto* row = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); ++x) {
                const auto pixel = row[x];
                row[x] = qRgba(qRound(qRed(pixel)*coverage), qRound(qGreen(pixel)*coverage),
                    qRound(qBlue(pixel)*coverage), qRound(qAlpha(pixel)*coverage));
            }
        }
    }
    return image;
}

QImage renderEffect(const EffectLayer& layer, double seconds)
{
    validateEffect(layer);
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 86400) throw std::invalid_argument("Invalid effect time");
    if (layer.trails == 0) return renderAtTime(layer, seconds);
    QImage result(LightingDocument::canvasSize(), QImage::Format_ARGB32_Premultiplied); result.fill(Qt::transparent);
    QPainter painter(&result);
    // Fixed temporal taps keep seeking reproducible and bound work independently of elapsed playback.
    for (int i=6; i>=0; --i) {
        const double time = seconds - layer.trails*i/6;
        if (time < 0) continue;
        painter.setOpacity(i == 0 ? 1 : (1-i/7.0)*0.35);
        painter.drawImage(0,0,renderAtTime(layer,time));
    }
    return result;
}
