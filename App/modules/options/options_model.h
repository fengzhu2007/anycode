#ifndef OPTIONS_MODEL_H
#define OPTIONS_MODEL_H

#include <QAbstractListModel>

namespace ady{
class OptionWidget;
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit OptionsModel(QObject *parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void appendItem(OptionWidget* item);


    inline OptionWidget* at(int row) const {return m_list.at(row);}


private:
    QList<OptionWidget*> m_list;
};
}
#endif // OPTIONS_MODEL_H
