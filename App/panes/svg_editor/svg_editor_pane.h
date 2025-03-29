#ifndef SVG_EDITOR_PANE_H
#define SVG_EDITOR_PANE_H

#include "global.h"
#include "../code_editor/editor.h"

namespace Ui {
class SVGEditorPane;
}
namespace ady{
class DockingPaneManager;
class SVGEditorPanePrivate;
class ANYENGINE_EXPORT SVGEditorPane : public Editor
{
    Q_OBJECT

public:
    explicit SVGEditorPane(QWidget *parent = nullptr);
    ~SVGEditorPane();
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

    virtual bool readFile(const QString& path) override;
    bool writeFile(const QString& path,bool autoSave=false);

    virtual QString path() override;
    virtual void rename(const QString& nname) override;
    virtual void autoSave() override;
    virtual bool isModification() override;
    virtual int fileState() override;
    virtual void setFileState(int state) override;
    virtual void invokeFileState() override;
    virtual void reload() override;
    virtual CodeEditorView* editor() override;


    static SVGEditorPane* make(DockingPaneManager* dockingManager,const QJsonObject& data={});

    void onZoom(int zoom);
public slots:
    void onModificationChanged(bool changed);
    void onCursorPositionChanged();
    void onFileOpend();
protected:
    virtual void showEvent(QShowEvent* e) override;
    virtual void resizeEvent(QResizeEvent* e) override;

private:
    void updateInfoBar();

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::SVGEditorPane *ui;
    SVGEditorPanePrivate* d;
    static int SN;
};
}
#endif // SVG_EDITOR_PANE_H
