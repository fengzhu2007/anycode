#ifndef CODEEDITOR_H
#define CODEEDITOR_H
#include "global.h"
#include <QPlainTextEdit>

namespace ady{
class LineNumberArea;

class Language;
class CodeEditorPrivate;
class ANYENGINE_EXPORT CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget* parent);
    ~CodeEditor();
    void init();
    bool readFile(const QString& path);
    bool writeFile(const QString& path);
    QString path();
    void rename(const QString& path);


    //void setLanguage(Language* language);
    void setLanguage(const QString& name);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    bool isShowed();

public slots:
    void fillPlainText(const QString& text);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

private:
    QString textUnderCursor() const;


private slots:
    void insertCompletion(const QString &completion);

    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    CodeEditorPrivate* d;
    static int COUNT;


};


class LineNumberArea : public QWidget
{
public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor)
    {}

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};




}

#endif // CODEEDITOR_H
