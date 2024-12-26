#ifndef S3EXPORT_H
#define S3EXPORT_H
#include "s3_global.h"
#include "interface/form_panel.h"
#include "network/network_request.h"


#ifdef __cplusplus
extern "C" {
#endif

S3_EXPORT size_t getFormPanelSize(QString);
S3_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
S3_EXPORT ady::NetworkRequest* getRequest(QString name);
S3_EXPORT int requestConnect(void* ptr);
S3_EXPORT ady::NetworkRequest* initRequest(long long id);
S3_EXPORT int install();
S3_EXPORT int uninstall();

#ifdef __cplusplus
}
#endif


#endif // S3EXPORT_H
