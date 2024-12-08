#ifndef SHELL_PROCESS_H
#define SHELL_PROCESS_H

#include <QIODevice>
#include <QStringList>
#include <QSharedData>
#include <QProcess>
#include <functional>

QT_BEGIN_NAMESPACE

class ShellProcessPrivate;
class ShellProcessEnvironmentPrivate;

class  ShellProcessEnvironment
{
public:
    enum Initialization { InheritFromParent };

    ShellProcessEnvironment();
    ShellProcessEnvironment(Initialization) noexcept;
    ShellProcessEnvironment(const ShellProcessEnvironment &other);
    ~ShellProcessEnvironment();

    void swap(ShellProcessEnvironment &other) noexcept { d.swap(other.d); }


    bool isEmpty() const;
    [[nodiscard]] bool inheritsFromParent() const;
    void clear();

    bool contains(const QString &name) const;
    void insert(const QString &name, const QString &value);
    void remove(const QString &name);
    QString value(const QString &name, const QString &defaultValue = QString()) const;

    QStringList toStringList() const;

    QStringList keys() const;

    void insert(const ShellProcessEnvironment &e);

    static ShellProcessEnvironment systemEnvironment();

private:
    friend Q_CORE_EXPORT bool comparesEqual(const ShellProcessEnvironment &lhs,
                                            const ShellProcessEnvironment &rhs);
    friend class ShellProcessPrivate;
    friend class ShellProcessEnvironmentPrivate;
    QSharedDataPointer<ShellProcessEnvironmentPrivate> d;
};



class  ShellProcess : public QIODevice
{
    Q_OBJECT
public:

    explicit ShellProcess(QObject *parent = nullptr);
    virtual ~ShellProcess();

    void start(const QString &program, const QStringList &arguments = {}, OpenMode mode = ReadWrite);
    void start(OpenMode mode = ReadWrite);
    void startCommand(const QString &command, OpenMode mode = ReadWrite);
    bool startDetached(qint64 *pid = nullptr);
    bool open(OpenMode mode = ReadWrite) override;

    QString program() const;
    void setProgram(const QString &program);

    QStringList arguments() const;
    void setArguments(const QStringList & arguments);

    QProcess::ProcessChannelMode processChannelMode() const;
    void setProcessChannelMode(QProcess::ProcessChannelMode mode);
    QProcess::InputChannelMode inputChannelMode() const;
    void setInputChannelMode(QProcess::InputChannelMode mode);

    QProcess::ProcessChannel readChannel() const;
    void setReadChannel(QProcess::ProcessChannel channel);

    void closeReadChannel(QProcess::ProcessChannel channel);
    void closeWriteChannel();

    void setStandardInputFile(const QString &fileName);
    void setStandardOutputFile(const QString &fileName, OpenMode mode = Truncate);
    void setStandardErrorFile(const QString &fileName, OpenMode mode = Truncate);
    void setStandardOutputProcess(ShellProcess *destination);


#if defined(Q_OS_UNIX) || defined(Q_QDOC)
    std::function<void(void)> childProcessModifier() const;
    void setChildProcessModifier(const std::function<void(void)> &modifier);
    Q_NORETURN void failChildProcessModifier(const char *description, int error = 0) noexcept;

    enum class UnixProcessFlag : quint32 {
        ResetSignalHandlers                 = 0x0001, // like POSIX_SPAWN_SETSIGDEF
        IgnoreSigPipe                       = 0x0002,
        // some room if we want to add IgnoreSigHup or so
        CloseFileDescriptors                = 0x0010,
        UseVFork                            = 0x0020, // like POSIX_SPAWN_USEVFORK
        CreateNewSession                    = 0x0040, // like POSIX_SPAWN_SETSID
        DisconnectControllingTerminal       = 0x0080,
        ResetIds                            = 0x0100, // like POSIX_SPAWN_RESETIDS
    };
    Q_DECLARE_FLAGS(UnixProcessFlags, UnixProcessFlag)
    struct UnixProcessParameters
    {
        UnixProcessFlags flags = {};
        int lowestFileDescriptorToClose = 0;

        quint32 _reserved[6] {};
    };
    UnixProcessParameters unixProcessParameters() const noexcept;
    void setUnixProcessParameters(const UnixProcessParameters &params);
    void setUnixProcessParameters(UnixProcessFlags flagsOnly);
#endif

    QString workingDirectory() const;
    void setWorkingDirectory(const QString &dir);

    void setEnvironment(const QStringList &environment);
    QStringList environment() const;
    void setProcessEnvironment(const ShellProcessEnvironment &environment);
    ShellProcessEnvironment processEnvironment() const;

    QProcess::ProcessError error() const;
    QProcess::ProcessState state() const;

    qint64 processId() const;

    bool waitForStarted(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000) override;
    bool waitForBytesWritten(int msecs = 30000) override;
    bool waitForFinished(int msecs = 30000);

    QByteArray readAllStandardOutput();
    QByteArray readAllStandardError();

    int exitCode() const;
    QProcess::ExitStatus exitStatus() const;

    // QIODevice
    qint64 bytesToWrite() const override;
    bool isSequential() const override;
    void close() override;

    static int execute(const QString &program, const QStringList &arguments = {});
    static bool startDetached(const QString &program, const QStringList &arguments = {},
                              const QString &workingDirectory = QString(), qint64 *pid = nullptr);

    static QStringList systemEnvironment();

    static QString nullDevice();

    static QStringList splitCommand(QStringView command);

public slots:
    void terminate();
    void kill();

signals:
    void started(QPrivateSignal);
    void finished(int exitCode, QProcess::ExitStatus exitStatus = QProcess::NormalExit);
    void errorOccurred(QProcess::ProcessError error);
    void stateChanged(QProcess::ProcessState state, QPrivateSignal);

    void readyReadStandardOutput(QPrivateSignal);
    void readyReadStandardError(QPrivateSignal);

protected:
    void setProcessState(QProcess::ProcessState state);

    // QIODevice
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    Q_DECLARE_PRIVATE(ShellProcess)
    Q_DISABLE_COPY(ShellProcess)

#if QT_VERSION < QT_VERSION_CHECK(7,0,0)
    // ### Qt7: Remove this struct and the virtual function; they're here only
    // to cause build errors in Qt 5 code that wasn't updated to Qt 6's
    // setChildProcessModifier()
    struct Use_setChildProcessModifier_Instead {};
    QT_DEPRECATED_X("Use setChildProcessModifier() instead")
    virtual Use_setChildProcessModifier_Instead setupChildProcess();
#endif

    Q_PRIVATE_SLOT(d_func(), bool _q_canReadStandardOutput())
    Q_PRIVATE_SLOT(d_func(), bool _q_canReadStandardError())
#ifdef Q_OS_UNIX
    Q_PRIVATE_SLOT(d_func(), bool _q_canWrite())
#endif
    Q_PRIVATE_SLOT(d_func(), bool _q_startupNotification())
    Q_PRIVATE_SLOT(d_func(), void _q_processDied())
};

#ifdef Q_OS_UNIX
Q_DECLARE_OPERATORS_FOR_FLAGS(ShellProcess::UnixProcessFlags)
#endif


QT_END_NAMESPACE


#endif // SHELL_PROCESS_H
