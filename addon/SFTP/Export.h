#ifndef SFTPEXPORT_H
#define SFTPEXPORT_H
#include "sftp_global.h"
#include "interface/form_panel.h"
#include "network/network_request.h"


#ifdef __cplusplus
extern "C" {
#endif

SFTP_EXPORT size_t getFormPanelSize(QString);
SFTP_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
SFTP_EXPORT ady::NetworkRequest* getRequest(QString name);
SFTP_EXPORT int requestConnect(void* ptr);
SFTP_EXPORT ady::NetworkRequest* initRequest(long long id);

#ifdef __cplusplus
}
#endif


#endif // SFTPEXPORT_H
