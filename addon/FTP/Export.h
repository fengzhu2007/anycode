#ifndef FTPEXPORT_H
#define FTPEXPORT_H
#include "FTP_global.h"
#include "interface/Panel.h"
#include "interface/FormPanel.h"
#include "network/network_request.h"
#include <memory>


#ifdef __cplusplus
extern "C" {
#endif

FTP_EXPORT ady::Panel* getPanel(long long id,QWidget* parent,QString name);
FTP_EXPORT size_t getFormPanelSize(QString);
FTP_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
FTP_EXPORT ady::NetworkRequest* getRequest(QString name);
FTP_EXPORT int requestConnect(void* ptr);
FTP_EXPORT ady::NetworkRequest* initRequest(long long id);

#ifdef __cplusplus
}
#endif


#endif // FTPEXPORT_H
