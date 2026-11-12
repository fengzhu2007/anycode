#include "viewer3dwindow.h"
#include "viewer3dconfig.h"
#include "viewer3dthumbcache.h"
#include "viewer3dthumbnailrenderer.h"
#include "viewer3dwidget.h"
#include "w_qss.h"
#include "w_window_nclient.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

Viewer3DWindow::Viewer3DWindow(QWidget* parent)
    : wWindow(parent)
{
    m_cache = new Viewer3DThumbCache();
    m_renderer = new Viewer3DThumbnailRenderer(m_cache, this);

    setupUi();
    resize(1200, 760);

    connect(m_renderer, &Viewer3DThumbnailRenderer::thumbnailGenerated,
            this, &Viewer3DWindow::onThumbnailGenerated);
    connect(m_renderer, &Viewer3DThumbnailRenderer::thumbnailFailed,
            this, &Viewer3DWindow::onThumbnailFailed);
    connect(m_renderer, &Viewer3DThumbnailRenderer::queueProgressChanged,
            this, [this](int) { updateStatus(); });

    const QString defaultDir = QDir::currentPath();
    QTimer::singleShot(0, this, [this, defaultDir]() {
        scanDirectory(defaultDir);
    });
}

Viewer3DWindow::~Viewer3DWindow()
{
    delete m_renderer;
    m_renderer = nullptr;
    delete m_cache;
    m_cache = nullptr;
}

void Viewer3DWindow::setupUi()
{
    //wRect 标题栏高度为 32，内容区域需要让出该高度
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(6, 38, 6, 6);
    outerLayout->setSpacing(0);

    m_content = new QWidget(this);
    m_content->setObjectName(QStringLiteral("viewer3dContent"));
    outerLayout->addWidget(m_content);
    setLayout(outerLayout);

    if (nClient() != nullptr) {
        nClient()->setWindowTitle(tr("3D Model Viewer"));
    }

    //顶部工具栏
    auto* toolLayout = new QHBoxLayout();
    toolLayout->setContentsMargins(4, 2, 4, 2);
    toolLayout->setSpacing(6);

    auto* openDirButton = new QPushButton(tr("Open Directory"), m_content);
    auto* openFileButton = new QPushButton(tr("Open File"), m_content);
    auto* refreshButton = new QPushButton(tr("Refresh"), m_content);
    auto* regenerateButton = new QPushButton(tr("Regenerate Thumbnails"), m_content);

    m_searchEdit = new QLineEdit(m_content);
    m_searchEdit->setPlaceholderText(tr("Filter by model name..."));
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setMaximumWidth(220);

    m_formatCombo = new QComboBox(m_content);
    m_formatCombo->addItem(tr("All formats"), QString());
    for (const QString& ext : ady::Viewer3D::supportedExtensions()) {
        m_formatCombo->addItem(QStringLiteral(".%1").arg(ext), ext);
    }

    m_wireframeButton = new QPushButton(tr("Wireframe"), m_content);
    m_wireframeButton->setCheckable(true);
    m_gridButton = new QPushButton(tr("Grid"), m_content);
    m_gridButton->setCheckable(true);
    m_gridButton->setChecked(true);

    auto* fitButton = new QPushButton(tr("Fit View"), m_content);

    toolLayout->addWidget(openDirButton);
    toolLayout->addWidget(openFileButton);
    toolLayout->addWidget(refreshButton);
    toolLayout->addSpacing(8);
    toolLayout->addWidget(m_searchEdit);
    toolLayout->addWidget(m_formatCombo);
    toolLayout->addWidget(regenerateButton);
    toolLayout->addStretch(1);
    toolLayout->addWidget(fitButton);
    toolLayout->addWidget(m_wireframeButton);
    toolLayout->addWidget(m_gridButton);

    //左侧缩略图网格 + 右侧 3D 视图
    auto* splitter = new QSplitter(Qt::Horizontal, m_content);
    splitter->setHandleWidth(4);

    m_modelList = new QListWidget(splitter);
    m_modelList->setViewMode(QListView::IconMode);
    m_modelList->setResizeMode(QListView::Adjust);
    m_modelList->setMovement(QListView::Static);
    m_modelList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_modelList->setUniformItemSizes(true);
    m_modelList->setWordWrap(true);
    m_modelList->setIconSize(QSize(ady::Viewer3D::thumbnailSize(), ady::Viewer3D::thumbnailSize()));
    m_modelList->setGridSize(QSize(156, 176));
    m_modelList->setSpacing(8);
    m_modelList->setContextMenuPolicy(Qt::CustomContextMenu);

    m_viewer = new Viewer3DWidget(splitter);
    m_viewer->setMinimumSize(360, 240);

    splitter->addWidget(m_modelList);
    splitter->addWidget(m_viewer);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setSizes(QList<int>{360, 840});

    //底部状态栏
    m_statusLabel = new QLabel(m_content);
    m_statusLabel->setObjectName(QStringLiteral("viewer3dStatus"));

    auto* contentLayout = new QVBoxLayout(m_content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(4);
    contentLayout->addLayout(toolLayout);
    contentLayout->addWidget(splitter, 1);
    contentLayout->addWidget(m_statusLabel);
    m_content->setLayout(contentLayout);

    //深色网格视图，和 3D 视口保持一致的观感
    if (wQSS::theme() == wQSS::Dark) {
        m_modelList->setStyleSheet(QStringLiteral(
            "QListWidget{background:#21252b;color:#dcdfe4;border:1px solid #3a3f4b;padding:6px;font-size:12px;}"
            "QListWidget::item{border-radius:6px;padding:4px;}"
            "QListWidget::item:hover{background:#2c313a;}"
            "QListWidget::item:selected{background:#323844;border:1px solid #61afef;color:#ffffff;}"));
        m_statusLabel->setStyleSheet(QStringLiteral("color:#abb2bf;"));
    } else {
        m_modelList->setStyleSheet(QStringLiteral(
            "QListWidget{background:#ffffff;color:#1e1e1e;border:1px solid #cccedb;padding:6px;font-size:12px;}"
            "QListWidget::item{border-radius:6px;padding:4px;}"
            "QListWidget::item:hover{background:#eef4fb;}"
            "QListWidget::item:selected{background:#c9def5;border:1px solid #007acc;}"));
        m_statusLabel->setStyleSheet(QStringLiteral("color:#444444;"));
    }

    connect(openDirButton, &QPushButton::clicked, this, &Viewer3DWindow::onOpenDirectory);
    connect(openFileButton, &QPushButton::clicked, this, &Viewer3DWindow::onOpenFile);
    connect(refreshButton, &QPushButton::clicked, this, &Viewer3DWindow::onRefresh);
    connect(regenerateButton, &QPushButton::clicked, this, &Viewer3DWindow::onRegenerateThumbnails);
    connect(fitButton, &QPushButton::clicked, m_viewer, &Viewer3DWidget::fitView);
    connect(m_wireframeButton, &QPushButton::toggled, m_viewer, &Viewer3DWidget::setWireframe);
    connect(m_gridButton, &QPushButton::toggled, m_viewer, &Viewer3DWidget::setShowGrid);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &Viewer3DWindow::onFilterChanged);
    connect(m_formatCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &Viewer3DWindow::onFilterChanged);

    connect(m_modelList, &QListWidget::itemClicked, this, &Viewer3DWindow::onItemClicked);
    connect(m_modelList, &QListWidget::itemActivated, this, &Viewer3DWindow::onItemClicked);
    connect(m_modelList, &QWidget::customContextMenuRequested, this, &Viewer3DWindow::onContextMenuRequested);
    connect(m_modelList->verticalScrollBar(), &QScrollBar::valueChanged, this, &Viewer3DWindow::onListScrolled);

    connect(m_viewer, &Viewer3DWidget::boundsChanged, this, [this](const QString&) { updateStatus(); });
    connect(m_viewer, &Viewer3DWidget::modelFailed, this, [this](const QString& filePath, const QString& error) {
        auto item = m_items.value(filePath, nullptr);
        if (item != nullptr) {
            item->setToolTip(QStringLiteral("%1\n%2").arg(filePath, error));
        }
        updateStatus();
    });
}

void Viewer3DWindow::onOpenDirectory()
{
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Select the directory that contains 3D models"), m_currentDir);
    if (directory.isEmpty()) {
        return;
    }
    scanDirectory(directory);
}

void Viewer3DWindow::onOpenFile()
{
    QStringList filters;
    for (const QString& ext : ady::Viewer3D::supportedExtensions()) {
        filters << QStringLiteral("*.%1").arg(ext);
    }
    const QString filePath = QFileDialog::getOpenFileName(
        this, tr("Open 3D model"), m_currentDir,
        tr("3D models (%1)").arg(filters.join(' ')));
    if (filePath.isEmpty()) {
        return;
    }

    //打开单个文件时把视图切换到该文件所在目录
    const QFileInfo info(filePath);
    if (QDir::cleanPath(info.absolutePath()) != QDir::cleanPath(m_currentDir)) {
        scanDirectory(info.absolutePath());
    }
    auto item = m_items.value(info.absoluteFilePath(), nullptr);
    if (item != nullptr) {
        m_modelList->setCurrentItem(item);
        m_modelList->scrollToItem(item);
    }
    openModel(info.absoluteFilePath());
}

void Viewer3DWindow::onRefresh()
{
    scanDirectory(m_currentDir);
}

void Viewer3DWindow::onFilterChanged()
{
    applyFilter();
    updateStatus();
}

void Viewer3DWindow::onRegenerateThumbnails()
{
    if (m_currentDir.isEmpty()) {
        return;
    }
    scanDirectory(m_currentDir, true);
}

void Viewer3DWindow::onItemClicked(QListWidgetItem* item)
{
    if (item == nullptr) {
        return;
    }
    openModel(item->data(Qt::UserRole).toString());
}

void Viewer3DWindow::onThumbnailGenerated(const QString& modelFilePath, const QString& thumbnailPath)
{
    Q_UNUSED(thumbnailPath);
    auto item = m_items.value(modelFilePath, nullptr);
    if (item == nullptr) {
        return;
    }

    QPixmap pixmap = m_cache->load(modelFilePath);
    if (!pixmap.isNull()) {
        item->setIcon(QIcon(pixmap));
    }
}

void Viewer3DWindow::onThumbnailFailed(const QString& modelFilePath, const QString& errorString)
{
    auto item = m_items.value(modelFilePath, nullptr);
    if (item == nullptr) {
        return;
    }
    item->setIcon(QIcon(m_cache->placeholder(modelFilePath, true)));
    item->setToolTip(QStringLiteral("%1\n%2").arg(modelFilePath, errorString));
}

void Viewer3DWindow::onListScrolled()
{
    prioritizeVisibleItems();
}

void Viewer3DWindow::onContextMenuRequested(const QPoint& pos)
{
    QListWidgetItem* item = m_modelList->itemAt(pos);
    if (item == nullptr) {
        return;
    }

    const QString filePath = item->data(Qt::UserRole).toString();
    QMenu menu(this);
    QAction* openAction = menu.addAction(tr("Preview"));
    QAction* thumbnailAction = menu.addAction(tr("Regenerate Thumbnail"));
    QAction* folderAction = menu.addAction(tr("Open Containing Folder"));
    menu.addSeparator();
    QAction* copyAction = menu.addAction(tr("Copy Path"));

    QAction* selected = menu.exec(m_modelList->viewport()->mapToGlobal(pos));
    if (selected == openAction) {
        openModel(filePath);
    } else if (selected == thumbnailAction) {
        m_cache->remove(filePath);
        auto currentItem = m_items.value(filePath, nullptr);
        if (currentItem != nullptr) {
            currentItem->setIcon(QIcon(m_cache->placeholder(filePath)));
        }
        m_renderer->enqueue(filePath, true);
    } else if (selected == folderAction) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
    } else if (selected == copyAction) {
        QApplication::clipboard()->setText(filePath);
    }
}

void Viewer3DWindow::scanDirectory(const QString& dirPath, bool regenerate)
{
    if (dirPath.isEmpty() || !QDir(dirPath).exists()) {
        return;
    }

    m_renderer->clearQueue();
    m_modelList->clear();
    m_items.clear();
    m_viewer->clear();
    m_currentDir = QDir(dirPath).absolutePath();

    if (regenerate) {
        m_cache->clearDirectory(m_currentDir);
    }

    //缓存目录 ".cache" 与模型同级，扫描时只取当前目录的文件，不递归
    const QFileInfoList entries = QDir(m_currentDir).entryInfoList(QDir::Files, QDir::Name);
    for (const QFileInfo& info : entries) {
        const QString filePath = info.absoluteFilePath();
        if (ady::Viewer3D::isSupported(filePath)) {
            addModelItem(filePath);
        }
    }

    applyFilter();
    updateStatus();
    prioritizeVisibleItems();
}

void Viewer3DWindow::addModelItem(const QString& filePath)
{
    const QFileInfo info(filePath);

    auto* item = new QListWidgetItem(m_modelList);
    item->setText(info.fileName());
    item->setData(Qt::UserRole, filePath);
    item->setToolTip(QStringLiteral("%1\n%2 KB").arg(filePath).arg(info.size() / 1024));
    item->setTextAlignment(int(Qt::AlignHCenter | Qt::AlignTop));

    const QPixmap cached = m_cache->load(filePath);
    if (!cached.isNull()) {
        item->setIcon(QIcon(cached));
    } else {
        item->setIcon(QIcon(m_cache->placeholder(filePath)));
        m_renderer->enqueue(filePath);
    }

    m_items.insert(filePath, item);
    m_modelList->addItem(item);
}

void Viewer3DWindow::openModel(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }
    m_viewer->loadModel(filePath);
    updateStatus();
}

void Viewer3DWindow::applyFilter()
{
    const QString keyword = m_searchEdit != nullptr ? m_searchEdit->text().trimmed() : QString();
    const QString extension = m_formatCombo != nullptr ? m_formatCombo->currentData().toString() : QString();

    for (auto iter = m_items.constBegin(); iter != m_items.constEnd(); ++iter) {
        QListWidgetItem* item = iter.value();
        const QString suffix = QFileInfo(iter.key()).suffix();
        const bool matchExtension = extension.isEmpty() || suffix.compare(extension, Qt::CaseInsensitive) == 0;
        const bool matchKeyword = keyword.isEmpty() || item->text().contains(keyword, Qt::CaseInsensitive);
        item->setHidden(!(matchExtension && matchKeyword));
    }
}

void Viewer3DWindow::prioritizeVisibleItems()
{
    if (m_modelList == nullptr) {
        return;
    }
    const QRect viewport = m_modelList->viewport()->rect();
    QStringList visibleFiles;
    for (auto iter = m_items.constBegin(); iter != m_items.constEnd(); ++iter) {
        QListWidgetItem* item = iter.value();
        if (item->isHidden()) {
            continue;
        }
        if (viewport.intersects(m_modelList->visualItemRect(item))) {
            visibleFiles << iter.key();
        }
    }
    if (!visibleFiles.isEmpty()) {
        m_renderer->prioritize(visibleFiles);
    }
}

void Viewer3DWindow::updateStatus()
{
    if (m_statusLabel == nullptr) {
        return;
    }

    int visibleCount = 0;
    for (auto item : m_items) {
        if (!item->isHidden()) {
            ++visibleCount;
        }
    }

    QString message = tr("Directory: %1   |   %2 / %3 models   |   Thumbnail cache: %4")
                          .arg(m_currentDir, QString::number(visibleCount), QString::number(m_items.size()),
                               QDir(m_currentDir).absoluteFilePath(QStringLiteral(".cache")));

    const int pending = m_renderer != nullptr ? m_renderer->pendingCount() : 0;
    if (pending > 0) {
        message += tr("   |   Rendering thumbnails: %1").arg(pending);
    }
    if (!m_viewer->currentModelPath().isEmpty()) {
        message += tr("   |   Preview: %1").arg(QFileInfo(m_viewer->currentModelPath()).fileName());
        if (!m_viewer->boundsSummary().isEmpty()) {
            message += QStringLiteral(" (") + m_viewer->boundsSummary() + QStringLiteral(")");
        }
    }
    m_statusLabel->setText(message);
}

void Viewer3DWindow::closeEvent(QCloseEvent* event)
{
    if (m_renderer != nullptr) {
        m_renderer->clearQueue();
    }
    if (m_modelList != nullptr) {
        m_modelList->clear();
    }
    m_items.clear();
    wWindow::closeEvent(event);
}
