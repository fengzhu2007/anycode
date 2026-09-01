#include "chat_message_widget.h"
#include "ui_chat_message_widget.h"
#include "core/theme.h"

#include <QDateTime>
#include <QIcon>
#include <QMetaObject>

namespace ady{

static const char* RoleNames[] = {
    "user",       // User
    "assistant",  // Assistant
    "system",     // System
    "error",      // Error
    "event"       // Event
};

ChatMessageWidget::ChatMessageWidget(Type type, const QString &content, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatMessageWidget)
    , m_type(type)
{
    ui->setupUi(this);
    this->setObjectName("ady--ChatMessageWidget");
    ui->timeLabel->setText(QDateTime::currentDateTime().toString("HH:mm"));
    this->applyStyle();
    this->setContent(content);

    connect(ui->contentLabel, &QLabel::linkActivated, this, &ChatMessageWidget::onToggleThink);
}

ChatMessageWidget::~ChatMessageWidget()
{
    delete ui;
}

// ---- public api ----

void ChatMessageWidget::setContent(const QString &text)
{
    m_plainContent = text;
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
    ui->contentLabel->setText(formatContent(m_plainContent));
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

void ChatMessageWidget::applyStyle()
{
    auto theme = Theme::getInstance();
    const QString bg = theme->secondaryBackgroundColor().name(QColor::HexRgb);
    const QString border = theme->borderColor().name(QColor::HexRgb);
    const QString dim = theme->secondaryTextColor().name(QColor::HexRgb);

    ui->timeLabel->setStyleSheet(QString("color:%1;font-size:11px;").arg(dim));

    switch(m_type){
    case User:
        ui->iconLabel->setPixmap(QIcon(":/Resource/icons/Computer_16x.svg").pixmap(16, 16));
        ui->roleLabel->setText(tr("You"));
        ui->roleLabel->setStyleSheet("color:#3a8ee6;font-weight:bold;");
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid %2;border-radius:6px;}"
        ).arg(bg, border));
        break;
    case Assistant:
        ui->iconLabel->setPixmap(QIcon(":/Resource/icons/EnableDiagnostics_16x.svg").pixmap(16, 16));
        ui->roleLabel->setText(tr("Assistant"));
        ui->roleLabel->setStyleSheet("color:#42b983;font-weight:bold;");
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid %2;border-radius:6px;}"
        ).arg(bg, border));
        break;
    case System:
        ui->iconLabel->setPixmap(QIcon(":/Resource/icons/StatusHelp_16x.svg").pixmap(16, 16));
        ui->roleLabel->setText(tr("System"));
        ui->roleLabel->setStyleSheet("color:#e6a23c;font-weight:bold;");
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid %2;border-radius:6px;}"
        ).arg(bg, border));
        break;
    case Error:
        ui->iconLabel->setPixmap(QIcon(":/Resource/icons/StatusCriticalError_16x.svg").pixmap(16, 16));
        ui->roleLabel->setText(tr("Error"));
        ui->roleLabel->setStyleSheet("color:#e74c3c;font-weight:bold;");
        this->setStyleSheet(QString(
            "#ady--ChatMessageWidget{background-color:%1;border:1px solid #e74c3c;border-radius:6px;}"
        ).arg(bg));
        break;
    case Event:
        ui->iconLabel->hide();
        ui->roleLabel->hide();
        ui->timeLabel->hide();
        ui->contentLabel->setAlignment(Qt::AlignCenter);
        ui->contentLabel->setStyleSheet(QString(
            "color:%1;font-size:11px;font-style:italic;"
        ).arg(dim));
        break;
    }
}

// ---- content formatting ----

QString ChatMessageWidget::formatContent(const QString &text) const
{
    if(text.isEmpty()) return {};

    QString cleaned;
    QString thinkHtml = extractAndFormatThinkContent(text, cleaned);

    const QStringList parts = cleaned.split("```");
    QString html;
    for(int i = 0; i < parts.size(); ++i){
        if(i % 2 == 0){
            QString escaped = parts[i].toHtmlEscaped();
            escaped.replace("\n", "<br/>");
            html += escaped;
        }else{
            html += formatCodeBlock(parts[i]);
        }
    }

    return thinkHtml + html;
}

QString ChatMessageWidget::extractAndFormatThinkContent(const QString &text, QString &cleaned) const
{
    QString thinkContent;
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
        result += text.mid(pos, thinkStart - pos);
        int thinkEnd = text.indexOf(closeTag, thinkStart + openTag.length());
        if(thinkEnd < 0){
            thinkContent += text.mid(thinkStart + openTag.length());
            break;
        }
        thinkContent += text.mid(thinkStart + openTag.length(), thinkEnd - thinkStart - openTag.length());
        pos = thinkEnd + closeTag.length();
    }

    cleaned = result.trimmed();

    if(thinkContent.isEmpty()){
        return QString();
    }

    const_cast<ChatMessageWidget*>(this)->m_thinkContent = thinkContent.trimmed();

    auto theme = Theme::getInstance();
    QString thinkColor = (theme->style() == Theme::Dark) ? "#888888" : "#666666";
    QString thinkBg = (theme->style() == Theme::Dark) ? "#1a1a1a" : "#f0f0f0";
    QString linkColor = "#4a9eff";

    QString escapedThink = thinkContent.trimmed().toHtmlEscaped();
    escapedThink.replace("\n", "<br/>");

    if(m_thinkExpanded){
        return QString(
            "<div style='background-color:%1;border-left:3px solid %2;padding:8px;margin:4px 0;"
            "border-radius:4px;color:%2;font-size:12px;'>"
            "<b>💭 思考过程：</b><br/>%3<br/>"
            "<a href='collapse' style='color:%4;'>收起 ▲</a>"
            "</div><br/>"
        ).arg(thinkBg, thinkColor, escapedThink, linkColor);
    }else{
        QString firstLine = thinkContent.trimmed();
        int nlPos = firstLine.indexOf('\n');
        if(nlPos > 0){
            firstLine = firstLine.left(nlPos);
        }
        if(firstLine.length() > 80){
            firstLine = firstLine.left(80) + "...";
        }
        QString escapedFirstLine = firstLine.toHtmlEscaped();

        return QString(
            "<div style='background-color:%1;border-left:3px solid %2;padding:8px;margin:4px 0;"
            "border-radius:4px;color:%2;font-size:12px;'>"
            "<b>💭 思考过程：</b> %3 "
            "<a href='expand' style='color:%4;'>展开 ▼</a>"
            "</div><br/>"
        ).arg(thinkBg, thinkColor, escapedFirstLine, linkColor);
    }
}

void ChatMessageWidget::onToggleThink()
{
    m_thinkExpanded = !m_thinkExpanded;
    doUpdate();
}

QString ChatMessageWidget::formatCodeBlock(const QString &code) const
{
    QString body = code;

    int nl = body.indexOf('\n');
    if(nl > 0){
        const QString lang = body.left(nl).trimmed();
        if(!lang.isEmpty() && !lang.contains(' ')){
            body = body.mid(nl + 1);
        }
    }
    while(body.endsWith('\n')){
        body.chop(1);
    }

    auto theme = Theme::getInstance();
    QString codeBg, codeFg;
    if(theme->style() == Theme::Dark){
        codeBg = "#2b2b2b";
        codeFg = "#dcdcdc";
    }else{
        codeBg = "#f6f8fa";
        codeFg = "#24292e";
    }

    return QString(
        "<pre style='background-color:%1;color:%2;padding:8px;margin:4px 0;"
        "border-radius:4px;font-family:Consolas,monospace;'>%3</pre>"
    ).arg(codeBg, codeFg, body.toHtmlEscaped());
}

}
