#include "search_combobox.h"
#include <QStringListModel>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QDebug>
namespace ady{
class SearchComboBoxPrivate{
public:
    QStringListModel* model;


};



SearchComboBox::SearchComboBox(QWidget* parent):QComboBox(parent) {

   // d = new SearchComboBoxPrivate;
    connect(this->view(),&QAbstractItemView::pressed,this,&SearchComboBox::onIndexSelected);
}

void SearchComboBox::addItem(const QString& text){
    int count = this->count();
    for(int i=0;i<count;i++){
        auto one = this->itemText(i);
        if(one==text){
            this->removeItem(i);
            break;
        }
    }
    this->insertItem(0,text);

}


void SearchComboBox::onIndexSelected(const QModelIndex& index){
    const QString text = this->itemText(index.row());
    emit search(text);
    //this->addItem(text);
    this->setCurrentText(text);
}

void SearchComboBox::keyPressEvent(QKeyEvent* e){
    QComboBox::keyPressEvent(e);
    if(e->key()==Qt::Key_Return){
        const QString text = this->currentText().trimmed();
        emit search(text);
        if(!text.isEmpty()){

            //add to model
            //this->addItem(text);
        }
    }
}

}
