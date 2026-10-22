#pragma once
#include "effects/CustomGradientEffect.h"
#include "effects/BreathingEffect.h"
#include "effects/CometEffect.h"
#include "effects/MarqueeEffect.h"
#include "effects/RadialRainbowEffect.h"
#include "effects/RainbowWaveEffect.h"
#include "effects/SpiralEffect.h"
#include "effects/StaticColorEffect.h"
#include "effects/SwirlCirclesEffect.h"

inline bool isBuiltInEffect(const std::string& name) {
    return CustomEffectDefinition::validPattern(name) || name=="StaticColor" || name=="RainbowWave" || name=="RadialRainbow" || name=="SwirlCircles"
        || name=="Aurora" || name=="Ocean" || name=="Ember" || name=="Pastel" || name=="Polar" || name=="Sunset";
}

inline CustomEffectDefinition effectDefinition(const std::string& name,const MsiColor& color,const std::vector<CustomEffectDefinition>& saved={}) {
    for(const auto& d:saved)if(name=="custom:"+d.name || (name==d.name && !isBuiltInEffect(name)))return d;
    static const std::vector<CustomEffectDefinition> palettes = {
        {"Aurora",{{38,210,167},{76,100,242},{196,74,220}},9000,900},
        {"Ocean",{{8,30,110},{25,155,210},{120,240,225}},8000,800},
        {"Ember",{{90,5,2},{235,53,8},{255,175,48}},7000,800},
        {"Pastel",{{240,160,215},{165,180,250},{160,235,215}},12000,1100},
        {"Polar",{{35,65,140},{145,215,245},{230,250,255}},10000,1000},
        {"Sunset",{{252,150,65},{225,75,120},{93,48,165}},10000,1000}};
    for(auto d:palettes)if(d.name==name)return d;
    CustomEffectDefinition d{name,{{255,0,0},{255,180,0},{0,255,100},{0,180,255},{80,0,255},{255,0,160}},6000,720};
    d.pattern=CustomEffectDefinition::validPattern(name)?name:name=="Spiral"?"Spiral":name=="SwirlCircles"?"Radial":"Wave";
    if(name=="StaticColor" || name=="Breathing" || name=="Comet" || name=="Marquee")d.colors={color,color};
    if(name=="Breathing")d.colors={color,{0,0,0}};
    if(name=="StaticColor")d.pattern="Static gradient";
    if(name=="RadialRainbow")d.pattern="Angular";
    if(name=="Comet" || name=="Marquee")d.pattern=name;
    if(name=="RadialRainbow" || name=="Spiral" || name=="SwirlCircles")d.path="Canvas positions";
    return d;
}

// Shared by hardware rendering and the preview so effect behavior cannot drift.
inline std::shared_ptr<IEffect> buildEffect(const std::string& effectName, const MsiColor& color, int referenceX, int referenceY, const std::vector<CustomEffectDefinition>& customEffects = {})
{
    // Explicit custom IDs avoid a saved recipe named Spiral replacing the built-in Spiral.
    for(const auto& definition:customEffects)if(effectName=="custom:"+definition.name || (effectName==definition.name && !isBuiltInEffect(effectName)))return std::make_shared<CustomGradientEffect>(definition);
    // shortcut: comet track is 300 scene pixels; expose it when its editor supports width.
    constexpr int kCometTrackWidthPx = 300;
    if (effectName == "StaticColor") return std::make_shared<StaticColorEffect>(color);
    if (effectName == "RainbowWave") return std::make_shared<RainbowWaveEffect>();
    if (effectName == "Breathing") return std::make_shared<CustomGradientEffect>(effectDefinition(effectName,color));
    if (effectName == "RadialRainbow") return std::make_shared<RadialRainbowEffect>(referenceX, referenceY);
    if (effectName == "Spiral") return std::make_shared<SpiralEffect>(referenceX, referenceY);
    if (effectName == "Comet") return std::make_shared<CometEffect>(color, kCometTrackWidthPx);
    if (effectName == "Marquee") return std::make_shared<MarqueeEffect>(color);
    if (effectName == "SwirlCircles") return std::make_shared<SwirlCirclesEffect>(referenceX, referenceY);
    if(CustomEffectDefinition::validPattern(effectName))return std::make_shared<CustomGradientEffect>(effectDefinition(effectName,color));
    const auto definition=effectDefinition(effectName,color);
    if(effectName=="Aurora" || effectName=="Ocean" || effectName=="Ember" || effectName=="Pastel" || effectName=="Polar" || effectName=="Sunset")return std::make_shared<CustomGradientEffect>(definition);
    for (const auto& definition : customEffects) {
        if (effectName == "custom:" + definition.name) return std::make_shared<CustomGradientEffect>(definition);
    }
    return nullptr; // unknown/empty name -- Canvas tolerates a null effect by skipping the channel for the frame
}

