#include "EffectRenderer.h"
#include "LightingDocument.h"
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
constexpr double pi = 3.14159265358979323846;
double noise2d(double x,double y,quint32 seed)
{
    const int ix=int(std::floor(x)),iy=int(std::floor(y));
    auto sample=[seed](int x,int y) {
        quint32 hash=quint32(x)*374761393u+quint32(y)*668265263u+seed*1442695041u;
        hash=(hash^(hash>>13))*1274126177u;hash^=hash>>16;
        return (hash&0xffffff)/double(0xffffff);
    };
    const double fx=x-ix,fy=y-iy,sx=fx*fx*(3-2*fx),sy=fy*fy*(3-2*fy);
    const double top=sample(ix,iy)*(1-sx)+sample(ix+1,iy)*sx;
    const double bottom=sample(ix,iy+1)*(1-sx)+sample(ix+1,iy+1)*sx;
    return top*(1-sy)+bottom*sy;
}
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
        {EffectKind::Paint, "paint", "Paint layer", .25, 80, "Paint pixels with the drawing tools. Source and mask remain independently editable."},
        {EffectKind::StaticColor, "static", "Static Color", .25, 80, "One solid color across the canvas."},
        {EffectKind::LinearGradient, "linear", "Linear Gradient", .25, 80, "Colors follow a straight line. Drag the gradient endpoints to set its direction and length."},
        {EffectKind::RadialGradient, "radial", "Radial Gradient", .25, 80, "Colors spread from the center outwards. Resize and move the source to place the glow."},
        {EffectKind::ConicalGradient, "conical", "Conical Gradient", .25, 80, "Colors wrap around the source center like a color wheel."},
        {EffectKind::Stripes, "stripes", "Stripes", .25, 80, "Repeating hard color bands. Repetitions controls their count."}, {EffectKind::Waves, "waves", "Waves", .25, 80, "Repeating smooth color waves along the gradient direction."},
        {EffectKind::Rectangle, "rectangle", "Rectangle", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Circle, "circle", "Circle", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Ring, "ring", "Ring", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."},
        {EffectKind::Star, "star", "Star", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Heart, "heart", "Heart", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."},
        {EffectKind::Triangle, "triangle", "Triangle", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Hexagon, "hexagon", "Hexagon", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."},
        {EffectKind::Diamond, "diamond", "Diamond", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Cross, "cross", "Cross", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."},
        {EffectKind::Moon, "moon", "Moon", .25, 80, "An editable shape with solid or gradient fill, geometry and a separate mask."}, {EffectKind::Image, "image", "Image", .25, 80, "An imported image with editable fit, crop, geometry and mask."},
        {EffectKind::Rain, "rain", "Particle Rain", .25, 80, "Seeded falling particles with adjustable count, speed and size."}, {EffectKind::Orbit, "orbit", "Particle Orbit", .25, 80, "Particles travel around the source center."},
        {EffectKind::Fireflies, "fireflies", "Fireflies", .25, 80, "Seeded drifting, flickering points of light."}, {EffectKind::Burst, "burst", "Particle Burst", .25, 80, "Short particle cohorts explode outward with seeded velocities, gravity and fading. Trails are optional."},
        {EffectKind::Pinwheel, "pinwheel", "Gradient Pinwheel", .25, 80, "Repeating color sectors around the center. Positive speed rotates clockwise; negative reverses."}, {EffectKind::Spiral, "spiral", "Spiral", .25, 80, "A rotating spiral. Repetitions controls its winding; signed speed controls direction."},
        {EffectKind::Visor, "visor", "Visor", .25, 80, "A soft band sweeps back and forth across the source."}, {EffectKind::SineWave, "sine-wave", "Sine Wave", .25, 80, "A moving sine wave. Width, amplitude and repetitions control its shape."},
        {EffectKind::Dna, "dna", "DNA", .25, 80, "Two moving sine waves connected by rungs."}, {EffectKind::PixelFill, "pixel-fill", "Pixel Fill", .25, 80, "Cells fill in rows, snake or spiral order, then restart."},
        {EffectKind::Voronoi, "voronoi", "Voronoi", .1, 80, "Moving sites divide the canvas into colored cells. Distance changes how cell boundaries form."}, {EffectKind::Fireplace, "fireplace", "Fireplace", .1, 80, "Rising flame texture. Detail controls texture density; height stretches the flames."},
        {EffectKind::Aurora, "aurora", "Aurora", .08, 80, "Legacy wave source retained for existing projects. Use Sine Wave for new ribbon layouts."}, {EffectKind::LavaLamp, "lava-lamp", "Lava Lamp", .08, 80, "Soft blobs merge and separate. Blob count and seed change their layout.", 6},
        {EffectKind::Checkerboard, "checkerboard", "Checkerboard", .1, 80, "A moving checker pattern. Grid size controls the number of cells per axis."}, {EffectKind::Kaleidoscope, "kaleidoscope", "Kaleidoscope", .1, 80, "Mirrored radial color patterns. Repetitions controls the number of symmetric sectors."},
        {EffectKind::Plasma, "plasma", "Plasma", .1, 80, "Smooth flowing colors formed by overlapping animated sine fields."}, {EffectKind::Ripples, "ripples", "Ripples", .1, 80, "Concentric color rings. Positive speed expands them; negative contracts them."},
        {EffectKind::Lightning, "lightning", "Lightning", .25, 3, "Seeded branching lightning flashes. Flash rate, bolt count and width shape the discharge."},
        {EffectKind::Starfield, "starfield", "Starfield", .25, 120, "Stars approach through perspective depth, accelerating and growing towards the viewer."},
        {EffectKind::Radar, "radar", "Radar", .2, 80, "A rotating radar beam. Speed is revolutions per minute: positive clockwise, negative counterclockwise."}, {EffectKind::Fireworks, "fireworks", "Fireworks", .25, 320, "Staggered launches explode into fading ballistic spark trails."}
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

bool isPattern(EffectKind kind) { return kind>=EffectKind::Pinwheel && (kind<=EffectKind::Radar || (kind>=EffectKind::Plasma && kind<=EffectKind::Kaleidoscope)); }
bool isGradient(EffectKind kind) { return (kind >= EffectKind::LinearGradient && kind <= EffectKind::Waves) || isPattern(kind); }

bool canCreateEffect(EffectKind kind) { return kind!=EffectKind::Aurora; }
bool isParticle(EffectKind kind) { return (kind >= EffectKind::Rain && kind <= EffectKind::Burst) || kind==EffectKind::Fireworks || kind==EffectKind::Lightning || kind==EffectKind::Starfield; }
const QVector<EffectParameter>& advancedParameters()
{
    static const QVector<EffectParameter> parameters{
        {"driftX", "Horizontal speed", &EffectLayer::driftX, -500, 500},
        {"driftY", "Vertical speed", &EffectLayer::driftY, -500, 500},
        {"zoom", "Size change %", &EffectLayer::zoom, 0, 90},
        {"warp", "Horizontal distortion", &EffectLayer::warp, 0, 100},
        {"warpY", "Vertical distortion", &EffectLayer::warpY, 0, 100, 21},
        {"breathe", "Breathe rate", &EffectLayer::breathe, 0, 10},
        {"trails", "Trail duration", &EffectLayer::trails, 0, 2},
        {"cycleRate", "Palette loops / sec", &EffectLayer::cycleRate, 0, 10},
        {"particleCount", "Particle count", &EffectLayer::particleCount, 1, 400},
        {"particleSeed", "Random pattern", &EffectLayer::particleSeed, 0, 1000000},
        {"particleSpeed", "Movement speed", &EffectLayer::particleSpeed, 0, 500},
        {"particleSize", "Particle radius", &EffectLayer::particleSize, 1, 40},
        {"patternSpeed", "Pattern rate", &EffectLayer::patternSpeed, -10, 10, 16},
        {"patternWidth", "Band width", &EffectLayer::patternWidth, .005, .5, 16},
        {"patternAmplitude", "Wave amplitude", &EffectLayer::patternAmplitude, 0, .45, 16},
        {"patternCells", "Grid size", &EffectLayer::patternCells, 2, 64, 16},
        {"fillOrder", "Fill order", &EffectLayer::fillOrder, 0, 2, 16},
        {"patternSeed", "Random pattern", &EffectLayer::patternSeed, 0, 1000000, 17},
        {"patternSites", "Sites", &EffectLayer::patternSites, 2, 48, 17},
        {"patternDistance", "Distance", &EffectLayer::patternDistance, 0, 2, 17},
        {"patternDetail", "Flame detail", &EffectLayer::patternDetail, 1, 16, 17},
        {"patternHeight", "Flame height", &EffectLayer::patternHeight, .1, 2, 17},
        {"patternFrequency", "Spatial frequency", &EffectLayer::patternFrequency, .25, 4, 22},
        {"patternContrast", "Color contrast", &EffectLayer::patternContrast, .25, 3, 22},
        {"patternWarp", "Pattern distortion", &EffectLayer::patternWarp, 0, 1, 22},
        {"patternWavelength", "Wave width", &EffectLayer::patternWavelength, .25, 4, 22}
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
        throw std::invalid_argument("Source palettes require 2..16 colors");
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
    if(std::floor(layer.patternCells)!=layer.patternCells || std::floor(layer.fillOrder)!=layer.fillOrder)
        throw std::invalid_argument("Pattern grid size and fill order must be integers");
    if(std::floor(layer.patternSeed)!=layer.patternSeed || std::floor(layer.patternSites)!=layer.patternSites ||
       std::floor(layer.patternDistance)!=layer.patternDistance)
        throw std::invalid_argument("Pattern seed, sites and distance metric must be integers");
    if(layer.effect==EffectKind::Lightning && layer.particleCount>12) throw std::invalid_argument("Lightning supports at most 12 bolts");
    if (std::floor(layer.particleCount) != layer.particleCount || std::floor(layer.particleSeed) != layer.particleSeed ||
        layer.colorCycle.size() < 2 || layer.colorCycle.size() > 16)
        throw std::invalid_argument("Invalid particle count/seed or color cycle");
    for (const auto& color : layer.colorCycle) if (!color.isValid()) throw std::invalid_argument("Invalid cycle color");
    if(!layer.colorCyclePositions.isEmpty()) {
        if(layer.colorCyclePositions.size()!=layer.colorCycle.size()) throw std::invalid_argument("Cycle stop count mismatch");
        double before=-1;
        for(double position:layer.colorCyclePositions) {
            if(!std::isfinite(position) || position<0 || position>1 || position<=before) throw std::invalid_argument("Cycle stops must increase within 0..1");
            before=position;
        }
    }
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
        const double phase=seconds*layer.patternSpeed;
        QVector<QPointF> sites;
        if(layer.effect==EffectKind::Voronoi || layer.effect==EffectKind::LavaLamp) {
            QRandomGenerator random(quint32(layer.patternSeed));
            // Move sites once per frame, rather than evaluating trigonometry for every site/pixel pair.
            for(int i=0;i<int(layer.patternSites);++i) {
                const double x=random.generateDouble(),y=random.generateDouble(),offset=random.generateDouble();
                sites.append({.45*std::sin(2*pi*(phase+x)),.45*std::cos(2*pi*(phase+offset+y))});
            }
        }
        const auto* sitePoints=sites.constData();
        auto positions=layer.gradientPositions;
        // Repeating wave colors must close too; wrapping an open palette creates a hard seam every turn.
        const bool closedPalette=layer.effect==EffectKind::Pinwheel || layer.effect==EffectKind::Spiral ||
            layer.effect==EffectKind::SineWave || layer.effect==EffectKind::Dna || layer.effect==EffectKind::Ripples;
        if(closedPalette || (travelling && (layer.effect==EffectKind::LinearGradient || layer.effect==EffectKind::RadialGradient))) {
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
                double coverage=1;
                const auto wrap=[](double value) { return value-std::floor(value); };
                switch (layer.effect) {
                case EffectKind::Aurora: {
                    const double noise=noise2d((u+.5)*layer.patternDetail,phase*2,quint32(layer.patternSeed));
                    const double curtain=layer.patternAmplitude*std::sin(u*10+phase*2*pi)+.12*(noise-.5);
                    const double distance=std::abs(v-curtain);
                    coverage=std::exp(-distance/std::max(.01,layer.patternWidth))* (.55+.45*noise);
                    t=std::clamp(.5+v+noise*.25,0.0,1.0);break;
                }
                case EffectKind::LavaLamp: {
                    double field=0;
                    for(int i=0;i<sites.size();++i) {
                        const double sx=u-sitePoints[i].x(),sy=v-sitePoints[i].y();
                        field+=.012/(sx*sx+sy*sy+.008);
                    }
                    t=std::clamp((field-.35)*.7,0.0,1.0);break;
                }
                case EffectKind::Checkerboard: {
                    const double a=wrap(u+.5+phase),b=wrap(v+.5+phase);
                    t=(int(a*layer.patternCells)+int(b*layer.patternCells))%2;break;
                }
                case EffectKind::Kaleidoscope: {
                    const double radius=std::hypot(u,v)*layer.patternFrequency;
                    const double sector=std::cos((std::atan2(v,u)+layer.patternWarp*radius*4)*layer.repetitions);
                    t=.5+layer.patternContrast*.25*(std::sin(sector*radius*20+2*pi*phase)+std::sin(radius*16-2*pi*phase));break;
                }
                case EffectKind::Plasma: {
                    const double a=2*pi*phase;
                    const double x=u*layer.patternFrequency+layer.patternWarp*.3*std::sin(v*9+a);
                    const double y=v*layer.patternFrequency+layer.patternWarp*.3*std::sin(u*9-a);
                    t=.5+layer.patternContrast*(std::sin(x*12+a)+std::sin(y*12-a)+std::sin((x+y)*9+a)+std::sin(std::hypot(x,y)*18-a))/8;
                    break;
                }
                case EffectKind::Ripples:
                    t=wrap(std::hypot(u,v)*layer.repetitions-phase);break;
                case EffectKind::Voronoi: {
                    double nearest=std::numeric_limits<double>::max();int site=0;
                    for(int i=0;i<sites.size();++i) {
                        const double sx=std::abs(u-sitePoints[i].x()),sy=std::abs(v-sitePoints[i].y());
                        const double distance=layer.patternDistance==1 ? sx+sy : layer.patternDistance==2 ? std::max(sx,sy) : sx*sx+sy*sy;
                        if(distance<nearest) {nearest=distance;site=i;}
                    }
                    t=site/double(sites.size()-1);break;
                }
                case EffectKind::Fireplace: {
                    if(std::abs(u)>.5 || std::abs(v)>.5) {coverage=0;t=0;break;}
                    const double x=(u+.5)*layer.patternDetail*3+.3*std::sin(v*8+phase*4),y=(v+.5)*layer.patternDetail+phase*8;
                    const auto seed=quint32(layer.patternSeed);
                    const double noise=.65*noise2d(x,y,seed)+.25*noise2d(x*2,y*2,seed+1)+.1*noise2d(x*4,y*4,seed+2);
                    // Fade the source footprint independently of palette endpoint colors.
                    coverage=std::clamp((.5-std::abs(u))*30,0.0,1.0)*std::clamp((.5-std::abs(v))*30,0.0,1.0);
                    const double rise=(.5-v)/layer.patternHeight;
                    const double base=std::clamp(1-rise,0.0,1.0);
                    t=std::clamp((base*1.15-(1-noise)*.85)*1.15,0.0,1.0)*std::clamp((.5-std::abs(u))*8,0.0,1.0);
                    break;
                }
                case EffectKind::Radar: {
                    const double radius=std::hypot(u,v);
                    if(radius>=.5) {coverage=0;t=1;break;}
                    const double angle=wrap(std::atan2(v,u)/(2*pi));
                    const double age=wrap(phase-angle),beam=std::clamp(1-age/layer.patternWidth,0.0,1.0);
                    const double ring=std::abs(std::remainder(radius*2*layer.repetitions,1.0));
                    const double grid=ring<.018 || std::min(std::abs(u),std::abs(v))<.002 ? .16 : 0;
                    coverage=std::max(beam,grid)*std::clamp((.5-radius)*200,0.0,1.0);
                    t=std::clamp(age/layer.patternWidth,0.0,1.0)*.7;break;
                }
                case EffectKind::Pinwheel:
                    t=wrap(std::atan2(v,u)/(2*pi)*layer.repetitions-phase); break;
                case EffectKind::Spiral:
                    t=wrap(std::atan2(v,u)/(2*pi)+std::hypot(u,v)*layer.repetitions-phase); break;
                case EffectKind::Visor: {
                    const double center=.5+.5*std::sin(2*pi*phase);
                    coverage=std::clamp(1-std::abs(u+.5-center)/layer.patternWidth,0.0,1.0);
                    t=std::clamp(.5+(u+.5-center)/(2*layer.patternWidth),0.0,1.0); break;
                }
                case EffectKind::SineWave:
                case EffectKind::Dna: {
                    const double repeats=layer.repetitions/(layer.effect==EffectKind::Dna ? layer.patternWavelength : 1);
                    const double wave=(u+.5)*repeats-phase;
                    const double height=layer.patternAmplitude*std::sin(2*pi*wave);
                    double distance=std::abs(v-height);
                    if(layer.effect==EffectKind::Dna) {
                        distance=std::min(distance,std::abs(v+height));
                        // Two rungs per turn connect the strands; their phase follows the same clock.
                        if(std::abs(v)<=std::abs(height)) distance=std::min(distance,
                            std::abs(std::remainder(wave-.25,.5))/repeats*2);
                    }
                    coverage=std::clamp(1-distance/layer.patternWidth,0.0,1.0);
                    t=wrap(wave); break;
                }
                case EffectKind::PixelFill: {
                    const int cells=int(layer.patternCells);
                    const int cx=std::clamp(int(std::floor((u+.5)*cells)),0,cells-1);
                    const int cy=std::clamp(int(std::floor((v+.5)*cells)),0,cells-1);
                    int order=cy*cells+cx;
                    if(layer.fillOrder==1) order=cy*cells+(cy%2 ? cells-1-cx : cx);
                    else if(layer.fillOrder==2) {
                        // Closed-form clockwise spiral rank avoids rebuilding a grid for each frame/pixel.
                        const int ring=std::min({cx,cy,cells-1-cx,cells-1-cy}),side=cells-2*ring;
                        order=cells*cells-side*side;
                        if(cy==ring) order+=cx-ring;
                        else if(cx==cells-1-ring) order+=side-1+cy-ring;
                        else if(cy==cells-1-ring) order+=2*(side-1)+cells-1-ring-cx;
                        else order+=3*(side-1)+cells-1-ring-cy;
                    }
                    coverage=u<-.5 || u>=.5 || v<-.5 || v>=.5 ? 0 : std::clamp(wrap(phase)*cells*cells-order,0.0,1.0);
                    t=order/double(cells*cells-1); break;
                }
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
                if(coverage<1) row[x]=interpolate(layer.effect==EffectKind::Fireplace ? QRgb(0) : qPremultiply(colors.last().rgba()),row[x],coverage);
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
        const auto particleColor=[&](double position) {
            if(layer.gradientColors.isEmpty()) return QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(layer.color.rgba()),qPremultiply(layer.secondaryColor.rgba()),position)));
            const auto& colors=layer.gradientColors;
            int right=0;
            while(right<colors.size() && layer.gradientPositions.value(right,right/double(colors.size()-1))<=position)++right;
            if(right==0)return colors.first();if(right==colors.size())return colors.last();
            const double start=layer.gradientPositions.value(right-1,(right-1)/double(colors.size()-1));
            const double finish=layer.gradientPositions.value(right,right/double(colors.size()-1));
            return QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(colors[right-1].rgba()),qPremultiply(colors[right].rgba()),(position-start)/(finish-start))));
        };
        if(layer.effect==EffectKind::Lightning) {
            const double phase=seconds*layer.particleSpeed/200,age=wrap(phase);
            if(age>=.24)return image;
            random.seed(quint32(layer.particleSeed)^(quint32(std::floor(phase))*0x9e3779b9u));
            // High-speed reference: a growing branched leader precedes bright return strokes on the same channel.
            const double progress=std::min(1.0,age/.035);
            const double brightness=age<.035 ? .32 : .18*(1-age/.24)+
                std::exp(-std::pow((age-.055)/.028,2))+.65*std::exp(-std::pow((age-.14)/.022,2));
            painter.setCompositionMode(QPainter::CompositionMode_Plus);
            for(int bolt=0;bolt<int(layer.particleCount);++bolt) {
                const double start=-.35+.7*unit(),finish=std::clamp(start+.6*(unit()-.5),-.43,.43);
                QPolygonF channel;channel.append({start,-.5});double wander=0;
                for(int step=1;step<=36;++step) {
                    wander=.7*wander+.035*(unit()-.5);const double t=step/36.;
                    channel.append({start+(finish-start)*t+wander,t-.5});
                }
                const auto color=particleColor(unit());
                const auto stroke=[&](const QPolygonF& line,double width,double opacity) {
                    if(line.size()<2)return;
                    for(int pass=0;pass<3;++pass) {
                        auto ink=pass==2 ? color.lighter(175) : color;
                        ink.setAlphaF(std::clamp(color.alphaF()*brightness*opacity*(pass==0 ? .10 : pass==1 ? .30 : .9),0.0,1.0));
                        painter.setPen(QPen(ink,width*(pass==0 ? 4 : pass==1 ? 1.7 : .55)/layer.extent.width(),Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
                        painter.drawPolyline(line);
                    }
                };
                stroke(channel.mid(0,1+int(progress*36)),layer.particleSize,1);
                const int forks=5+int(unit()*4);
                for(int branch=0;branch<forks;++branch) {
                    const int root=4+int(unit()*26),steps=6+int(unit()*8);const double direction=unit()<.5 ? -1 : 1;
                    QPolygonF fork;fork.append(channel[root]);QPointF point=channel[root];
                    for(int step=1;step<=steps;++step){point+=QPointF(direction*(.01+.018*unit()),.008+.019*unit());fork.append(point);}
                    if(progress*36>root)stroke(fork.mid(0,1+std::min(steps,int((progress*36-root)*1.6))),layer.particleSize*.5,.65);
                    // A short secondary fork gives depth without unbounded recursion or per-frame state.
                    QPolygonF twig;twig.append(fork[steps/2]);point=twig.first();
                    for(int step=0;step<4;++step){point+=QPointF(-direction*(.008+.01*unit()),.015);twig.append(point);}
                    if(progress==1)stroke(twig,layer.particleSize*.25,.35);
                }
            }
            return image;
        }
        if(layer.effect==EffectKind::Starfield) {
            painter.setClipRect(QRectF(-.5,-.5,1,1));
            for(int i=0;i<int(layer.particleCount);++i) {
                const double x=(unit()-.5)*.5,y=(unit()-.5)*.5,offset=unit();
                const double depth=.06+.94*wrap(offset-seconds*layer.particleSpeed/200);
                const QPointF point(x/depth,y/depth);
                auto color=particleColor(unit());color.setAlphaF(color.alphaF()*(1.05-depth));painter.setBrush(color);
                const double radius=layer.particleSize*(.25+.2/depth);
                painter.drawEllipse(point,radius/layer.extent.width(),radius/layer.extent.height());
            }
            return image;
        }

        if(layer.effect==EffectKind::Fireworks) {
            const int emitters=std::min(4,int(layer.particleCount));
            QVector<QPointF> targets;QVector<double> offsets,colors;
            for(int i=0;i<emitters;++i) {targets.append({-.28+.56*unit(),-.22+.2*unit()});offsets.append(unit());colors.append(unit());}
            painter.setCompositionMode(QPainter::CompositionMode_Plus);
            for(int i=0;i<int(layer.particleCount);++i) {
                const int emitter=i%emitters;const auto target=targets[emitter];
                const double cycle=wrap(seconds*layer.particleSpeed/200+offsets[emitter]);
                const double angle=2*pi*unit(),depth=2*unit()-1,velocity=(.8+.4*unit())*std::sqrt(1-depth*depth);
                const double flicker=unit();auto color=particleColor(std::clamp(colors[emitter]+.08*(unit()-.5),0.0,1.0));
                if(cycle<.25) {
                    if(i>=emitters)continue;
                    const double progress=cycle/.25;const QPointF start(target.x(),.5),point=start+(target-start)*progress;
                    painter.setPen(QPen(color.lighter(150),2/layer.extent.width(),Qt::SolidLine,Qt::RoundCap));
                    painter.drawLine(point,point-(target-start)*.10);continue;
                }
                // Reference photographs integrate curved spark paths: sample a finite trail, not a permanent radial spoke.
                // Spherical projection, drag and gravity stay analytic so seek/freeze/editor/service remain deterministic.
                const double age=(cycle-.25)/.75;
                const auto point=[&](double t) {const double distance=velocity*1.4*(1-std::exp(-2.4*t))/2.4;
                    return target+QPointF(std::cos(angle)*distance,std::sin(angle)*distance+.26*t*t);};
                for(int tail=0;tail<10;++tail) {
                    const double a=std::max(0.0,age-.30*(10-tail)/10),b=std::max(0.0,age-.30*(9-tail)/10);
                    auto ink=color;const double sparkle=.75+.25*std::sin(flicker*40+b*90);
                    ink.setAlphaF(color.alphaF()*std::pow(1-age,1.3)*(.12+.65*(tail+1)/10)*sparkle);
                    painter.setPen(QPen(ink,layer.particleSize*(.25+.5*(tail+1)/10)/layer.extent.width(),Qt::SolidLine,Qt::RoundCap));
                    painter.drawLine(point(a),point(b));
                }
                auto halo=color;halo.setAlphaF(color.alphaF()*.12*std::pow(1-age,1.3));painter.setPen(Qt::NoPen);painter.setBrush(halo);
                painter.drawEllipse(point(age),layer.particleSize*1.5/layer.extent.width(),layer.particleSize*1.5/layer.extent.height());
                auto core=color.lighter(180);core.setAlphaF(color.alphaF()*std::pow(1-age,1.3));painter.setPen(Qt::NoPen);painter.setBrush(core);
                painter.drawEllipse(point(age),layer.particleSize*.45/layer.extent.width(),layer.particleSize*.45/layer.extent.height());
            }
            return image;
        }
        for (int i = 0; i < static_cast<int>(layer.particleCount); ++i) {
            const double a = unit(), b = unit(), c = unit();
            const double phase = seconds * layer.particleSpeed / 200;
            QPointF point;double opacity=1,sizeScale=1;
            switch (layer.effect) {
            case EffectKind::Rain: point = {a - 0.5, wrap(b + phase) - 0.5}; break;
            case EffectKind::Orbit: {
                const double angle = 2*pi*(a + phase*(0.2 + b));
                point = {0.45*std::sqrt(c)*std::cos(angle), 0.45*std::sqrt(c)*std::sin(angle)}; break;
            }
            case EffectKind::Fireflies:
                point = {0.35*std::sin(2*pi*(a + phase*(0.1+c))), 0.35*std::sin(2*pi*(b + phase*0.17))}; break;
            default: {
                // A finite cohort has a lifetime and ballistic spread, rather than permanent radial spokes.
                const double age=wrap(.2+phase-b*.18),angle=2*pi*a,velocity=.3+.3*c;
                point={velocity*age*std::cos(angle),velocity*age*std::sin(angle)+.18*age*age};
                opacity=std::pow(1-age,1.5);sizeScale=.55+.45*(1-age);break;
            }
            }
            auto color=particleColor(c);color.setAlphaF(color.alphaF()*opacity);painter.setBrush(color);
            painter.drawEllipse(point, sizeScale*layer.particleSize/layer.extent.width(), sizeScale*layer.particleSize/layer.extent.height());
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
    // Gradient drift moves the texture, not its geometry; handles and linked masks must agree with rendering.
    transform.translate(periodic(layer.center.x() + (isGradient(layer.effect) ? 0 : layer.driftX*seconds),640),periodic(layer.center.y() + (isGradient(layer.effect) ? 0 : layer.driftY*seconds),400));
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

static QImage warpRaster(const QImage& source, double amplitude, double vertical, double seconds, QColor background)
{
    QImage result(source.size(),QImage::Format_ARGB32_Premultiplied); result.fill(background);
    // Extend edge pixels instead of wrapping unrelated opposite edges into the image.
    // Bilinear sampling preserves soft gradients at fractional wave offsets.
    for(int y=0;y<result.height();++y) for(int x=0;x<result.width();++x) {
        const double sx=std::clamp(x-amplitude*std::sin(2*pi*(y/100.0+seconds)),0.0,double(source.width()-1));
        const double sy=std::clamp(y-vertical*std::sin(2*pi*(x/100.0+seconds)),0.0,double(source.height()-1));
        const int ix=int(sx),iy=int(sy);const double fx=sx-ix,fy=sy-iy;
        const QRgb a=source.pixel(ix,iy),b=source.pixel(std::min(ix+1,source.width()-1),iy);
        const QRgb c=source.pixel(ix,std::min(iy+1,source.height()-1)),d=source.pixel(std::min(ix+1,source.width()-1),std::min(iy+1,source.height()-1));
        const auto blend=[&](int va,int vb,int vc,int vd){return qRound((va*(1-fx)+vb*fx)*(1-fy)+(vc*(1-fx)+vd*fx)*fy);};
        result.setPixel(x,y,qRgba(blend(qRed(a),qRed(b),qRed(c),qRed(d)),blend(qGreen(a),qGreen(b),qGreen(c),qGreen(d)),blend(qBlue(a),qBlue(b),qBlue(c),qBlue(d)),blend(qAlpha(a),qAlpha(b),qAlpha(c),qAlpha(d))));
    }
    return result;
}
QImage renderMask(const EffectLayer& layer, double seconds)
{
    if (!layer.maskLinked) return layer.mask;
    auto mask=transformedMask(layer,effectTransform(layer).inverted()*effectTransform(layer,seconds));
    if ((layer.warp>0 || layer.warpY>0)) mask=warpRaster(mask,layer.warp,layer.warpY,seconds,Qt::black).convertToFormat(QImage::Format_Grayscale8);
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
        const int size=layer.colorCycle.size();
        // Reserve the legacy closing segment; moving stops changes timing without changing loop speed.
        auto stop=[&](int i) { return layer.colorCyclePositions.isEmpty() ? i/double(size) : layer.colorCyclePositions[i]*(size-1)/double(size); };
        auto cycleColor=[&](double phase) {
            phase=std::fmod(phase,1.0);
            int a=size-1,b=0; double start=stop(a),finish=stop(0)+1;
            if(phase<stop(0)) phase+=1;
            else for(int i=0;i+1<size;++i) if(phase<stop(i+1)) {a=i;b=i+1;start=stop(a);finish=stop(b);break;}
            return QColor::fromRgba(qUnpremultiply(interpolate(qPremultiply(layer.colorCycle[a].rgba()),
                qPremultiply(layer.colorCycle[b].rgba()),(phase-start)/(finish-start))));
        };
        const double position = seconds*layer.cycleRate;
        evaluated.color = cycleColor(position);
        if(isGradient(layer.effect) || isParticle(layer.effect) || layer.gradientFill) {
            const int count=layer.gradientColors.isEmpty() ? 2 : layer.gradientColors.size(); evaluated.gradientColors.clear();
            for(int i=0;i<count;++i) {
                evaluated.gradientColors.append(cycleColor(position+i*double(size-1)/(size*(count-1))));
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
    if ((layer.warp>0 || layer.warpY>0)) image=warpRaster(image,layer.warp,layer.warpY,seconds,Qt::transparent);
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
