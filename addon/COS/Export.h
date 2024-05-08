#ifndef COSEXPORT_H
#define COSEXPORT_H
#include "COS_global.h"
#include "interface/Panel.h"
#include "interface/FormPanel.h"
#include "network/NetworkRequest.h"


#ifdef __cplusplus
extern "C" {
#endif

COS_EXPORT ady::Panel* getPanel(long long id,QWidget* parent,QString name);
COS_EXPORT size_t getFormPanelSize(QString);
COS_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
COS_EXPORT ady::NetworkRequest* getRequest(QString name);
COS_EXPORT int requestConnect(void* ptr);

#ifdef __cplusplus
}
#endif


#endif // COSEXPORT_H
