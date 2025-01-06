#include "theme.h"


namespace ady{

Theme* Theme::instance = nullptr;

Theme::Theme() {

}

Theme::~Theme(){

}

void Theme::destory(){
    if(instance!=nullptr){
        delete instance;
        instance = nullptr;
    }
}

void Theme::setup(QApplication& app){

}

}
