#include "frames_merge_dialog.h"
#include "ui_frames_merge_dialog.h"

#include "frames_model.h"
#include <w_toast.h>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QVector>
#include <cmath>
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
    connect(ui->margin,QOverload<int>::of(&QSpinBox::valueChanged),this,&FramesMergeDialog::onColumnsChanged);
    connect(ui->separation,QOverload<int>::of(&QSpinBox::valueChanged),this,&FramesMergeDialog::onColumnsChanged);
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
    auto margin = ui->margin->value();           // 合并图四周保留的空白边距
    auto separation = ui->separation->value();   // 图像之间上下/左右的间隔
    int total = list.size();
    if(total == 0){
        ui->image->load(QPixmap());
        return;
    }

    int rows = std::ceil(total * 1.0 / columns);

    // 逐行统计：行高取该行最高图像，行宽为该行所有图像宽度加上图像之间的间隔
    QVector<int> rowHeights(rows, 0);
    int width = 0;
    int height = 0;
    for(int i=0;i<rows;i++){
        int h = 0;
        int w = 0;
        int count = 0;
        for(int j=0;j<columns;j++){
            int index = i * columns + j;
            if(index<total){
                auto item = list.at(index);
                auto img = item.image;
                h = std::max(h,img.height());
                if(count > 0){
                    w += separation;
                }
                w += img.width();
                count++;
            }
        }
        rowHeights[i] = h;
        height += h;
        width = std::max(width,w);
    }
    // 行与行之间的间隔，以及四周的边距
    height += (rows - 1) * separation;
    width += margin * 2;
    height += margin * 2;

    d->image = QImage(width,height,QImage::Format_ARGB32);
    d->image.fill(Qt::transparent);
    QPainter painter(&d->image);
    int offsetY = margin;
    for(int i=0;i<rows;i++){
        int offsetX = margin;
        for(int j=0;j<columns;j++){
            int index = i * columns + j;
            if(index<total){
                auto item = list.at(index);
                auto img = item.image;
                painter.drawImage(offsetX, offsetY,img.toImage());
                offsetX += img.width() + separation;
            }
        }
        offsetY += rowHeights[i] + separation;
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
