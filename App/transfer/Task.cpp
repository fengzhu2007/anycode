#include "Task.h"
namespace ady {
    long long Task::seq = 1l;

    constexpr const  char Task::MODE[];
    constexpr const  char Task::APPLY_CHILDREN[] ;


    Task::Task()
    {
        this->id = Task::seq++;
        this->siteid = 0l;
        this->filesize = 0l;
        this->readysize = 0l;
        this->file = nullptr;
        this->type = 0;//0=file 1=dir
        //this->direction = 0;//0=upload 1=download 2=delete
        this->cmd = -1;//-1 unknow 0=upload 1=download 2=del 3=chmod
        this->status = 0;
        this->abort = false;

    }

    Task::Task(long long siteid,QString local,QString remote)
    {
        Task();
        this->siteid = siteid;
        this->local = local;
        this->remote = remote;
    }
}
