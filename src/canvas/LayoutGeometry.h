#pragma once
#include "canvas/Canvas.h"
#include <algorithm>
#include <cmath>

struct LayoutBounds {int left=0,top=0,right=0,bottom=0;};
inline LayoutBounds layoutBounds(const std::vector<Component>& components) {
    LayoutBounds b;bool first=true;
    for(const auto& c:components)for(int i=0;i<c.ledCount;++i) {
        if(c.ledNames[i].find("Wire slot")==0)continue;
        const auto [x,y]=c.ledCoordinates[i];
        if(first){b={x,y,x,y};first=false;}
        b.left=std::min(b.left,x);b.right=std::max(b.right,x);b.top=std::min(b.top,y);b.bottom=std::max(b.bottom,y);
    }
    return b;
}
// This exact transform is shared by HID sampling and both preview views.
// Rotating/scaling changes the sampled effect footprint, never wire addresses.
inline std::pair<int,int> layoutSamplePoint(std::pair<int,int> point,const LayoutBounds& b,const CanvasPlacement& p,bool global=true) {
    const double nx=b.right>b.left?(point.first-b.left)/double(b.right-b.left):.5;
    const double ny=b.bottom>b.top?(point.second-b.top)/double(b.bottom-b.top):.5;
    double x=global?nx*(p.width-1):point.first*p.width/70.;
    double y=global?ny*(p.height-1):point.second*p.height/40.;
    const double cx=global?(p.width-1)/2:(b.left+b.right)/2.*p.width/70.;
    const double cy=global?(p.height-1)/2:(b.top+b.bottom)/2.*p.height/40.;
    const double a=p.rotation*3.141592653589793/180.,dx=x-cx,dy=y-cy;
    return {int((global?p.x:0)+cx+dx*std::cos(a)-dy*std::sin(a)),int((global?p.y:0)+cy+dx*std::sin(a)+dy*std::cos(a))};
}
