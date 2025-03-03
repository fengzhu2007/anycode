#ifndef SQL_PANE_H
#define SQL_PANE_H

#include <docking_pane.h>
#include "../code_editor/editor.h"
namespace Ui {
class SQLPane;
}

namespace ady{
class SQLPanePrivate;
class SQLPane : public Editor
{
    Q_OBJECT

public:
    explicit SQLPane(QWidget *parent = nullptr);
    ~SQLPane() override;

    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual QString description() override;
    virtual void activation() override;
    virtual bool save(bool rename) override;
    virtual void contextMenu(const QPoint& pos) override;
    virtual QJsonObject toJson() override;
    virtual bool closeEnable() override;
    virtual bool doAction(int a) override;

    void rename(const QString& name) override;
    void autoSave() override;
    bool readFile(const QString& path) override;
    QString path() override;//editor
    bool isModified() const;
    CodeEditorView* editor() override;

    int fileState() override;
    void setFileState(int state) override;
    void invokeFileState() override;
    bool isModification() override;
    void reload() override;

    static SQLPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:
    SQLPanePrivate* d;
    Ui::SQLPane *ui;
    static int SN;
};
}
#endif // SQL_PANE_H
