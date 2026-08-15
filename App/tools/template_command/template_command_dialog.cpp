#include "template_command_dialog.h"
#include "ui_template_command_dialog.h"
#include "components/dir_selector.h"
#include "components/file_selector.h"
#include "storage/template_command_storage.h"
#include "storage/template_command_history.h"
#include <ptyqt.h>
#include <QProcessEnvironment>
#include <QPlainTextEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QJsonArray>
#include <QJsonDocument>

namespace ady {

TemplateCommandDialog* TemplateCommandDialog::instance = nullptr;

class TemplateCommandDialogPrivate {
public:
    QList<TemplateCommand> templates;
    int currentIndex = -1;
    QList<ParamType> currentParamTypes;
    QList<QWidget*> runFormWidgets;
    QPlainTextEdit* output = nullptr;
    IPtyProcess* currentProcess = nullptr;
    QList<TemplateCommandHistoryRecord> historyRecords;
};

static QString paramTypesToString(const QList<ParamType>& types)
{
    QStringList list;
    for (auto t : types) {
        list << QString::number(static_cast<int>(t));
    }
    return list.join(",");
}

static QList<ParamType> paramTypesFromString(const QString& str)
{
    QList<ParamType> types;
    if (str.isEmpty()) return types;
    for (auto& s : str.split(",")) {
        types.append(static_cast<ParamType>(s.trimmed().toInt()));
    }
    return types;
}

// ── Construction / Destruction ───────────────────────────────

TemplateCommandDialog::TemplateCommandDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::TemplateCommandDialog)
{
    d = new TemplateCommandDialogPrivate;
    ui->setupUi(this);
    this->resetupUi();

    // splitter proportion: form area gets more space
    ui->runSplitter->setStretchFactor(0, 3);
    ui->runSplitter->setStretchFactor(1, 2);

    // populate shell combo in Info tab (Windows: cmd + powershell)
#ifdef Q_OS_WIN
    ui->infoShellCombo->addItem("Command Prompt", "cmd");
    QString ps = QStandardPaths::findExecutable("powershell.exe");
    if (!ps.isEmpty()) {
        ui->infoShellCombo->addItem("PowerShell", "powershell");
    }
#else
    ui->infoShellCombo->addItem("Shell", "sh");
#endif

    // create output widget (replace placeholder)
    d->output = new QPlainTextEdit(this);
    d->output->setReadOnly(true);
    d->output->setFont(QFont("Consolas", 10));
    d->output->setStyleSheet(
        "QPlainTextEdit{background-color:#1e1e1e;color:#d4d4d4;"
        "border:1px solid #333;padding:4px;}");
    // right-click context menu: Clear
    d->output->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(d->output, &QPlainTextEdit::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu menu;
        menu.addAction(tr("Clear"), [this]() { d->output->clear(); });
        menu.exec(d->output->mapToGlobal(pos));
    });
    // replace the placeholder QTextEdit with our QPlainTextEdit
    auto* splitter = ui->runSplitter;
    int outputIdx = splitter->indexOf(ui->outputPlaceholder);
    delete ui->outputPlaceholder;
    splitter->insertWidget(outputIdx, d->output);

    // set form layouts on scroll-area containers
    auto* runFormLayout = new QFormLayout(ui->runFormContainer);
    runFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    ui->runFormContainer->setLayout(runFormLayout);

    auto* infoConfigLayout = new QFormLayout(ui->infoConfigContainer);
    infoConfigLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    ui->infoConfigContainer->setLayout(infoConfigLayout);

    // signals
    connect(ui->listWidget, &QListWidget::currentRowChanged,
            this, &TemplateCommandDialog::onListCurrentRowChanged);
    connect(ui->addBtn, &QPushButton::clicked,
            this, &TemplateCommandDialog::onAddTemplate);
    connect(ui->removeBtn, &QPushButton::clicked,
            this, &TemplateCommandDialog::onRemoveTemplate);
    connect(ui->infoTitle, &QLineEdit::textChanged,
            this, &TemplateCommandDialog::onInfoTitleChanged);
    connect(ui->infoCommand, &QLineEdit::textChanged,
            this, &TemplateCommandDialog::onInfoCommandChanged);
    connect(ui->tabWidget, &QTabWidget::currentChanged,
            this, &TemplateCommandDialog::onTabChanged);
    connect(ui->runBtn, &QPushButton::clicked,
            this, &TemplateCommandDialog::onRunClicked);
    connect(ui->infoSaveBtn, &QPushButton::clicked,
            this, &TemplateCommandDialog::onInfoSaveClicked);
    connect(ui->historyListWidget, &QListWidget::itemClicked,
            this, [this](QListWidgetItem* item) {
        int row = ui->historyListWidget->row(item);
        onHistoryRowClicked(row);
    });
    connect(ui->historyDeleteBtn, &QPushButton::clicked,
            this, &TemplateCommandDialog::onHistoryDeleteClicked);
    connect(ui->runTemplateLabel, &QLineEdit::textChanged,
            this, [this](const QString& text) {
        if (m_loading) return;
        // Only adjust currentParamTypes for run form, don't touch data model or info tab
        int count = parsePlaceholderCount(text);
        while (d->currentParamTypes.size() < count)
            d->currentParamTypes.append(Text);
        while (d->currentParamTypes.size() > count)
            d->currentParamTypes.removeLast();
        rebuildRunForm();
    });

    // init
    loadTemplates();
    loadHistory();
    ui->removeBtn->setEnabled(d->templates.size() > 0);
    if (d->templates.size() > 0) {
        ui->listWidget->setCurrentRow(0);
    }

    this->setWindowTitle(tr("Template Command"));
    this->setMinimumSize(800, 600);
}

TemplateCommandDialog::~TemplateCommandDialog()
{
    if (d->currentProcess) {
        d->currentProcess->kill();
        auto* p = d->currentProcess;
        d->currentProcess = nullptr;
        QTimer::singleShot(0, this, [p]() { delete p; });
    }
    instance = nullptr;
    delete ui;
    delete d;
}

// ── Public static ────────────────────────────────────────────

TemplateCommandDialog* TemplateCommandDialog::open(QWidget* parent)
{
    if (instance == nullptr) {
        instance = new TemplateCommandDialog(parent);
    }
    instance->show();
    instance->raise();
    instance->activateWindow();
    return instance;
}

// ── Template parsing ─────────────────────────────────────────

int TemplateCommandDialog::parsePlaceholderCount(const QString& cmd) const
{
    int maxIndex = 0;
    QRegularExpression re("%(\\d+)");
    auto it = re.globalMatch(cmd);
    while (it.hasNext()) {
        auto match = it.next();
        int idx = match.captured(1).toInt();
        if (idx > maxIndex) maxIndex = idx;
    }
    return maxIndex;
}

QString TemplateCommandDialog::paramTypeName(ParamType type)
{
    switch (type) {
    case Text:   return "Text";
    case Number: return "Number";
    case Dir:    return "Directory";
    case File:   return "File";
    }
    return "Text";
}

QList<ParamType> TemplateCommandDialog::currentParamTypes() const
{
    QList<ParamType> types;
    auto* layout = qobject_cast<QFormLayout*>(ui->infoConfigContainer->layout());
    if (!layout) return types;
    for (int i = 0; i < layout->rowCount(); i++) {
        auto* item = layout->itemAt(i, QFormLayout::FieldRole);
        if (!item) continue;
        auto* w = item->widget();
        // combo is inside a container widget
        auto* combo = w->findChild<QComboBox*>();
        if (!combo) continue;
        types.append(static_cast<ParamType>(combo->currentIndex()));
    }
    return types;
}

QString TemplateCommandDialog::resolveCommand() const
{
    QString cmd = ui->runTemplateLabel->text();
    if (cmd.isEmpty()) return {};

    // collect values from run form
    QStringList values;
    auto* layout = qobject_cast<QFormLayout*>(ui->runFormContainer->layout());
    if (layout) {
        for (int i = 0; i < layout->rowCount(); i++) {
            auto* item = layout->itemAt(i, QFormLayout::FieldRole);
            if (!item) { values << ""; continue; }
            auto* w = item->widget();
            if (auto* le = qobject_cast<QLineEdit*>(w)) {
                values << le->text();
            } else if (auto* sb = qobject_cast<QSpinBox*>(w)) {
                values << QString::number(sb->value());
            } else if (auto* ds = qobject_cast<DirSelector*>(w)) {
                values << ds->text();
            } else if (auto* fs = qobject_cast<FileSelector*>(w)) {
                values << fs->text();
            } else {
                values << "";
            }
        }
    }

    // replace %1..%N using QString::arg chain behaviour
    for (int i = 0; i < values.size(); i++) {
        cmd = cmd.arg(values[i]);
    }
    return cmd;
}

// Strip ANSI/VT escape sequences and other control characters from terminal output
static QString stripAnsiCodes(const QString& text)
{
    // Match all common ANSI/VT escape sequences
    static QRegularExpression ansiRegex(
        "\x1b\\]"          // OSC: ESC ] (title changes, etc.)
        "[^\x07\x1b]*"     // content until terminator
        "(?:\x07|\x1b\\\\)" // terminated by BEL or ST (ESC \)
        "|\x1b\\["          // CSI: ESC [
        "[\\x30-\\x3f]*"   // parameter bytes
        "[\\x20-\\x2f]*"   // intermediate bytes
        "[\\x40-\\x7e]"    // final byte
        "|\x1b[\\(\\)]"    // charset selection: ESC ( or ESC )
        "[\\x20-\\x7e]"    // charset character
        "|\x1b."           // other 2-char ESC sequences
        , QRegularExpression::DotMatchesEverythingOption
    );
    QString result = text;
    result.replace(ansiRegex, "");
    // strip any remaining standalone control chars (ESC, BEL, etc.)
    result.remove(QChar('\x1b'));
    result.remove(QChar('\x07'));
    result.remove(QChar('\x00'));
    return result;
}

// ── Execute command ──────────────────────────────────────────

void TemplateCommandDialog::executeCommand()
{
    QString resolved = resolveCommand();
    if (resolved.isEmpty()) {
        d->output->appendPlainText("[Error] Command template is empty.");
        return;
    }

    // collect param values for history
    QStringList paramValues;
    for (auto* w : d->runFormWidgets) {
        if (auto* le = qobject_cast<QLineEdit*>(w))
            paramValues << le->text();
        else if (auto* sb = qobject_cast<QSpinBox*>(w))
            paramValues << QString::number(sb->value());
        else if (auto* ds = qobject_cast<DirSelector*>(w))
            paramValues << ds->text();
        else if (auto* fs = qobject_cast<FileSelector*>(w))
            paramValues << fs->text();
    }
    QString templateTitle = d->currentIndex >= 0 && d->currentIndex < d->templates.size()
                                ? d->templates[d->currentIndex].title : QString();
    QString commandTemplate = ui->runTemplateLabel->text();
    QString shellType = d->currentIndex >= 0 && d->currentIndex < d->templates.size()
                            ? d->templates[d->currentIndex].shellType : QString();
    saveToHistory(templateTitle, commandTemplate, resolved, paramValues, d->currentParamTypes, shellType);

    // kill previous process if still running
    if (d->currentProcess) {
        d->currentProcess->kill();
        auto* p = d->currentProcess;
        d->currentProcess = nullptr;
        QTimer::singleShot(0, this, [p]() { delete p; });
    }

    QString shellExe;
    QStringList shellArgs;
    if (shellType == "powershell") {
        shellExe = QStandardPaths::findExecutable("powershell.exe");
        // Force PowerShell to use system locale encoding and disable progress bar
        QString psCmd = "[Console]::OutputEncoding=[System.Text.Encoding]::Default;"
                        "$ProgressPreference='SilentlyContinue';"
                        + resolved;
        // Use -EncodedCommand (Base64 of UTF-16LE) to avoid quote parsing issues
        QByteArray utf16(reinterpret_cast<const char*>(psCmd.utf16()), psCmd.size() * 2);
        QString base64 = QString::fromLatin1(utf16.toBase64());
        shellArgs << "-EncodedCommand" << base64;
    } else {
        shellExe = QStandardPaths::findExecutable("cmd.exe");
        shellArgs << "/c" << resolved;
    }

    d->output->appendPlainText(QString("\n========== [%1] ==========").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    d->output->appendPlainText(QString("> %1").arg(resolved));

    d->currentProcess = PtyQt::createPtyProcess(IPtyProcess::AutoPty);
    auto env = QProcessEnvironment::systemEnvironment().toStringList();
    QString workDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);

    bool ret = d->currentProcess->startProcess(shellExe, shellArgs, workDir, env, 120, 50);
    if (ret) {
        connect(d->currentProcess->notifier(), &QIODevice::readyRead, this, [this]() {
            if (!d->currentProcess) return;
            QByteArray data = d->currentProcess->readAll();
            QString text = stripAnsiCodes(QString::fromUtf8(data));
            if (!text.trimmed().isEmpty())
                d->output->appendPlainText(text);
        });
        connect(d->currentProcess->notifier(), &QIODevice::aboutToClose, this, [this]() {
            if (!d->currentProcess) return;
            auto* p = d->currentProcess;
            d->currentProcess = nullptr;
            d->output->appendPlainText("[Process finished]");
            QTimer::singleShot(0, this, [p]() { delete p; });
        });
    } else {
        d->output->appendPlainText(QString("[Failed to start: %1]").arg(d->currentProcess->lastError()));
        delete d->currentProcess;
        d->currentProcess = nullptr;
    }
}

// ── Template list management ─────────────────────────────────

void TemplateCommandDialog::onAddTemplate()
{
    TemplateCommand tc;
    tc.title = tr("New Command %1").arg(d->templates.size() + 1);
    tc.commandTemplate = "echo \"%1\"";
    tc.paramTypes = {Text};
    tc.defaultValues = QStringList("");
    tc.shellType = "cmd";

    // insert into database
    TemplateCommandStorage storage;
    TemplateCommandRecord rec;
    rec.title = tc.title;
    rec.command_template = tc.commandTemplate;
    rec.param_types = paramTypesToString(tc.paramTypes);
    rec.default_values = tc.defaultValues.join(",");
    rec.shell_type = tc.shellType;
    rec.listorder = d->templates.size();
    tc.id = storage.insert(rec);

    d->templates.append(tc);

    ui->listWidget->addItem(tc.title);
    ui->listWidget->setCurrentRow(d->templates.size() - 1);
    ui->removeBtn->setEnabled(true);
}

void TemplateCommandDialog::onRemoveTemplate()
{
    int row = ui->listWidget->currentRow();
    if (row < 0 || row >= d->templates.size()) return;

    QString title = d->templates[row].title;
    if (QMessageBox::question(this, tr("Confirm Remove"),
                              tr("Are you sure you want to remove \"%1\"?").arg(title),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // delete from database
    TemplateCommandStorage storage;
    long long id = d->templates[row].id;
    if (id > 0) {
        storage.del(id);
    }

    d->templates.removeAt(row);
    delete ui->listWidget->takeItem(row);

    ui->removeBtn->setEnabled(d->templates.size() > 0);

    if (d->templates.size() > 0) {
        int newRow = qMin(row, d->templates.size() - 1);
        ui->listWidget->setCurrentRow(newRow);
    } else {
        d->currentIndex = -1;
        ui->infoTitle->clear();
        ui->infoCommand->clear();
        ui->runTemplateLabel->clear();
        // clear dynamic forms
        auto* infoLayout = qobject_cast<QFormLayout*>(ui->infoConfigContainer->layout());
        if (infoLayout) {
            while (infoLayout->rowCount() > 0) infoLayout->removeRow(0);
        }
        auto* runLayout = qobject_cast<QFormLayout*>(ui->runFormContainer->layout());
        if (runLayout) {
            while (runLayout->rowCount() > 0) runLayout->removeRow(0);
        }
    }
}

// ── List selection ───────────────────────────────────────────

void TemplateCommandDialog::onListCurrentRowChanged(int currentRow)
{
    if (m_loading) return;
    m_loading = true;
    d->currentIndex = currentRow;
    if (currentRow >= 0 && currentRow < d->templates.size()) {
        loadTemplateToInfoTab(currentRow);
        rebuildRunForm();
    }
    m_loading = false;
}

// ── Info-tab callbacks ───────────────────────────────────────

void TemplateCommandDialog::onInfoTitleChanged(const QString& text)
{
    // title change is local, saved only on Save click
}

void TemplateCommandDialog::onInfoCommandChanged(const QString& text)
{
    if (m_loading) return;
    // adjust currentParamTypes to match placeholder count
    int count = parsePlaceholderCount(text);
    while (d->currentParamTypes.size() < count)
        d->currentParamTypes.append(Text);
    while (d->currentParamTypes.size() > count)
        d->currentParamTypes.removeLast();
    rebuildInfoConfigTab();
}

void TemplateCommandDialog::onParamTypeChanged(int index)
{
    if (m_loading) return;
    auto* combo = qobject_cast<QComboBox*>(sender());
    if (!combo) return;
    int paramIndex = combo->property("paramIndex").toInt();
    if (paramIndex >= 0 && paramIndex < d->currentParamTypes.size()) {
        d->currentParamTypes[paramIndex] = static_cast<ParamType>(index);
    }
}

void TemplateCommandDialog::onDefaultValueChanged(const QString& text)
{
    // default value change is local, saved only on Save click
}

// ── Tab switching ────────────────────────────────────────────

void TemplateCommandDialog::onTabChanged(int index)
{
    // Don't rebuild run form on tab switch - preserve user edits
}

void TemplateCommandDialog::onRunClicked()
{
    executeCommand();
}

// ── Load / Save templates (SQLite) ───────────────────────────

void TemplateCommandDialog::loadTemplates()
{
    TemplateCommandStorage storage;
    auto records = storage.all();
    for (auto& rec : records) {
        qDebug() << "  record: id=" << rec.id << "title=" << rec.title
                 << "cmd=" << rec.command_template << "params=" << rec.param_types
                 << "defaults=" << rec.default_values << "shell=" << rec.shell_type;
        TemplateCommand tc;
        tc.id = rec.id;
        tc.title = rec.title;
        tc.commandTemplate = rec.command_template;
        tc.paramTypes = paramTypesFromString(rec.param_types);
        tc.defaultValues = rec.default_values.split(",", Qt::KeepEmptyParts);
        tc.shellType = rec.shell_type.isEmpty() ? "cmd" : rec.shell_type;
        d->templates.append(tc);
        ui->listWidget->addItem(tc.title);
    }
}

void TemplateCommandDialog::saveTemplates()
{
    // batch sync: not used in per-item mode
}

void TemplateCommandDialog::saveCurrentTemplate()
{
    if (m_loading) return;
    if (d->currentIndex < 0 || d->currentIndex >= d->templates.size()) return;

    auto& tc = d->templates[d->currentIndex];
    tc.title = ui->infoTitle->text();
    tc.commandTemplate = ui->infoCommand->text();
    tc.paramTypes = currentParamTypes();
    tc.shellType = ui->infoShellCombo->currentData().toString();

    TemplateCommandStorage storage;
    TemplateCommandRecord rec;
    rec.id = tc.id;
    rec.title = tc.title;
    rec.command_template = tc.commandTemplate;
    rec.param_types = paramTypesToString(tc.paramTypes);
    // collect default values from info config tab
    QStringList defaultVals;
    for (int i = 0; i < d->currentParamTypes.size(); i++) {
        auto* edit = ui->infoConfigContainer->findChild<QLineEdit*>(QString("defVal_%1").arg(i));
        defaultVals << (edit ? edit->text() : QString());
    }
    tc.defaultValues = defaultVals;
    rec.default_values = defaultVals.join(",");
    rec.shell_type = tc.shellType;
    rec.listorder = d->currentIndex;

    if (rec.id > 0) {
        bool ok = storage.update(rec);
    } else {
        long long newId = storage.insert(rec);
        tc.id = newId;
    }

    // update list widget title
    auto* item = ui->listWidget->item(d->currentIndex);
    if (item) item->setText(tc.title);
}

void TemplateCommandDialog::onInfoSaveClicked()
{
    saveCurrentTemplate();
}

// ── Load template into Info tab ──────────────────────────────

void TemplateCommandDialog::loadTemplateToInfoTab(int index)
{
    if (index < 0 || index >= d->templates.size()) return;
    // Note: m_loading is managed by the caller

    const auto& tc = d->templates[index];
    ui->infoTitle->setText(tc.title);
    ui->infoCommand->setText(tc.commandTemplate);
    d->currentParamTypes = tc.paramTypes;

    // set shell combo
    QString shellType = tc.shellType.isEmpty() ? "cmd" : tc.shellType;
    int shellIdx = ui->infoShellCombo->findData(shellType);
    if (shellIdx >= 0) ui->infoShellCombo->setCurrentIndex(shellIdx);

    // adjust length
    int count = parsePlaceholderCount(tc.commandTemplate);
    while (d->currentParamTypes.size() < count) {
        d->currentParamTypes.append(Text);
    }
    while (d->currentParamTypes.size() > count) {
        d->currentParamTypes.removeLast();
    }
    // sync defaultValues length
    while (tc.defaultValues.size() < count)
        const_cast<TemplateCommand&>(tc).defaultValues.append("");
    while (tc.defaultValues.size() > count)
        const_cast<TemplateCommand&>(tc).defaultValues.removeLast();

    rebuildInfoConfigTab();
    updateRunTemplateLabel();
}

// ── Rebuild Info-tab config form ─────────────────────────────

void TemplateCommandDialog::rebuildInfoConfigTab()
{
    auto* layout = qobject_cast<QFormLayout*>(ui->infoConfigContainer->layout());
    if (!layout) return;

    // clear existing rows
    while (layout->rowCount() > 0) {
        layout->removeRow(0);
    }

    for (int i = 0; i < d->currentParamTypes.size(); i++) {
        auto* label = new QLabel(QString("Param %1:").arg(i + 1), ui->infoConfigContainer);
        auto* combo = new QComboBox(ui->infoConfigContainer);
        combo->setProperty("paramIndex", i);
        combo->addItem("Text");
        combo->addItem("Number");
        combo->addItem("Directory");
        combo->addItem("File");
        combo->setCurrentIndex(static_cast<int>(d->currentParamTypes[i]));
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TemplateCommandDialog::onParamTypeChanged);

        // default value input
        auto* defEdit = new QLineEdit(ui->infoConfigContainer);
        defEdit->setObjectName(QString("defVal_%1").arg(i));
        defEdit->setProperty("paramIndex", i);
        defEdit->setPlaceholderText(tr("Default value"));
        if (d->currentIndex >= 0 && d->currentIndex < d->templates.size()) {
            auto& dv = d->templates[d->currentIndex].defaultValues;
            if (i < dv.size()) defEdit->setText(dv[i]);
        }
        connect(defEdit, &QLineEdit::textChanged,
                this, &TemplateCommandDialog::onDefaultValueChanged);

        // use a container widget for combo + default value side by side
        auto* container = new QWidget(ui->infoConfigContainer);
        auto* hLayout = new QHBoxLayout(container);
        hLayout->setContentsMargins(0, 0, 0, 0);
        hLayout->setSpacing(4);
        combo->setParent(container);
        defEdit->setParent(container);
        hLayout->addWidget(combo, 1);
        hLayout->addWidget(defEdit, 2);
        container->setLayout(hLayout);

        layout->addRow(label, container);
    }
}

// ── Rebuild Run-tab form ─────────────────────────────────────

void TemplateCommandDialog::rebuildRunForm()
{
    auto* layout = qobject_cast<QFormLayout*>(ui->runFormContainer->layout());
    if (!layout) return;

    // clear existing rows
    while (layout->rowCount() > 0) {
        layout->removeRow(0);
    }
    d->runFormWidgets.clear();

    if (d->currentIndex < 0 || d->currentIndex >= d->templates.size()) return;
    const auto& tc = d->templates[d->currentIndex];

    for (int i = 0; i < d->currentParamTypes.size(); i++) {
        auto* label = new QLabel(QString("%1:").arg(i + 1), ui->runFormContainer);
        QWidget* widget = nullptr;

        // get default value
        QString defVal;
        if (i < tc.defaultValues.size()) defVal = tc.defaultValues[i];

        switch (d->currentParamTypes[i]) {
        case Text: {
            auto* le = new QLineEdit(ui->runFormContainer);
            le->setPlaceholderText(QString("Parameter %1").arg(i + 1));
            if (!defVal.isEmpty()) le->setText(defVal);
            widget = le;
            break;
        }
        case Number: {
            auto* sb = new QSpinBox(ui->runFormContainer);
            sb->setRange(-999999, 999999);
            if (!defVal.isEmpty()) sb->setValue(defVal.toInt());
            widget = sb;
            break;
        }
        case Dir: {
            auto* ds = new DirSelector(ui->runFormContainer);
            ds->setMinimumHeight(24);
            ds->setMaximumHeight(24);
            if (!defVal.isEmpty()) ds->setText(defVal);
            widget = ds;
            break;
        }
        case File: {
            auto* fs = new FileSelector(ui->runFormContainer);
            fs->setMinimumHeight(24);
            fs->setMaximumHeight(24);
            if (!defVal.isEmpty()) fs->setText(defVal);
            widget = fs;
            break;
        }
        }

        if (widget) {
            layout->addRow(label, widget);
            d->runFormWidgets.append(widget);
        }
    }
}

// ── Update run-tab template label ────────────────────────────

void TemplateCommandDialog::updateRunTemplateLabel()
{
    if (d->currentIndex >= 0 && d->currentIndex < d->templates.size()) {
        ui->runTemplateLabel->setText(d->templates[d->currentIndex].commandTemplate);
    } else {
        ui->runTemplateLabel->clear();
    }
}

// ── History ──────────────────────────────────────────────────

void TemplateCommandDialog::saveToHistory(const QString& templateTitle,
                                           const QString& commandTemplate,
                                           const QString& resolvedCommand,
                                           const QStringList& paramValues,
                                           const QList<ParamType>& paramTypes,
                                           const QString& shellType)
{
    // serialize params as JSON array
    QJsonArray arr;
    for (const auto& v : paramValues)
        arr.append(v);
    QString paramsJson = QString::fromUtf8(QJsonDocument(arr).toJson(QJsonDocument::Compact));

    // serialize param types
    QString paramTypesStr = paramTypesToString(paramTypes);

    TemplateCommandHistoryStorage storage;

    // dedup: if same command + params exists, just update timestamp
    long long existingId = storage.findDuplicate(resolvedCommand, paramsJson);
    if (existingId > 0) {
        storage.updateTimestamp(existingId);
    } else {
        TemplateCommandHistoryRecord rec;
        rec.template_title = templateTitle;
        rec.command_template = commandTemplate;
        rec.command = resolvedCommand;
        rec.params = paramsJson;
        rec.param_types = paramTypesStr;
        rec.shell_type = shellType;
        rec.created_at = QDateTime::currentDateTime().toString(Qt::ISODate);
        long long newId = storage.insert(rec);
    }

    // trim to max 200
    storage.trimTo(200);

    // reload history list
    loadHistory();
}

void TemplateCommandDialog::loadHistory()
{
    m_loading = true;
    ui->historyListWidget->clear();
    d->historyRecords.clear();

    TemplateCommandHistoryStorage storage;
    d->historyRecords = storage.all();

    for (const auto& rec : d->historyRecords) {
        QString display = QString("[%1] %2")
                              .arg(rec.created_at.left(16).replace("T", " "))
                              .arg(rec.template_title.isEmpty() ? rec.command.left(40) : rec.template_title);
        ui->historyListWidget->addItem(display);
    }

    m_loading = false;
}

void TemplateCommandDialog::loadHistoryToRunTab(int historyIndex)
{
    if (historyIndex < 0 || historyIndex >= d->historyRecords.size())
        return;

    const auto& rec = d->historyRecords[historyIndex];

    // find matching template by title
    int templateIdx = -1;
    for (int i = 0; i < d->templates.size(); ++i) {
        if (d->templates[i].title == rec.template_title) {
            templateIdx = i;
            break;
        }
    }

    // for old records with empty command_template/param_types, fall back to template data
    QString cmdTemplate = rec.command_template;
    QString paramTypesStr = rec.param_types;
    if ((cmdTemplate.isEmpty() || paramTypesStr.isEmpty()) && templateIdx >= 0) {
        const auto& tc = d->templates[templateIdx];
        if (cmdTemplate.isEmpty()) cmdTemplate = tc.commandTemplate;
        if (paramTypesStr.isEmpty()) paramTypesStr = paramTypesToString(tc.paramTypes);
    }

    // select the matching template silently (don't switch left tab)
    if (templateIdx >= 0) {
        m_loading = true;
        d->currentIndex = templateIdx;
        ui->listWidget->setCurrentRow(templateIdx);
        m_loading = false;
    }

    // switch to Run tab
    ui->tabWidget->setCurrentIndex(0);

    // restore the original command template (with %1, %2 placeholders)
    m_loading = true;
    ui->runTemplateLabel->setText(cmdTemplate);
    m_loading = false;

    // restore param types from saved data
    d->currentParamTypes = paramTypesFromString(paramTypesStr);

    // rebuild run form with correct param types
    rebuildRunForm();

    // fill param values from history
    QJsonArray arr = QJsonDocument::fromJson(rec.params.toUtf8()).array();
    for (int i = 0; i < d->runFormWidgets.size() && i < arr.size(); ++i) {
        auto* w = d->runFormWidgets[i];
        QString val = arr[i].toString();
        if (auto* le = qobject_cast<QLineEdit*>(w))
            le->setText(val);
        else if (auto* sb = qobject_cast<QSpinBox*>(w))
            sb->setValue(val.toInt());
        else if (auto* ds = qobject_cast<DirSelector*>(w))
            ds->setText(val);
        else if (auto* fs = qobject_cast<FileSelector*>(w))
            fs->setText(val);
    }
}

void TemplateCommandDialog::onHistoryRowClicked(int row)
{
    if (m_loading) return;
    loadHistoryToRunTab(row);
}

void TemplateCommandDialog::onHistoryDeleteClicked()
{
    int row = ui->historyListWidget->currentRow();
    if (row < 0 || row >= d->historyRecords.size()) return;

    const auto& rec = d->historyRecords[row];
    TemplateCommandHistoryStorage storage;
    storage.del(rec.id);
    loadHistory();
}

} // namespace ady
