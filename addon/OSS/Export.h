#ifndef OSSEXPORT_H
#define OSSEXPORT_H
#include "OSS_global.h"
#include "interface/Panel.h"
#include "interface/FormPanel.h"
#include "network/NetworkRequest.h"

#ifdef __cplusplus
extern "C" {
#endif

OSS_EXPORT ady::Panel* getPanel(long long id,QWidget* parent,QString name);
OSS_EXPORT size_t getFormPanelSize(QString);
OSS_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
OSS_EXPORT ady::NetworkRequest* getRequest(QString name);
OSS_EXPORT int requestConnect(void* ptr);

#ifdef __cplusplus
}
#endif



#endif // OSSEXPORT_H
