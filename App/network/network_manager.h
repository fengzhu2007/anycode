#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H
#include "global.h"
#include "network_request.h"
#include <QMap>

namespace ady {
    class NetworkRequest;
    class Task;
    class NetworkManagerPrivate;
    class ANYENGINE_EXPORT NetworkManager
    {
    private:
        NetworkManager();
    public:
        ~NetworkManager();
        static NetworkManager* getInstance();
        static void destory();
        NetworkRequest* request(long long id);
        NetworkRequest* initRequest(long long id,const QString& type);

        bool contains(long long id);

        void clear();
        void remove(long long id);
        int exec(Task* task);

        template<class T>
        NetworkRequest* newRequest(long long id=0)
        {
            auto req = new T(curl_easy_init(),id);
            if(id!=0){
                //this->requests[id] = req;
                this->requests.insert(id,req);
            }
            return req;
        }

    private:
        static NetworkManager* instance;
        QMap<long long ,NetworkRequest*> requests;
        NetworkManagerPrivate* d;
    };
}
#endif // NETWORK_MANAGER_H
