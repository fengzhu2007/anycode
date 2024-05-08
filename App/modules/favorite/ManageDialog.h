#ifndef FAVORITEMANAGEDIALOG_H
#define FAVORITEMANAGEDIALOG_H
#include <QDialog>
namespace Ui {
    class FavoriteManageDialog;
}
namespace ady {
    class FavoriteManageDialog : public QDialog{
        Q_OBJECT
    public:
        FavoriteManageDialog(QWidget* parent,long long id);
        ~FavoriteManageDialog();

    signals:
        void onSaved();

    public slots:
        void onRename(const QModelIndex& index,QString name);
        void onContextMenu(const QPoint& point);
        void onDelete();
    private:
        long long m_id;
        Ui::FavoriteManageDialog *ui;

    };
}
#endif // FAVORITEMANAGEDIALOG_H
