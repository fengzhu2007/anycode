#include "app_oss.h"
#include "core/theme.h"


namespace ady{


QString AppOSS::global(){



    /*return QString::fromUtf8(".QStatusBar{background: #007acc;}"
                             ".QStatusBar::item{border:0;}"
                             ".ady--PasswordEdit,.QLineEdit{height:24px;border:1px solid #ccc;font-size:12px;}"
                             ".ady--PasswordEdit:focus,.QLineEdit:focus,.ady--PasswordEdit:hover,.QLineEdit:hover{border:1px solid #007acc}"

#ifdef Q_OS_WIN
                             ".QSpinBox{border:1px solid #ccc;font-size:12px;height:24px;}"
                             ".QSpinBox:hover{border:1px solid #007acc}"
                             ".QSpinBox::up-button{background:#EEEEF2;}"
                             ".QSpinBox::up-button:hover{background:#c9def5;}"
                             ".QSpinBox::up-button:pressed{background:#007acc;}"
                             ".QSpinBox::down-button{background:#EEEEF2;}"
                             ".QSpinBox::down-button:hover{background:#c9def5;}"
                             ".QSpinBox::down-button:pressed{background:#007acc;}"
                             ".QSpinBox::up-arrow{image:url(:/Resource/icons/GlyphUp_16x.svg);width:14px;height:14px;}"
                             ".QSpinBox::down-arrow{image:url(:/Resource/icons/GlyphDown_16x.svg);width:14px;height:14px;}"
#else


#endif
                             ".QComboBox{height:24px}");*/
    return {};
}


QString AppOSS::options(){
    //201,222,245
    auto theme = Theme::getInstance();
    auto primaryColor = theme->primaryColor().name(QColor::HexRgb);
    auto secondaryColor = theme->secondaryColor().name(QColor::HexRgb);
    auto textColor = theme->textColor().name(QColor::HexRgb);
    return (".QListView{border:1px solid "+secondaryColor+"}"
            ".QListView::item{height:30px; }"
                             ".QListView::item:hover{border:1px solid "+secondaryColor+";background-color:"+secondaryColor+" }"
                             ".QListView::item:selected{border:1px solid "+primaryColor+";background-color:"+primaryColor+";color:"+textColor+";}"
                             ".QTabBar::tab{height:24px}");
}


}
