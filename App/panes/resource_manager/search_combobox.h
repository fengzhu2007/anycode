#ifndef SEARCH_COMBOBOX_H
#define SEARCH_COMBOBOX_H
#include <QComboBox>
#include <QModelIndex>
namespace ady{
class SearchComboBoxPrivate;
class SearchComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit SearchComboBox(QWidget* parent);

    void addItem(const QString& text);

public slots:
    void onIndexSelected(const QModelIndex& index);
signals:
    void search(const QString& text);
    void reset();


protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    //SearchComboBoxPrivate* d;


};
}
#endif // SEARCH_COMBOBOX_H
