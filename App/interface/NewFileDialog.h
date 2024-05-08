#ifndef NEWFILEDIALOG_H
#define NEWFILEDIALOG_H
#include <QDialog>
#include "global.h"
namespace Ui {
class NewFileDialog;
}
namespace ady {
    class ANYENGINE_EXPORT NewFileDialog : public QDialog{

        Q_OBJECT
        public:
            enum Type{
                File=0,
                Folder

            };

            NewFileDialog(Type type,QWidget* parent=nullptr);
            ~NewFileDialog();
            QString fileName();
            void setName(QString name);
            void setPath(QString path);

    public slots:
            void onOK();
            void onNameChanged(const QString& name);

        private:
            Ui::NewFileDialog* ui;
            QString m_path;
            QString m_name;

    };
}
#endif // NEWFILEDIALOG_H
