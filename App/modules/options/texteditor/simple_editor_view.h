#ifndef SIMPLE_EDITOR_VIEW_H
#define SIMPLE_EDITOR_VIEW_H

#include <QPlainTextEdit>

namespace TextEditor{
class CodeFormatter;
class FontSettings;
class SyntaxHighlighter;
}

namespace ady{
class SimpleEditorViewPrivate;
class SimpleEditorView : public QPlainTextEdit
{
public:
    explicit SimpleEditorView(QWidget* parent);
    ~SimpleEditorView();
    void setFormatter(TextEditor::CodeFormatter* formatter);
    void setHighlighter(TextEditor::SyntaxHighlighter* highlighter);
    void setColorScheme(const QString& name);
    void setFamily(const QString& name);
    void setFontSize(int size);
    void setZoom(int zoom);


    //void setFontSettings(const FontSettings& setting);


private:
    SimpleEditorViewPrivate* d;
};
}

#endif // SIMPLE_EDITOR_VIEW_H
