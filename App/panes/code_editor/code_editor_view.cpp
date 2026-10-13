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
#include <colorpreviewhoverhandler.h>
#include <languages/loader.h>
#include <texteditorenvironment.h>
#include "addon_loader.h"
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
#include <QKeyEvent>
#include <QMetaObject>
#include <QMargins>
#include <QStringList>


namespace ady{


class CodeEditorViewPrivate{
public:
    int line=0;
    int column=0;
    TextEditor::DocumentContentCompletionProvider* provider;
    TextEditor::ColorPreviewHoverHandler* colorHoverHandler = nullptr;
};

CodeEditorView::CodeEditorView(QWidget* parent)
    :TextEditor::TextEditorWidget(parent)
{
    d = new CodeEditorViewPrivate;
    d->provider = new TextEditor::DocumentContentCompletionProvider;
    auto doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    this->setTextDocument(doc);
    doc->setCompletionAssistProvider(d->provider);

    d->colorHoverHandler = new TextEditor::ColorPreviewHoverHandler;
    this->addHoverHandler(d->colorHoverHandler);

    // A disk reload replaces the document, invalidating any active diff
    // view.  Restore the view state so the editor becomes editable again;
    // only put the saved text back when the reload failed and the document
    // still holds the rebuilt diff content.
    connect(textDocument(), &TextEditor::TextDocument::reloadFinished, this,
            [this](bool success) {
                if (m_diffViewActive)
                    resetDiffViewState(!success);
            });
}

CodeEditorView::~CodeEditorView(){
    delete m_diffScrollBarController;
    delete d->provider;
    delete d->colorHoverHandler;
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

static bool isDiffViewEditingKey(const QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return true;
    default:
        break;
    }
    if (e->modifiers() & Qt::ControlModifier) {
        switch (e->key()) {
        case Qt::Key_V:
        case Qt::Key_X:
        case Qt::Key_Z:
        case Qt::Key_Y:
            return true;
        default:
            break;
        }
    }
    const QString text = e->text();
    return !text.isEmpty() && text.at(0).isPrint();
}

void CodeEditorView::keyPressEvent(QKeyEvent *e)
{
    // Any key that would edit the text first leaves the read-only diff view
    // (restoring the file content); Escape closes it explicitly.  Navigation,
    // selection and copying keys pass through to the editor.
    if (m_diffViewActive && (e->key() == Qt::Key_Escape || isDiffViewEditingKey(e))) {
        resetDiffViewState(true);
        return;
    }
    TextEditor::TextEditorWidget::keyPressEvent(e);
}

void CodeEditorView::rename(const QString& name){
    this->textDocument()->setFilePath(Utils::FilePath::fromString(name));
}

static QMap<QString,QString> s_languageAddonMap;

TextEditor::LanguageLoader *CodeEditorView::createLanguageLoader(const QString &languageName, QTextDocument *doc)
{
    auto loader = AddonLoader::getInstance();

    //try addon language loaders (AddonLoader handles filter + match + cache internally)
    if(auto langLoader = loader->createLanguageLoader(languageName, doc)){
        return langLoader;
    }

    //built-in syntax highlighter for Markdown/JSON
    if(languageName=="Markdown" || languageName=="JSON"){
        this->applySyntaxHighlighter(languageName, /*useEditorThemeColors=*/true);
        return nullptr;
    }
    // Fall back to built-in registry
    return TextEditor::TextEditorWidget::createLanguageLoader(languageName, doc);
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
    // Never rebuild the document while it has unsaved edits — the diff view
    // replaces the document content and would silently discard them.
    if (document()->isModified())
        return;

    // Leave a previous diff view first: its rebuilt content must not be used
    // as the "new file" source below.
    if (m_diffViewActive)
        resetDiffViewState(true);

    if (content.isEmpty()) {
        clearDiffHighlights();
        return;
    }

    // The document currently holds the complete new file (the pane reloaded
    // it from disk right before the diff was fetched).  Build the diff view
    // by interleaving deleted lines at their original positions, so a
    // replaced line shows as a "-" line followed by its "+" replacement
    // (git-diff style), while every other line keeps its line number.
    const QString fileText = document()->toPlainText();
    m_savedFileText = fileText;
    m_savedReadOnly = isReadOnly();
    m_savedLineNumbersVisible = lineNumbersVisible();
    m_diffViewActive = true;

    // Disable syntax highlighting — the diff view mixes old and new content.
    if (!m_savedHighlighter)
        m_savedHighlighter = textDocument()->takeSyntaxHighlighter();

    m_lineInfo.clear();
    QString text;
    int docLine = 0;

    QStringList fileLines = fileText.split('\n');
    if (!fileLines.isEmpty() && fileLines.last().isEmpty())
        fileLines.removeLast();

    int fileIdx = 0;         // 0-based index into fileLines
    int oldNewOffset = 0;    // oldLineNo = newLineNo + offset for unchanged lines

    const auto hunks = content.hunks();
    for (const auto &hunk : hunks) {
        const int hunkNewStart = hunk.newStart();

        // Emit unchanged file lines before the hunk
        while (fileIdx < hunkNewStart - 1 && fileIdx < fileLines.size()) {
            LineInfo info;
            info.type = cvs::DiffLine::Context;
            info.newLineNo = fileIdx + 1;
            info.oldLineNo = fileIdx + 1 + oldNewOffset;
            m_lineInfo[docLine] = info;
            text += fileLines.at(fileIdx) + '\n';
            ++docLine;
            ++fileIdx;
        }

        const auto lines = hunk.lines();
        for (const auto &line : lines) {
            if (line.type() == cvs::DiffLine::Deletion) {
                // Deleted line — no longer in the file; its content comes
                // from the diff itself (the +/-/space prefix is stripped,
                // the gutter marker is the visual cue).
                QString contentText = line.content();
                if (!contentText.isEmpty()) {
                    const QChar first = contentText.at(0);
                    if (first == '+' || first == '-' || first == ' ')
                        contentText = contentText.mid(1);
                }
                LineInfo info;
                info.type = cvs::DiffLine::Deletion;
                info.oldLineNo = line.oldLineNo();
                m_lineInfo[docLine] = info;
                text += contentText + '\n';
                ++docLine;
            } else {
                // Context / addition line — shown with the file content
                if (fileIdx >= fileLines.size())
                    continue;
                LineInfo info;
                info.type = line.type();
                info.oldLineNo = line.oldLineNo();
                info.newLineNo = line.newLineNo();
                m_lineInfo[docLine] = info;
                text += fileLines.at(fileIdx) + '\n';
                ++docLine;
                ++fileIdx;
                // Track the offset for the unchanged lines after this hunk
                if (info.newLineNo > 0 && info.oldLineNo > 0)
                    oldNewOffset = info.oldLineNo - info.newLineNo;
            }
        }
    }

    // Emit remaining unchanged file lines after the last hunk
    while (fileIdx < fileLines.size()) {
        LineInfo info;
        info.type = cvs::DiffLine::Context;
        info.newLineNo = fileIdx + 1;
        info.oldLineNo = fileIdx + 1 + oldNewOffset;
        m_lineInfo[docLine] = info;
        text += fileLines.at(fileIdx) + '\n';
        ++docLine;
        ++fileIdx;
    }

    // Rebuild the document in one go (block signals to avoid expensive
    // layout passes for every intermediate step).
    setUpdatesEnabled(false);
    document()->blockSignals(true);

    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    cursor.insertText(text);

    document()->blockSignals(false);
    setUpdatesEnabled(true);

    // The rebuilt document is a read-only view, not file content: keep the
    // modified flag clean and drop the undo history created by the rebuild.
    document()->setModified(false);
    document()->clearUndoRedoStacks();

    setReadOnly(true);
    setLineNumbersVisible(false);

    cursor.movePosition(QTextCursor::Start);
    setTextCursor(cursor);

    // Update viewport margins BEFORE applying highlights so the gutter is
    // wide enough when the first paint event arrives.  This is safe to run
    // synchronously: viewport margins and extraAreaWidth() stay consistent
    // (see additionalExtraAreaWidth()), so the re-layout triggered by the
    // margin change terminates instead of re-entering endlessly.
    updateDiffGutterWidth();

    applyLineHighlights();
    updateScrollBarMarkers();

    document()->markContentsDirty(0, document()->characterCount());
    viewport()->update();
}

void CodeEditorView::clearDiffHighlights()
{
    resetDiffViewState(true);
}

bool CodeEditorView::isDiffViewActive() const
{
    return m_diffViewActive;
}

void CodeEditorView::resetDiffViewState(bool restoreDocument)
{
    const bool wasActive = m_diffViewActive;
    m_diffViewActive = false;

    // Restore the file content unless the caller already replaced the
    // document (e.g. the pane reloaded it from disk).
    if (wasActive && restoreDocument && !m_savedFileText.isNull())
        textDocument()->setPlainText(m_savedFileText);
    m_savedFileText.clear();

    if (wasActive) {
        setReadOnly(m_savedReadOnly);
        setLineNumbersVisible(m_savedLineNumbersVisible);
    }

    m_lineInfo.clear();

    if (m_diffScrollBarController) {
        m_diffScrollBarController->removeHighlights(Utils::Id("DiffEditor.ScrollBarAddition"));
        m_diffScrollBarController->removeHighlights(Utils::Id("DiffEditor.ScrollBarDeletion"));
    }

    setExtraSelections(TextEditorWidget::OtherSelection, {});

    // Restore syntax highlighter if we took it away.
    if (m_savedHighlighter) {
        textDocument()->setSyntaxHighlighter(m_savedHighlighter);
        m_savedHighlighter = nullptr;
    }

    // Restore viewport margins to the base-class width (no diff markers).
    int baseWidth = TextEditor::TextEditorWidget::extraAreaWidth();
    QMargins desired{isLeftToRight() ? baseWidth : 0, 0, isLeftToRight() ? 0 : baseWidth, 0};
    if (viewportMargins() != desired)
        setViewportMargins(desired.left(), desired.top(), desired.right(), desired.bottom());

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

void CodeEditorView::updateDiffGutterWidth()
{
    int extra = diffGutterExtraWidth();
    if (extra <= 0)
        return;

    int baseWidth = TextEditor::TextEditorWidget::extraAreaWidth();
    int total = baseWidth + extra;

    QMargins desired{isLeftToRight() ? total : 0, 0, isLeftToRight() ? 0 : total, 0};
    if (viewportMargins() != desired)
        setViewportMargins(desired.left(), desired.top(), desired.right(), desired.bottom());
}

int CodeEditorView::additionalExtraAreaWidth() const
{
    return diffGutterExtraWidth();
}

int CodeEditorView::diffGutterExtraWidth() const
{
    if (m_lineInfo.isEmpty())
        return 0;

    // The diff gutter draws its own line numbers (the new number, or the old
    // one for an interleaved deleted line) plus one +/-/~ marker column, so
    // it reserves room for both.
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

    return pad + lineColW + pad + markerW + pad;
}

int CodeEditorView::extraAreaWidth(int *markWidthPtr) const
{
    int baseWidth = TextEditor::TextEditorWidget::extraAreaWidth(markWidthPtr);
    return baseWidth + diffGutterExtraWidth();
}

void CodeEditorView::extraAreaPaintEvent(QPaintEvent *e)
{
    // Diff view: the gutter is fully custom (the base line numbers are
    // hidden while the diff view is active) so every line can show its own
    // number — the new line number, or the old one for an interleaved
    // deleted line — next to its +/-/~ marker.
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
            auto it = m_lineInfo.find(block.blockNumber());
            if (it != m_lineInfo.end()) {
                const LineInfo &info = it.value();
                const int top = int(blockRect.top());
                const int h   = int(blockRect.height());

                if (info.newLineNo > 0) {
                    painter.setPen(QColor(120, 120, 120));
                    painter.drawText(QRect(lineX, top, lineColW, h),
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     QString::number(info.newLineNo));
                } else if (info.oldLineNo > 0) {
                    // Interleaved deleted line: show its old number so it can
                    // be located in the original file.
                    painter.setPen(QColor(150, 150, 150));
                    painter.drawText(QRect(lineX, top, lineColW, h),
                                     Qt::AlignRight | Qt::AlignVCenter,
                                     QString::number(info.oldLineNo));
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

