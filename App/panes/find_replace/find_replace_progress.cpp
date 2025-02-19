#include "find_replace_progress.h"
#include "search_result_model.h"
#include "utils/filesearch.h"
#include "utils/stringutils.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/code_editor/code_editor_pane.h"
//#include "common/utils.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QRegularExpression>
#include <QHash>
#include <QTextCursor>
#include <QTextBlock>
#include <QDebug>

namespace ady{

const int MAX_LINE_SIZE = 400;

QString clippedText(const QString &text, int maxLength)
{
    if (text.length() > maxLength)
        return text.left(maxLength) + QChar(0x2026); // '...'
    return text;
}


static QList<QRegularExpression> filtersToRegExps(const QStringList &filters)
{
    QList<QRegularExpression> list;
    for(auto filter:filters){
        list<<QRegularExpression(Utils::wildcardToRegularExpression(filter),QRegularExpression::CaseInsensitiveOption);
    }
    return list;
}

static bool orMatches(const QList<QRegularExpression> &exprList, const QString &filePath)
{
    bool ret = false;
    for(auto reg:exprList){
        ret = reg.match(filePath).hasMatch();
        if(ret){
            break;
        }
    }
    return ret;
}


class FindReplaceProgressPrivate{
public:
    QString before;
    QString after;
    QString folder;
    FindReplaceProgress::Mode mode;
    int flags;
    //bool interrupt = false;
    bool regular = false;
    bool caseSensitive;
    bool wholeWord;
    int termMaxIndex;
    const QChar *termData;
    const QChar *termDataLower;
    const QChar *termDataUpper;
    QList<SearchResultItem>* list;
    //QList<QTextDocument*> docList;
    QMap<QString,QTextDocument*> docList;
    QRegularExpression expression;


    //int matchCount=0;
    int matchFileCount=0;
    int searchFileCount=0;
    int replaceFiles = 0;
    int replaceCount = 0;
    QList<QRegularExpression> fileFilters;
    QList<QRegularExpression> exclusions;
    QStringList openedFiles;
};

FindReplaceProgress::FindReplaceProgress(Mode mode,const QString& before,const QString& after,int flags,const QString& folder,const QString& filter,const QString& exclusion)
    :QThread()
{
    d = new FindReplaceProgressPrivate{before,after,folder,mode,flags,false};
    d->regular = false;
    d->caseSensitive = (flags & QTextDocument::FindCaseSensitively)>0;
    d->wholeWord = (flags & QTextDocument::FindWholeWords)>0;
    d->termMaxIndex = before.length() - 1;
    d->termData = before.constData();
    d->termDataLower = before.toLower().constData();
    d->termDataUpper = before.toUpper().constData();
    d->list = new  QList<SearchResultItem>;
    if(d->wholeWord || (flags & 0x08)>0){
        d->regular = true;
        QString term = before;
        if (flags & QTextDocument::FindWholeWords)
            term = QString::fromLatin1("\\b%1\\b").arg(term);
        const QRegularExpression::PatternOptions patternOptions = (flags & QTextDocument::FindCaseSensitively)
                                                                      ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
        d->expression = QRegularExpression(term, patternOptions);
    }
    if(!filter.isEmpty()){
        d->fileFilters = filtersToRegExps(filter.split(","));
    }
    if(!exclusion.isEmpty()){
        d->exclusions = filtersToRegExps(exclusion.split(","));
    }
}

FindReplaceProgress::FindReplaceProgress(Mode mode,const QString& before,const QString& after,int flags,const QString& filename,QTextDocument* doc,const QString& filter,const QString& exclusion){
    d = new FindReplaceProgressPrivate{before,after,{},mode,flags,false};
    d->regular = false;
    d->caseSensitive = (flags & QTextDocument::FindCaseSensitively)>0;
    d->wholeWord = (flags & QTextDocument::FindWholeWords)>0;
    d->termMaxIndex = before.length() - 1;
    d->termData = before.constData();
    d->termDataLower = before.toLower().constData();
    d->termDataUpper = before.toUpper().constData();
    d->list = new  QList<SearchResultItem>;
    if(d->wholeWord || (flags & 0x08)>0){
        d->regular = true;
        QString term = before;
        if (flags & QTextDocument::FindWholeWords)
            term = QString::fromLatin1("\\b%1\\b").arg(term);
        const QRegularExpression::PatternOptions patternOptions = (flags & QTextDocument::FindCaseSensitively)
                                                                      ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
        d->expression = QRegularExpression(term, patternOptions);
    }
    if(!filter.isEmpty()){
        d->fileFilters = filtersToRegExps(filter.split(","));
    }
    if(!exclusion.isEmpty()){
        d->exclusions = filtersToRegExps(exclusion.split(","));
    }
    d->docList.insert(filename,doc);
}

FindReplaceProgress::FindReplaceProgress(Mode mode,const QString& before,const QString& after,int flags,QMap<QString,QTextDocument*>docs,const QString& filter,const QString& exclusion){
    d = new FindReplaceProgressPrivate{before,after,{},mode,flags,false};
    d->regular = false;
    d->caseSensitive = (flags & QTextDocument::FindCaseSensitively)>0;
    d->wholeWord = (flags & QTextDocument::FindWholeWords)>0;
    d->termMaxIndex = before.length() - 1;
    d->termData = before.constData();
    d->termDataLower = before.toLower().constData();
    d->termDataUpper = before.toUpper().constData();
    d->list = new  QList<SearchResultItem>;
    if(d->wholeWord || (flags & 0x08)>0){
        d->regular = true;
        QString term = before;
        if (flags & QTextDocument::FindWholeWords)
            term = QString::fromLatin1("\\b%1\\b").arg(term);
        const QRegularExpression::PatternOptions patternOptions = (flags & QTextDocument::FindCaseSensitively)
                                                                      ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption;
        d->expression = QRegularExpression(term, patternOptions);
    }
    if(!filter.isEmpty()){
        d->fileFilters = filtersToRegExps(filter.split(","));
    }
    if(!exclusion.isEmpty()){
        d->exclusions = filtersToRegExps(exclusion.split(","));
    }
    d->docList=docs;
}

FindReplaceProgress::~FindReplaceProgress(){
    delete d;
}

void FindReplaceProgress::run(){
    if(d->docList.size()>0){
        auto iter = d->docList.begin();
        while(iter!=d->docList.end()){
            if(this->isInterruptionRequested()){
                break;
            }
            this->searchDocument(iter.value(),iter.key());
            iter++;
        }
    }else{
        QFileInfo fi(d->folder);
        if(fi.isFile()){
            auto instance = CodeEditorManager::getInstance();
            auto pane = instance->get(d->folder);
            if(pane!=nullptr){
                this->searchDocument(pane->editor()->document(),d->folder);
                d->searchFileCount += 1;
            }else{
                if(d->regular){
                    this->searchFileRegExp(d->folder);
                }else{
                    this->searchFile(d->folder);
                }
            }
        }else if(fi.isDir()){
            this->searchFolder(d->folder);
        }
    }


    if(!this->isInterruptionRequested()){
        if(d->mode == SearcnOnly){
            //qDebug()<<"search count"<<d->searchFileCount;
            emit searchResult(d->list,d->matchFileCount,d->searchFileCount);
        }else{
            //emit replaceResult(d->list,d->matchFileCount,d->searchFileCount);
            //replace all
            //emit searchResult(d->list,d->matchFileCount,d->searchFileCount);
            this->replaceAll(d->after,*d->list,false);
            //qDebug()<<"opend files"<<d->openedFiles;
            emit replaceOpendFiles(d->openedFiles,d->before,d->after,d->flags,d->replaceCount,d->replaceFiles);
        }
    }else{
        d->list->clear();
        delete d->list;
    }
}



void FindReplaceProgress::searchFolder(const QString& folder){
    if(d->exclusions.size() > 0 && orMatches(d->exclusions,folder)){
        //exclusion
        return ;
    }
    QDir dir(folder);
    if(dir.exists()){
        auto instance = CodeEditorManager::getInstance();
        QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name|QDir::DirsFirst|QDir::IgnoreCase);
        for(auto one:list){
            if(one.isFile()){
                const QString path = one.absoluteFilePath();
                auto pane = instance->get(path);
                if(pane!=nullptr){
                    this->searchDocument(pane->editor()->document(),path);
                    d->searchFileCount += 1;
                }else{
                    if(d->regular){
                        this->searchFileRegExp(path);
                    }else{
                        this->searchFile(path);
                    }
                }
            }else if(one.isDir()){
                this->searchFolder(one.absoluteFilePath());
            }
            if(this->isInterruptionRequested()){
                break;
            }
        }
    }
}



void FindReplaceProgress::searchFile(const QString& path){
    if(d->fileFilters.size() > 0 && !orMatches(d->fileFilters,path)){
        //file pattern not matched
        return ;
    }
    if(d->exclusions.size() > 0 && orMatches(d->exclusions,path)){
        //exclusion
        return ;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ;
    }
    /*QByteArray data = file.read(1024);
    for (char byte : data) {
        if (byte < 0x09 || (byte > 0x0D && byte < 0x20) || byte == 0x7F) {
            file.close();
            return ;
        }
    }*/
    d->searchFileCount += 1;
    bool matched = false;
    file.seek(0);
    int lineNr = 0;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    while (!stream.atEnd()) {
        ++lineNr;
        const QString chunk = stream.readLine();
        const int chunkLength = chunk.length();
        const QChar *chunkPtr = chunk.constData();
        const QChar *chunkEnd = chunkPtr + chunkLength - 1;
        for (const QChar *regionPtr = chunkPtr; regionPtr + d->termMaxIndex <= chunkEnd; ++regionPtr) {
            if(this->isInterruptionRequested()){
                break;
            }
            const QChar *regionEnd = regionPtr + d->termMaxIndex;
            if ( /* optimization check for start and end of region */
                // case sensitive
                (d->caseSensitive && *regionPtr == d->termData[0]
                 && *regionEnd == d->termData[d->termMaxIndex])
                ||
                // case insensitive
                (!d->caseSensitive && (*regionPtr == d->termDataLower[0]
                                       || *regionPtr == d->termDataUpper[0])
                 && (*regionEnd == d->termDataLower[d->termMaxIndex]
                     || *regionEnd == d->termDataUpper[d->termMaxIndex]))
                ) {
                bool equal = true;

                // whole word check
                const QChar *beforeRegion = regionPtr - 1;
                const QChar *afterRegion = regionEnd + 1;
                if (d->wholeWord
                    && (((beforeRegion >= chunkPtr)
                         && (beforeRegion->isLetterOrNumber()
                             || ((*beforeRegion) == QLatin1Char('_'))))
                        ||
                        ((afterRegion <= chunkEnd)
                         && (afterRegion->isLetterOrNumber()
                             || ((*afterRegion) == QLatin1Char('_'))))
                        )) {
                    equal = false;
                } else {
                    // check all chars
                    int regionIndex = 1;
                    for (const QChar *regionCursor = regionPtr + 1;
                         regionCursor < regionEnd;
                         ++regionCursor, ++regionIndex) {
                        if (  // case sensitive
                            (d->caseSensitive
                             && *regionCursor != d->termData[regionIndex])
                            ||
                            // case insensitive
                            (!d->caseSensitive
                             && *regionCursor != d->termDataLower[regionIndex]
                             && *regionCursor != d->termDataUpper[regionIndex])
                            ) {
                            equal = false;
                            break;
                        }
                    }
                }
                if (equal) {
                    matched = true;
                    const QString resultItemText = clippedText(chunk, MAX_LINE_SIZE);
                    *d->list << SearchResultItem(lineNr,path,resultItemText,regionPtr - chunkPtr,d->termMaxIndex + 1,{});
                    regionPtr += d->termMaxIndex; // another +1 done by for-loop
                }
            }
        }
    }
    file.close();
    if(matched){
        d->matchFileCount += 1;
    }
}

void FindReplaceProgress::searchFileRegExp(const QString& path){
    if(d->fileFilters.size() > 0 && !orMatches(d->fileFilters,path)){
        //file pattern not matched
        return ;
    }
    if(d->exclusions.size() > 0 && orMatches(d->exclusions,path)){
        //exclusion
        return ;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ;
    }
    QByteArray data = file.read(1024);
    for (char byte : data) {
        if (byte < 0x09 || (byte > 0x0D && byte < 0x20) || byte == 0x7F) {
            file.close();
            return ;
        }
    }
    d->searchFileCount += 1;
    bool matched = false;
    file.seek(0);
    int lineNr = 0;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString line;
    QRegularExpressionMatch match;
    while (!stream.atEnd()) {
        ++lineNr;
        line = stream.readLine();

        const QString resultItemText = clippedText(line, MAX_LINE_SIZE);
        int lengthOfLine = line.size();
        int pos = 0;
        while ((match = this->doGuardedMatch(line, pos)).hasMatch()) {
            if(this->isInterruptionRequested()){
                break;
            }
            pos = match.capturedStart();
            matched = true;
            *d->list << SearchResultItem(lineNr,path,resultItemText,pos,match.capturedLength(),match.capturedTexts());
            if (match.capturedLength() == 0)
                break;
            pos += match.capturedLength();
            if (pos >= lengthOfLine)
                break;
        }
    }
    file.close();
    if(matched){
        d->matchFileCount += 1;
    }
}

void FindReplaceProgress::searchDocument(const QTextDocument* doc,const QString& path){
    bool matched = false;
    int lineNr = 1;
    QString line;
    QTextBlock block = doc->begin();
    if(d->regular){
        QRegularExpressionMatch match;
        while(block.isValid()){
            line = block.text();
            const QString resultItemText = clippedText(line, MAX_LINE_SIZE);
            int lengthOfLine = line.size();
            int pos = 0;
            while ((match = this->doGuardedMatch(line, pos)).hasMatch()) {
                if(this->isInterruptionRequested()){
                    break;
                }
                pos = match.capturedStart();
                matched = true;
                *d->list << SearchResultItem(lineNr,path,resultItemText,pos,match.capturedLength(),match.capturedTexts());
                if (match.capturedLength() == 0)
                    break;
                pos += match.capturedLength();
                if (pos >= lengthOfLine)
                    break;
            }
            block = block.next();
            ++lineNr;
        }
    }else{
        while(block.isValid()){
            line = block.text();

            //const QString chunk = stream.readLine();
            const int chunkLength = line.length();
            const QChar *chunkPtr = line.constData();
            const QChar *chunkEnd = chunkPtr + chunkLength - 1;
            for (const QChar *regionPtr = chunkPtr; regionPtr + d->termMaxIndex <= chunkEnd; ++regionPtr) {
                if(this->isInterruptionRequested()){
                    break;
                }
                const QChar *regionEnd = regionPtr + d->termMaxIndex;
                if ( /* optimization check for start and end of region */
                    // case sensitive
                    (d->caseSensitive && *regionPtr == d->termData[0]
                     && *regionEnd == d->termData[d->termMaxIndex])
                    ||
                    // case insensitive
                    (!d->caseSensitive && (*regionPtr == d->termDataLower[0]
                                           || *regionPtr == d->termDataUpper[0])
                     && (*regionEnd == d->termDataLower[d->termMaxIndex]
                         || *regionEnd == d->termDataUpper[d->termMaxIndex]))
                    ) {
                    bool equal = true;

                    // whole word check
                    const QChar *beforeRegion = regionPtr - 1;
                    const QChar *afterRegion = regionEnd + 1;
                    if (d->wholeWord
                        && (((beforeRegion >= chunkPtr)
                             && (beforeRegion->isLetterOrNumber()
                                 || ((*beforeRegion) == QLatin1Char('_'))))
                            ||
                            ((afterRegion <= chunkEnd)
                             && (afterRegion->isLetterOrNumber()
                                 || ((*afterRegion) == QLatin1Char('_'))))
                            )) {
                        equal = false;
                    } else {
                        // check all chars
                        int regionIndex = 1;
                        for (const QChar *regionCursor = regionPtr + 1;
                             regionCursor < regionEnd;
                             ++regionCursor, ++regionIndex) {
                            if (  // case sensitive
                                (d->caseSensitive
                                 && *regionCursor != d->termData[regionIndex])
                                ||
                                // case insensitive
                                (!d->caseSensitive
                                 && *regionCursor != d->termDataLower[regionIndex]
                                 && *regionCursor != d->termDataUpper[regionIndex])
                                ) {
                                equal = false;
                                break;
                            }
                        }
                    }
                    if (equal) {
                        matched = true;
                        const QString resultItemText = clippedText(line, MAX_LINE_SIZE);
                        *d->list << SearchResultItem(lineNr,path,resultItemText,regionPtr - chunkPtr,d->termMaxIndex + 1,{});
                        regionPtr += d->termMaxIndex; // another +1 done by for-loop
                    }
                }
            }
            block = block.next();
            ++lineNr;
        }
    }
    if(matched){
        d->matchFileCount += 1;
    }
}


QRegularExpressionMatch FindReplaceProgress::doGuardedMatch(const QString &line, int offset) const
{
    //QMutexLocker lock(&mutex);
    return d->expression.match(line, offset);
}




QStringList FindReplaceProgress::replaceAll(const QString &text,const QList<SearchResultItem> &items,bool preserveCase)
{
    if (items.isEmpty())
        return {};

    QHash<QString,QList<SearchResultItem>> changes;
    for (const SearchResultItem &item : items)
        changes[item.filePath].append(item);

    QTextDocument doc;
    auto instance = CodeEditorManager::getInstance();
    for (auto it = changes.cbegin(), end = changes.cend(); it != end; ++it) {
        const QString path = it.key();
        auto pane = instance->get(path);
        if(pane!=nullptr){
            //pane->repl
            d->openedFiles<<path;
            continue;
        }
        QSet<QPair<int, int>> processed;

        QFile file(path);
        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            continue;
        }
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        doc.setPlainText(stream.readAll());
        const QList<SearchResultItem> changeItems = it.value();

        QTextCursor editCursor(&doc);
        int offset = 0;
        int line = 0;
        int count = 0;
        for (const SearchResultItem &item : changeItems) {
            if(this->isInterruptionRequested()){
                break;
            }
            const QPair<int, int> p{item.line, item.matchStart};
            if (processed.contains(p))
                continue;
            processed.insert(p);

            QString replacement;
            if (!item.regexpCapturedTexts.isEmpty()) {
                replacement = Utils::expandRegExpReplacement(text, item.regexpCapturedTexts);
            } else if (preserveCase) {
                const QString originalText = item.searchItem;
                replacement = Utils::matchCaseReplacement(originalText, text);
            } else {
                replacement = text;
            }
            QTextBlock block = doc.findBlockByLineNumber(item.line - 1);
            if(block.isValid()){
                if(line!=item.line){
                    offset = 0;
                    line = item.line;
                }
                int position = block.position() + item.matchStart + offset;
                editCursor.setPosition(position);
                editCursor.setPosition(position + item.matchLength ,QTextCursor::KeepAnchor);
                editCursor.insertText(replacement);
                offset += (replacement.length() - item.matchLength);
                count += 1;
            }
        }
        file.seek(0);
        auto ret = file.write(doc.toPlainText().toUtf8());
        file.close();
        d->replaceFiles += 1;
        d->replaceCount += count;
    }
    return changes.keys();
}





}
