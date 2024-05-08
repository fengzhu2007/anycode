#ifndef DIRCOMBOBOX_H
#define DIRCOMBOBOX_H
#include "../global.h"
#include <QComboBox>
#include <QTreeView>
#include <QDirModel>

namespace ady {
    class DirModel : public QDirModel
    {
        Q_OBJECT
    public:
        enum Column {
            Name=0,
            Max
        };
        DirModel(QObject* parent=nullptr);
        virtual int  columnCount(const QModelIndex &parent = QModelIndex()) const override;



    };


    class DirTreeView : public QTreeView
    {
        Q_OBJECT
    public:
        DirTreeView(QWidget* parent=nullptr);
        inline QString currentText(){return m_currentText;}


    protected:

        virtual void leaveEvent(QEvent *event);

        virtual void focusOutEvent(QFocusEvent *event);

        virtual bool event(QEvent *event) override;




    private:
        QString m_currentText;

    };





    class ANYENGINE_EXPORT DirComboBox : public QComboBox
    {
        Q_OBJECT
    public:
        DirComboBox(QWidget* parent=nullptr);
        void setPath(const QString & path);
        virtual void showPopup() override;
        virtual void hidePopup() override;

        virtual bool eventFilter(QObject* object, QEvent* event) override;



    public slots:
        void onActivated(const QModelIndex &index);
        void onPathChanged(QString & path);

    protected:

    private:
        DirTreeView* m_popupView;
        bool skipNextHide;
        QString m_selectedText;
    };
}



#endif // DIRCOMBOBOX_H
