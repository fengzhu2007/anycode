#ifndef FILE_SELECTOR_H
#define FILE_SELECTOR_H

#include <QWidget>
namespace ady{
class FileSelectorPrivate;
class FileSelector : public QWidget
{
    Q_OBJECT
public:
    explicit FileSelector(QWidget *parent,const QString& filter={});
    ~FileSelector();
    void setEnable(bool enable);
    void setFilter(const QString& filter);
    QString& filter();
    void setDir(const QString& dir);
    QString& dir();

    void setText(const QString& text);
    QString text() const;

public slots:
    void onSelectFile();

signals:
    void fileChanged(const QString& path);

private:
    FileSelectorPrivate* d;
};
}
#endif // FILE_SELECTOR_H
