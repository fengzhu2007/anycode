#ifndef FINDREPLACEPROGRESS_H
#define FINDREPLACEPROGRESS_H
#include "search_result_model.h"
#include <QThread>
#include <QTextCursor>
#include <QTextDocument>
namespace ady{
class FindReplaceProgressPrivate;
class FindReplaceProgress : public QThread
{
    Q_OBJECT
public:
    enum Mode{
        SearcnOnly=0,
        SearchAndReplace
    };
    FindReplaceProgress(Mode mode,const QString& before,const QString& after,int flags,const QString& folder,const QString& filter,const QString& exclusion);
    ~FindReplaceProgress();

signals:
    void searchResult(QList<SearchResultItem>* list,int matchFileCount,int searchFileCount);
    void replaceResult(QList<SearchResultItem>* list,int matchFileCount,int searchFileCount);
    void replaceOpendFiles(const QStringList& list,const QString& before,const QString& after,int flags,int replaceCount,int replaceFiles);

protected:
    virtual void run() override;

private:
    void searchFolder(const QString& folder);
    void searchFile(const QString& path);
    void searchFileRegExp(const QString& path);
    QRegularExpressionMatch doGuardedMatch(const QString &line, int offset) const;
    QStringList replaceAll(const QString &text,const QList<SearchResultItem> &items,bool preserveCase);

private:
    FindReplaceProgressPrivate* d;

};
}
#endif // FINDREPLACEPROGRESS_H
