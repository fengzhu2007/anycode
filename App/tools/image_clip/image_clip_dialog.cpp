#include "image_clip_dialog.h"
#include "ui_image_clip_dialog.h"
#include "resize_tab.h"
#include "scale_tab.h"
#include "cut_tab.h"
#include "workflow_tab.h"
#include "workflow_model.h"
#include "components/message_dialog.h"
#include <QDebug>
namespace ady{
ImageClipDialog* ImageClipDialog::instance = nullptr;

class ImageClipDialogPrivate{

public:
    ImageProcessThread* thread=nullptr;
    WorkflowTab* workflow = nullptr;
};

ImageClipDialog::ImageClipDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::ImageClipDialog)
{
    d = new ImageClipDialogPrivate;
    ui->setupUi(this);
    ui->addToWorkflow->setStyleSheet("min-width:120px");

    this->resetupUi();

    this->initView();

    connect(ui->ok,&QPushButton::clicked,this,&ImageClipDialog::onOk);
    connect(ui->addToWorkflow,&QPushButton::clicked,this,&ImageClipDialog::onAddToWorkflow);
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
    auto cutTab = new CutTab(this);
    ui->tabWidget->addTab(cutTab,tr("Cut"));

    d->workflow = new WorkflowTab(this);
    ui->tabWidget->addTab(d->workflow,tr("Workflow"));
}

void ImageClipDialog::onAddToWorkflow(){
     auto current = ui->tabWidget->currentIndex();

    if(current==ImageProcessThread::ProcessName::Scale){
        auto scaleTab = static_cast<ScaleTab*>(ui->tabWidget->currentWidget());
        WorkflowData data{ImageProcessThread::ProcessName::Scale,tr("Scale"), ScaleOption{scaleTab->optionWidth(),scaleTab->optionHeight()}};

        d->workflow->addFlow(data);

    }else if(current==ImageProcessThread::ProcessName::Resize){
        auto resizeTab = static_cast<ResizeTab*>(ui->tabWidget->currentWidget());

        WorkflowData data{ImageProcessThread::ProcessName::Resize,tr("Resize"),ResizeOption{resizeTab->optionLeft(),resizeTab->optionTop(),resizeTab->optionRight(),resizeTab->optionBottom(),resizeTab->optionRelative()}};
        d->workflow->addFlow(data);
    }else if(current==ImageProcessThread::ProcessName::Cut){
        auto cutTab = static_cast<CutTab*>(ui->tabWidget->currentWidget());

        WorkflowData data{ImageProcessThread::ProcessName::Cut,tr("Cut"),CutOption{cutTab->optionLeft(),cutTab->optionTop(),cutTab->optionRight(),cutTab->optionBottom(),cutTab->optionWidth(),cutTab->optionHeight()}};
        d->workflow->addFlow(data);
    }
    ui->tabWidget->setCurrentIndex(ImageProcessThread::ProcessName::Workflow);
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
        }else if(current==ImageProcessThread::ProcessName::Cut){
            auto cutTab = static_cast<ResizeTab*>(ui->tabWidget->currentWidget());
            d->thread->setCutParams(cutTab->optionLeft(),cutTab->optionTop(),cutTab->optionRight(),cutTab->optionBottom());
        }else if(current==ImageProcessThread::ProcessName::Workflow){
            auto list = d->workflow->queue();
            if(list.size()>0){
                for(auto item:list){
                    if(item.name==ImageProcessThread::ProcessName::Scale){
                        auto option = item.scale;
                        d->thread->setScaleParams(option.width,option.height);
                    }else if(item.name==ImageProcessThread::ProcessName::Resize){
                        auto option = item.resize;
                        d->thread->setResizeParams(option.left,option.top,option.right,option.bottom,option.relative);
                    }else if(item.name==ImageProcessThread::ProcessName::Cut){
                        auto option = item.cut;
                        d->thread->setCutParams(option.left,option.top,option.right,option.bottom,option.width,option.height);
                    }
                }
            }else{
                MessageDialog::info(this,tr("Undefined processing flow!"));
                return ;
            }
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
