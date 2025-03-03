#ifndef IMAGE_EDITOR_PANE_H
#define IMAGE_EDITOR_PANE_H
#include "global.h"
#include "../code_editor/editor.h"

namespace Ui{
class ImageEditorPane;
}
namespace ady{
class DockingPaneManager;
class ImageEditorPanePrivate;
class ANYENGINE_EXPORT ImageEditorPane  : public Editor
{
    Q_OBJECT
public:
    explicit ImageEditorPane(QWidget *parent = nullptr);
    ~ImageEditorPane();
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

    bool readFile(const QString& path) override;

    virtual QString path() override;
    virtual void rename(const QString& nname) override;
    virtual void autoSave() override;
    virtual bool isModification() override;
    virtual int fileState() override;
    virtual void setFileState(int state) override;
    virtual void invokeFileState() override;
    virtual void reload() override;
    virtual CodeEditorView* editor() override;


    static ImageEditorPane* make(DockingPaneManager* dockingManager,const QJsonObject& data={});

    void onZoom(int zoom);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::ImageEditorPane* ui;
    ImageEditorPanePrivate* d;
    static int SN;
};
}
#endif // IMAGE_EDITOR_PANE_H
