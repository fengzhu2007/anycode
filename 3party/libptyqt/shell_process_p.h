#ifndef SHELL_PROCESS_P_H
#define SHELL_PROCESS_P_H


#include "shell_process.h"
#include <QStringList>
#include <QHash>
#include <QMap>
#include <QSharedData>
#include <QIODevice>
#include <QProcess>
#include <private/qiodevice_p.h>
#include <QMutexLocker>
#include <QMutex>
#include <QDeadlineTimer>


#ifdef Q_OS_UNIX
#include <QtCore/private/qorderedmutexlocker_p.h>
#endif

typedef int Q_PIPE;
#define INVALID_Q_PIPE -1


QT_BEGIN_NAMESPACE

class QSocketNotifier;
class QWindowsPipeReader;
class QWindowsPipeWriter;
class QWinEventNotifier;
class QTimer;


using QProcEnvKey = QByteArray;

class QProcEnvValue
{
public:
    QProcEnvValue() = default;
    explicit QProcEnvValue(const QString &value) : stringValue(value) {}
    explicit QProcEnvValue(const QByteArray &value) : byteValue(value) {}
    bool operator==(const QProcEnvValue &other) const
    {
        return byteValue.isEmpty() && other.byteValue.isEmpty()
                ? stringValue == other.stringValue
                : bytes() == other.bytes();
    }
    QByteArray bytes() const
    {
        if (byteValue.isEmpty() && !stringValue.isEmpty())
            byteValue = stringValue.toLocal8Bit();
        return byteValue;
    }
    QString string() const
    {
        if (stringValue.isEmpty() && !byteValue.isEmpty())
            stringValue = QString::fromLocal8Bit(byteValue);
        return stringValue;
    }

    mutable QByteArray byteValue;
    mutable QString stringValue;
};
Q_DECLARE_TYPEINFO(QProcEnvValue, Q_MOVABLE_TYPE);


class ShellProcessEnvironmentPrivate: public QSharedData
{
public:
    typedef QProcEnvKey Key;
    typedef QProcEnvValue Value;
#ifdef Q_OS_WIN
    inline Key prepareName(const QString &name) const { return Key(name); }
    inline QString nameToString(const Key &name) const { return name; }
    inline Value prepareValue(const QString &value) const { return value; }
    inline QString valueToString(const Value &value) const { return value; }
#else
    /*struct NameMapMutexLocker : public QMutexLocker<QMutex>
    {
        NameMapMutexLocker(const ShellProcessEnvironmentPrivate *d) : QMutexLocker(&d->nameMapMutex) {}
    };*/

    struct OrderedNameMapMutexLocker : public QOrderedMutexLocker
    {
        OrderedNameMapMutexLocker(const ShellProcessEnvironmentPrivate *d1,
                                  const ShellProcessEnvironmentPrivate *d2)
            : QOrderedMutexLocker(&d1->nameMapMutex, &d2->nameMapMutex)
        {}
    };

    inline Key prepareName(const QString &name) const
    {
        //const NameMapMutexLocker locker(this);
        Key &ent = nameMap[name];
        if (ent.isEmpty())
            ent = name.toLocal8Bit();
        return ent;
    }
    inline QString nameToString(const Key &name) const
    {
        const QString sname = QString::fromLocal8Bit(name);
        {
            //const NameMapMutexLocker locker(this);
            nameMap[sname] = name;
        }
        return sname;
    }
    inline Value prepareValue(const QString &value) const { return Value(value); }
    inline QString valueToString(const Value &value) const { return value.string(); }

    ShellProcessEnvironmentPrivate() : QSharedData() {}
    ShellProcessEnvironmentPrivate(const ShellProcessEnvironmentPrivate &other) :
        QSharedData(), vars(other.vars)
    {
        // We don't need to lock our own mutex, as this object is new and
        // consequently not shared. For the same reason, non-const methods
        // do not need a lock, as they detach objects (however, we need to
        // ensure that they really detach before using prepareName()).
        //NameMapMutexLocker locker(&other);
        nameMap = other.nameMap;
        // We need to detach our nameMap, so that our mutex can protect it.
        // As we are being detached, it likely would be detached a moment later anyway.
        nameMap.detach();
    }
#endif

    using Map = QMap<Key, Value>;
    Map vars;

#ifdef Q_OS_UNIX
    typedef QHash<QString, Key> NameHash;
    mutable NameHash nameMap;
    mutable QMutex nameMapMutex;
#endif

    static ShellProcessEnvironment fromList(const QStringList &list);
    QStringList toList() const;
    QStringList keys() const;
    void insert(const ShellProcessEnvironmentPrivate &other);
};

template<> Q_INLINE_TEMPLATE void QSharedDataPointer<ShellProcessEnvironmentPrivate>::detach()
{
    if (d && d->ref.loadRelaxed() == 1)
        return;
    ShellProcessEnvironmentPrivate *x = (d ? new ShellProcessEnvironmentPrivate(*d)
                                     : new ShellProcessEnvironmentPrivate);
    x->ref.ref();
    if (d && !d->ref.deref())
        delete d;
    d = x;
}

class ShellProcessPrivate : public QIODevicePrivate
{
public:
    Q_DECLARE_PUBLIC(ShellProcess)

    struct Channel {
        enum ProcessChannelType : char {
            Normal = 0,
            PipeSource = 1,
            PipeSink = 2,
            Redirect = 3
        };

        void clear();

        Channel &operator=(const QString &fileName)
        {
            clear();
            file = fileName;
            type = fileName.isEmpty() ? Normal : Redirect;
            return *this;
        }

        void pipeTo(ShellProcessPrivate *other)
        {
            clear();
            process = other;
            type = PipeSource;
        }

        void pipeFrom(ShellProcessPrivate *other)
        {
            clear();
            process = other;
            type = PipeSink;
        }

        QString file;
        ShellProcessPrivate *process = nullptr;
#ifdef Q_OS_UNIX
        QSocketNotifier *notifier = nullptr;
#else
        union {
            QWindowsPipeReader *reader = nullptr;
            QWindowsPipeWriter *writer;
        };
#endif
        Q_PIPE pipe[2] = {INVALID_Q_PIPE, INVALID_Q_PIPE};

        ProcessChannelType type = Normal;
        bool closed = false;
        bool append = false;
    };

    ShellProcessPrivate();
    virtual ~ShellProcessPrivate();

    // private slots
    bool _q_canReadStandardOutput();
    bool _q_canReadStandardError();
#ifdef Q_OS_WIN
    qint64 pipeWriterBytesToWrite() const;
    void _q_bytesWritten(qint64 bytes);
    void _q_writeFailed();
#else
    bool _q_canWrite();
    bool writeToStdin();
#endif
    bool _q_startupNotification();
    void _q_processDied();

    Channel stdinChannel;
    Channel stdoutChannel;
    Channel stderrChannel;
    bool openChannels();
    bool openChannelsForDetached();
    bool openChannel(Channel &channel);
    void closeChannel(Channel *channel);
    void closeWriteChannel();
    void closeChannels();
    bool tryReadFromChannel(Channel *channel); // obviously, only stdout and stderr

    QString program;
    QStringList arguments;
    QString workingDirectory;
    ShellProcessEnvironment environment = ShellProcessEnvironment::InheritFromParent;

#if defined(Q_OS_WIN)
    QString nativeArguments;
    QProcess::CreateProcessArgumentModifier modifyCreateProcessArgs;
    QWinEventNotifier *processFinishedNotifier = nullptr;
    Q_PROCESS_INFORMATION *pid = nullptr;
#else
    struct UnixExtras {
        std::function<void(void)> childProcessModifier;
        ShellProcess::UnixProcessParameters processParameters;
    };
    std::unique_ptr<UnixExtras> unixExtras;
    QSocketNotifier *stateNotifier = nullptr;
    Q_PIPE childStartedPipe[2] = {INVALID_Q_PIPE, INVALID_Q_PIPE};
    pid_t pid = 0;
    int forkfd = -1;
#endif

    int exitCode = 0;
    quint8 processState = QProcess::NotRunning;
    quint8 exitStatus = QProcess::NormalExit;
    quint8 processError = QProcess::UnknownError;
    quint8 processChannelMode = QProcess::SeparateChannels;
    quint8 inputChannelMode = QProcess::ManagedInputChannel;
    bool emittedReadyRead = false;
    bool emittedBytesWritten = false;

    void start(QIODevice::OpenMode mode);
    void startProcess();
#if defined(Q_OS_UNIX)
    void commitChannels() const;
#endif
    bool processStarted(QString *errorMessage = nullptr);
    void processFinished();
    void terminateProcess();
    void killProcess();
#ifdef Q_OS_UNIX
    void waitForDeadChild();
#else
    void findExitCode();
#endif
#ifdef Q_OS_WIN
    STARTUPINFOW createStartupInfo();
    bool callCreateProcess(QProcess::CreateProcessArguments *cpargs);
    bool drainOutputPipes();
#endif

    bool startDetached(qint64 *pPid);

    bool waitForStarted(const QDeadlineTimer &deadline);
    bool waitForReadyRead(const QDeadlineTimer &deadline);
    bool waitForBytesWritten(const QDeadlineTimer &deadline);
    bool waitForFinished(const QDeadlineTimer &deadline);

    qint64 bytesAvailableInChannel(const Channel *channel) const;
    qint64 readFromChannel(const Channel *channel, char *data, qint64 maxlen);

    void destroyPipe(Q_PIPE pipe[2]);
    void cleanup();
    void setError(QProcess::ProcessError error, const QString &description = QString());
    void setErrorAndEmit(QProcess::ProcessError error, const QString &description = QString());

    const ShellProcessEnvironmentPrivate *environmentPrivate() const
    { return environment.d.constData(); }
};


QT_END_NAMESPACE


#endif // SHELL_PROCESS_P_H
