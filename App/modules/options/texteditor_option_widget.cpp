#include "texteditor_option_widget.h"
#include "ui_texteditor_option_widget.h"
#include <QIcon>

namespace ady{
TextEditorOptionWidget::TextEditorOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::TextEditorOptionWidget)
{
    ui->setupUi(this);


    this->setWindowTitle(tr("TextEditor"));
    this->setWindowIcon(QIcon(":/Resource/icons/Editor_16x.svg"));
}

TextEditorOptionWidget::~TextEditorOptionWidget()
{
    delete ui;
}


QString TextEditorOptionWidget::name(){
    return QLatin1String("core::texteditor");
}



void TextEditorOptionWidget::initValue(const QJsonObject& value){


}


QJsonObject TextEditorOptionWidget::toJson() {

    return {};
}


}
