#include "addon_loader_helper.h"
namespace ady{

AddonLoaderHelper::AddonLoaderHelper() {}

QList<AddonRecord> AddonLoaderHelper::filter(AddonLoader::ExportType type){
    auto instance =  AddonLoader::getInstance();
    return instance->d->filter(type);
}
}
