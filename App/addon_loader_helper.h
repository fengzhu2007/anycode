#ifndef ADDON_LOADER_HELPER_H
#define ADDON_LOADER_HELPER_H
#include "storage/addon_storage.h"
#include "addon_loader.h"
namespace ady{


class AddonLoaderPrivate{
public:
    QList<AddonRecord> m_addons;

    QList<AddonRecord> filter(AddonLoader::ExportType type){
        QList<AddonRecord> result;
        int flag = static_cast<int>(type);
        for(const auto& record : m_addons){
            if((record.export_type & flag) == flag){
                result.append(record);
            }
        }
        return result;
    }
};


class AddonLoaderHelper
{
public:
    AddonLoaderHelper();
    QList<AddonRecord> filter(AddonLoader::ExportType type);
};
}
#endif // ADDON_LOADER_HELPER_H
