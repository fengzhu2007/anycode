#include "file_item.h"
namespace ady{
	
	FileItem::FileItem(){
        type = Unkown;
        size = 0;
        enabled = true;
        match_path = true;
	}

    QJsonObject FileItem::toJson(){
        return {
            {"name",name},
            {"path",path},
            {"permission",permission},
            {"ext",ext},
            {"size",size},
            {"modifyTime",modifyTime},
            {"owner",owner},
            {"group",group},
            {"type",type},
            {"enabled",enabled},
            {"match_path",match_path},
        };
    }

    FileItem FileItem::fromJson(QJsonObject data){
        FileItem item;
        item.name = data.find("name")->toString();
        item.path = data.find("path")->toString();
        item.permission = data.find("permission")->toString();
        item.ext = data.find("ext")->toString();
        item.size = data.find("size")->toInt(0);
        item.modifyTime = data.find("modifyTime")->toString();
        item.owner = data.find("owner")->toString();
        item.group = data.find("group")->toString();
        item.type = static_cast<FileItem::Type>(data.find("type")->toInt(0));
        item.enabled = data.find("enabled")->toBool();
        item.match_path = data.find("match_path")->toBool();
        return item;
    }
	
	
}
