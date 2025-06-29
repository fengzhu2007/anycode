#ifndef DIR_SELECTOR_H
#define DIR_SELECTOR_H
#include <QWidget>

namespace ady{
class DirSelectorPrivate;
class DirSelector : public QWidget
{
    Q_OBJECT
public:
    explicit DirSelector(QWidget *parent = nullptr);
    ~DirSelector();
    void setEnable(bool enable);
    void setText(const QString& text);
    QString text() const;

public slots:
    void onSelectDir();


signals:
    void dirChanged(const QString& path);


private:
    DirSelectorPrivate* d;
};

}
#endif // DIR_SELECTOR_H
