#include "ide_application.h"
#include "core/theme.h"
#include "modules/options/options_settings.h"
#include "modules/options/environment_settings.h"
#include "themes/light_theme.h"
#include "themes/dark_theme.h"
namespace ady{
IDEApplication::IDEApplication(int &argc, char **argv):QApplication(argc,argv) {

    OptionsSettings::init();//init options settings
    auto settings = OptionsSettings::getInstance()->environmentSettings();
    if(settings.m_theme==QLatin1String("dark")){
        Theme::init<DarkTheme>();
    }else{
        Theme::init<LightTheme>();
    }
}

IDEApplication::~IDEApplication(){
    OptionsSettings::destory();
}



}
