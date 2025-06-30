#include "image_process_thread.h"
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QPainter>

namespace ady{

struct ScaleOption{
    int width;
    int height;
};

struct ResizeOption{
    int left;
    int top;
    int right;
    int bottom;
    bool relative;
};

struct CutOption{
    int left;
    int top;
    int right;
    int bottom;
};

class ImageProcessThreadPrivate{
public:
    ImageProcessThread::ProcessName name;
    QString source;
    QString destination;
    ScaleOption scale;
    ResizeOption resize;
    CutOption cut;

    QImage process;
};

ImageProcessThread::ImageProcessThread(ProcessName name,const QString& source,const QString& destination,QObject* parent):QThread(parent){
    d = new ImageProcessThreadPrivate;
    d->source = source;
    d->destination = destination;
    d->name = name;
}

ImageProcessThread::~ImageProcessThread(){
    delete d;
}

void ImageProcessThread::setScaleParams(int width,int height){
    d->scale.width = width;
    d->scale.height = height;
}

void ImageProcessThread::setResizeParams(int left,int top,int right,int bottom,bool relative){
    d->resize.left = left;
    d->resize.top = top;
    d->resize.right = right;
    d->resize.bottom = bottom;
    d->resize.relative = relative;
}

void ImageProcessThread::setCutParams(int left,int top,int right,int bottom){
    d->cut.left = left;
    d->cut.top = top;
    d->cut.right = right;
    d->cut.bottom = bottom;
}
void ImageProcessThread::run(){
    QDir dir(d->source);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    // QDir output(d->destination);
    // if(!output.exists()){
    //     output.mkdir();
    // }

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
    for(auto file:list){
        if(isInterruptionRequested()){
            break;
        }
        if(d->name==Scale){
            this->scale(file);
        }else if(d->name==Resize){
            this->resize(file);
        }else if(d->name==Cut){
            this->cut(file);
        }
    }
}

void ImageProcessThread::scale(const QFileInfo& fi){
    auto path = fi.absoluteFilePath();
    if(d->process.load(path)){
        QString extension = fi.suffix().toUpper();
        if(extension!="PNG"){
            extension = "JPG";
        }
        QImage scaledImage = d->process.scaled(QSize(d->scale.width,d->scale.height),
                                                  Qt::KeepAspectRatio,
                                                  Qt::SmoothTransformation);
        auto outputPath = d->destination + "/"+fi.fileName();
        if (scaledImage.save(outputPath, extension.toStdString().c_str())) {

            emit finishOne(d->name,ProcessResult::OK,path,outputPath);
        }else{
            emit finishOne(d->name,ProcessResult::Failed,path,outputPath);
        }
    }
}

void ImageProcessThread::resize(const QFileInfo& fi){
    auto path = fi.absoluteFilePath();
    if(d->process.load(path)){
        QString extension = fi.suffix().toUpper();
        if(extension!="PNG"){
            extension = "JPG";
        }
        auto width = d->process.width() + d->resize.left + d->resize.right;
        auto height = d->process.height() + d->resize.top + d->resize.bottom;

        QImage resizeImage(width,height,QImage::Format_ARGB32);
        resizeImage.fill(Qt::transparent);
        QPainter painter(&resizeImage);
        painter.drawImage(d->resize.left, d->resize.top, d->process);
        //painter.end();
        auto outputPath = d->destination + "/"+fi.fileName();
        if (resizeImage.save(outputPath, extension.toStdString().c_str())) {

            emit finishOne(d->name,ProcessResult::OK,path,outputPath);
        }else{
            emit finishOne(d->name,ProcessResult::Failed,path,outputPath);
        }
    }
}

void ImageProcessThread::cut(const QFileInfo& fi){
    auto path = fi.absoluteFilePath();
    if(d->process.load(path)){
        QString extension = fi.suffix().toUpper();
        if(extension!="PNG"){
            extension = "JPG";
        }
        auto width = d->process.width() - d->cut.left - d->cut.right;
        auto height = d->process.height()-d->cut.top - d->cut.bottom;
        QRect cropRect(d->cut.left, d->cut.top, width, height);
        QImage croppedImage = d->process.copy(cropRect);
        auto outputPath = d->destination + "/"+fi.fileName();
        if (croppedImage.save(outputPath, extension.toStdString().c_str())) {

            emit finishOne(d->name,ProcessResult::OK,path,outputPath);
        }else{
            emit finishOne(d->name,ProcessResult::Failed,path,outputPath);
        }
    }
}


}
