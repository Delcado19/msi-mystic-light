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
        std::sort(result.begin()+5,result.end(),[](const auto& a,const auto& b){return a.displayName<b.displayName;});return result;
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
