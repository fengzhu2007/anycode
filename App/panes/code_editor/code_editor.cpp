#include "code_editor.h"
//#include "language/php_high_lighter.h"
#include "code_high_lighter.h"
#include "language.h"
#include "auto_completion_view.h"
#include "language_manager.h"
#include "core/backend_thread.h"
#include "code_editor_read_file_task.h""
#include <QPainter>
#include <QTextBlock>
#include <QTextStream>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QFileInfo>

#include <QDebug>

namespace ady{

int CodeEditor::COUNT = 0;

class CodeEditorPrivate{
public:
    LineNumberArea* lineNumberArea;
    QCompleter* completer;
    //Language* language = nullptr;
    AutoCompletionModel* model;
    QString path;
    bool showed = false;
};

CodeEditor::CodeEditor(QWidget* parent):
    QPlainTextEdit(parent)
{
    //this->setStyleSheet("border:0");
    QFont font("Microsoft YaHei", 10);
    this->setFrameStyle(QFrame::NoFrame);
    this->setFont(font);
    this->setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4 );
    d = new CodeEditorPrivate;
    if(CodeEditor::COUNT==0){
        AutoCompletionModel::init();
        LanguageManager::init();
    }
    CodeEditor::COUNT += 1;
    d->model = AutoCompletionModel::getInstance();
    d->completer = new QCompleter(d->model,this);
    d->completer->setCompletionRole(Qt::DisplayRole);
    d->completer->setWidget(this);
    d->completer->setCompletionMode(QCompleter::PopupCompletion);
    d->completer->setCaseSensitivity(Qt::CaseInsensitive);

    d->lineNumberArea = new LineNumberArea(this);
    d->lineNumberArea->setFont(font);
    //PHPHighlighter *highlighter = new PHPHighlighter(document());
    new CodeHighLighter(document());

    connect(d->completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CodeEditor::insertCompletion);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    //this->setLanguage(new PhpLanguage);

}

CodeEditor::~CodeEditor(){
    CodeEditor::COUNT -= 1;
    if(CodeEditor::COUNT==0){
        AutoCompletionModel::destory();
        LanguageManager::destory();
    }
    delete d;
}

void CodeEditor::init(){
    if(!d->path.isEmpty()){
        QFileInfo fi(d->path);
        this->setLanguage(fi.suffix().toLower());
    }
}

bool CodeEditor::readFile(const QString& path){
    QFile file(path);
    if(file.exists()){
        d->path = path;
        this->init();
        if(file.size()>50*1024){
            //start thread
            this->setPlaceholderText(tr("Loading File..."));
            BackendThread::getInstance()->appendTask(new CodeEditorReadFileTask(d->path));
            return true;
        }else{
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                return false;
            }
            QTextStream in(&file);
            in.setCodec("UTF-8");
            //this->setPlainText(in.readAll());
            this->fillPlainText(in.readAll());
            file.close();
            return true;
        }
    }else{
        return false;
    }
}

bool CodeEditor::writeFile(const QString& path){
    //write to file
    QString textContent = toPlainText();
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        d->path = path;
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << textContent;
        file.close();
        //document()->setModified(false);
        return true;
    } else {
        return false;
    }
}

QString CodeEditor::path(){
    return d->path;
}

void CodeEditor::rename(const QString& path){
    d->path = path;
}


void CodeEditor::setLanguage(const QString& name){
    d->model->initLanguage(LanguageManager::getInstance()->get(name));
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

bool CodeEditor::isShowed(){
    return d->showed;
}

void CodeEditor::fillPlainText(const QString& text){
    QPlainTextEdit::setPlainText(text);
    this->setPlaceholderText("");
    this->document()->setModified(false);
}

void CodeEditor::insertCompletion(const QString &completion){
    if (d->completer->widget() != this)
        return;
    //qDebug()<<"completion:"<<completion;
    QTextCursor tc = textCursor();
    int length = d->completer->completionPrefix().length();
    //int extra = completion.length() - length;
    //tc.movePosition(QTextCursor::Left);
    //tc.movePosition(QTextCursor::EndOfWord);
    //tc.movePosition(QTextCursor::Left);
    //tc.movePosition(QTextCursor::EndOfWord);
    tc.movePosition(QTextCursor::Left,QTextCursor::KeepAnchor, length);
    //qDebug()<<"select:"<<tc.selectedText();
    //tc.deleteChar();
    tc.insertText(completion);
    setTextCursor(tc);
}

//![extraAreaWidth]

//![slotUpdateExtraAreaWidth]

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//![slotUpdateExtraAreaWidth]

//![slotUpdateRequest]

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        d->lineNumberArea->scroll(0, dy);
    else
        d->lineNumberArea->update(0, rect.y(), d->lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

//![slotUpdateRequest]

//![resizeEvent]

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    d->lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::keyPressEvent(QKeyEvent *event){
    if (d->completer && d->completer->popup()->isVisible()) {
        // Ignore keys for the completer
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            event->ignore();
            return; // Let the completer handle these key events
        default:
            break;
        }
    }

    QPlainTextEdit::keyPressEvent(event);
    const bool ctrlOrShift = event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!d->completer || (ctrlOrShift && event->text().isEmpty()))
        return;
    static QString eow("~!@#%^&*()+{}|:\"<>?,./;'[]\\-=");
    const bool hasModifier = (event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = this->textUnderCursor();

    if(completionPrefix.isEmpty()){
        d->completer->popup()->hide();
        return ;
    }

    if (!hasModifier && !event->text().isEmpty() && eow.contains(event->text().right(1))) {
        d->completer->popup()->hide();
        return;
    }

    if (completionPrefix != d->completer->completionPrefix()) {
        d->completer->setCompletionPrefix(completionPrefix);
        d->completer->popup()->setCurrentIndex(d->completer->completionModel()->index(0, 0));
    }

    QRect cr = cursorRect();
    cr.setWidth(d->completer->popup()->sizeHintForColumn(0)
                + d->completer->popup()->verticalScrollBar()->sizeHint().width());
    d->completer->complete(cr);
}

void CodeEditor::paintEvent(QPaintEvent *event){
    QPlainTextEdit::paintEvent(event);
    if (toPlainText().isEmpty() && !this->placeholderText().isEmpty()) {
        QPainter painter(viewport());
        painter.setPen(Qt::gray);
        painter.drawText(rect().adjusted(2, 2, -2, -2), Qt::AlignTop | Qt::TextWordWrap, this->placeholderText());
    }
}

void CodeEditor::showEvent(QShowEvent *event){
    QPlainTextEdit::showEvent(event);
    d->showed = true;
}

QString CodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    QString text = tc.selectedText();
    int position = tc.position();
    int anchor = tc.anchor();
    QTextDocument *doc = tc.document();
    while (anchor > 0 && doc->characterAt(anchor - 1) == '$') {
        anchor--;
    }
    tc.setPosition(anchor, QTextCursor::MoveAnchor);
    tc.setPosition(position, QTextCursor::KeepAnchor);
    text = tc.selectedText();
    return text;
}

//![resizeEvent]

//![cursorPositionChanged]

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(175);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

//![cursorPositionChanged]

//![extraAreaPaintEvent_0]

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(d->lineNumberArea);
    painter.fillRect(event->rect(), QColor(245,245,245));

    //![extraAreaPaintEvent_0]

    //![extraAreaPaintEvent_1]
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    //![extraAreaPaintEvent_1]

    //![extraAreaPaintEvent_2]
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::darkGray);
            painter.drawText(0, top, d->lineNumberArea->width() - 4, fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

}
