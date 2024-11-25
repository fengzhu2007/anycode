#include "texteditor_option_widget.h"
#include "ui_texteditor_option_widget.h"
#include "texteditor/general_tab.h"
#include "texteditor/color_tab.h"
#include <QIcon>
#include <texteditorsettings.h>
#include <tabsettings.h>


namespace ady{
class TextEditorOptionWidgetPrivate{
public:
    QList<OptionTab*> list;
};

TextEditorOptionWidget::TextEditorOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::TextEditorOptionWidget)
{
    d = new TextEditorOptionWidgetPrivate;
    ui->setupUi(this);

    this->setWindowTitle(tr("TextEditor"));
    this->setWindowIcon(QIcon(":/Resource/icons/Editor_16x.svg"));
    this->initView();

}

TextEditorOptionWidget::~TextEditorOptionWidget()
{
    delete d;
    delete ui;
}


QString TextEditorOptionWidget::name(){
    return QLatin1String("texteditor");
}

void TextEditorOptionWidget::apply(){
    for(auto one:d->list){
        one->apply();
    }
}

void TextEditorOptionWidget::initValue(const QJsonObject& value){


}


QJsonObject TextEditorOptionWidget::toJson() {
    return TextEditor::TextEditorSettings::instance()->toJson();
}

void TextEditorOptionWidget::initView(){
    {
        auto tab = new GeneralTab(ui->tabWidget);
        ui->tabWidget->addTab(tab,tab->windowTitle());
        d->list<<tab;
    }
    {
        auto tab = new ColorTab(ui->tabWidget);
        ui->tabWidget->addTab(tab,tab->windowTitle());
        d->list<<tab;
    }
}

}
