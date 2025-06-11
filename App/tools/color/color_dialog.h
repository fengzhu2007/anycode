#ifndef COLOR_DIALOG_H
#define COLOR_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class ColorDialog;
}
namespace ady{
class ColorDialogPrivate;
class ColorDialog : public wDialog
{
    Q_OBJECT

public:



    ~ColorDialog();

    static ColorDialog* getInstance();
    static ColorDialog* open(QWidget* parent);

public slots:
    void onTextChanged(const QString& text);
    void onColorPicked(const QColor& color);
    void startPicker();
private:
    explicit ColorDialog(QWidget *parent);
    QString convertRgbToHex(const QString& rgbStr);
    QString convertHexToRgb(const QString& hexStr, bool includeAlpha = false );


private:
    Ui::ColorDialog *ui;
    ColorDialogPrivate* d;
    static ColorDialog* instance;
};
}

#endif // COLOR_DIALOG_H
