#pragma once
#include "device/Component.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QJsonDocument>
#include <QVersionNumber>
#include <algorithm>

inline const std::vector<Component>& hardwareLibrary() {
    static const std::vector<Component> catalog=[] {
        std::vector<Component> result={Component::makeFan(6,true),Component::makeFan(12,false),Component::makeFan(16,false)};
        for(auto& c:result)c.displayName=c.productName+" / "+std::to_string(c.ledCount)+" addresses";
        for(int count:{30,60}){auto c=Component::makeGenericPlaceholder(count);c.type="Strip";c.productName=c.displayName="Generic strip / "+std::to_string(count)+" LEDs";result.push_back(c);}
        // Generic templates are explicitly configurable, not vendor protocol support.
        for(const auto& type:{"AIO","Water Block","Case","Cable","Heatsink"}){auto c=Component::makeGenericPlaceholder(12);c.type=type;c.productName=c.displayName=std::string("Generic ")+type+" / 12 addresses";c.arrange(std::string(type)=="AIO" || std::string(type)=="Water Block"?"Circle":"Rectangle");c.layoutDefaults=c.ledCoordinates;result.push_back(c);}
        const auto genericCount=result.size();
        // Invoice models, independently checked against manufacturer pages. M25
        // pairing follows the user's observed six addresses / twelve physical LEDs.
        for(const auto* variant:{"M25 Gen2 120 White","M25 Gen2 120 Reverse White"}){auto c=Component::makeFan(6,true);c.brand="Phanteks";c.productName=c.displayName=variant;result.push_back(c);}
        for(const auto& entry:std::vector<std::pair<std::string,std::string>>{{"Glacier One 360M25 Gen2 / pump","AIO"},{"NV5 MK2 / PSU shroud strip","Case"}}){
            auto c=Component::makeGenericPlaceholder(1);c.brand=entry.second=="RAM"?"ADATA XPG":"Phanteks";c.productName=c.displayName=entry.first;c.type=entry.second+" (count required)";result.push_back(c);
        }
        // Eight light-bar addresses were read from both actual ENE descriptors.
        {auto c=Component::makeGenericPlaceholder(8);c.brand="ADATA XPG";c.productName=c.displayName="XPG Lancer Blade RGB DDR5 / 24 GB module";c.type="RAM";result.push_back(c);}
        {auto c=Component::makeGenericPlaceholder(1);c.brand="Gigabyte";c.productName=c.displayName="RTX 5080 AERO OC / logo zone";c.type="GPU";result.push_back(c);}
        {auto c=Component::makeGenericPlaceholder(1);c.brand="MSI";c.productName=c.displayName="MPG B850 Edge TI / I/O shield zone";c.type="Motherboard";result.push_back(c);}
        // Installed product models precede generic templates in the add picker.
        std::rotate(result.begin(),result.begin()+genericCount,result.end());
        const auto builtins=result.size();
        // Read installed component metadata only. No SignalRGB runtime, effects,
        // images or binaries are copied or executed; selected definitions persist.
        QDir engine(qEnvironmentVariable("LOCALAPPDATA")+"/VortxEngine");
        auto versions=engine.entryList({"app-*"},QDir::Dirs|QDir::NoDotAndDotDot);
        std::sort(versions.begin(),versions.end(),[](const QString& a,const QString& b){return QVersionNumber::fromString(a.mid(4))>QVersionNumber::fromString(b.mid(4));});
        for(const auto& version:versions) {
            const QString root=engine.filePath(version+"/Signal-x64/Components");if(!QDir(root).exists())continue;
            QDirIterator files(root,{"*.json"},QDir::Files,QDirIterator::Subdirectories);
            while(files.hasNext() && result.size()<2000) {
                QFile file(files.next());if(file.size()>128*1024 || !file.open(QIODevice::ReadOnly))continue;
                try {auto c=Component::fromJson(QJsonDocument::fromJson(file.readAll()).object());if(c.valid() && c.ledCount<=120 && !c.productName.empty())result.push_back(std::move(c));}catch(const std::invalid_argument&){}
            }
            break;
        }
        std::sort(result.begin()+builtins,result.end(),[](const auto& a,const auto& b){return a.displayName<b.displayName;});return result;
    }();return catalog;
}

inline Component instantiateHardware(Component c,int index) {
    // Product files use different grid units; each product starts at a consistent
    // size, with its complete LedMapping preserved for the output wire chain.
    int left=c.ledCoordinates[0].first,top=c.ledCoordinates[0].second,right=left,bottom=top;
    for(const auto& [x,y]:c.ledCoordinates){left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);}
    const int ox=(index%3)*130,oy=(index/3)*130;
    for(auto& point:c.ledCoordinates)point={ox+(right>left?(point.first-left)*100/(right-left):50),oy+(bottom>top?(point.second-top)*100/(bottom-top):50)};
    c.layoutDefaults=c.ledCoordinates;c.displayName+=" #"+std::to_string(index+1);return c;
}
