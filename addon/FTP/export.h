#ifndef FTPEXPORT_H
#define FTPEXPORT_H
#include "ftp_global.h"
#include "interface/form_panel.h"
#include "network/network_request.h"
#include <memory>


#ifdef __cplusplus
extern "C" {
#endif

FTP_EXPORT size_t getFormPanelSize(QString);
FTP_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
FTP_EXPORT ady::NetworkRequest* getRequest(QString name);
FTP_EXPORT int requestConnect(void* ptr);
FTP_EXPORT ady::NetworkRequest* initRequest(long long id);

#ifdef __cplusplus
}
#endif


#endif // FTPEXPORT_H
