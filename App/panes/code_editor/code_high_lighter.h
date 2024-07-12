#ifndef CODEHIGHLIGHTER_H
#define CODEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
namespace ady{
class CodeHighLighterPrivate;
class CodeHighLighter : public QSyntaxHighlighter
{
public:
    enum State{
        Unkown=-1,
        Code=0,
        S_Quot_Start,
        S_Quot,
        S_Quot_End,
        D_Quot_Start,
        D_Quot,
        D_Quot_End,
        Comment_Start,
        Comment,
        Comment_End,
    };
    explicit CodeHighLighter(QTextDocument *parent);
    ~CodeHighLighter();

protected:
    virtual void highlightBlock(const QString &text) override;

private:

    CodeHighLighterPrivate *d;
};
}

#endif // CODEHIGHLIGHTER_H
