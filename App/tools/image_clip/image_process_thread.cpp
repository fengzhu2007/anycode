#include "image_process_thread.h"
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

namespace ady{



class ImageProcessThreadPrivate{
public:
    ImageProcessThread::ProcessName name;
    QString source;
    QString destination;
    ScaleOption scale;
    ResizeOption resize;
    CutOption cut;
    QList<ImageProcessThread::ProcessName> processlist;
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
    if(d->name==ProcessName::Workflow){
        d->processlist.append(ProcessName::Scale);
    }
}

void ImageProcessThread::setResizeParams(int left,int top,int right,int bottom,bool relative){
    d->resize.left = left;
    d->resize.top = top;
    d->resize.right = right;
    d->resize.bottom = bottom;
    d->resize.relative = relative;
    if(d->name==ProcessName::Workflow){
        d->processlist.append(ProcessName::Resize);
    }
}

void ImageProcessThread::setCutParams(int left,int top,int right,int bottom,int width,int height){
    d->cut.left = left;
    d->cut.top = top;
    d->cut.right = right;
    d->cut.bottom = bottom;
    d->cut.width = width;
    d->cut.height = height;
    if(d->name==ProcessName::Workflow){
        d->processlist.append(ProcessName::Cut);
    }
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
        }else if(d->name==Workflow){
            this->workflow(file);
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
        auto width = d->scale.width;
        auto height = d->scale.height;
        if(height==0 && width>0){
            height = d->process.height() * ( width * 1.0 / d->process.width() );
        }
        if(width==0 && height>0){
            width = d->process.width() * ( height * 1.0 / d->process.height() );
        }

        QImage scaledImage = d->process.scaled(QSize(width,height),
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
        if(d->cut.width>0){
            width = d->cut.width;
        }

        auto height = d->process.height()-d->cut.top - d->cut.bottom;
        if(d->cut.height>0){
            height = d->cut.height;
        }
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

void ImageProcessThread::workflow(const QFileInfo& fi){
    if(d->processlist.size()>0){
        auto path = fi.absoluteFilePath();
        if(d->process.load(path)){
            for(auto processname:d->processlist){
                if(processname==ProcessName::Scale){
                    auto width = d->scale.width;
                    auto height = d->scale.height;
                    if(height==0 && width>0){
                        height = d->process.height() * ( width * 1.0 / d->process.width() );
                    }
                    if(width==0 && height>0){
                        width = d->process.width() * ( height * 1.0 / d->process.height() );
                    }
                    qDebug()<<"width:"<<width<<height;

                    d->process = d->process.scaled(QSize(width,height),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
                }else if(processname==ProcessName::Resize){
                    auto width = d->process.width() + d->resize.left + d->resize.right;
                    auto height = d->process.height() + d->resize.top + d->resize.bottom;
                    QImage resizeImage(width,height,QImage::Format_ARGB32);
                    resizeImage.fill(Qt::transparent);
                    QPainter painter(&resizeImage);
                    painter.drawImage(d->resize.left, d->resize.top, d->process);
                    d->process = resizeImage;

                }else if(processname==ProcessName::Cut){
                    auto width = d->process.width() - d->cut.left - d->cut.right;
                    auto height = d->process.height()-d->cut.top - d->cut.bottom;

                    if(d->cut.width>0){
                        width = d->cut.width;
                    }

                    if(d->cut.height>0){
                        height = d->cut.height;
                    }


                    QRect cropRect(d->cut.left, d->cut.top, width, height);
                    d->process = d->process.copy(cropRect);
                }
            }
            QString extension = fi.suffix().toUpper();
            if(extension!="PNG"){
                extension = "JPG";
            }
            auto outputPath = d->destination + "/"+fi.fileName();
            if (d->process.save(outputPath, extension.toStdString().c_str())) {
                emit finishOne(d->name,ProcessResult::OK,path,outputPath);
            }else{
                emit finishOne(d->name,ProcessResult::Failed,path,outputPath);
            }
        }
    }
}




}
