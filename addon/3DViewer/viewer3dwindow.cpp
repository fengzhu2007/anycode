#include "viewer3dwindow.h"
#include "viewer3dwidget.h"
#include "ui_viewer3dwindow.h"

#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QListWidget>
#include <QPainter>
#include <QFont>
#include <QDebug>

Viewer3DWindow::Viewer3DWindow(QWidget* parent)
    : wWindow(parent)
{
    ui = new Ui::Viewer3DWindow;
    auto* central = new QWidget(this);
    ui->setupUi(central);

    // Splitter stretch
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes(QList<int>{260, 760});

    setCentralWidget(central);
    this->resetupUi();
    resize(1024, 700);

    // Connections
    connect(ui->openDirBtn, &QPushButton::clicked, this, &Viewer3DWindow::onOpenDirectory);
    connect(ui->thumbnailList, &QListWidget::itemDoubleClicked,
            this, &Viewer3DWindow::onThumbnailClicked);
}

Viewer3DWindow::~Viewer3DWindow()
{
    delete ui;
}

QStringList Viewer3DWindow::supportedExtensions()
{
    return QStringList{"stl", "obj", "ply", "3ds", "off"};
}

void Viewer3DWindow::onOpenDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, QString::fromUtf8("Select 3D Files Directory"), m_currentDir);
    if(dir.isEmpty())
        return;
    m_currentDir = dir;
    scanDirectory(dir);
}

void Viewer3DWindow::scanDirectory(const QString& dirPath)
{
    ui->thumbnailList->clear();
    ui->viewer->clearMesh();

    QDir dir(dirPath);
    QStringList exts = supportedExtensions();
    QStringList filters;
    for(const QString& ext : exts){
        filters << QString("*.%1").arg(ext);
    }

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for(const QFileInfo& fi : files){
        QString filePath = fi.absoluteFilePath();
        QString displayName = fi.fileName();

        QListWidgetItem* item = new QListWidgetItem(displayName);
        item->setData(Qt::UserRole, filePath);
        item->setIcon(QIcon(generateThumbnail(filePath)));
        item->setToolTip(filePath);
        ui->thumbnailList->addItem(item);
    }

    ui->statusLabel->setText(
        QString::fromUtf8("Found %1 3D file(s) in %2").arg(files.size()).arg(dirPath));
}

void Viewer3DWindow::onThumbnailClicked(QListWidgetItem* item)
{
    if(!item)
        return;
    QString filePath = item->data(Qt::UserRole).toString();
    ui->viewer->loadFile(filePath);
    ui->statusLabel->setText(
        QString::fromUtf8("Loaded: %1").arg(filePath));
}

QPixmap Viewer3DWindow::generateThumbnail(const QString& filePath)
{
    QPixmap pixmap(96, 96);
    pixmap.fill(QColor(45, 45, 48));

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);

    // Top face
    painter.setBrush(QColor(80, 160, 240));
    QPolygonF top;
    top << QPointF(48, 18) << QPointF(72, 30) << QPointF(48, 42) << QPointF(24, 30);
    painter.drawPolygon(top);

    // Left face
    painter.setBrush(QColor(60, 130, 210));
    QPolygonF left;
    left << QPointF(24, 30) << QPointF(48, 42) << QPointF(48, 66) << QPointF(24, 54);
    painter.drawPolygon(left);

    // Right face
    painter.setBrush(QColor(100, 180, 255));
    QPolygonF right;
    right << QPointF(48, 42) << QPointF(72, 30) << QPointF(72, 54) << QPointF(48, 66);
    painter.drawPolygon(right);

    // File extension label
    QFileInfo fi(filePath);
    QString ext = fi.suffix().toUpper();
    painter.setPen(QColor(200, 200, 200));
    QFont font = painter.font();
    font.setPixelSize(10);
    painter.setFont(font);
    painter.drawText(pixmap.rect().adjusted(0, 72, 0, -2), Qt::AlignHCenter, ext);

    painter.end();
    return pixmap;
}
