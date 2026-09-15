#ifndef FILE_DIFF_LIST_WIDGET_H
#define FILE_DIFF_LIST_WIDGET_H

#include "chat_service.h"
#include <QWidget>
#include <QListWidget>

namespace Ui {
class FileDiffListWidget;
}

namespace ady {

/**
 * FileDiffListWidget - 文件变更列表组件
 *
 * 显示 AI 对话产生的文件变更列表，每个文件显示：
 * - 文件名（相对路径）
 * - 状态标签（M=修改, A=新增, D=删除）
 * - 增加/删除行数
 * - 单个文件的 reject/accept 按钮
 *
 * 底部有全局 accept/reject 按钮。
 */
class FileDiffListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileDiffListWidget(QWidget *parent = nullptr);
    ~FileDiffListWidget();

    /** 设置文件变更列表 */
    void setDiffs(const QList<FileDiffInfo> &diffs);

    /** 清空列表 */
    void clear();

    /** 获取当前 diff 数据 */
    QList<FileDiffInfo> diffs() const { return m_diffs; }

signals:
    /** 全局确认变更 */
    void acceptAll();
    /** 全局拒绝变更 */
    void rejectAll();
    /** 单个文件拒绝（file path） */
    void fileRejected(const QString &filePath);
    /** 单个文件确认（file path） */
    void fileAccepted(const QString &filePath);

private:
    void createFileItem(int index);
    static QString statusIcon(const QString &status);
    static QString statusColor(const QString &status);

    Ui::FileDiffListWidget *ui;
    QList<FileDiffInfo> m_diffs;
};

}

#endif // FILE_DIFF_LIST_WIDGET_H
