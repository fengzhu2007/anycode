#include "ai_option_widget.h"
#include "ui_ai_option_widget.h"
#include "options_settings.h"
#include "ai_settings.h"
#include "components/select_model.h"
#include "components/list_item_delegate.h"
#include <QIcon>
#include <QAbstractItemView>


namespace ady{
class AIOptionWidgetPrivate{
public:

};

AIOptionWidget::AIOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::AIOptionWidget)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("AI Assistant"));
    this->setWindowIcon(QIcon(":/Resource/icons/CordovaMultiDevice_16x.svg"));

    ui->name->view()->setItemDelegate(new ListItemDelegate(22,ui->name));
    ui->model->view()->setItemDelegate(new ListItemDelegate(22,ui->model));
    ui->triggerPolicy->view()->setItemDelegate(new ListItemDelegate(22,ui->triggerPolicy));

    connect(ui->enableAIAssiant,&QCheckBox::clicked,this,&AIOptionWidget::onEnabled);

    this->initView();
}

AIOptionWidget::~AIOptionWidget()
{
    delete ui;
}


QString AIOptionWidget::name(){
    return AISettings::name();
}

void AIOptionWidget::apply(int *state){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->aiSettings();
    bool changed = false;
    {
        if(setting.m_enable!=ui->enableAIAssiant->isChecked()){
            changed = true;
            setting.m_enable = !setting.m_enable;
        }
    }
    {
        int index = ui->name->currentIndex();
        auto model = static_cast<SelectModel<QString>*>(ui->name->model());
        auto value = model->value(index);
        if(setting.m_name!=value){
            setting.m_name = value;
            changed = true;
        }
    }
    {
        int index = ui->model->currentIndex();
        auto model = static_cast<SelectModel<QString>*>(ui->model->model());
        auto value = model->value(index);
        if(setting.m_model!=value){
            setting.m_model = value;
            changed = true;
        }
    }

    {
        int index = ui->triggerPolicy->currentIndex();
        if(setting.m_triggerPolicy!=index){
            setting.m_triggerPolicy = index;
            changed = true;
        }
    }

    auto apiKey = ui->apiKey->text();
    if(setting.m_apiKey!=apiKey){
        setting.m_apiKey = apiKey;
        changed = true;
    }

    auto timeout = ui->timeout->value();
    if(setting.m_triggerTimeout!=timeout){
        setting.m_triggerTimeout = timeout;
        changed = true;
    }


    if(changed){
        instance->setAiSettings(setting);
    }
}

void AIOptionWidget::initValue(const QJsonObject& value){

}

QJsonObject AIOptionWidget::toJson() {
    return OptionsSettings::getInstance()->aiSettings().toJson();
}

void AIOptionWidget::initView(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->aiSettings();
    {
        auto model = new SelectModel<QString>(ui->name);
        model->setDataSource(AISettings::servers());
        ui->name->setModel(model);
    }
    {
        auto model = new SelectModel<QString>(ui->model);
        model->setDataSource(AISettings::models({}));
        ui->model->setModel(model);
    }

    ui->apiKey->setText(setting.m_apiKey);
    ui->enableAIAssiant->setChecked(setting.m_enable);
    ui->timeout->setValue(setting.m_triggerTimeout);

    QList<QPair<int,QString>> list;
    list.append({AISettings::Auto,tr("Auto")});
    list.append({AISettings::Manual,tr("Manual")});
    {
        auto model = new SelectModel<int>(ui->model);
        model->setDataSource(list);
        ui->triggerPolicy->setModel(model);
    }
    ui->triggerPolicy->setCurrentIndex(setting.m_triggerPolicy);
    this->onEnabled(setting.m_enable);
}

void AIOptionWidget::onEnabled(bool checked){
    ui->name->setEnabled(checked);
    ui->model->setEnabled(checked);
    ui->triggerPolicy->setEnabled(checked);
    ui->timeout->setEnabled(checked);
    ui->apiKey->setEnabled(checked);
}

}
