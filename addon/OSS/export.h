#ifndef OSSEXPORT_H
#define OSSEXPORT_H
#include "oss_global.h"
#include "interface/Panel.h"
#include "interface/FormPanel.h"
#include "network/network_request.h"

#ifdef __cplusplus
extern "C" {
#endif

OSS_EXPORT ady::Panel* getPanel(long long id,QWidget* parent,QString name);
OSS_EXPORT size_t getFormPanelSize(QString);
OSS_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
OSS_EXPORT ady::NetworkRequest* getRequest(QString name);
OSS_EXPORT int requestConnect(void* ptr);
OSS_EXPORT ady::NetworkRequest* initRequest(long long id);

#ifdef __cplusplus
}
#endif



#endif // OSSEXPORT_H