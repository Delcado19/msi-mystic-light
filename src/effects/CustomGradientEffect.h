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
        return name=="Wave" || name=="Vertical" || name=="Diagonal" || name=="Radial" || name=="Spiral"
            || name=="Plasma" || name=="Breathing" || name=="Sweep" || name=="Static gradient";
    }
    bool validStops() const {
        if(stops.empty())return true;
        if(stops.size()!=colors.size())return false;
        for(size_t i=0;i<stops.size();++i)
            if(!std::isfinite(stops[i]) || stops[i]<0 || stops[i]>=1 || (i && stops[i]-stops[i-1]<0.004999))return false;
        return true;
    }
};

// Declarative palettes only: no downloaded JavaScript executes in the app.
class CustomGradientEffect : public IEffect {
public:
    explicit CustomGradientEffect(CustomEffectDefinition definition) : definition_(std::move(definition)) {}
    MsiColor colorAt(int x, int y, int64_t time) const override {
        if (definition_.colors.empty()) return {};
        const double scale=static_cast<double>(std::max(1,definition_.wavelength));
        const double t=(time % std::max(1,definition_.periodMs))/static_cast<double>(std::max(1,definition_.periodMs));
        const auto& kind=definition_.pattern;
        double phase=x/scale+t;
        if(kind=="Vertical")phase=y/scale+t;
        else if(kind=="Diagonal")phase=(x+y)/scale+t;
        else if(kind=="Radial")phase=std::hypot(x-550.,y-350.)/scale-t;
        else if(kind=="Spiral")phase=std::atan2(y-350.,x-550.)/6.283185307+std::hypot(x-550.,y-350.)/scale-t;
        else if(kind=="Plasma")phase=(std::sin(x/scale*6.283185307+t*6.283185307)+std::sin(y/scale*6.283185307-t*6.283185307))*.24+.5;
        else if(kind=="Breathing")phase=x/scale+t;
        else if(kind=="Sweep")phase=x/scale-t;
        else if(kind=="Static gradient")phase=x/scale;
        return paletteAt(phase,t);
    }
    bool usesSceneCoordinates() const override{return definition_.path=="Canvas positions";}
    MsiColor colorForLed(int x,int y,int64_t time,int wire,int count,int localWire,int localCount)const override {
        if(usesSceneCoordinates())return colorAt(x,y,time);
        const double t=(time%std::max(1,definition_.periodMs))/double(std::max(1,definition_.periodMs));
        const double position=definition_.path=="Entire chain"?wire/double(std::max(1,count)):localWire/double(std::max(1,localCount));
        // A full palette cycle traverses actual LED addresses. Breathing keeps
        // its pulse but colors are phase-offset per LED instead of one flat hue.
        return paletteAt(position+(definition_.pattern=="Static gradient"?0:t),t);
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
        const double brightness=kind=="Breathing"?.15+.85*(.5-.5*std::cos(t*6.283185307))
            :kind=="Sweep"?std::pow(.5+.5*std::cos(phase*6.283185307),4):1.;
        const auto mix=[&](int a,int b){return static_cast<uint8_t>(std::clamp(std::lround((a+(b-a)*blend)*brightness),0L,255L));};
        return {mix(a.r,b.r),mix(a.g,b.g),mix(a.b,b.b)};
    }

private:
    CustomEffectDefinition definition_;
};
