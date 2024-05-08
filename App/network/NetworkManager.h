#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H
#include "global.h"
#include "NetworkRequest.h"
#include <QMap>
namespace ady {
    class NetworkRequest;
    class Task;
    class ANYENGINE_EXPORT NetworkManager
    {
    private:
        NetworkManager();
    public:
        static NetworkManager* getInstance();
        NetworkRequest* request(long long id);
        void clear();
        void remove(long long id);
        int exec(Task* task);

        template<class T>
        T* newRequest(long long id=0)
        {
            T* request =  new T(curl_easy_init());
            if(id!=0){
                this->requests[id] = request;
            }
            return request;
        }

    private:
        static NetworkManager* instance;
        QMap<long long ,NetworkRequest*> requests;


    };
}
#endif // NETWORKMANAGER_H
