#include "chat_message_widget.h"
#include "ui_chat_message_widget.h"
#include "core/theme.h"

#include <QMetaObject>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include <QResizeEvent>
#include <QDebug>

namespace ady{

static const char* RoleNames[] = {
    "user",       // User
    "assistant",  // Assistant
    "system",     // System
    "error",      // Error
    "event"       // Event
};

// braille spinner frames for streaming indicator
static const QChar SPINNER_CHARS[] = {
    QChar(0x280B), QChar(0x2819), QChar(0x2839), QChar(0x2838),
    QChar(0x283C), QChar(0x2834), QChar(0x2826), QChar(0x2827),
    QChar(0x2807), QChar(0x280F)
};
static const int SPINNER_COUNT = 10;
int ChatMessageWidget::s_spinnerFrame = 0;

ChatMessageWidget::ChatMessageWidget(Type type, const QString &content, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatMessageWidget)
    , m_type(type)
{
    ui->setupUi(this);
    this->setObjectName("ady--ChatMessageWidget");
    this->applyStyle();
    this->setContent(content);

    // spinner animation timer
    m_spinnerTimer = new QTimer(this);
    m_spinnerTimer->setInterval(100);
    connect(m_spinnerTimer, &QTimer::timeout, this, &ChatMessageWidget::updateSpinner);

    connect(ui->contentLabel, &QLabel::linkActivated, this, &ChatMessageWidget::onLinkActivated);
}

ChatMessageWidget::~ChatMessageWidget()
{
    delete ui;
}

// ---- public api ----

void ChatMessageWidget::setContent(const QString &text)
{
    m_plainContent = text;
    m_streaming = false;
    m_thinkFromEvent = false;
    doUpdate();
}

void ChatMessageWidget::appendText(const QString &delta)
{
    m_plainContent.append(delta);
    scheduleUpdate();
}

void ChatMessageWidget::scheduleUpdate()
{
    if(m_updateScheduled) return;
    m_updateScheduled = true;
    QMetaObject::invokeMethod(this, [this](){
        m_updateScheduled = false;
        doUpdate();
    }, Qt::QueuedConnection);
}

void ChatMessageWidget::doUpdate()
{
    // QTextDocument wraps rich-text content in <p> tags with ~12px default
    // margins.  Prepend a <style> block to reset them (Qt 5 compatible).
    QString html = formatContent(m_plainContent);
    ui->contentLabel->setText(html);
    emit contentUpdated();
}

void ChatMessageWidget::updateSpinner()
{
    s_spinnerFrame = (s_spinnerFrame + 1) % SPINNER_COUNT;
    doUpdate();
}

QString ChatMessageWidget::roleOf(Type type)
{
    if(type >= 0 && type <= Event){
        return QLatin1String(RoleNames[type]);
    }
    return QLatin1String("system");
}

ChatMessageWidget::Type ChatMessageWidget::typeOf(const QString &role)
{
    if(role == "user") return User;
    if(role == "assistant") return Assistant;
    if(role == "error") return Error;
    if(role == "event") return Event;
    return System;
}

// ---- style ----

void ChatMessageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}



QSize ChatMessageWidget::sizeHint() const
{
    int w = width();
    if (w <= 0) {
        w = parentWidget() ? parentWidget()->width() : 400;
    }
    return QSize(w, heightForWidth(w));
}

bool ChatMessageWidget::hasHeightForWidth() const
{
    return true;
}

int ChatMessageWidget::heightForWidth(int w) const
{
    if (w <= 0) return 0;

    auto *ml = ui->mainLayout;
    int margins = ml->contentsMargins().left() + ml->contentsMargins().right();
    int innerW = w - margins;
    if (innerW <= 0) return 0;

    int topBot = ml->contentsMargins().top() + ml->contentsMargins().bottom();

    if (ui->contentLabel->text().isEmpty())
        return topBot;

    // User bubbles are right-aligned and shrink to their natural content
    // width, so measure the wrap height at the width the layout will
    // actually assign to the label (min of natural width and inner width).
    int labelW = innerW;


    int labelH = ui->contentLabel->heightForWidth(labelW);
    // Small buffer: QLabel::heightForWidth() already includes font-metric
    // line height; a large buffer adds visible whitespace for short text.
    return topBot + qMax(labelH, 0) + 2;
}

void ChatMessageWidget::applyStyle()
{
    auto theme = Theme::getInstance();
    const QString bg = theme->secondaryBackgroundColor().lighter(130).name(QColor::HexRgb);
    const QString border = theme->borderColor().name(QColor::HexRgb);
    const QString dim = theme->secondaryTextColor().name(QColor::HexRgb);

    switch(m_type){
    case User:
        // Right-aligned bubble: a layout-item alignment makes the label keep
        // its natural sizeHint width (shrinking to fit the content) instead
        // of being stretched to the full row width.
        //ui->contentLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget QLabel{background-color:%1;border:1px solid %2;border-radius:6px;padding:8px;}"
        ).arg("#1f261f", border));
        break;
    case Assistant:
        break;
    case System:
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid %2;border-radius:6px;}"
        ).arg(bg, border));
        break;
    case Error:
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid #e74c3c;border-radius:6px;}"
        ).arg(bg));
        break;
    case Event:
        ui->contentLabel->setAlignment(Qt::AlignCenter);
        ui->contentLabel->setStyleSheet(QString(
            "color:%1;font-size:11px;font-style:italic;"
        ).arg(dim));
        break;
    }
}

// ---- content formatting ----

QString ChatMessageWidget::formatContent(const QString &text)
{
    // Clear code block lists (rebuilt each render); tool blocks persist
    m_codeBlocks.clear();
    m_codeHtml.clear();

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

    // Streaming indicator: braille spinner shown left-aligned before the content
    QString spinnerHtml;
    if (m_streaming) {
        auto theme = Theme::getInstance();
        const QString dim = theme->secondaryTextColor().name(QColor::HexRgb);
        spinnerHtml = QString("<span style='color:%1;font-size:13px;'>%2</span>&nbsp;")
                          .arg(dim, QString(SPINNER_CHARS[s_spinnerFrame]));
    }

    if(displayText.isEmpty() && m_toolBlocks.isEmpty() && !m_thinkFromEvent) {
        // Waiting for the first chunk: show only the spinner (left-aligned)
        return spinnerHtml;
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

    const QStringList parts = cleaned.split("```");
    QString html;
    int codeIdx = 0;
    for(int i = 0; i < parts.size(); ++i){
        if(i % 2 == 0){
            QString escaped = parts[i].toHtmlEscaped();
            escaped.replace("\n", "<br/>");
            html += escaped;
        }else{
            html += formatCodeBlock(parts[i], codeIdx);
            ++codeIdx;
        }
    }

    // Replace tool block markers with actual HTML
    for (int i = 0; i < m_toolBlocks.size(); ++i) {
        QString marker = QString("<!--TOOL_%1-->").arg(i);
        html.replace(marker, m_toolBlocks[i]);
    }

    // Replace code block markers with actual HTML
    for (int i = 0; i < m_codeHtml.size(); ++i) {
        QString marker = QString("<!--CODE_%1-->").arg(i);
        html.replace(marker, m_codeHtml[i]);
    }

    return spinnerHtml + thinkHtml + html;
}

QString ChatMessageWidget::extractAndFormatThinkContent(const QString &text, QString &cleaned) const
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

    const_cast<ChatMessageWidget*>(this)->m_thinkContent = thinkContent.trimmed();
    return formatThinkBlock(thinkContent.trimmed());
}

QString ChatMessageWidget::formatThinkBlock(const QString &thinkContent) const
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

void ChatMessageWidget::onToggleThink()
{
    m_thinkExpanded = !m_thinkExpanded;
    doUpdate();
}

void ChatMessageWidget::onLinkActivated(const QString &link)
{
    if (link == "expand" || link == "collapse") {
        onToggleThink();
    } else if (link.startsWith("copy:")) {
        int idx = link.mid(5).toInt();
        copyCode(idx);
    }
}

void ChatMessageWidget::copyCode(int index) const
{
    if (index >= 0 && index < m_codeBlocks.size()) {
        QApplication::clipboard()->setText(m_codeBlocks.at(index));
    }
}

QString ChatMessageWidget::formatCodeBlock(const QString &code, int &outIndex)
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
    outIndex = m_codeBlocks.size();
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

    QString marker = QString("<!--CODE_%1-->").arg(outIndex);

    QString html = QString(
        "<div style='background-color:%1;border-radius:4px;margin:4px 0;'>"
        "<div style='text-align:right;padding:4px 8px 0 8px;'>"
        "<span style='color:%3;font-size:10px;'>%4</span> "
        "<a href='copy:%5' style='color:%6;font-size:11px;text-decoration:none;'>[copy] %7</a>"
        "</div>"
        "<pre style='background-color:%1;color:%2;padding:8px;margin:0;"
        "font-family:Consolas,monospace;font-size:12px;'>%8</pre>"
        "</div>"
    ).arg(codeBg, codeFg, dimColor, lang.toHtmlEscaped(),
          QString::number(outIndex), linkColor, tr("Copy"),
          body.toHtmlEscaped());

    m_codeHtml.append(html);
    return marker + "\n";
}

// ---- streaming indicator ----

void ChatMessageWidget::setStreaming(bool streaming)
{
    m_streaming = streaming;
    if (streaming) {
        m_spinnerTimer->start();
        updateSpinner();
    } else {
        m_spinnerTimer->stop();
        doUpdate();  // re-render to remove the spinner
    }
}

// ---- thinking content (from SSE event) ----

void ChatMessageWidget::appendThink(const QString &content)
{
    m_thinkFromEvent = true;
    m_thinkContent = content.trimmed();
    m_thinkExpanded = false;
    doUpdate();
}

// ---- tool use / tool result block ----

void ChatMessageWidget::appendToolBlock(const QString &toolName, const QString &body, bool isResult)
{
    auto theme = Theme::getInstance();
    QString blockBg, blockFg, accentColor, label;
    if (theme->style() == Theme::Dark) {
        blockBg = "#1e2a1e";
        blockFg = "#c8d6c8";
    } else {
        blockBg = "#f0f7f0";
        blockFg = "#2d3b2d";
    }

    if (isResult) {
        accentColor = "#42b983";
        label = tr("Result");
    } else {
        accentColor = "#e6a23c";
        label = tr("Tool");
    }

    QString escapedBody = body.toHtmlEscaped();
    escapedBody.replace("\n", "<br/>");

    // truncate long tool results for readability
    if (isResult && body.length() > 500) {
        escapedBody = body.left(500).toHtmlEscaped().replace("\n", "<br/>") + "<br/><i>... (truncated)</i>";
    }

    QString html;
    if (body.isEmpty()) {
        // Compact form for pending/running tool states (no body content)
        html = QString(
            "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
            "padding:4px 8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
            "<b style='color:%3;'>[tool] %4: %5</b>"
            "</div>"
        ).arg(blockBg, blockFg, accentColor, label, toolName.toHtmlEscaped());
    } else {
        html = QString(
            "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
            "padding:6px 8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
            "<b style='color:%3;'>[tool] %4: %5</b><br/>"
            "<code style='font-family:Consolas,monospace;font-size:11px;'>%6</code>"
            "</div>"
        ).arg(blockBg, blockFg, accentColor, label, toolName.toHtmlEscaped(), escapedBody);
    }

    // Insert a marker in plain text; formatContent replaces it with actual HTML
    int idx = m_toolBlocks.size();
    m_toolBlocks.append(html);
    m_plainContent += QString("<!--TOOL_%1-->\n").arg(idx);
    scheduleUpdate();
}

}
