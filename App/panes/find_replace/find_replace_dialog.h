#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H
#include "global.h"
#include <QDialog>
#include "w_dialog.h"
#include <QStringListModel>
#include <QHideEvent>
namespace Ui {
class FindReplaceDialog;
}

namespace ady{
class SearchScopeModel;
class FindReplaceDialogPrivate;
class ANYENGINE_EXPORT FindReplaceDialog : public wDialog
{
    Q_OBJECT

public:
    ~FindReplaceDialog();
    static FindReplaceDialog* getInstance();
    static FindReplaceDialog* open(QWidget* parent);
    QStringListModel* findModel();
    QStringListModel* replaceModel();
    QStringListModel* filePatternModel();
    QStringListModel* exclusionModel();
    SearchScopeModel* searchScodeModel();
    void addSearch(const QString& text);
    void addReplace(const QString& before,const QString& after);

    void setFindText(const QString& text);
    void setSearchScope(const QString& folder);
    void setMode(int mode);


protected:
    virtual void hideEvent(QHideEvent *event) override;

signals:
    void search(const QString& text,int flags,bool hightlight=true);
    void searchAll(const QString& text,const QString& scope,int flags,const QString& filter,const QString& exclusion);
    void replaceAll(const QString& before,const QString& after,const QString& scope,int flags,const QString& filter,const QString& exclusion);
    void cancelSearch();
    void replace(const QString& before,const QString& after,int flags,bool hightlight=true);

private:
    explicit FindReplaceDialog(QWidget *parent = nullptr);

private:
    Ui::FindReplaceDialog *ui;
    FindReplaceDialogPrivate* d;
    static FindReplaceDialog* instance;
};
}
#endif // FIND_REPLACE_DIALOG_H
