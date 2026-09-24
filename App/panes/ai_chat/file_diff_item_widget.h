#ifndef FILE_DIFF_ITEM_WIDGET_H
#define FILE_DIFF_ITEM_WIDGET_H

#include "chat_service.h"
#include <QWidget>

namespace Ui {
class FileDiffItemWidget;
}

namespace ady {

/**
 * FileDiffItemWidget - 单个文件变更列表项
 *
 * 显示一个文件的变更信息：
 * - 状态标签（M=修改, A=新增, D=删除）
 * - 文件名 + 目录路径 + 增删行数
 * - 单个文件的 reject/accept 按钮
 */
class FileDiffItemWidget : public QWidget
{
    Q_OBJECT
public:
    enum ActionState { None, Rejected, Accepted };

    explicit FileDiffItemWidget(const FileDiffInfo &info, QWidget *parent = nullptr);
    ~FileDiffItemWidget();

    QString filePath() const { return m_filePath; }
    ActionState actionState() const { return m_actionState; }
    void setActionState(ActionState state);

signals:
    void rejected(const QString &filePath);
    void accepted(const QString &filePath);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    static QString statusIcon(const QString &status);
    static QString statusColor(const QString &status);

    Ui::FileDiffItemWidget *ui;
    QString m_filePath;
    ActionState m_actionState = None;
};

} // namespace ady

#endif // FILE_DIFF_ITEM_WIDGET_H
