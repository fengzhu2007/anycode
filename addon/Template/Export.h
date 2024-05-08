#ifndef {TPL}EXPORT_H
#define {TPL}EXPORT_H
#include "{TPL}_global.h"
#include "interface/Panel.h"
#include "interface/FormPanel.h"
#include "network/NetworkRequest.h"


#ifdef __cplusplus
extern "C" {
#endif

{TPL}_EXPORT ady::Panel* getPanel(long long id,QWidget* parent,QString name);
{TPL}_EXPORT size_t getFormPanelSize(QString);
{TPL}_EXPORT ady::FormPanel* getFormPanel(QWidget* parent,QString,size_t n);
{TPL}_EXPORT ady::NetworkRequest* getRequest(QString name);
{TPL}_EXPORT int requestConnect(void* ptr);

#ifdef __cplusplus
}
#endif


#endif // {TPL}EXPORT_H
