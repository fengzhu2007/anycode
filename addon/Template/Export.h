#ifndef {TPL}EXPORT_H
#define {TPL}EXPORT_H
#include "{tpl}_global.h"
#include "interface/form_panel.h"
#include "network/network_request.h"


#ifdef __cplusplus
extern "C" {
#endif

{TPL}_EXPORT size_t getFormPanelSize(QString);
{TPL}_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
{TPL}_EXPORT ady::NetworkRequest* getRequest(QString name);
{TPL}_EXPORT int requestConnect(void* ptr);

#ifdef __cplusplus
}
#endif


#endif // {TPL}EXPORT_H
