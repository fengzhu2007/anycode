#ifndef IMAGE_PROCESS_THREAD_H
#define IMAGE_PROCESS_THREAD_H
#include <QThread>
#include <QFileInfo>
namespace ady{

struct ScaleOption{
    int width;
    int height;

    bool operator==(const ScaleOption& other)  const{
        return this->width==other.width && this->height==other.height;
    }
};

struct ResizeOption{
    int left;
    int top;
    int right;
    int bottom;
    bool relative;
    bool operator==(const ResizeOption& other) const{
        return this->left==other.left && this->top==other.top && this->right==other.right && this->bottom==other.bottom && this->relative==other.relative;
    }
};

struct CutOption{
    int left;
    int top;
    int right;
    int bottom;
    int width;
    int height;
    bool operator==(const CutOption& other) const{
        return this->left==other.left && this->top==other.top && this->right==other.right && this->bottom==other.bottom && this->width==other.width && this->height==other.height;
    }
};


class ImageProcessThreadPrivate;

class ImageProcessThread : public QThread
{
    Q_OBJECT
public:
    enum ProcessName{
        Scale=0,
        Resize,
        Cut,
        Workflow,
    };
    enum ProcessResult{
        OK,
        Ignore,
        Failed
    };

    ImageProcessThread(ProcessName name,const QString& source,const QString& destination,QObject* parent);
    ~ImageProcessThread();



    virtual void run();

    void setScaleParams(int width,int height);
    void setResizeParams(int left,int top,int right,int bottom,bool relative);
    void setCutParams(int left,int top,int right,int bottom,int width=0,int height=0);


signals:
    void finishOne(int name,int result,const QString& from,const QString& to);

private:
    void scale(const QFileInfo& fi);
    void resize(const QFileInfo& fi);
    void cut(const QFileInfo& fi);
    void workflow(const QFileInfo& fi);
private:
    ImageProcessThreadPrivate* d;

};
}
#endif // IMAGE_PROCESS_THREAD_H
