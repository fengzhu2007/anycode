#ifndef VIEWER3DWINDOW_H
#define VIEWER3DWINDOW_H

#include "3dviewer_global.h"
#include "w_window.h"

namespace Ui {
class Viewer3DWindow;
}

class QListWidgetItem;

class Viewer3DWindow : public wWindow
{
    Q_OBJECT
public:
    explicit Viewer3DWindow(QWidget* parent = nullptr);
    ~Viewer3DWindow();

private slots:
    void onOpenDirectory();
    void onThumbnailClicked(QListWidgetItem* item);

private:
    void scanDirectory(const QString& dirPath);
    QPixmap generateThumbnail(const QString& filePath);

    Ui::Viewer3DWindow *ui;
    QString m_currentDir;

    static QStringList supportedExtensions();
};

#endif // VIEWER3DWINDOW_H
