#ifndef FAVORITEADDDIALOG_H
#define FAVORITEADDDIALOG_H
#include <QDialog>

namespace Ui {
    class FavoriteAddDialog;
}
namespace ady {
    class FavoriteAddDialog : public QDialog
    {
        Q_OBJECT

    public:
        FavoriteAddDialog(QWidget* parent,long long id);
        ~FavoriteAddDialog();
        void setCurrentPath(QString path);

    signals:
        void onSaved();

    public slots:
        void onSave();

    private:
        Ui::FavoriteAddDialog *ui;
        long long m_id;

    };
}
#endif // FAVORITEADDDIALOG_H
