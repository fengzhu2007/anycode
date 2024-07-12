#include "code_high_lighter.h"
#include <QRegularExpression>
#include <QVector>
#include <QDebug>

namespace ady{


struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
};

class CodeHighLighterPrivate{
public:
    QTextCharFormat keywordFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat commentFormat;
    QVector<HighlightingRule> rules;
    QRegularExpression blockPattern;
    QRegularExpression quotaPattern;
    QRegularExpression sQuotaPattern;
    QRegularExpression dQuotaPattern;
    QRegularExpression commentStartPattern;
    QRegularExpression commentEndPattern;

    QString commentStartTag;
    QString commentEndTag;

};

CodeHighLighter::CodeHighLighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    d = new CodeHighLighterPrivate;
    d->stringFormat.setForeground(Qt::darkGreen);
    d->keywordFormat.setForeground(Qt::blue);
    d->commentFormat.setForeground(Qt::darkGray);
    //d->commentFormat.setFontItalic(true);

    HighlightingRule rule;
    QStringList keywordPatterns = {
        "\\babstract\\b", "\\barray\\b", "\\bas\\b", "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bclass\\b", "\\bconst\\b",
        "\\bcontinue\\b", "\\bdeclare\\b", "\\bdefault\\b", "\\bdo\\b", "\\becho\\b", "\\belse\\b", "\\belseif\\b", "\\bempty\\b",
        "\\bextends\\b", "\\bfor\\b", "\\bforeach\\b", "\\bfunction\\b", "\\bglobal\\b", "\\bif\\b", "\\bimplements\\b",
        "\\binclude\\b", "\\binclude_once\\b", "\\binterface\\b", "\\bis\\b", "\\bisset\\b", "\\bnamespace\\b", "\\bnew\\b",
        "\\bprint\\b", "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\brequire\\b", "\\brequire_once\\b", "\\breturn\\b",
        "\\bstatic\\b", "\\bswitch\\b", "\\bthrow\\b", "\\btry\\b", "\\buse\\b", "\\bvar\\b", "\\bwhile\\b", "\\bexit\\b", "<\\?php\\b","\\$_POST\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = d->keywordFormat;
        d->rules.append(rule);
    }

    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = d->commentFormat;
    d->rules.append(rule);


    d->commentStartTag = "/*";
    d->commentEndTag = "*/";

    d->quotaPattern.setPattern("\"|\'");
    d->sQuotaPattern.setPattern("\'");
    d->dQuotaPattern.setPattern("\"");
    d->commentStartPattern.setPattern("/\\*");
    d->commentEndPattern.setPattern("\\*/");
    d->blockPattern.setPattern(R"('|"|/\*|\*/)");
    //d->blockPattern.setPattern(QString("\"|\'|%1|%2").arg(d->commentStartTag).arg(d->commentEndTag));


}

CodeHighLighter::~CodeHighLighter(){
    delete d;
}

void CodeHighLighter::highlightBlock(const QString &text)
{
    int pbs = previousBlockState();
    //qDebug()<<"pbs:"<<pbs<<";"<<text;

    int startIndex = 0;
    if(pbs==S_Quot_Start||pbs==S_Quot){
        startIndex = text.indexOf(d->sQuotaPattern);
        if(startIndex>=0){
            //string end
            startIndex += 1;
            setFormat(0,startIndex,d->stringFormat);
            setCurrentBlockState(S_Quot_End);

        }else{
            //set string format
            setFormat(0,text.length(),d->stringFormat);
            setCurrentBlockState(S_Quot);
        }
    }else if(pbs==D_Quot_Start||pbs==D_Quot){
        startIndex = text.indexOf(d->dQuotaPattern);
        if(startIndex>=0){
            //string end
            startIndex += 1;
            setFormat(0,startIndex,d->stringFormat);
            setCurrentBlockState(D_Quot_End);
        }else{
            setFormat(0,text.length(),d->stringFormat);
            setCurrentBlockState(D_Quot);
            return ;
        }
    }else if(pbs==Comment_Start || pbs==Comment){
        startIndex = text.indexOf(d->commentEndPattern);
        if(startIndex>=0){
            //string end
            startIndex += 2;
            setFormat(0,startIndex,d->commentFormat);
            setCurrentBlockState(Comment_End);
        }else{
            setFormat(0,text.length(),d->commentFormat);
            setCurrentBlockState(Comment);
            return ;
        }
    }

    for (const HighlightingRule &rule : qAsConst(d->rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text,startIndex);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    //mutiple string
    bool commentOpen = false;
    bool quotaOpen = false;
    int blockStart = -1;
    int blockEnd = -1;
    QString blockTag;

    QRegularExpressionMatchIterator matchIterator = d->blockPattern.globalMatch(text,startIndex);
    while(matchIterator.hasNext()){
        QRegularExpressionMatch match = matchIterator.next();
        QString tag = match.captured();
        if(quotaOpen==false){
            if(tag==d->commentStartTag){
                if(!commentOpen){
                    commentOpen = !commentOpen;
                    blockTag = tag;
                    blockStart = match.capturedStart();
                }
            }else if(tag==d->commentEndTag){
                commentOpen = !commentOpen;
                blockEnd = match.capturedStart();
                blockTag.clear();
                setFormat(blockStart,blockEnd - blockStart + 2,d->commentFormat);
                setCurrentBlockState(Code);
            }else if(commentOpen==false){
                if(tag!=blockTag){
                    quotaOpen = !quotaOpen;
                    blockTag = tag;
                    blockStart = match.capturedStart();
                }
            }
         }else{
            if(tag==blockTag){
                //qDebug()<<"text:"<<text<<"tag:"<<tag<<";q:"<<quotaOpen;
                quotaOpen = !quotaOpen;
                blockEnd = match.capturedStart();
                blockTag.clear();
                setFormat(blockStart,blockEnd - blockStart + 1,d->stringFormat);
                setCurrentBlockState(Code);
            }
        }
    }
    if(commentOpen){
        setFormat(blockStart,text.size() - blockStart,d->commentFormat);
        setCurrentBlockState(Comment_Start);
    }else if(quotaOpen){
        if(blockTag=="\""){
            setFormat(blockStart,text.size() - blockStart,d->stringFormat);
            setCurrentBlockState(D_Quot_Start);
        }else if(blockTag=="\'"){
            setFormat(blockStart,text.size() - blockStart,d->stringFormat);
            setCurrentBlockState(S_Quot_Start);
        }
    }


}

}
