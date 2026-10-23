#include "LegacyHardware.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTransform>
#include <cmath>
#include <stdexcept>

QStringList importLegacyHardware(LightingDocument& document,const QString& path,const QVector<ProfileCandidate>& candidates)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot read previous Lighting Studio settings");
    const auto bytes=file.read(4*1024*1024+1);
    QJsonParseError error; const auto json=QJsonDocument::fromJson(bytes,&error);
    if(bytes.size()>4*1024*1024 || error.error!=QJsonParseError::NoError || !json.isObject()) throw std::runtime_error("Invalid or oversized previous settings");
    const auto root=json.object(); QJsonObject profile;
    for(const auto& value:root.value("profiles").toArray()) if(value.toObject().value("name")==root.value("activeProfile")) profile=value.toObject();
    if(profile.isEmpty() || !profile.value("channels").isArray()) throw std::runtime_error("Previous active profile is missing");
    QStringList notes; int imported=0;
    document.edit("Restore previous hardware layout",[&](LightingState& state) {
        for(const auto& value:profile.value("channels").toArray()) {
            const auto old=value.toObject(); const auto name=old.value("channelName").toString();
            QVector<ChannelAssignment> matches;
            for(const auto& candidate:candidates) {
                const auto& device=assignmentProfile(document.profiles(),candidate.profileId);
                for(const auto& channel:device.channels) if(channel.id==name && (channel.direct || old.value("mode").toString()=="Native")) {
                    ChannelAssignment assignment; assignment.endpoint=candidate.device.path; assignment.profileId=device.id;
                    assignment.channelId=channel.id; assignment.software=old.value("mode").toString()!="Native";
                    matches.append(assignment);
                }
            }
            const auto components=old.value("components").toArray();
            if(components.isEmpty() && old.value("mode").toString()!="Native") continue;
            if(matches.size()!=1) { notes.append(name+": not imported (no unique supported endpoint)"); continue; }
            auto assignment=matches[0]; bool exists=false;
            for(const auto& current:state.assignments) if(current.endpoint==assignment.endpoint && current.channelId==name) exists=true;
            if(exists) { notes.append(name+": existing setup kept"); continue; }
            if(!assignment.software) {
                const auto native=old.value("nativeSettings").toObject();
                const QStringList modes{"off","wave","steady","flame","breathing","color-ring","lightning","recreation","meteor","","godlike"};
                assignment.native.id=modes.value(native.value("lightingMode").toInt(-1));
                assignment.native.brightness=native.value("brightness").toInt(5); assignment.native.speed=native.value("speed").toInt();
                assignment.native.inward=native.value("directionInward").toBool(); assignment.native.colors.clear();
                const int count=native.value("colorCount").toInt();
                if(count<1 || count>4) throw std::runtime_error("Invalid previous native color count");
                for(int i=1;i<=count;++i) {
                    const auto color=native.value("color"+QString::number(i)).toObject();
                    for(const auto* key:{"r","g","b"}) if(!color.value(key).isDouble() || color.value(key).toInt(-1)<0 || color.value(key).toInt(256)>255) throw std::runtime_error("Invalid previous native color");
                    assignment.native.colors.append({quint8(color.value("r").toInt()),quint8(color.value("g").toInt()),quint8(color.value("b").toInt())});
                }
                state.assignments.append(assignment); ++imported; continue;
            }
            // The old shared canvas was 1100 x 700. Preserve its normalized sampling footprint.
            QRectF bounds;
            for(const auto& component:components) for(const auto& coordinate:component.toObject().value("LedCoordinates").toArray()) {
                const auto point=coordinate.toArray();
                if(point.size()!=2 || !point[0].isDouble() || !point[1].isDouble()) throw std::runtime_error("Invalid saved LED coordinate");
                bounds=bounds.united(QRectF(QPointF(point[0].toDouble(),point[1].toDouble()),QSizeF(.0001,.0001)));
            }
            const auto box=old.value("placement").toObject();
            for(const auto* key:{"x","y","width","height","rotation"})
                if(!box.value(key).isDouble() || !std::isfinite(box.value(key).toDouble())) throw std::runtime_error("Invalid previous placement coordinate");
            const double width=box.value("width").toDouble(), height=box.value("height").toDouble(), angle=box.value("rotation").toDouble();
            if(!std::isfinite(width) || !std::isfinite(height) || width<=0 || height<=0 || width>10000 || height>10000 || !std::isfinite(angle)) throw std::runtime_error("Invalid saved placement");
            QTransform rotate; rotate.translate((width-1)/2,(height-1)/2); rotate.rotate(angle); rotate.translate(-(width-1)/2,-(height-1)/2);
            for(const auto& item:components) {
                const auto component=item.toObject(); QJsonArray points;
                for(const auto& coordinate:component.value("LedCoordinates").toArray()) {
                    const auto point=coordinate.toArray();
                    const auto rotated=rotate.map(QPointF((point[0].toDouble()-bounds.left())/std::max(1.0,bounds.width())*(width-1),
                        (point[1].toDouble()-bounds.top())/std::max(1.0,bounds.height())*(height-1)));
                    points.append(QJsonArray{(box.value("x").toDouble()+rotated.x())*640/1100/20,
                        (box.value("y").toDouble()+rotated.y())*400/700/20});
                }
                const QString id="restored."+QUuid::createUuid().toString(QUuid::WithoutBraces);
                auto definition=ComponentDefinition::fromJson({{"schemaVersion",1},{"id",id},
                    {"name",component.value("DisplayName").toString(component.value("ProductName").toString())},
                    {"ledCount",component.value("LedCount")},{"mapping",component.value("LedMapping")},{"positions",points}});
                if(state.components.size()>=256) throw std::runtime_error("Component catalog limit reached");
                QRectF localBounds;
                for(const auto point:definition.positions) localBounds=localBounds.united(QRectF(point,QSizeF(.0001,.0001)));
                const auto center=localBounds.center();
                for(auto& point:definition.positions) point-=center;
                state.components.insert(id,definition);
                ComponentPlacement placement; placement.componentId=id; placement.origin=center*20;
                state.placements.append(placement); assignment.placements.append(placement.id);
            }
            state.assignments.append(assignment); ++imported;
        }
    });
    notes.prepend(QString("Restored %1 channel chains. Create or select an effect before starting output. Previous effect recipes remain in the original settings file; they are not converted by this hardware-only import.").arg(imported));
    return notes;
}
