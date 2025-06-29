#include "image_clip_dialog.h"
#include "ui_image_clip_dialog.h"
#include "resize_tab.h"
#include "scale_tab.h"
#include "components/message_dialog.h"
#include <QDebug>
namespace ady{
ImageClipDialog* ImageClipDialog::instance = nullptr;

class ImageClipDialogPrivate{

public:
    ImageProcessThread* thread=nullptr;
};

ImageClipDialog::ImageClipDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::ImageClipDialog)
{
    d = new ImageClipDialogPrivate;
    ui->setupUi(this);

    this->resetupUi();

    this->initView();

    connect(ui->ok,&QPushButton::clicked,this,&ImageClipDialog::onOk);
}

ImageClipDialog::~ImageClipDialog()
{
    if(d->thread!=nullptr){
        d->thread->requestInterruption();
        d->thread->wait();
    }
    delete ui;
    delete d;
}

void ImageClipDialog::initView(){
    auto scaleTab = new ScaleTab(this);
    ui->tabWidget->addTab(scaleTab,tr("Scale"));
    auto resizeTab = new ResizeTab(this);
    ui->tabWidget->addTab(resizeTab,tr("Resize"));
}


void ImageClipDialog::onOk(){
    if(d->thread==nullptr){
        auto current = ui->tabWidget->currentIndex();
        d->thread = new ImageProcessThread((ImageProcessThread::ProcessName)current,ui->source->text(),ui->destination->text(),this);

        connect(d->thread,&ImageProcessThread::finishOne,this,&ImageClipDialog::onFinishOne);
        connect(d->thread,&QThread::finished,d->thread,&ImageProcessThread::deleteLater);
        connect(d->thread,&QThread::finished,this,&ImageClipDialog::onProressComplete);

        connect(d->thread,&QThread::finished,[this](){
            d->thread = nullptr;
            //MessageDialog::info(this,tr("Process Successfully"));
        });

        if(current==ImageProcessThread::ProcessName::Scale){
            auto scaleTab = static_cast<ScaleTab*>(ui->tabWidget->currentWidget());
            d->thread->setScaleParams(scaleTab->optionWidth(),scaleTab->optionHeight());
        }else if(current==ImageProcessThread::ProcessName::Resize){
            auto resizeTab = static_cast<ResizeTab*>(ui->tabWidget->currentWidget());
            d->thread->setResizeParams(resizeTab->optionLeft(),resizeTab->optionTop(),resizeTab->optionRight(),resizeTab->optionBottom(),resizeTab->optionRelative());
        }
        d->thread->start();
        ui->progress->start();
    }
}

void ImageClipDialog::onFinishOne(int name,int result,const QString& source,const QString& destination){
    qDebug()<<"path:"<<source<<destination;
}

void ImageClipDialog::onProressComplete(){
    d->thread = nullptr;
    ui->progress->stop();
    MessageDialog::info(this,tr("Process Successfully"));
}


ImageClipDialog* ImageClipDialog::getInstance(){
    return instance;
}

ImageClipDialog* ImageClipDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new ImageClipDialog(parent);
    }
    instance->show();
    return instance;
}
}
