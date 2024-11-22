#include "simple_editor_view.h"
#include <codeformatter.h>
#include <languages/javascript/jscodeformatter.h>
#include <languages/javascript/jshighlighter.h>
#include <fontsettings.h>
#include <QDebug>

namespace ady{
class SimpleEditorViewPrivate{
public:
    TextEditor::CodeFormatter* formatter;
    TextEditor::SyntaxHighlighter* highlighter;
    TextEditor::FontSettings setting;

};

SimpleEditorView::SimpleEditorView(QWidget* parent):QPlainTextEdit(parent) {

    d = new SimpleEditorViewPrivate;

    d->formatter = new Javascript::CodeFormatter;
    d->highlighter = new Javascript::Highlighter(this->document());

}

SimpleEditorView::~SimpleEditorView(){
    if(d->formatter!=nullptr){
        delete d->formatter;
    }
    if(d->highlighter!=nullptr){
        delete d->highlighter;
    }
    delete d;
}


void SimpleEditorView::setFormatter(TextEditor::CodeFormatter* formatter){
    if(d->formatter!=nullptr){
        delete d->formatter;
    }
    d->formatter = formatter;

}

void SimpleEditorView::setHighlighter(TextEditor::SyntaxHighlighter* highlighter){
    if(d->formatter!=nullptr){
        delete d->formatter;
    }
    d->highlighter = highlighter;
}

void SimpleEditorView::setColorScheme(const QString& name){
    //TextEditor::FontSettings setting;
    d->setting.clear();
    d->setting.loadColorScheme(name);
    d->highlighter->setFontSettings(d->setting);
    d->highlighter->rehighlight();
}

void SimpleEditorView::setFamily(const QString& name){
    d->setting.setFamily(name);
    this->setFont(d->setting.font());
}

void SimpleEditorView::setFontSize(int size){
    d->setting.setFontSize(size);
    this->setFont(d->setting.font());
}
void SimpleEditorView::setZoom(int zoom){
    d->setting.setFontZoom(zoom);
    this->setFont(d->setting.font());
    d->highlighter->rehighlight();
}


}
