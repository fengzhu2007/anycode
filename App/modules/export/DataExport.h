#ifndef DATAEXPORT_H
#define DATAEXPORT_H
#include <QXmlStreamWriter>






namespace ady {
    class DataExport{

    public:
        DataExport();
        ~DataExport();
        int doExport(QWidget* parent);
    private:
        void writeBaseInfo();
        void writeData();
        void writeDataNode(const QString& name,const QString& text);
    private:
        int version = 1;
        QXmlStreamWriter* pWriter;

    };
}
#endif // DATAEXPORT_H
