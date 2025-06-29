#ifndef FRAMES_MODEL_H
#define FRAMES_MODEL_H

#include <QAbstractTableModel>
#include <QPixmap>

namespace ady{
class FrameItem{
public:
    QString filename;
    QPixmap image;
};
class FramesModelPrivate;
class FramesModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit FramesModel(QObject *parent = nullptr);
    ~FramesModel();

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override{
        return 1;
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setDatasource(const QStringList& list);

    const QPixmap& image(int row) const;
private:
    QList<FrameItem> m_data;
    //QStringList m_data;
    //FramesModelPrivate* d;

};
}
#endif // FRAMES_MODEL_H
