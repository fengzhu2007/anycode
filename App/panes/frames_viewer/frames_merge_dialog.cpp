#include "frames_merge_dialog.h"
#include "ui_frames_merge_dialog.h"

#include "frames_model.h"
#include <w_toast.h>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
namespace ady{

FramesMergeDialog* FramesMergeDialog::instance = nullptr;
class FramesMergeDialogPrivate{

public:
    QImage image;
    FramesModel* model;
    QString outputDir;
};

FramesMergeDialog::FramesMergeDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::FramesMergeDialog)
{
    d = new FramesMergeDialogPrivate;
    d->model = nullptr;
    ui->setupUi(this);

    this->resetupUi();

    connect(ui->save,&QPushButton::clicked,this,&FramesMergeDialog::onSave);
    connect(ui->columns,QOverload<int>::of(&QSpinBox::valueChanged),this,&FramesMergeDialog::onColumnsChanged);
}

FramesMergeDialog::~FramesMergeDialog()
{
    instance = nullptr;
    delete ui;
    delete d;
}

void FramesMergeDialog::saveTo(const QString& outputDir){
    d->outputDir = outputDir;
}

FramesMergeDialog* FramesMergeDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new FramesMergeDialog(parent);
    }
    instance->setModal(true);
    return instance;
}

void FramesMergeDialog::setModel(FramesModel* model){
    d->model = model;
    this->merge();

}

void FramesMergeDialog::merge(){
    auto list = d->model->results();
    auto columns = ui->columns->value();
    qDebug()<<"columns"<<columns;
    int width = 0;
    int height = 0;
    int total = list.size();
    int rows = std::ceil(total * 1.0 / columns);

    for(int i=0;i<rows;i++){
        int h = 0;
        int w = 0;
        for(int j=0;j<columns;j++){
            int index = i * columns + j;
            if(index<total){
                auto img = d->model->image(index);
                h = std::max(h,img.height());
                w += img.width();
            }
        }
        height += h;
        width = std::max(width,w);
    }
    d->image = QImage(width,height,QImage::Format_ARGB32);
    d->image.fill(Qt::transparent);
    QPainter painter(&d->image);
    int offsetY = 0;
    int offsetX = 0;
    int hh = 0;
    for(int i=0;i<total;i++){
        if(i%columns==0){
            hh = 0;
            offsetX = 0;
        }
        auto img = d->model->image(i);
        hh = std::max(hh,img.height());


        painter.drawImage(offsetX, offsetY,img.toImage());

        offsetX += img.width();
        if(i%columns==columns-1){
            offsetY += hh;
        }
    }
    painter.end();
    ui->image->load(QPixmap::fromImage(d->image));
   // ui->image->setPix
}


void FramesMergeDialog::onSave(){
    auto outputPath = QFileDialog::getSaveFileName(this, tr("Save Image"), d->outputDir, tr("PNG Image Files (*.png)"));
    if(outputPath.isEmpty()==false){
        if (d->image.save(outputPath, "PNG")) {
            wToast::showText(tr("Save Successfully"));
        }
    }

}

void FramesMergeDialog::onColumnsChanged(int n){
    this->merge();

}


}
