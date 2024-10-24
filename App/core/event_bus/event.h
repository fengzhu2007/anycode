#ifndef EVENT_H
#define EVENT_H
#include "global.h"
#include <QString>
#include <QJsonValue>
namespace ady{

class EventPrivate;
class ANYENGINE_EXPORT Event
{
public:
    explicit Event(const QString& id);
    explicit Event(const QString& id,void *data);
    explicit Event(const QString& id,QJsonValueRef& data);
    virtual ~Event();
    void ignore();
    bool isIgnore();
    bool isJsonData();
    QJsonValue jsonData();

    template<class T>
    QJsonValue toJsonOf(){
        if(!this->isJsonData()){
            void *data = this->data();
            if(data==nullptr){
                return {};
            }else{
                return static_cast<T*>(data)->toJson();
            }
        }else{
            return this->jsonData();
        }
    }
    const QString id();
    void* data();


private:
    EventPrivate* d;

};

}



#endif // EVENT_H
