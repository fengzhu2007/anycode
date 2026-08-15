#ifndef TEMPLATE_COMMAND_DIALOG_H
#define TEMPLATE_COMMAND_DIALOG_H

#include <w_dialog.h>
#include <QStringList>

namespace Ui {
class TemplateCommandDialog;
}

namespace ady {

enum ParamType {
    Text = 0,
    Number,
    Dir,
    File
};

struct TemplateCommand {
    long long id = 0;
    QString title;
    QString commandTemplate;
    QList<ParamType> paramTypes;
    QStringList defaultValues;  // default value for each param
    QString shellType;   // "cmd" or "powershell"
};

class TemplateCommandDialogPrivate;
class TemplateCommandDialog : public wDialog
{
    Q_OBJECT

public:
    ~TemplateCommandDialog();
    static TemplateCommandDialog* open(QWidget* parent);

private:
    explicit TemplateCommandDialog(QWidget *parent = nullptr);

    void initView();
    void loadTemplates();
    void saveTemplates();
    void saveCurrentTemplate();
    void loadTemplateToInfoTab(int index);
    int parsePlaceholderCount(const QString& cmd) const;
    void rebuildInfoConfigTab();
    void rebuildRunForm();
    void updateRunTemplateLabel();
    void executeCommand();
    QString resolveCommand() const;
    static QString paramTypeName(ParamType type);
    QList<ParamType> currentParamTypes() const;
    void saveToHistory(const QString& templateTitle, const QString& commandTemplate,
                       const QString& resolvedCommand, const QStringList& paramValues,
                       const QList<ParamType>& paramTypes, const QString& shellType);
    void loadHistory();
    void loadHistoryToRunTab(int historyIndex);

private slots:
    void onListCurrentRowChanged(int currentRow);
    void onAddTemplate();
    void onRemoveTemplate();
    void onInfoTitleChanged(const QString& text);
    void onInfoCommandChanged(const QString& text);
    void onParamTypeChanged(int index);
    void onDefaultValueChanged(const QString& text);
    void onTabChanged(int index);
    void onRunClicked();
    void onInfoSaveClicked();
    void onHistoryRowClicked(int row);
    void onHistoryDeleteClicked();

private:
    Ui::TemplateCommandDialog *ui;
    TemplateCommandDialogPrivate *d;
    static TemplateCommandDialog* instance;
    bool m_loading = false;
};

}
#endif // TEMPLATE_COMMAND_DIALOG_H
