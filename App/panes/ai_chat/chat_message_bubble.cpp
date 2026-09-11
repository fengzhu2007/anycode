#include "chat_message_bubble.h"

#include "core/theme.h"

#include <QMetaObject>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QLabel>
#include <QTextBrowser>
#include <QTextDocument>
#include <QFontMetrics>
#include <QTextOption>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QStyleOption>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>
#include <QDebug>
#include <functional>

namespace ady{

static const char* RoleNames[] = {
    "user",       // User
    "assistant",  // Assistant
    "system",     // System
    "error",      // Error
    "event",      // Event
    "permission"  // Permission
};

// braille spinner frames for streaming indicator
static const QChar SPINNER_CHARS[] = {
    QChar(0x280B), QChar(0x2819), QChar(0x2839), QChar(0x2838),
    QChar(0x283C), QChar(0x2834), QChar(0x2826), QChar(0x2827),
    QChar(0x2807), QChar(0x280F)
};
static const int SPINNER_COUNT = 10;
static int s_spinnerFrame = 0;

// \x01 sentinel wrapping stashed fragments (tool blocks, inline md parts).
// Unlike '\0' it survives QString construction from literals and passes
// through toHtmlEscaped() untouched, so it can carry placeholders through
// the whole escape/split pipeline.
static const QChar PH(1);

// ---- markdown helpers (file-local) ----
namespace {

/**
 * Rebuild `input` with every match of `re` replaced by fn(match).
 * Used instead of QString::replace(re, QString) because replacements need
 * custom per-match processing (placeholder stashing, attribute escaping).
 */
QString replaceRegex(const QString &input, const QRegularExpression &re,
                     const std::function<QString(const QRegularExpressionMatch &)> &fn)
{
    QString out;
    out.reserve(input.size() + 32);
    int last = 0;
    QRegularExpressionMatchIterator it = re.globalMatch(input);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const int start = static_cast<int>(m.capturedStart());
        const int end = static_cast<int>(m.capturedEnd());
        out += input.mid(last, start - last);
        out += fn(m);
        last = end;
    }
    out += input.mid(last);
    return out;
}

// Inline markdown on an already HTML-escaped fragment: inline code, links,
// images, bold, italic, strikethrough. Complex fragments are stashed behind
// \x01S<i>\x01 placeholders so later rules cannot corrupt their HTML.
QString inlineMarkdown(const QString &escaped)
{
    QStringList stash;
    auto stashHtml = [&stash](const QString &html) {
        stash.append(html);
        return QString(PH) + "S" + QString::number(stash.size() - 1) + QString(PH);
    };

    QString s = escaped;

    // 1. inline code first (protect its content from other rules)
    static QRegularExpression codeRe("`([^`\n]+)`");
    s = replaceRegex(s, codeRe, [&stashHtml](const QRegularExpressionMatch &m) {
        return stashHtml(QString("<code style='font-family:Consolas,monospace;'>%1</code>")
                             .arg(m.captured(1)));
    });

    // 2. images: rendered as links showing the alt text (no remote loading)
    static QRegularExpression imgRe("!\\[([^\\]]*)\\]\\(([^)\\s]+)\\)");
    s = replaceRegex(s, imgRe, [&stashHtml](const QRegularExpressionMatch &m) {
        return stashHtml(QString("<a href='%1'>%2</a>")
                             .arg(m.captured(2).replace('\'', "%27"), m.captured(1)));
    });

    // 3. links
    static QRegularExpression linkRe("\\[([^\\]]+)\\]\\(([^)\\s]+)\\)");
    s = replaceRegex(s, linkRe, [&stashHtml](const QRegularExpressionMatch &m) {
        return stashHtml(QString("<a href='%1'>%2</a>")
                             .arg(m.captured(2).replace('\'', "%27"), m.captured(1)));
    });

    // 4. bold (must run before italic)
    static QRegularExpression boldRe("\\*\\*([^*\n]+)\\*\\*");
    s = replaceRegex(s, boldRe, [](const QRegularExpressionMatch &m) {
        return QString("<b>%1</b>").arg(m.captured(1));
    });
    static QRegularExpression boldUnderscoreRe("(?<![A-Za-z0-9])__([^_\n]+)__(?![A-Za-z0-9])");
    s = replaceRegex(s, boldUnderscoreRe, [](const QRegularExpressionMatch &m) {
        return QString("<b>%1</b>").arg(m.captured(1));
    });

    // 5. italic — the underscore variant needs boundary guards so that
    //    snake_case identifiers are left alone
    static QRegularExpression italicRe("\\*([^*\n]+)\\*");
    s = replaceRegex(s, italicRe, [](const QRegularExpressionMatch &m) {
        return QString("<i>%1</i>").arg(m.captured(1));
    });
    static QRegularExpression italicUnderscoreRe("(?<![A-Za-z0-9])_([^_\n]+)_(?![A-Za-z0-9])");
    s = replaceRegex(s, italicUnderscoreRe, [](const QRegularExpressionMatch &m) {
        return QString("<i>%1</i>").arg(m.captured(1));
    });

    // 6. strikethrough
    static QRegularExpression strikeRe("~~([^~\n]+)~~");
    s = replaceRegex(s, strikeRe, [](const QRegularExpressionMatch &m) {
        return QString("<s>%1</s>").arg(m.captured(1));
    });

    // restore stashed fragments
    for (int i = 0; i < stash.size(); ++i) {
        s.replace(QString(PH) + "S" + QString::number(i) + QString(PH), stash.at(i));
    }
    return s;
}

/**
 * Block-level markdown -> HTML: headings, bullet/ordered/task lists,
 * blockquotes, horizontal rules and plain paragraphs. Designed to be
 * re-run on partial streaming content: whatever is currently valid renders
 * correctly and self-corrects as more text arrives.
 */
QString markdownToHtml(const QString &text, const QString &quoteBorder, const QString &quoteFg)
{
    if (text.isEmpty()) return {};

    static QRegularExpression headingRe("^(#{1,6})\\s+(.*)$");
    static QRegularExpression hrRe("^\\s*(?:-{3,}|\\*{3,}|_{3,})\\s*$");
    static QRegularExpression quoteRe("^\\s*>\\s?(.*)$");
    static QRegularExpression ulRe("^\\s*[-*+]\\s+(.*)$");
    static QRegularExpression olRe("^\\s*(\\d+)[.)]\\s+(.*)$");
    static const int headingSizes[] = {18, 16, 15, 14, 13, 13};

    QString html;
    html.reserve(text.size() * 2);

    int listMode = 0;   // 0 = none, 1 = bullet list, 2 = ordered list
    int olCounter = 0;
    // Lists are rendered as plain indented lines with manually drawn markers:
    // Qt's <ul>/<li> rendering adds extra vertical spacing, and <ol> restarts
    // its numbering whenever a blank line splits the list into a new one.
    auto closeList = [&listMode, &olCounter]() {
        listMode = 0;
        olCounter = 0;
    };

    const QStringList lines = text.split(QLatin1Char('\n'));
    const int lineCount = lines.size();
    for (int i = 0; i < lineCount; ++i) {
        QString line = lines.at(i);
        if (line.endsWith(QLatin1Char('\r'))) line.chop(1);
        const bool lastLine = (i == lineCount - 1);

        if (line.trimmed().isEmpty()) {
            // Whitespace-only lines carry no effective content: skip them
            // entirely (no <br/>) so they never add vertical spacing.
            // Do NOT close the list here: a blank line inside a list must not
            // restart the ordered numbering.
            continue;
        }

        QRegularExpressionMatch m = headingRe.match(line);
        if (m.hasMatch()) {
            closeList();
            const int level = m.captured(1).size();
            html += QString("<b><span style='font-size:%1px;'>%2</span></b><br/>")
                        .arg(headingSizes[level - 1])
                        .arg(inlineMarkdown(m.captured(2).toHtmlEscaped()));
            continue;
        }

        m = ulRe.match(line);
        if (m.hasMatch()) {
            if (listMode != 1) {
                closeList();
                listMode = 1;
            }
            QString item = m.captured(1);
            QString marker = QString(QChar(0x2022)) + QLatin1Char(' ');   // bullet
            if (item.startsWith(QLatin1String("[ ] "))) {
                marker = QString(QChar(0x2610)) + QLatin1Char(' ');       // ballot box
                item = item.mid(4);
            } else if (item.startsWith(QLatin1String("[x] "))
                       || item.startsWith(QLatin1String("[X] "))) {
                marker = QString(QChar(0x2611)) + QLatin1Char(' ');       // checked box
                item = item.mid(4);
            }
            html += QString("<div style='margin-left:14px;'>%1%2</div>")
                        .arg(marker, inlineMarkdown(item.toHtmlEscaped()));
            continue;
        }

        m = olRe.match(line);
        if (m.hasMatch()) {
            if (listMode != 2) {
                closeList();
                listMode = 2;
                olCounter = m.captured(1).toInt();
            } else {
                ++olCounter;
            }
            html += QString("<div style='margin-left:14px;'>%1. %2</div>")
                        .arg(QString::number(olCounter),
                             inlineMarkdown(m.captured(2).toHtmlEscaped()));
            continue;
        }

        closeList();

        m = hrRe.match(line);
        if (m.hasMatch()) {
            html += QLatin1String("<hr/>");
            continue;
        }

        m = quoteRe.match(line);
        if (m.hasMatch()) {
            html += QString("<div style='border-left:3px solid %1;padding-left:8px;color:%2;'>%3</div>")
                        .arg(quoteBorder, quoteFg, inlineMarkdown(m.captured(1).toHtmlEscaped()));
            continue;
        }

        html += inlineMarkdown(line.toHtmlEscaped());
        if (!lastLine) html += QLatin1String("<br/>");
    }

    return html;
}

} // namespace

// ---- construction / destruction ----

ChatMessageBubble::ChatMessageBubble(Type type, const QString &content, QWidget *parent)
    : QWidget(parent)
    , m_type(type)
{
    this->setObjectName("ady--ChatMessageBubble");
    this->buildUi();
    this->applyStyle();
    this->setContent(content);

    // Permission type: build interactive card with buttons
    if(m_type == Permission){
        setupPermissionUI();
    }
}

ChatMessageBubble::~ChatMessageBubble() = default;

// ---- ui construction ----

void ChatMessageBubble::buildUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(8, 4, 8, 4);
    m_mainLayout->setSpacing(2);

    // Streaming indicator: a plain QLabel fully decoupled from the document,
    // so spinner ticks never re-render the (possibly large) content.
    m_spinnerLabel = new QLabel(this);
    m_spinnerLabel->hide();
    m_mainLayout->addWidget(m_spinnerLabel);


    m_browser = new QTextBrowser(this);

    m_browser->setLineWrapMode(QTextEdit::WidgetWidth);
    m_browser->setWordWrapMode(QTextOption::WrapAnywhere);


    m_browser->setFrameShape(QFrame::NoFrame);
    m_browser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_browser->setOpenLinks(false);          // emit anchorClicked, never navigate
    m_browser->setFocusPolicy(Qt::NoFocus);  // keep keyboard focus in the input box
    m_browser->setTextInteractionFlags(Qt::TextSelectableByMouse
                                       | Qt::LinksAccessibleByMouse
                                       | Qt::LinksAccessibleByKeyboard);
    m_browser->viewport()->setAutoFillBackground(false);
    // border:none overrides the pane-wide "QTextEdit{border:1px solid ...}"
    // rule from ai_chat_pane, which otherwise draws a frame around every
    // bubble (QTextBrowser matches QTextEdit's type rule).
    m_browser->setStyleSheet("QTextBrowser{background:transparent;border:none;color:#dcdcdc}");
    // The row height is driven entirely by heightForWidth(); neutralize the
    // browser's own minimumSizeHint so the layout can shrink below it.
    m_browser->setMinimumHeight(0);

    QTextDocument *doc = m_browser->document();
    doc->setDocumentMargin(0);
    QTextOption opt;
    opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    doc->setDefaultTextOption(opt);

    m_mainLayout->addWidget(m_browser, 1);

    m_spinnerTimer = new QTimer(this);
    m_spinnerTimer->setInterval(100);
    connect(m_spinnerTimer, &QTimer::timeout, this, &ChatMessageBubble::updateSpinner);
    connect(m_browser, &QTextBrowser::anchorClicked, this, &ChatMessageBubble::onAnchorClicked);
}

// ---- public api ----

void ChatMessageBubble::setContent(const QString &text)
{
    m_plainContent = text;
    m_streaming = false;
    m_thinkFromEvent = false;
    doUpdate();
}

void ChatMessageBubble::appendText(const QString &delta)
{
    m_plainContent.append(delta);
    scheduleUpdate();
}

void ChatMessageBubble::scheduleUpdate()
{
    if(m_updateScheduled) return;
    m_updateScheduled = true;
    QMetaObject::invokeMethod(this, [this](){
        m_updateScheduled = false;
        doUpdate();
    }, Qt::QueuedConnection);
}

void ChatMessageBubble::doUpdate()
{
    QString html = formatContent(m_plainContent);
    if(html == m_lastHtml)
        return;  // nothing changed — skip the re-parse and row-height churn
    m_lastHtml = html;
    m_browser->setHtml(html);
    //applyLineHeight();
    emit contentUpdated();
}

// Qt has no document-wide default line height, and the generated HTML is
// mostly bare text + <br/> lines: those form implicit blocks that no CSS
// selector can target, so a defaultStyleSheet rule cannot cover the body
// text. Instead, after each setHtml() walk every block and set a
// proportional line height on it. The parsed QTextBlockFormat is kept and
// only the line height field changes, preserving margins and indentation.
//
// ProportionalHeight extends each line below the text (the extra leading
// lands on the descent side), so every block ends with half a line of dead
// space — on a user bubble that reads as a larger bottom padding. Offset
// it with a negative bottom margin, kept additive to whatever margin the
// HTML already carries.
void ChatMessageBubble::applyLineHeight()
{
    QTextDocument *doc = m_browser->document();
    doc->setDefaultStyleSheet("div{line-height:130%}");
    /*for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        QTextBlockFormat fmt = block.blockFormat();
        fmt.setLineHeight(150, QTextBlockFormat::ProportionalHeight);  // 150%
        QTextCursor(block).setBlockFormat(fmt);
    }*/
}

void ChatMessageBubble::updateSpinner()
{
    s_spinnerFrame = (s_spinnerFrame + 1) % SPINNER_COUNT;
    if(m_spinnerLabel)
        m_spinnerLabel->setText(QString(SPINNER_CHARS[s_spinnerFrame]));
    // No document re-render: the spinner lives outside the document.
}

void ChatMessageBubble::setStreaming(bool streaming)
{
    if(m_streaming == streaming) return;
    m_streaming = streaming;
    if(streaming){
        m_spinnerLabel->show();
        m_spinnerLabel->setText(QString(SPINNER_CHARS[s_spinnerFrame]));
        m_spinnerTimer->start();
    }else{
        m_spinnerTimer->stop();
        m_spinnerLabel->hide();
    }
    emit contentUpdated();  // spinner row visibility changes the row height
}

QString ChatMessageBubble::roleOf(Type type)
{
    if(type >= 0 && type <= Permission){
        return QLatin1String(RoleNames[type]);
    }
    return QLatin1String("system");
}

ChatMessageBubble::Type ChatMessageBubble::typeOf(const QString &role)
{
    if(role == "user") return User;
    if(role == "assistant") return Assistant;
    if(role == "error") return Error;
    if(role == "event") return Event;
    if(role == "permission") return Permission;
    return System;
}

// ---- style ----

void ChatMessageBubble::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

QSize ChatMessageBubble::sizeHint() const
{
    int w = width();
    if (w <= 0) {
        w = parentWidget() ? parentWidget()->width() : 400;
    }
    return QSize(w, heightForWidth(w));
}

bool ChatMessageBubble::hasHeightForWidth() const
{
    return true;
}

int ChatMessageBubble::heightForWidth(int w) const
{
    if (w <= 0) return 0;

    const QMargins mc = m_mainLayout->contentsMargins();
    const int innerW = w - mc.left() - mc.right();
    if (innerW <= 0) return 0;

    int total = mc.top() + mc.bottom();

    // Streaming spinner row above the content
    if (m_streaming && m_spinnerLabel) {
        total += m_spinnerLabel->sizeHint().height() + m_mainLayout->spacing();
    }

    // Document height at the width the browser will actually get.
    // setTextWidth() dirties the layout; the size() query then performs a
    // single layout pass whose result is cached for repeated calls.
    // (document() returns a mutable pointer even from a const method.)
    QTextDocument *doc = m_browser->document();
    doc->setTextWidth(innerW);
    total += qCeil(doc->size().height());

    // Restore the wrap width the browser is actually displaying at, so a
    // transient measurement width can never stick to the rendered document.
    // QTextEdit only re-asserts its viewport width when its own width
    // changes, so a stale pin would otherwise persist (narrow content).
    const int viewportW = m_browser->viewport()->width();
    if (viewportW > 0 && viewportW != innerW)
        doc->setTextWidth(viewportW);

    // Permission type: account for the button row height
    if (m_type == Permission && m_permissionReply.isEmpty() && m_buttonRow) {
        total += 32 + m_mainLayout->spacing();
    }

    return total;
}

void ChatMessageBubble::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // The browser's own relayout only fires on width changes; re-pin the
    // document width here so the wrap always matches the real display width,
    // even if a measurement call left a stale value behind.
    syncDocumentWidth();
}

void ChatMessageBubble::syncDocumentWidth()
{
    if (!m_browser || !m_mainLayout)
        return;
    // The browser spans the full inner width of this widget (layout margins
    // on both sides). The document wrap width must equal it at all times;
    // documentMargin is applied inside the layout on top of this width.
    const QMargins mc = m_mainLayout->contentsMargins();
    const int browserW = width() - mc.left() - mc.right();
    // Exact compare: both values are pinned from integer widths. Skipping
    // equal widths avoids a full document relayout on height-only resizes,
    // which are frequent while streaming.
    if (browserW > 0 && m_browser->document()->textWidth() != (qreal)browserW)
        m_browser->document()->setTextWidth(browserW);
}

void ChatMessageBubble::applyStyle()
{
    auto theme = Theme::getInstance();
    const QString bg = theme->secondaryBackgroundColor().lighter(130).name(QColor::HexRgb);
    const QString dim = theme->secondaryTextColor().name(QColor::HexRgb);

    m_spinnerLabel->setStyleSheet(QString("color:%1;font-size:13px;").arg(dim));

    switch(m_type){
    case User:
        // The browser itself renders the bubble; the document margin doubles
        // as the bubble's inner padding (accounted for in heightForWidth).
        m_browser->document()->setDocumentMargin(8);
        m_browser->setStyleSheet(QString(
            "QTextBrowser{background-color:%1;border:none;border-radius:6px;}"
        ).arg("#1f261f"));
        break;
    case Assistant:
        // default transparent styling from buildUi()
        this->applyLineHeight();
        break;
    case System:
    case Error:
        m_browser->setStyleSheet("QTextBrowser{background:transparent;border:none;}");
        this->setStyleSheet(QString(
            "#ady--ChatMessageBubble{background-color:%1;border:none;border-radius:6px;}"
        ).arg(bg));
        break;
    case Event:
        m_browser->setStyleSheet(QString(
            "QTextBrowser{background:transparent;border:none;color:%1;font-size:11px;font-style:italic;}"
        ).arg(dim));
        {
            QTextOption opt = m_browser->document()->defaultTextOption();
            opt.setAlignment(Qt::AlignHCenter);
            m_browser->document()->setDefaultTextOption(opt);
        }
        break;
    case Permission:
        m_browser->setStyleSheet("QTextBrowser{background:transparent;border:none;}");
        this->setStyleSheet(QString(
            "#ady--ChatMessageBubble{background-color:%1;border:none;border-radius:6px;}"
        ).arg(bg));
        break;
    }
}

// ---- content formatting ----

QString ChatMessageBubble::formatContent(const QString &text)
{
    // Clear code block list (rebuilt each render)
    m_codeBlocks.clear();

    // Permission content is rendered by setupPermissionUI()
    if (m_type == Permission) {
        return {};
    }

    // For Event type, return just the plain text
    if (m_type == Event) {
        if (text.isEmpty()) return {};
        return text.toHtmlEscaped();
    }

    // Strip workspace directory context prefix (injected before sending, stored in history)
    static QRegularExpression wsContextRe(
        QStringLiteral("^\\[Current working director(?:y|ies):.*?\\]\\n?"),
        QRegularExpression::DotMatchesEverythingOption);
    QString displayText = text;
    displayText.remove(wsContextRe);

    // While waiting for the first chunk the spinner label alone is shown
    if (displayText.isEmpty() && m_toolBlocks.isEmpty() && !m_thinkFromEvent) {
        return {};
    }

    // ---- Build body HTML ----
    QString cleaned;
    QString thinkHtml;

    if (m_thinkFromEvent && !m_thinkContent.isEmpty()) {
        cleaned = displayText;
        thinkHtml = formatThinkBlock(m_thinkContent);
    } else {
        thinkHtml = extractAndFormatThinkContent(displayText, cleaned);
    }

    auto theme = Theme::getInstance();
    const QString quoteBorder = theme->borderColor().name(QColor::HexRgb);
    const QString quoteFg = theme->secondaryTextColor().name(QColor::HexRgb);

    // Replace tool markers with sentinel placeholders BEFORE HTML escaping,
    // because toHtmlEscaped() would turn <!--TOOL_N--> into &lt;!--TOOL_N--&gt;
    // making the later replacement fail.
    for (int i = 0; i < m_toolBlocks.size(); ++i) {
        cleaned.replace(QString("<!--TOOL_%1-->").arg(i),
                        QString(PH) + "T" + QString::number(i) + QString(PH));
    }

    // Fenced code blocks vs. markdown text
    const QStringList parts = cleaned.split("```");
    QString html;
    for(int i = 0; i < parts.size(); ++i){
        if(i % 2 == 0){
            html += markdownToHtml(parts[i], quoteBorder, quoteFg);
        }else{
            html += formatCodeBlock(parts[i]);
        }
    }

    // Insert tool block HTML at the sentinel positions
    for (int i = 0; i < m_toolBlocks.size(); ++i) {
        html.replace(QString(PH) + "T" + QString::number(i) + QString(PH),
                     m_toolBlocks.at(i));
    }

    return thinkHtml + html;
}

QString ChatMessageBubble::extractAndFormatThinkContent(const QString &text, QString &cleaned) const
{
    // If thinking arrived via SSE event, skip tag extraction from text
    if (m_thinkFromEvent) {
        cleaned = text;
        return QString();
    }

    QStringList thinkBlocks;
    QString result;
    int pos = 0;
    const QString openTag = QStringLiteral("<think>");
    const QString closeTag = QStringLiteral("</think>");

    while(pos < text.length()){
        int thinkStart = text.indexOf(openTag, pos);
        if(thinkStart < 0){
            result += text.mid(pos);
            break;
        }
        QString between = text.mid(pos, thinkStart - pos);
        result += between;
        int thinkEnd = text.indexOf(closeTag, thinkStart + openTag.length());
        if(thinkEnd < 0){
            thinkBlocks.append(text.mid(thinkStart + openTag.length()));
            break;
        }
        thinkBlocks.append(text.mid(thinkStart + openTag.length(),
                                    thinkEnd - thinkStart - openTag.length()));
        pos = thinkEnd + closeTag.length();
    }

    // Merge think blocks when there is no effective text between them
    QString thinkContent;
    for (int i = 0; i < thinkBlocks.size(); ++i) {
        if (!thinkBlocks[i].trimmed().isEmpty()) {
            if (!thinkContent.isEmpty())
                thinkContent += "\n";
            thinkContent += thinkBlocks[i].trimmed();
        }
    }

    cleaned = result.trimmed();

    if(thinkContent.isEmpty()){
        return QString();
    }

    const_cast<ChatMessageBubble*>(this)->m_thinkContent = thinkContent.trimmed();
    return formatThinkBlock(thinkContent.trimmed());
}

QString ChatMessageBubble::formatThinkBlock(const QString &thinkContent) const
{
    auto theme = Theme::getInstance();
    QString thinkColor = (theme->style() == Theme::Dark) ? "#888888" : "#666666";
    QString linkColor = "#4a9eff";

    if(m_thinkExpanded){
        QString escapedThink = thinkContent.toHtmlEscaped();
        escapedThink.replace("\n", "<br/>");
        return QString(
            "<div style='border-left:3px solid %1;padding:8px;margin:4px 0;"
            "color:%1;font-size:12px;'>"
            "<b>[thinking]</b><br/>%2<br/>"
            "<a href='collapse' style='color:%3;'>collapse</a>"
            "</div><br/>"
        ).arg(thinkColor, escapedThink, linkColor);
    }else{
        // Collapsed: show only first line
        QString firstLine = thinkContent;
        int nlPos = firstLine.indexOf('\n');
        if(nlPos > 0){
            firstLine = firstLine.left(nlPos);
        }
        if(firstLine.length() > 80){
            firstLine = firstLine.left(80) + "...";
        }
        QString escapedFirstLine = firstLine.toHtmlEscaped();
        return QString(
            "<div style='border-left:3px solid %1;padding:8px;margin:4px 0;"
            "color:%1;font-size:12px;'>"
            "<b>[thinking]</b> %2 "
            "<a href='expand' style='color:%3;'>expand</a>"
            "</div><br/>"
        ).arg(thinkColor, escapedFirstLine, linkColor);
    }
}

void ChatMessageBubble::onToggleThink()
{
    m_thinkExpanded = !m_thinkExpanded;
    doUpdate();
}

void ChatMessageBubble::onAnchorClicked(const QUrl &url)
{
    const QString link = url.toString();
    if (link == "expand" || link == "collapse") {
        onToggleThink();
    } else if (link.startsWith("copy:")) {
        copyCode(link.mid(5).toInt());
    }
    // Regular markdown links are intentionally inert (no navigation).
}

void ChatMessageBubble::copyCode(int index) const
{
    if (index >= 0 && index < m_codeBlocks.size()) {
        QApplication::clipboard()->setText(m_codeBlocks.at(index));
    }
}

QString ChatMessageBubble::formatCodeBlock(const QString &code)
{
    QString body = code;
    QString lang;

    int nl = body.indexOf('\n');
    if(nl > 0){
        lang = body.left(nl).trimmed();
        if(!lang.isEmpty() && !lang.contains(' ')){
            body = body.mid(nl + 1);
        }else{
            lang.clear();
        }
    }
    while(body.endsWith('\n')){
        body.chop(1);
    }

    // Store raw code for clipboard copy
    const int idx = m_codeBlocks.size();
    m_codeBlocks.append(body);

    auto theme = Theme::getInstance();
    QString codeBg, codeFg, dimColor;
    if(theme->style() == Theme::Dark){
        codeBg = "#2b2b2b";
        codeFg = "#dcdcdc";
        dimColor = "#666666";
    }else{
        codeBg = "#f6f8fa";
        codeFg = "#24292e";
        dimColor = "#999999";
    }
    QString linkColor = "#4a9eff";

    return QString(
        "<div style='background-color:%1;border-radius:4px;margin:4px 0;'>"
        "<div style='text-align:right;padding:4px 8px 0 8px;'>"
        "<span style='color:%3;font-size:10px;'>%4</span> "
        "<a href='copy:%5' style='color:%6;font-size:11px;text-decoration:none;'>[copy] %7</a>"
        "</div>"
        "<pre style='background-color:%1;color:%2;padding:8px;margin:0;"
        "font-family:Consolas,monospace;font-size:12px;'>%8</pre>"
        "</div>"
    ).arg(codeBg, codeFg, dimColor, lang.toHtmlEscaped(),
          QString::number(idx), linkColor, tr("Copy"),
          body.toHtmlEscaped());
}

// ---- streaming indicator ----

// (setStreaming/updateSpinner are grouped with the public api above)

// ---- thinking content (from SSE event) ----

void ChatMessageBubble::appendThink(const QString &content)
{
    m_thinkFromEvent = true;
    m_thinkContent = content.trimmed();
    m_thinkExpanded = false;
    doUpdate();
}

// ---- tool use / tool result block ----

QString ChatMessageBubble::detectShellType(const QString &toolName, const QString &command)
{
    QString tn = toolName.toLower();
    if(tn == "cmd" || tn.contains("cmd.exe") || tn.contains("cmd "))
        return "cmd";
    if(tn == "powershell" || tn.contains("powershell") || tn == "ps1")
        return "powershell";

    // Detect from command content
    QString cmd = command.trimmed();
    if(cmd.startsWith("$env:", Qt::CaseInsensitive)
       || cmd.contains("Write-Host", Qt::CaseInsensitive)
       || cmd.contains("Get-ChildItem", Qt::CaseInsensitive)
       || cmd.contains("Get-Content", Qt::CaseInsensitive)
       || cmd.contains("Set-Location", Qt::CaseInsensitive)
       || cmd.contains("Invoke-WebRequest", Qt::CaseInsensitive))
        return "powershell";
    if(cmd.startsWith("@echo", Qt::CaseInsensitive)
       || cmd.startsWith("dir ", Qt::CaseInsensitive)
       || cmd.startsWith("set ", Qt::CaseInsensitive)
       || cmd.startsWith("copy ", Qt::CaseInsensitive))
        return "cmd";

    return "shell";
}

void ChatMessageBubble::appendToolBlock(const QString &callID, const QString &toolType, const QString &toolName, const QString &body)
{
    ToolCallInfo info;
    info.callID = callID;
    info.toolType = toolType;
    info.toolName = toolName;
    info.status = ToolProcessing;
    info.body = body;

    // Detect command-line tools by raw tool type from SSE
    QString lowerType = toolType.toLower();
    if(lowerType == "bash" || lowerType == "cmd" || lowerType == "powershell"
       || lowerType == "shell" || lowerType.contains("terminal")) {
        info.isCommand = true;
        info.shellType = detectShellType(toolType, body);
    }

    info.blockIndex = m_toolBlocks.size();
    m_toolCalls.append(info);

    // Build initial HTML with Processing status
    QString html = buildToolBlockHtml(info);
    m_toolBlocks.append(html);
    m_plainContent += QString("<!--TOOL_%1-->\n").arg(info.blockIndex);
    scheduleUpdate();
}

void ChatMessageBubble::updateToolStatus(const QString &callID, ToolStatus status, const QString &output)
{
    // Find the tool call by callID
    for(int i = 0; i < m_toolCalls.size(); ++i) {
        if(m_toolCalls[i].callID == callID) {
            m_toolCalls[i].status = status;
            if(!output.isEmpty()) {
                m_toolCalls[i].body = output;
            }
            // Rebuild the HTML for this tool block
            if(m_toolCalls[i].blockIndex >= 0 && m_toolCalls[i].blockIndex < m_toolBlocks.size()) {
                m_toolBlocks[m_toolCalls[i].blockIndex] = buildToolBlockHtml(m_toolCalls[i]);
            }
            scheduleUpdate();
            return;
        }
    }
    // callID not found - this shouldn't happen normally, but handle gracefully
    qDebug() << "[ChatMessageBubble] updateToolStatus: callID not found:" << callID;
}

QString ChatMessageBubble::buildToolBlockHtml(const ToolCallInfo &info) const
{
    auto theme = Theme::getInstance();
    QString blockBg, blockFg, accentColor, statusIcon, statusText;

    if(theme->style() == Theme::Dark) {
        blockBg = "#1e2a1e";
        blockFg = "#c8d6c8";
    } else {
        blockBg = "#f0f7f0";
        blockFg = "#2d3b2d";
    }

    switch(info.status) {
    case ToolProcessing:
        accentColor = "#e6a23c";  // orange
        statusIcon = QChar(0x23F3);  // hourglass
        statusText = tr("Processing");
        break;
    case ToolSuccess:
        accentColor = "#42b983";  // green
        statusIcon = QChar(0x2714);  // checkmark
        statusText = tr("Success");
        break;
    case ToolFailure:
        accentColor = "#e74c3c";  // red
        statusIcon = QChar(0x2718);  // cross
        statusText = tr("Failure");
        break;
    }

    QString html;
    QString displayName = info.toolName.toHtmlEscaped();

    if(info.isCommand) {
        // Command-line tool: show shell type badge and full command
        QString shellBadge = info.shellType.toUpper();
        QString command = info.body.toHtmlEscaped();
        command.replace("\n", "<br/>");

        if(info.status == ToolProcessing) {
            // Compact form: just show command with spinner
            html = QString(
                "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
                "padding:8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
                "<b style='color:%3;'>%4 %5</b> "
                "<span style='background-color:%3;color:#fff;padding:1px 6px;border-radius:3px;"
                "font-size:10px;font-weight:bold;'>%6</span><br/>"
                "<code style='font-family:Consolas,monospace;font-size:11px;color:%2;'>%7</code>"
                "</div>"
            ).arg(blockBg, blockFg, accentColor, statusIcon, displayName, shellBadge, command);
        } else {
            // Completed: show command with status only, no output content
            html = QString(
                "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
                "padding:8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
                "<b style='color:%3;'>%4 %5</b> "
                "<span style='background-color:%3;color:#fff;padding:1px 6px;border-radius:3px;"
                "font-size:10px;font-weight:bold;'>%6</span> "
                "<span style='color:%3;font-size:11px;'>%7</span>"
                "</div>"
            ).arg(blockBg, blockFg, accentColor, statusIcon, displayName, shellBadge, statusText);
        }
    } else {
        // Non-command tool: show tool name and parameters/output
        QString bodyText = info.body.toHtmlEscaped();
        bodyText.replace("\n", "<br/>");

        // Truncate long output for readability
        if(info.body.length() > 500) {
            bodyText = info.body.left(500).toHtmlEscaped().replace("\n", "<br/>")
                       + "<br/><i>... (truncated)</i>";
        }

        if(info.status == ToolProcessing) {
            // Compact form: just tool name
            html = QString(
                "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
                "padding:8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
                "<b style='color:%3;'>%4 %5</b> <span style='color:%3;font-size:11px;'>%6</span>"
                "</div>"
            ).arg(blockBg, blockFg, accentColor, statusIcon, displayName, statusText);
        } else {
            // Completed: show tool name, status, and body
            html = QString(
                "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
                "padding:8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
                "<b style='color:%3;'>%4 %5</b> "
                "<span style='color:%3;font-size:11px;'>%6</span><br/>"
                "<code style='font-family:Consolas,monospace;font-size:11px;'>%7</code>"
                "</div>"
            ).arg(blockBg, blockFg, accentColor, statusIcon, displayName, statusText, bodyText);
        }
    }

    return html;
}

// ---- permission request UI ----

void ChatMessageBubble::setupPermissionUI()
{
    auto theme = Theme::getInstance();
    const QString accent = theme->primaryColor().name(QColor::HexRgb);
    const QString fg = theme->secondaryTextColor().name(QColor::HexRgb);

    // Parse JSON content to extract permission data
    QJsonDocument doc = QJsonDocument::fromJson(m_plainContent.toUtf8());
    QJsonObject obj = doc.object();
    m_permissionRequestId = obj["requestId"].toString();
    QString toolName = obj["toolName"].toString();
    QString detail = obj["detail"].toString();

    // Render the permission description into the document
    const QString html = QString(
        "<div style='color:%1;font-size:12px;'>"
        "<b style='color:%2;'>\u26a0 Permission Request</b><br/>"
        "Tool [<b>%3</b>] is requesting permission<br/>"
        "<span style='color:%4;'>%5</span>"
        "</div>"
    ).arg(fg, accent, toolName.toHtmlEscaped(), fg, detail.toHtmlEscaped());
    m_lastHtml = html;
    m_browser->setHtml(html);
    applyLineHeight();

    // If already replied (widget recreated after scroll), show disabled state
    if(!m_permissionReply.isEmpty()){
        setPermissionReplied(m_permissionReply);
        return;
    }

    // Create button row below the content
    m_buttonRow = new QWidget(this);
    QHBoxLayout *btnLayout = new QHBoxLayout(m_buttonRow);
    btnLayout->setSpacing(6);
    btnLayout->setContentsMargins(0, 4, 0, 0);

    auto makeBtn = [&](const QString &text, const QString &bg, const QString &fgColor) -> QPushButton* {
        auto *btn = new QPushButton(text, m_buttonRow);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setStyleSheet(QString(
            "QPushButton{background-color:%1;color:%2;border:none;border-radius:4px;"
            "padding:4px 12px;font-size:11px;}"
            "QPushButton:hover{background-color:%1;}"
        ).arg(bg, fgColor));
        return btn;
    };

    QPushButton *btnOnce   = makeBtn(tr("Allow Once"),   "#2d6a4f", "#ffffff");
    QPushButton *btnAlways = makeBtn(tr("Allow Always"), "#1a759f", "#ffffff");
    QPushButton *btnReject = makeBtn(tr("Reject"),       "#6c757d", "#ffffff");

    btnLayout->addStretch();
    btnLayout->addWidget(btnOnce);
    btnLayout->addWidget(btnAlways);
    btnLayout->addWidget(btnReject);

    m_mainLayout->addWidget(m_buttonRow);

    // Connect button clicks — defer the state transition via QTimer::singleShot
    // so the button is not destroyed while its own clicked() signal is dispatching.
    connect(btnOnce, &QPushButton::clicked, this, [this](){
        QTimer::singleShot(0, this, [this](){
            setPermissionReplied("once");
            emit permissionReplied(m_permissionRequestId, "once");
        });
    });
    connect(btnAlways, &QPushButton::clicked, this, [this](){
        QTimer::singleShot(0, this, [this](){
            setPermissionReplied("always");
            emit permissionReplied(m_permissionRequestId, "always");
        });
    });
    connect(btnReject, &QPushButton::clicked, this, [this](){
        QTimer::singleShot(0, this, [this](){
            setPermissionReplied("reject");
            emit permissionReplied(m_permissionRequestId, "reject");
        });
    });

    emit contentUpdated();
}

void ChatMessageBubble::setPermissionReplied(const QString &reply)
{
    m_permissionReply = reply;

    // Map reply to display text and color
    QString displayText;
    QString color;
    if(reply == "once"){
        displayText = tr("\u2714 Allowed (once)");
        color = "#2d6a4f";
    }else if(reply == "always"){
        displayText = tr("\u2714 Allowed (always)");
        color = "#1a759f";
    }else{
        displayText = tr("\u2718 Rejected");
        color = "#e74c3c";
    }

    // Remove the button row (deferred delete: this may run from a
    // deferred clicked handler).
    if (m_buttonRow) {
        m_buttonRow->hide();
        m_buttonRow->deleteLater();
        m_buttonRow = nullptr;
    }

    // Append the reply status after the permission description
    QTextCursor cursor(m_browser->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertHtml(QString(
        "<span style='color:%1;font-size:11px;font-weight:bold;'>%2</span>"
    ).arg(color, displayText));

    emit contentUpdated();
}

}
