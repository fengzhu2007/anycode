#ifndef FILE_DIFF_LIST_WIDGET_H
#define FILE_DIFF_LIST_WIDGET_H

#include "chat_service.h"
#include "file_diff_item_widget.h"
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

    /** 追加文件变更到列表 */
    void appendDiffs(const QList<FileDiffInfo> &diffs);

    /** 清空列表 */
    void clear();

    /** 标记所有未操作项为已接受 */
    void markAllAccepted();

    /** 标记所有未操作项为已拒绝 */
    void markAllRejected();

    /** 是否存在未操作的项 */
    bool hasPendingItems() const;

signals:
    /** 单个文件拒绝（file path） */
    void fileRejected(const QString &filePath);
    /** 单个文件确认（file path） */
    void fileAccepted(const QString &filePath);

private:
    Ui::FileDiffListWidget *ui;
    QList<FileDiffInfo> m_diffs;
};

}

#endif // FILE_DIFF_LIST_WIDGET_H
