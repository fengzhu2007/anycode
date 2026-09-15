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
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>

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

    // Permission type: build interactive card with buttons
    if(m_type == Permission){
        setupPermissionUI();
    }
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
    if(type >= 0 && type <= Permission){
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
    if(role == "permission") return Permission;
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
    int total = topBot + qMax(labelH, 0) + 2;

    // Permission type: account for button row height
    if(m_type == Permission && m_permissionReply.isEmpty() && ui->mainLayout->count() > 1){
        total += 32;  // button row: ~24px button + 4px margin + 4px spacing
    }

    return total;
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
            "#ady--ChatMessageWidget QLabel{background-color:%1;border-radius:6px;padding:8px;}"
        ).arg("#1f261f"));
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
    case Permission:
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid %2;border-radius:6px;}"
        ).arg(bg, border));
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

    // Replace tool markers with null-byte placeholders BEFORE HTML escaping,
    // because toHtmlEscaped() would turn <!--TOOL_0--> into &lt;!--TOOL_0--&gt;
    // making the later replacement fail.
    for (int i = 0; i < m_toolBlocks.size(); ++i) {
        QString marker = QString("<!--TOOL_%1-->").arg(i);
        QString placeholder = QString("\x00TOOL_%1\x00").arg(i);
        cleaned.replace(marker, placeholder);
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

    // Replace null-byte placeholders with actual tool block HTML
    for (int i = 0; i < m_toolBlocks.size(); ++i) {
        QString placeholder = QString("\x00TOOL_%1\x00").arg(i);
        html.replace(placeholder, m_toolBlocks[i]);
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

QString ChatMessageWidget::detectShellType(const QString &toolName, const QString &command)
{
    QString tn = toolName.toLower();
    if(tn == "cmd" || tn.contains("cmd.exe") || tn.contains("cmd "))
        return "cmd";
    if(tn == "powershell" || tn.contains("powershell") || tn == "ps1")
        return "powershell";

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

void ChatMessageWidget::appendToolBlock(const QString &callID, const QString &toolType, const QString &toolName, const QString &body)
{
    // Check if a tool block with this callID already exists (from ToolCallStart/ToolCallDelta).
    // If so, update it in place instead of creating a duplicate.
    for(int i = 0; i < m_toolCalls.size(); ++i) {
        if(m_toolCalls[i].callID == callID) {
            m_toolCalls[i].toolType = toolType;
            m_toolCalls[i].toolName = toolName;
            m_toolCalls[i].body = body;
            QString lowerType = toolType.toLower();
            if(lowerType == "bash" || lowerType == "cmd" || lowerType == "powershell"
               || lowerType == "shell" || lowerType.contains("terminal")) {
                m_toolCalls[i].isCommand = true;
                m_toolCalls[i].shellType = detectShellType(toolType, body);
            }
            if(m_toolCalls[i].blockIndex >= 0 && m_toolCalls[i].blockIndex < m_toolBlocks.size()) {
                m_toolBlocks[m_toolCalls[i].blockIndex] = buildToolBlockHtml(m_toolCalls[i]);
            }
            scheduleUpdate();
            return;
        }
    }

    // New tool block
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

    QString html = buildToolBlockHtml(info);
    m_toolBlocks.append(html);
    m_plainContent += QString("<!--TOOL_%1-->\n").arg(info.blockIndex);
    scheduleUpdate();
}

void ChatMessageWidget::updateToolStatus(const QString &callID, ToolStatus status, const QString &output)
{
    for(int i = 0; i < m_toolCalls.size(); ++i) {
        if(m_toolCalls[i].callID == callID) {
            m_toolCalls[i].status = status;
            if(!output.isEmpty()) {
                m_toolCalls[i].body = output;
            }
            if(m_toolCalls[i].blockIndex >= 0 && m_toolCalls[i].blockIndex < m_toolBlocks.size()) {
                m_toolBlocks[m_toolCalls[i].blockIndex] = buildToolBlockHtml(m_toolCalls[i]);
            }
            scheduleUpdate();
            return;
        }
    }
    qDebug() << "[ChatMessageWidget] updateToolStatus: callID not found:" << callID;
}

QString ChatMessageWidget::buildToolBlockHtml(const ToolCallInfo &info) const
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
        accentColor = "#e6a23c";
        statusIcon = QChar(0x23F3);
        statusText = tr("Processing");
        break;
    case ToolSuccess:
        accentColor = "#42b983";
        statusIcon = QChar(0x2714);
        statusText = tr("Success");
        break;
    case ToolFailure:
        accentColor = "#e74c3c";
        statusIcon = QChar(0x2718);
        statusText = tr("Failure");
        break;
    }

    QString html;
    QString displayName = info.toolName.toHtmlEscaped();

    if(info.isCommand) {
        QString shellBadge = info.shellType.toUpper();
        QString command = info.body.toHtmlEscaped();
        command.replace("\n", "<br/>");

        if(info.status == ToolProcessing) {
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
        QString bodyText = info.body.toHtmlEscaped();
        bodyText.replace("\n", "<br/>");

        if(info.body.length() > 500) {
            bodyText = info.body.left(500).toHtmlEscaped().replace("\n", "<br/>")
                       + "<br/><i>... (truncated)</i>";
        }

        if(info.status == ToolProcessing) {
            html = QString(
                "<div style='background-color:%1;color:%2;border-left:3px solid %3;"
                "padding:8px;margin:4px 0;border-radius:4px;font-size:12px;'>"
                "<b style='color:%3;'>%4 %5</b> <span style='color:%3;font-size:11px;'>%6</span>"
                "</div>"
            ).arg(blockBg, blockFg, accentColor, statusIcon, displayName, statusText);
        } else {
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

void ChatMessageWidget::setupPermissionUI()
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

    // Update content label with permission description
    ui->contentLabel->setText(QString(
        "<div style='color:%1;font-size:12px;'>"
        "<b style='color:%2;'>\u26a0 Permission Request</b><br/>"
        "Tool [<b>%3</b>] is requesting permission<br/>"
        "<span style='color:%4;'>%5</span>"
        "</div>"
    ).arg(fg, accent, toolName.toHtmlEscaped(), fg, detail.toHtmlEscaped()));

    // If already replied (widget recreated after scroll), show disabled state
    if(!m_permissionReply.isEmpty()){
        setPermissionReplied(m_permissionReply);
        return;
    }

    // Create button row below the content label
    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(6);
    btnLayout->setContentsMargins(0, 4, 0, 0);

    auto makeBtn = [&](const QString &text, const QString &bg, const QString &fgColor) -> QPushButton* {
        auto *btn = new QPushButton(text, this);
        btn->setCursor(Qt::PointingHandCursor);
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

    // Insert button layout into the main layout (after contentLabel)
    ui->mainLayout->addLayout(btnLayout);

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

void ChatMessageWidget::setPermissionReplied(const QString &reply)
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

    // Remove only the button layout (last item), keep contentLabel.
    // Widgets use deleteLater() because this runs from a deferred clicked handler.
    while(ui->mainLayout->count() > 1){
        QLayoutItem *child = ui->mainLayout->takeAt(ui->mainLayout->count() - 1);
        if(child->layout()){
            // Deleting the QLayout also destroys the QLayoutItem wrapper,
            // so do NOT call 'delete child' afterwards (double-delete).
            QLayout *btnLayout = child->layout();
            while(QLayoutItem *item = btnLayout->takeAt(0)){
                if(item->widget()) item->widget()->deleteLater();
                delete item;
            }
            delete btnLayout;   // also invalidates 'child'
        }else{
            // Non-layout item (e.g. spacer) — safe to delete directly
            if(child->widget() && child->widget() != ui->contentLabel){
                child->widget()->deleteLater();
            }
            delete child;
        }
    }

    // Append reply status to content label
    QString currentHtml = ui->contentLabel->text();
    ui->contentLabel->setText(currentHtml + QString(
        "<div style='color:%1;font-size:11px;margin-top:4px;font-weight:bold;'>%2</div>"
    ).arg(color, displayText));

    emit contentUpdated();
}

}
