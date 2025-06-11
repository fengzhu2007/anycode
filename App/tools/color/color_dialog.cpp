#include "color_dialog.h"
#include "color_picker.h"
#include "ui_color_dialog.h"
#include <QRegularExpression>
#include <QApplication>
#include <QDebug>

namespace ady{

ColorDialog* ColorDialog::instance = nullptr;


class ColorDialogPrivate{
public:
    ColorPicker* picker = nullptr;
};

ColorDialog::ColorDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::ColorDialog)
{
    ui->setupUi(this);
    d = new ColorDialogPrivate;
    this->resetupUi();
    connect(ui->oct,&QLineEdit::textChanged,this,&ColorDialog::onTextChanged);
    connect(ui->hex,&QLineEdit::textChanged,this,&ColorDialog::onTextChanged);
    connect(ui->picker,&QPushButton::clicked,this,&ColorDialog::startPicker);

}

ColorDialog::~ColorDialog()
{
    delete d;
    delete ui;
}


ColorDialog* ColorDialog::getInstance(){
    return instance;
}

ColorDialog* ColorDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new ColorDialog(parent);
    }
    instance->show();
    return instance;
}

void ColorDialog::onTextChanged(const QString& text){
    auto sender = this->sender();
    if(sender==ui->oct && ui->oct->hasFocus()){
        ui->hex->setText(this->convertRgbToHex(text));
    }else if(sender==ui->hex && ui->hex->hasFocus()){
        ui->oct->setText(this->convertHexToRgb(text));
    }
}


void ColorDialog::onColorPicked(const QColor& color){
    ui->oct->setText(QString::fromUtf8("gba(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue()));
    ui->hex->setText(color.name(QColor::HexRgb));
    this->parentWidget()->showMaximized();
}

void ColorDialog::startPicker(){
    //topLevelWidget()->showMinimized();
    this->parentWidget()->showMinimized();
    //qDebug()<<"widget"<<this->parentWidget();

    auto picker = new ColorPicker();
    connect(picker,&ColorPicker::colorPicked,this,&ColorDialog::onColorPicked);
}



QString ColorDialog::convertRgbToHex(const QString& rgbStr) {
    QRegularExpression regex(
        "(?:rgb|rgba)\\s*\\(\\s*(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*(?:,\\s*(\\d+)\\s*)?\\)"
        "|"
        "(\\d+)\\s*,\\s*(\\d+)\\s*,\\s*(\\d+)\\s*(?:,\\s*(\\d+)\\s*)?"
        );

    QRegularExpressionMatch match = regex.match(rgbStr);

    if (!match.hasMatch()) {
        return ""; //
    }

    QStringList numbers;
    for (int i = 1; i <= 8; ++i) {
        if (!match.captured(i).isEmpty()) {
            numbers << match.captured(i);
        }
    }

    if (numbers.size() < 3) {
        return "";
    }

    bool ok;
    int r = numbers[0].toInt(&ok);
    if (!ok || r < 0 || r > 255) return "";

    int g = numbers[1].toInt(&ok);
    if (!ok || g < 0 || g > 255) return "";

    int b = numbers[2].toInt(&ok);
    if (!ok || b < 0 || b > 255) return "";

    int a = 255;
    if (numbers.size() >= 4) {
        a = numbers[3].toInt(&ok);
        if (!ok || a < 0 || a > 255) a = 255;
    }

    QString hex;
    if (a == 255) {
        hex = QString("#%1%2%3")
        .arg(r, 2, 16, QLatin1Char('0'))
            .arg(g, 2, 16, QLatin1Char('0'))
            .arg(b, 2, 16, QLatin1Char('0'))
            .toUpper();
    } else {
        hex = QString("#%1%2%3%4")
        .arg(r, 2, 16, QLatin1Char('0'))
            .arg(g, 2, 16, QLatin1Char('0'))
            .arg(b, 2, 16, QLatin1Char('0'))
            .arg(a, 2, 16, QLatin1Char('0'))
            .toUpper();
    }

    return hex;
}

QString ColorDialog::convertHexToRgb(const QString& hexStr, bool includeAlpha) {
    QString hex = hexStr.trimmed().toUpper().remove('#');
    if (hex.length() != 3 && hex.length() != 4 && hex.length() != 6 && hex.length() != 8) {
        return "";
    }
    if (hex.length() == 3 || hex.length() == 4) {
        QString expanded;
        for (const QChar& c : hex) {
            expanded += c;
            expanded += c;
        }
        hex = expanded;
    }
    bool ok;
    int r = hex.mid(0, 2).toInt(&ok, 16);
    if (!ok || r < 0 || r > 255) return "";

    int g = hex.mid(2, 2).toInt(&ok, 16);
    if (!ok || g < 0 || g > 255) return "";

    int b = hex.mid(4, 2).toInt(&ok, 16);
    if (!ok || b < 0 || b > 255) return "";

    int a = 255;
    if (hex.length() >= 8) {
        a = hex.mid(6, 2).toInt(&ok, 16);
        if (!ok || a < 0 || a > 255) a = 255;
    }
    if (includeAlpha || (hex.length() >= 8 && a != 255)) {
        return QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(a / 255.0, 0, 'f', 2);
    } else {
        return QString("rgb(%1, %2, %3)").arg(r).arg(g).arg(b);
    }
}

}
