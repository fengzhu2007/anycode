#include "code_editor_view.h"
#include "code_editor_manager.h"
#include "cvs/diff_content.h"
#include <textdocument.h>
#include <core/coreconstants.h>
#include <core/find/highlightscrollbarcontroller.h>
#include <codeassist/documentcontentcompletion.h>
#include <syntaxhighlighter.h>
#include <semantichighlighter.h>
#include <textmark.h>
#include <texteditorsettings.h>
#include <fontsettings.h>
#include <utils/id.h>
#include <utils/theme/theme.h>

#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QTimer>
#include <QAction>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QMetaObject>


namespace ady{


class CodeEditorViewPrivate{
public:
    int line=0;
    int column=0;
    TextEditor::DocumentContentCompletionProvider* provider;
};

CodeEditorView::CodeEditorView(QWidget* parent)
    :TextEditor::TextEditorWidget(parent)
{
    d = new CodeEditorViewPrivate;
    d->provider = new TextEditor::DocumentContentCompletionProvider;
    auto doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    this->setTextDocument(doc);
    doc->setCompletionAssistProvider(d->provider);
}

CodeEditorView::~CodeEditorView(){
    delete m_diffScrollBarController;
    delete d->provider;
    delete d;
}

void CodeEditorView::contextMenuEvent(QContextMenuEvent *e){
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        QMenu contextMenu(this);
        instance->editorContextMenu(this,&contextMenu);
        contextMenu.exec(QCursor::pos());
    }else{
        TextEditor::TextEditorWidget::contextMenuEvent(e);
    }
}

void CodeEditorView::showEvent(QShowEvent *e){
    TextEditor::TextEditorWidget::showEvent(e);
    // Deferred scrollbar overlay recreation (same pattern as DiffEditorWidget)
    if (m_diffScrollBarController) {
        QMetaObject::invokeMethod(this, [this]() {
            if (m_diffScrollBarController) {
                m_diffScrollBarController->setScrollArea(nullptr);
                m_diffScrollBarController->setScrollArea(this);
            }
        }, Qt::QueuedConnection);
    }
}

void CodeEditorView::resizeEvent(QResizeEvent *e)
{
    TextEditor::TextEditorWidget::resizeEvent(e);
    if (m_diffScrollBarController) {
        m_diffScrollBarController->setLineHeight(fontMetrics().lineSpacing());
        m_diffScrollBarController->setVisibleRange(viewport()->rect().height());
        m_diffScrollBarController->setMargin(document()->documentMargin());
    }
}

void CodeEditorView::rename(const QString& name){
    this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
}




void CodeEditorView::addSemanticError(int line,int column,int length,const QString& message){
    //this->clearSemanticErrorMarks();
        if(line<0){
            return ;
        }
        if(length==0){
            auto doc = this->document();
            if(column>1){
                column -= 1;
            }
            length = doc->findBlockByLineNumber(line).length() - column;
        }
        line += 1;
        auto textDocument = this->textDocument();
        auto mark = new TextEditor::TextMark(textDocument->filePath(),line,TextEditor::Constants::SEMANTIC_ERROR_ID);
        mark->setIcon(QIcon(":/Resource/icons/StatusCriticalError_16x.svg"));
        mark->setColor(Utils::Theme::CodeModel_Error_TextMarkColor);
        mark->setDefaultToolTip(tr("Code Semantic Error"));
        mark->setToolTip(message);
        mark->setPriority(TextEditor::TextMark::HighPriority);
        mark->setLineAnnotation(message);

        mark->setActionsProvider([this,message](){
            QList<QAction*> actions;
            auto action = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy"),this);
            actions<<action;
            connect(action,&QAction::triggered,[this,message](){
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(message);
            });
            return actions;
        });

        textDocument->addMark(mark);
        auto highlighter = textDocument->syntaxHighlighter();
        //int line, int column, int length, int kind
        if(column<=0){
            column = 1;
        }
        //qDebug()<<"length:"<<length<<column<<line;
        TextEditor::HighlightingResult hlr(line,column,length,TextEditor::C_ERROR);
        QHash<int, QTextCharFormat> kindToFormat;
        QList<TextEditor::HighlightingResult> results;
        results<<hlr;
        QTextCharFormat format = TextEditor::TextEditorSettings::fontSettings().toTextCharFormat(TextEditor::C_ERROR);
        kindToFormat.insert(TextEditor::C_ERROR,format);
        TextEditor::SemanticHighlighter::setExtraAdditionalFormats(highlighter,results,kindToFormat);
}

void CodeEditorView::clearSemanticError(){
    auto textDocument = this->textDocument();
    auto highlighter = textDocument->syntaxHighlighter();
    TextEditor::SemanticHighlighter::setExtraAdditionalFormats(highlighter,{},{});

     //remove error marks
    this->clearSemanticErrorMarks();
}

void CodeEditorView::clearSemanticErrorMarks(){
    auto textDocument = this->textDocument();
    auto marks = textDocument->marks();
    for(auto mark:marks){
        if(mark->category()==TextEditor::Constants::SEMANTIC_ERROR_ID){
            textDocument->removeMark(mark);
        }
    }
}

//----------------------------------------------------------------------------
// Diff annotation infrastructure
//
// All methods in this section are no-ops when m_lineInfo is empty, so a
// plain CodeEditorView (without diff data) is completely unaffected.
//----------------------------------------------------------------------------

void CodeEditorView::setDiffHighlights(const cvs::DiffContent &content)
{
    m_lineInfo.clear();

    if (content.isEmpty()) {
        clearDiffHighlights();
        return;
    }

    // Build m_lineInfo from the diff hunks (diff-only mode — just the hunk
    // lines without reading the full file from disk).
    int docLine = 0;
    const auto hunks = content.hunks();
    for (const auto &hunk : hunks) {
        QString header = hunk.header().trimmed();
        if (header.isEmpty()) {
            header = QString("@@ -%1,%2 +%3,%4 @@")
                         .arg(hunk.oldStart()).arg(hunk.oldCount())
                         .arg(hunk.newStart()).arg(hunk.newCount());
        }
        m_lineInfo[docLine] = LineInfo{};
        m_lineInfo[docLine].isHeader = true;
        ++docLine;

        const auto lines = hunk.lines();
        for (const auto &line : lines) {
            LineInfo info;
            info.type = line.type();
            info.oldLineNo = line.oldLineNo();
            info.newLineNo = line.newLineNo();
            m_lineInfo[docLine] = info;
            ++docLine;
        }
    }

    applyLineHighlights();
    updateScrollBarMarkers();
    viewport()->update();
}

void CodeEditorView::clearDiffHighlights()
{
    m_lineInfo.clear();

    if (m_diffScrollBarController) {
        m_diffScrollBarController->removeHighlights(Utils::Id("DiffEditor.ScrollBarAddition"));
        m_diffScrollBarController->removeHighlights(Utils::Id("DiffEditor.ScrollBarDeletion"));
    }

    setExtraSelections(TextEditorWidget::OtherSelection, {});
    extraArea()->update();
    viewport()->update();
}

void CodeEditorView::applyLineHighlights()
{
    const auto &fontSettings = textDocument()->fontSettings();
    QTextCharFormat addedFormat = fontSettings.toTextCharFormat(TextEditor::C_DIFF_DEST_LINE);
    QTextCharFormat deletedFormat = fontSettings.toTextCharFormat(TextEditor::C_DIFF_SOURCE_LINE);

    QTextCharFormat headerFormat;
    headerFormat.setBackground(QColor(240, 240, 240));
    headerFormat.setProperty(QTextFormat::FullWidthSelection, true);

    QList<QTextEdit::ExtraSelection> selections;

    for (auto it = m_lineInfo.begin(); it != m_lineInfo.end(); ++it) {
        int blockNo = it.key();
        const LineInfo &info = it.value();

        QTextBlock block = document()->findBlockByNumber(blockNo);
        if (!block.isValid())
            continue;

        QTextEdit::ExtraSelection sel;
        sel.cursor = QTextCursor(block);
        sel.cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        if (info.isHeader) {
            sel.format = headerFormat;
            selections << sel;
            continue;
        }

        switch (info.type) {
        case cvs::DiffLine::Addition:
            sel.format = addedFormat;
            sel.format.setProperty(QTextFormat::FullWidthSelection, true);
            selections << sel;
            break;
        case cvs::DiffLine::Deletion:
            sel.format = deletedFormat;
            sel.format.setProperty(QTextFormat::FullWidthSelection, true);
            selections << sel;
            break;
        case cvs::DiffLine::Modification:
            sel.format = addedFormat;
            sel.format.setProperty(QTextFormat::FullWidthSelection, true);
            selections << sel;
            break;
        case cvs::DiffLine::Context:
        default:
            break;
        }
    }

    setExtraSelections(TextEditorWidget::OtherSelection, selections);
}

void CodeEditorView::updateScrollBarMarkers()
{
    if (!m_diffScrollBarController) {
        m_diffScrollBarController = new Core::HighlightScrollBarController;
        m_diffScrollBarController->setScrollArea(this);
    }
    auto *ctrl = m_diffScrollBarController;

    ctrl->setLineHeight(fontMetrics().lineSpacing());
    ctrl->setVisibleRange(viewport()->rect().height());
    ctrl->setMargin(document()->documentMargin());

    static const Utils::Id additionCategory("DiffEditor.ScrollBarAddition");
    static const Utils::Id deletionCategory("DiffEditor.ScrollBarDeletion");

    ctrl->removeHighlights(additionCategory);
    ctrl->removeHighlights(deletionCategory);

    for (auto it = m_lineInfo.constBegin(); it != m_lineInfo.constEnd(); ++it) {
        const LineInfo &info = it.value();
        if (info.isHeader)
            continue;

        QTextBlock block = document()->findBlockByNumber(it.key());
        if (!block.isValid())
            continue;

        const int position = block.firstLineNumber();

        switch (info.type) {
        case cvs::DiffLine::Addition:
        case cvs::DiffLine::Modification:
            ctrl->addHighlight({additionCategory, position,
                                Utils::Theme::VcsBase_FileAdded_TextColor,
                                Core::Highlight::NormalPriority});
            break;
        case cvs::DiffLine::Deletion:
            ctrl->addHighlight({deletionCategory, position,
                                Utils::Theme::VcsBase_FileDeleted_TextColor,
                                Core::Highlight::NormalPriority});
            break;
        default:
            break;
        }
    }
}

int CodeEditorView::extraAreaWidth(int *markWidthPtr) const
{
    int baseWidth = TextEditor::TextEditorWidget::extraAreaWidth(markWidthPtr);

    if (m_lineInfo.isEmpty())
        return baseWidth;

    const QFontMetrics fm(font());
    const int digitW = fm.horizontalAdvance('9');

    int maxLineNo = 1;
    for (const auto &info : m_lineInfo) {
        if (info.newLineNo > maxLineNo) maxLineNo = info.newLineNo;
        if (info.oldLineNo > maxLineNo) maxLineNo = info.oldLineNo;
    }
    const int digits = QString::number(maxLineNo).length();
    const int lineColW = digitW * digits;
    const int markerW  = digitW + 4;
    const int pad      = 4;

    int extra = pad + lineColW + pad + markerW + pad;
    int total = baseWidth + extra;

    const_cast<CodeEditorView*>(this)->setViewportMargins(
        isLeftToRight() ? total : 0, 0,
        isLeftToRight() ? 0 : total, 0);

    return total;
}

void CodeEditorView::extraAreaPaintEvent(QPaintEvent *e)
{
    if (m_lineInfo.isEmpty()) {
        TextEditor::TextEditorWidget::extraAreaPaintEvent(e);
        return;
    }

    QPainter painter(extraArea());
    painter.fillRect(e->rect(), extraArea()->palette().color(QPalette::Window));

    const QFontMetrics fm(font());
    const int digitW = fm.horizontalAdvance('9');

    int maxLineNo = 1;
    for (const auto &info : m_lineInfo) {
        if (info.newLineNo > maxLineNo) maxLineNo = info.newLineNo;
        if (info.oldLineNo > maxLineNo) maxLineNo = info.oldLineNo;
    }
    const int digits = QString::number(maxLineNo).length();
    const int lineColW = digitW * digits;
    const int markerW  = digitW + 4;
    const int pad      = 4;

    int x = pad;
    const int lineX = x;
    x += lineColW + pad;
    const int markerX = x;

    QTextBlock block = firstVisibleBlock();
    QPointF offset = contentOffset();

    while (block.isValid()) {
        QRectF blockRect = blockBoundingRect(block).translated(offset);

        if (blockRect.bottom() >= e->rect().top()
            && blockRect.top() <= e->rect().bottom())
        {
            int blockNo = block.blockNumber();
            auto it = m_lineInfo.find(blockNo);

            if (it != m_lineInfo.end()) {
                const LineInfo &info = it.value();
                const int top  = int(blockRect.top());
                const int h    = int(blockRect.height());

                if (info.newLineNo > 0) {
                    painter.setPen(QColor(120, 120, 120));
                    painter.drawText(QRect(lineX, top, lineColW, h),
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     QString::number(info.newLineNo));
                }

                if (!info.isHeader) {
                    switch (info.type) {
                    case cvs::DiffLine::Addition:
                        painter.setPen(QColor(0, 160, 0));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "+");
                        break;
                    case cvs::DiffLine::Deletion:
                        painter.setPen(QColor(220, 20, 20));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "-");
                        break;
                    case cvs::DiffLine::Modification:
                        painter.setPen(QColor(200, 140, 0));
                        painter.drawText(QRect(markerX, top, markerW, h),
                                         Qt::AlignCenter, "~");
                        break;
                    case cvs::DiffLine::Context:
                    default:
                        break;
                    }
                }
            }
        }

        offset.ry() += blockRect.height();
        if (offset.y() > height())
            break;

        block = block.next();
    }
}

} // namespace ady

