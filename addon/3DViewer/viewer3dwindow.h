#ifndef VIEWER3DWINDOW_H
#define VIEWER3DWINDOW_H

#include "3dviewer_global.h"
#include "w_window.h"

#include <QHash>
#include <QString>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;

class Viewer3DThumbCache;
class Viewer3DThumbnailRenderer;
class Viewer3DWidget;

/**
 * @brief 3D 模型浏览窗口（wRect 无边框新窗口）
 *
 * 左侧为模型缩略图网格，右侧为 Qt Quick 3D 预览视图；
 * 缩略图缓存写入模型同级目录的隐藏目录 ".cache"，文件名为 <模型名>_128x.png。
 */
class Viewer3DWindow : public wWindow
{
    Q_OBJECT
public:
    explicit Viewer3DWindow(QWidget* parent = nullptr);
    ~Viewer3DWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onOpenDirectory();
    void onOpenFile();
    void onRefresh();
    void onFilterChanged();
    void onRegenerateThumbnails();
    void onItemClicked(QListWidgetItem* item);
    void onThumbnailGenerated(const QString& modelFilePath, const QString& thumbnailPath);
    void onThumbnailFailed(const QString& modelFilePath, const QString& errorString);
    void onListScrolled();
    void onContextMenuRequested(const QPoint& pos);

private:
    void setupUi();
    void scanDirectory(const QString& dirPath, bool regenerate = false);
    void addModelItem(const QString& filePath);
    void openModel(const QString& filePath);
    void applyFilter();
    void prioritizeVisibleItems();
    void updateStatus();

    QWidget* m_content = nullptr;
    QLineEdit* m_searchEdit = nullptr;
    QComboBox* m_formatCombo = nullptr;
    QPushButton* m_wireframeButton = nullptr;
    QPushButton* m_gridButton = nullptr;
    QListWidget* m_modelList = nullptr;
    Viewer3DWidget* m_viewer = nullptr;
    QLabel* m_statusLabel = nullptr;

    Viewer3DThumbCache* m_cache = nullptr;
    Viewer3DThumbnailRenderer* m_renderer = nullptr;

    QHash<QString, QListWidgetItem*> m_items;
    QString m_currentDir;
};

#endif // VIEWER3DWINDOW_H
