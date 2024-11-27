#include "app_oss.h"



namespace ady{


QString AppOSS::global(){
    return QString::fromUtf8(".QStatusBar::item{border:0}");
}


QString AppOSS::options(){
    //201,222,245
    return QString::fromUtf8(".QLineEdit{border:1px solid #ccc;height:24px;}"
                             ".QLineEdit:focus{border:1px solid #007acc}"
                             ".QComoboBox{height:44px;}"
                             ".QSpinBox{border:1px solid #ccc;}"
                             ".QSpinBox:hover{border:1px solid #007acc}"
                             ".QSpinBox::up-button{background:#EEEEF2;}"
                             ".QSpinBox::up-button:hover{background:#c9def5;}"
                             ".QSpinBox::up-button:pressed{background:#007acc;}"
                             ".QSpinBox::down-button{background:#EEEEF2;}"
                             ".QSpinBox::down-button:hover{background:#c9def5;}"
                             ".QSpinBox::down-button:pressed{background:#007acc;}"
                             ".QSpinBox::up-arrow{image:url(:/Resource/icons/GlyphUp_16x.svg);width:14px;height:14px;}"
                             ".QSpinBox::down-arrow{image:url(:/Resource/icons/GlyphDown_16x.svg);width:14px;height:14px;}"
                             ".QListView{padding:0;border:1px solid #ccc;}"
                             ".QListView::item{height:30px;border:1px solid white; }"
                             ".QListView::item:hover{border:1px solid #c9def5;background-color:#c9def5 }"
                             ".QListView::item:selected{border:1px solid #007acc;background-color:#007acc;color:#fff;}"
                             ".QTabBar::tab{height:24px}");
}


}
