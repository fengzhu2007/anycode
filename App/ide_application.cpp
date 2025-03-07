#include "ide_application.h"
#include "core/theme.h"
#include "modules/options/options_settings.h"
#include "modules/options/environment_settings.h"
#include "themes/light_theme.h"
#include "themes/dark_theme.h"
#include <w_qss.h>
#include <QTranslator>
namespace ady{
IDEApplication::IDEApplication(int &argc, char **argv):QApplication(argc,argv) {

    OptionsSettings::init();//init options settings
    auto settings = OptionsSettings::getInstance()->environmentSettings();
    if(settings.m_theme==QLatin1String("dark")){
        Theme::init<DarkTheme>();
        wQSS::init(wQSS::Dark);
    }else{
        Theme::init<LightTheme>();
        wQSS::init(wQSS::Light);
    }
    Theme::getInstance()->setup(*this);

    QString dir = QCoreApplication::applicationDirPath();
    {
        QTranslator* translator = new QTranslator(this);
        if(translator->load(dir + QString("/translations/App_%1.qm").arg(settings.m_language))){
            this->installTranslator(translator);
        }
    }
    {
        QTranslator* translator = new QTranslator(this);
        if(translator->load(dir + QString("/translations/qt_%1.qm").arg(settings.m_language))){
            this->installTranslator(translator);
        }
    }
    {
        QTranslator* translator = new QTranslator(this);
        if(translator->load(dir + QString("/translations/qt_help_%1.qm").arg(settings.m_language))){
            this->installTranslator(translator);
        }
    }
}

IDEApplication::~IDEApplication(){
    OptionsSettings::destory();
    wQSS::destory();
}



}
