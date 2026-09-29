#include "addon_manager_dialog.h"
#include "ui_addon_manager_dialog.h"
#include "addon_manager_model.h"
#include "components/listview/listview_model.h"
#include "addon_loader.h"
#include "storage/addon_storage.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include "zip/minizip/unzip.h"
#ifdef _WIN32
#include "zip/minizip/iowin32.h"
#endif
namespace ady{
AddonManagerDialog* AddonManagerDialog::instance = nullptr;


class AddonManagerDialogPrivate{
public:
    AddonManagerModel* model;
    QString currentName;
};

AddonManagerDialog::AddonManagerDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::AddonManagerDialog)
{

    this->setStyleSheet(".ady--ListView{border:0;}");
    ui->setupUi(this);
    ui->enableButton->hide();
    ui->uninstallButton->hide();

    d = new AddonManagerDialogPrivate;
    d->model = new AddonManagerModel(ui->listView);
    connect(ui->close,&QPushButton::clicked,this,&AddonManagerDialog::close);
    connect(ui->listView,&ListView::itemClicked,this,&AddonManagerDialog::onItemClicked);
    connect(ui->install,&QPushButton::clicked,this,&AddonManagerDialog::onInstall);
    connect(ui->enableButton,&QPushButton::clicked,this,&AddonManagerDialog::onEnable);
    connect(ui->uninstallButton,&QPushButton::clicked,this,&AddonManagerDialog::onUninstall);
    this->resetupUi();
    ui->listView->setModel(d->model);
    this->initView();

    //set default path to addon directory under application dir
    QString addonDir = QCoreApplication::applicationDirPath() + "/addon";
    ui->file->setDir(addonDir);
    ui->file->setFilter("Addon files (*.zip *.dll)");

    //ui->widget->start();
}

void AddonManagerDialog::initView(){

    QList<AddonItem> list;
    AddonStorage storage;
    auto records = storage.all();
    for(const auto& record : records){
        AddonItem item;
        item.installed = (record.status == 1);
        item.is_system = record.is_system;
        item.title = record.title;
        item.description = record.description;
        item.author = record.author;
        item.version = record.version;
        list << item;
    }
    d->model->setDataSource(list);
}

AddonManagerDialog::~AddonManagerDialog()
{
    delete d;
    delete ui;
}

void AddonManagerDialog::onItemClicked(int index){
    auto item = d->model->itemAt(index);
    d->currentName = item.title;
    ui->author->setText(tr("<strong>Author:</strong>%1").arg(item.author));
    ui->version->setText(tr("<strong>Version:</strong>%1").arg(item.version));
    ui->url->setText(tr("<strong>Home Page:</strong>%1").arg(item.url));
    ui->enableButton->show();
    ui->uninstallButton->show();
    ui->enableButton->setText(item.installed ? tr("Disable") : tr("Enable"));
    ui->enableButton->setEnabled(!item.is_system);
    ui->uninstallButton->setEnabled(!item.is_system);
}

AddonManagerDialog* AddonManagerDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new AddonManagerDialog(parent);
        instance->setModal(true);
    }
    instance->show();
    return instance;
}

static bool extractZip(const QString& zipPath, const QString& destDir)
{
#ifdef _WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64A(&ffunc);
    unzFile zf = unzOpen2_64(zipPath.toLocal8Bit().constData(), &ffunc);
#else
    unzFile zf = unzOpen(zipPath.toLocal8Bit().constData());
#endif
    if(!zf) return false;

    int ret = unzGoToFirstFile(zf);
    while(ret == UNZ_OK){
        char filename[512];
        unz_file_info fi;
        if(unzGetCurrentFileInfo(zf, &fi, filename, sizeof(filename), NULL, 0, NULL, 0) != UNZ_OK)
            break;

        QString entryName = QString::fromUtf8(filename);
        QString fullPath = destDir + "/" + entryName;

        //if entry is a directory, create it
        if(entryName.endsWith('/')){
            QDir().mkpath(fullPath);
        }else{
            //ensure parent directory exists
            QDir().mkpath(QFileInfo(fullPath).absolutePath());

            if(unzOpenCurrentFile(zf) != UNZ_OK)
                break;

            QFile outFile(fullPath);
            if(outFile.open(QIODevice::WriteOnly)){
                char buf[4096];
                int bytesRead;
                do{
                    bytesRead = unzReadCurrentFile(zf, buf, sizeof(buf));
                    if(bytesRead > 0)
                        outFile.write(buf, bytesRead);
                }while(bytesRead > 0);
                outFile.close();
            }
            unzCloseCurrentFile(zf);
        }
        ret = unzGoToNextFile(zf);
    }
    unzClose(zf);
    return true;
}

void AddonManagerDialog::onInstall()
{
    QString filePath = ui->file->text();
    if(filePath.isEmpty()){
        QMessageBox::warning(this, tr("Warning"), tr("Please select an addon file."));
        return;
    }

    QFileInfo fi(filePath);
    if(!fi.exists()){
        QMessageBox::warning(this, tr("Warning"), tr("File does not exist."));
        return;
    }

    QString suffix = fi.suffix().toLower();
    if(suffix != "zip" && suffix != "dll"){
        QMessageBox::warning(this, tr("Warning"), tr("Unsupported file type."));
        return;
    }

    QString addonDir = QCoreApplication::applicationDirPath() + "/addon";
    QDir().mkpath(addonDir);

    ui->widget->start();
    ui->install->setEnabled(false);

    bool ok = false;
    if(suffix == "zip"){
        ok = extractZip(filePath, addonDir);
    }else{
        //for dll file
        QString addonName = fi.baseName(); //e.g., FTP from FTP.dll
        QString addonSubDir = addonDir + "/" + addonName;

        //check if dll is already in addon directory
        if(fi.absolutePath() == QDir(addonSubDir).absolutePath()){
            //dll is in addon dir, load and call install directly
            QString relPath = addonName + "/" + addonName; //e.g., "FTP/FTP"
            ady::AddonLoader* loader = ady::AddonLoader::getInstance();
            if(loader->loadFile(relPath)){
                ok = loader->install();
            }
        }else{
            //dll is not in addon dir, copy it
            QDir().mkpath(addonSubDir);
            QString destPath = addonSubDir + "/" + fi.fileName();
            ok = QFile::copy(filePath, destPath);
        }
    }

    ui->widget->stop();
    ui->install->setEnabled(true);

    if(ok){
        QMessageBox::information(this, tr("Success"), tr("Addon installed successfully. Please restart the application."));
    }else{
        QMessageBox::warning(this, tr("Error"), tr("Failed to install addon."));
    }
}

void AddonManagerDialog::onEnable()
{
    if(d->currentName.isEmpty()) return;
    AddonStorage storage;
    auto record = storage.one(d->currentName);
    if(record.id <= 0 || record.is_system) return;
    record.status = (record.status == 1) ? 0 : 1;
    storage.update(record);
    initView();
    QMessageBox::information(this, tr("Info"), tr("Changes will take effect after restart."));
}

void AddonManagerDialog::onUninstall()
{
    if(d->currentName.isEmpty()) return;
    AddonStorage storage;
    auto record = storage.one(d->currentName);
    if(record.id <= 0 || record.is_system) return;
    storage.del(record.id);
    d->currentName.clear();
    initView();
    QMessageBox::information(this, tr("Info"), tr("Changes will take effect after restart."));
}
}