#pragma once
#include "effects/IEffect.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

struct CustomEffectDefinition {
    std::string name;
    std::vector<MsiColor> colors;
    int periodMs = 6000;
    int wavelength = 720;
    std::string pattern = "Wave";
    std::string path="Component LEDs";
    static bool validPath(const std::string& p){return p=="Component LEDs" || p=="Entire chain" || p=="Canvas positions";}
    std::vector<double> stops; // Empty means evenly spaced legacy cyclic colors.
    static bool validPattern(const std::string& name) {
        return name=="Angular" || name=="Comet" || name=="Marquee" || name=="Scroll" || name=="Color source" || name=="Wave" || name=="Vertical" || name=="Diagonal" || name=="Radial" || name=="Spiral"
            || name=="Plasma" || name=="Breathing" || name=="Sweep" || name=="Static gradient";
    }
    // Legacy directional names remain readable but are edited as one Scroll pattern.
    void normalizeScroll() {
        if(pattern=="Wave"){pattern="Scroll";angle=180;}
        else if(pattern=="Vertical"){pattern="Scroll";angle=270;}
        else if(pattern=="Diagonal"){pattern="Scroll";angle=225;wavelength=std::max(40,int(std::lround(wavelength/std::sqrt(2.))));}
        for(auto& layer:layers)layer.normalizeScroll();
    }
    double angle = 0;
    double opacity = 1;
    bool reverse = false;
    bool enabled = true;
    double spreadDegrees = 360;
    int centerX = 550, centerY = 350;
    std::string sourceMotion = "Sine";
    bool zoneEnabled = false;
    int zoneX = 0, zoneY = 0, zoneWidth = 1100, zoneHeight = 700;
    std::string blend = "Screen";
    std::vector<CustomEffectDefinition> layers; // Flat stack, at most eight overlays.
    bool validLayerSettings() const {
        if((sourceMotion!="Sine" && sourceMotion!="Cosine" && sourceMotion!="Ping-pong" && sourceMotion!="Static") || centerX<0 || centerX>1100 || centerY<0 || centerY>700 || !std::isfinite(spreadDegrees) || spreadDegrees<15 || spreadDegrees>720 || !std::isfinite(angle) || angle < -360 || angle > 360 || !std::isfinite(opacity) || opacity<0 || opacity>1
            || zoneX<0 || zoneX>1100 || zoneY<0 || zoneY>700 || zoneWidth<1 || zoneWidth>1100 || zoneHeight<1 || zoneHeight>700
            || (blend!="Screen" && blend!="Add" && blend!="Normal") || layers.size()>8)return false;
        for(const auto& layer:layers)if(!layer.layers.empty() || !layer.validLayerSettings() || !layer.validStops() || !validPattern(layer.pattern)
            || layer.colors.size()<2 || layer.colors.size()>16 || layer.periodMs<500 || layer.periodMs>120000 || layer.wavelength<40 || layer.wavelength>4000)return false;
        return true;
    }
    bool validStops() const {
        if(stops.empty())return true;
        if(stops.size()!=colors.size())return false;
        for(size_t i=0;i<stops.size();++i)
            if(!std::isfinite(stops[i]) || stops[i]<0 || stops[i]>=1 || (i && stops[i]-stops[i-1]<0.004999))return false;
        return true;
    }
};

// Keep persisted protocol-era identifiers while presenting the actual visual behavior.
inline std::string effectDisplayName(const std::string& name) {
    if(name=="Radial" || name=="SwirlCircles")return "Concentric rings";
    if(name=="Angular" || name=="RadialRainbow")return "Color wheel";
    if(name=="StaticColor")return "Solid color";
    if(name=="Marquee")return "Chase";
    if(name=="Breathing")return "Breathing / color crossfade";
    return name;
}

// Declarative palettes only: no downloaded JavaScript executes in the app.
class CustomGradientEffect : public IEffect {
public:
    explicit CustomGradientEffect(CustomEffectDefinition definition) : definition_(std::move(definition)) {
        for(const auto& layer:definition_.layers)layers_.emplace_back(layer);
    }
    MsiColor colorAt(int x, int y, int64_t time) const override {
        // A base-layer zone clips only that layer; overlays retain their own zones.
        const bool inside=!definition_.zoneEnabled || (x>=definition_.zoneX && y>=definition_.zoneY && x<definition_.zoneX+definition_.zoneWidth && y<definition_.zoneY+definition_.zoneHeight);
        auto result=inside && definition_.enabled?baseColorAt(x,y,time):MsiColor{};
        for(const auto& layer:layers_) {
            const auto& d=layer.definition_;
            if(!d.enabled)continue;
            if(d.zoneEnabled && (x<d.zoneX || y<d.zoneY || x>=d.zoneX+d.zoneWidth || y>=d.zoneY+d.zoneHeight))continue;
            const auto color=layer.colorAt(x,y,time);
            const auto mix=[&](int a,int b){const double v=d.blend=="Add"?std::min(255,a+b):d.blend=="Normal"?b:255-(255-a)*(255-b)/255.;return uint8_t(std::clamp(std::lround(a+(v-a)*d.opacity),0L,255L));};
            result={mix(result.r,color.r),mix(result.g,color.g),mix(result.b,color.b)};
        }
        return result;
    }
    std::pair<double,double> sourceCenterAt(int64_t time)const {
        const double t=(time%std::max(1,definition_.periodMs))/double(std::max(1,definition_.periodMs));
        const double phase=definition_.sourceMotion=="Static"?0:definition_.sourceMotion=="Cosine"?std::cos(t*6.283185307):definition_.sourceMotion=="Ping-pong"?1.-4.*std::abs(t-.5):std::sin(t*6.283185307);
        const double angle=definition_.angle*3.141592653589793/180.;
        return {(definition_.zoneEnabled?definition_.zoneX:0)+(definition_.zoneEnabled?definition_.zoneWidth:1100)*(.5+.35*std::cos(angle)*phase),
                (definition_.zoneEnabled?definition_.zoneY:0)+(definition_.zoneEnabled?definition_.zoneHeight:700)*(.5+.35*std::sin(angle)*phase)};
    }
    MsiColor baseColorAt(int x, int y, int64_t time) const {
        if (definition_.colors.empty()) return {};
        const double scale=static_cast<double>(std::max(1,definition_.wavelength));
        const double t=(time % std::max(1,definition_.periodMs))/static_cast<double>(std::max(1,definition_.periodMs));
        const auto& kind=definition_.pattern;
        const double angle=definition_.angle*3.141592653589793/180.;
        const double motion=definition_.reverse?-t:t;
        double phase=x/scale+t;
        if(kind=="Breathing")return paletteAt(t,t);
        if(kind=="Scroll")phase=(x*std::cos(angle)+y*std::sin(angle))/scale-t;
        if(kind=="Color source") {
            const auto [cx,cy]=sourceCenterAt(time);
            const double brightness=std::exp(-((x-cx)*(x-cx)+(y-cy)*(y-cy))/(scale*scale*.25));
            const auto c=definition_.colors.front();return {uint8_t(c.r*brightness),uint8_t(c.g*brightness),uint8_t(c.b*brightness)};
        }
        if(kind=="Vertical")phase=y/scale+t;
        else if(kind=="Diagonal")phase=(x+y)/scale+t;
        else if(kind=="Radial")phase=std::hypot(x-double(definition_.centerX),y-double(definition_.centerY))/scale-motion;
        else if(kind=="Spiral")phase=std::atan2(y-double(definition_.centerY),x-double(definition_.centerX))/(definition_.spreadDegrees*3.141592653589793/180.)+angle/6.283185307+std::hypot(x-double(definition_.centerX),y-double(definition_.centerY))/scale-motion;
        else if(kind=="Plasma")phase=(std::sin(x/scale*6.283185307+t*6.283185307)+std::sin(y/scale*6.283185307-t*6.283185307))*.24+.5;
        else if(kind=="Angular")phase=std::atan2(y-double(definition_.centerY),x-double(definition_.centerX))/(definition_.spreadDegrees*3.141592653589793/180.)+angle/6.283185307-motion;
        else if(kind=="Comet" || kind=="Marquee")phase=(x*std::cos(angle)+y*std::sin(angle))/scale-t+(kind=="Comet"?.875:0);
        else if(kind=="Sweep")phase=x/scale-t;
        else if(kind=="Static gradient")phase=x/scale;
        return paletteAt(phase,t);
    }
    bool usesSceneCoordinates() const override{return definition_.path=="Canvas positions";}
    MsiColor colorForLed(int x,int y,int64_t time,int wire,int count,int localWire,int localCount)const override {
        if(usesSceneCoordinates() || definition_.pattern=="Breathing")return colorAt(x,y,time);
        const double t=(time%std::max(1,definition_.periodMs))/double(std::max(1,definition_.periodMs));
        const double position=definition_.path=="Entire chain"?wire/double(std::max(1,count)):localWire/double(std::max(1,localCount));
        const auto& kind=definition_.pattern;
        if(kind=="Radial" || kind=="Angular" || kind=="Spiral" || kind=="Plasma" || kind=="Color source")return colorAt(x,y,time);
        // Breathing already returned a uniform temporal color above.
        const bool stationary=kind=="Static gradient" || kind=="Breathing";
        const bool forward=std::cos(definition_.angle*3.141592653589793/180.)>=0;
        return paletteAt((kind=="Comet"?.875:0)+position+(stationary?0:(kind=="Scroll" || kind=="Comet" || kind=="Marquee")?(forward?-t:t):t),t);
    }
private:
    MsiColor paletteAt(double phase,double t)const {
        if(definition_.colors.empty())return {};
        const auto& kind=definition_.pattern;
        phase-=std::floor(phase);
        const auto count=definition_.colors.size();
        const auto stop=[&](size_t i){return definition_.stops.size()==count?definition_.stops[i]:i/static_cast<double>(count);};
        size_t first=count-1;
        for(size_t i=0;i<count;++i)if(stop(i)<=phase)first=i;
        const size_t next=(first+1)%count;
        double begin=stop(first),end=next?stop(next):stop(0)+1.;
        if(phase<begin)phase+=1.;
        double blend=(phase-begin)/std::max(.000001,end-begin);
        // Smooth interpolation removes visible velocity corners at palette stops.
        blend=blend*blend*(3.-2.*blend);
        const auto a=definition_.colors[first],b=definition_.colors[next];
        const double brightness=kind=="Comet"?std::max(0.,1.-(1.-phase)/.25):kind=="Marquee"?(phase<.5?1.:0.):kind=="Sweep"?std::pow(.5+.5*std::cos(phase*6.283185307),4):1.;
        const auto mix=[&](int a,int b){return static_cast<uint8_t>(std::clamp(std::lround((a+(b-a)*blend)*brightness),0L,255L));};
        return {mix(a.r,b.r),mix(a.g,b.g),mix(a.b,b.b)};
    }

private:
    CustomEffectDefinition definition_;
    std::vector<CustomGradientEffect> layers_;
};
