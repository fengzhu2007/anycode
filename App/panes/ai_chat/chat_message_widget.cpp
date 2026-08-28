#include "chat_message_widget.h"
#include "ui_chat_message_widget.h"
#include "core/theme.h"

#include <QDateTime>
#include <QIcon>

namespace ady{

// role 名称与消息类型映射
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
}

ChatMessageWidget::~ChatMessageWidget()
{
    delete ui;
}

// ---- public api ----

void ChatMessageWidget::setContent(const QString &text)
{
    m_plainContent = text;
    ui->contentLabel->setText(formatContent(text));
}

void ChatMessageWidget::appendText(const QString &delta)
{
    m_plainContent.append(delta);
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
        // 事件消息: 无边框,居中灰色小字
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

    // 按 ``` 分割: 偶数段为普通文本, 奇数段为代码块
    const QStringList parts = text.split("```");
    QString html;
    for(int i = 0; i < parts.size(); ++i){
        if(i % 2 == 0){
            // 普通文本: 转义 + 换行转 <br/>
            QString escaped = parts[i].toHtmlEscaped();
            escaped.replace("\n", "<br/>");
            html += escaped;
        }else{
            html += formatCodeBlock(parts[i]);
        }
    }
    return html;
}

QString ChatMessageWidget::formatCodeBlock(const QString &code) const
{
    QString body = code;

    // 代码块首行可能是语言标识 (如 ```cpp)
    int nl = body.indexOf('\n');
    if(nl > 0){
        const QString lang = body.left(nl).trimmed();
        if(!lang.isEmpty() && !lang.contains(' ')){
            body = body.mid(nl + 1);
        }
    }
    // 去掉结尾多余空行
    while(body.endsWith('\n')){
        body.chop(1);
    }

    // 根据主题选择代码块配色
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
