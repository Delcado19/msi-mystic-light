#pragma once
#include "components/ComponentCatalog.h"
#include <QRegularExpression>

// Presentation only: never rewrite a saved product's wire mapping or custom geometry.
inline QString hardwareManufacturer(const ComponentDefinition& c)
{
    if(!c.manufacturer.isEmpty()) return c.manufacturer;
    const auto text=c.name+" "+c.id;
    for(const auto* brand:{"Phanteks","Roccat","Razer","SteelSeries","Gigabyte","ADATA","MSI","Corsair","Lian Li","NZXT","ASUS","Cooler Master","Thermaltake","be quiet!"})
        if(text.contains(QLatin1String(brand),Qt::CaseInsensitive)) return QLatin1String(brand);
    if(text.contains("M25",Qt::CaseInsensitive) || text.contains("Glacier",Qt::CaseInsensitive) || text.contains("D30",Qt::CaseInsensitive)) return "Phanteks";
    return "Generic";
}
inline QString hardwareName(const ComponentDefinition& c,int chainPosition=0)
{
    auto name=c.name.trimmed(); const auto brand=hardwareManufacturer(c);
    name.replace(QRegularExpression("\\bPhantek\\b",QRegularExpression::CaseInsensitiveOption),"Phanteks");
    if(!name.startsWith(brand,Qt::CaseInsensitive)) name=brand+" "+name;
    name.remove(QRegularExpression("\\s*#\\d+\\b"));
    // Housing color does not identify a lighting model; retain all electrical/model suffixes.
    name.remove(QRegularExpression("\\b(?:black|white)\\b",QRegularExpression::CaseInsensitiveOption));
    name.replace(QRegularExpression("\\bAimo\\b",QRegularExpression::CaseInsensitiveOption),"AIMO");
    name=name.simplified();
    while(name.startsWith(brand+" "+brand+" ",Qt::CaseInsensitive))name=name.mid(brand.size()+1);
    if(chainPosition>0) name+=" #"+QString::number(chainPosition);
    return name;
}
inline QString hardwareCategory(const ComponentDefinition& c)
{
    if(!c.category.isEmpty()) return c.category;
    const auto text=(c.id+" "+c.name).toLower();
    if(text.contains("pump")) return "Pumps";
    if(text.contains("aio") || text.contains("glacier")) return "AiO";
    if(text.contains("fan") || text.contains("m25") || text.contains("d30")) return "Fans";
    if(text.contains("vulcan") || text.contains("apex") || text.contains("keyboard")) return "Keyboards";
    if(text.contains("basilisk") || text.contains("kone") || text.contains("mouse")) return "Mice";
    if(text.contains("ram") || text.contains("lancer")) return "RAM";
    if(text.contains("gpu") || text.contains("rtx")) return "Graphics cards";
    if(text.contains("ioshield") || text.contains("motherboard")) return "Motherboards";
    if(text.contains("strip") || text.contains("neon") || text.contains("nv5")) return "Strips";
    return "Other";
}
