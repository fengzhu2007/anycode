#include "shell_process.h"
#include "shell_process_p.h"


#include <qdebug.h>
#include <QDebug>
#include <QDir>
#include <QScopedValueRollback>
#include <QByteArray>
#include <QDeadlineTimer>
#include <QCoreApplication>
#include <QTimer>
#include <QProcess>



#if __has_include(<paths.h>)
#include <paths.h>
#endif

QT_BEGIN_NAMESPACE



QStringList ShellProcessEnvironmentPrivate::toList() const
{
    QStringList result;
    result.reserve(vars.size());
    for (auto it = vars.cbegin(), end = vars.cend(); it != end; ++it)
        result << nameToString(it.key()) + u'=' + valueToString(it.value());
    return result;
}

ShellProcessEnvironment ShellProcessEnvironmentPrivate::fromList(const QStringList &list)
{
    ShellProcessEnvironment env;
    QStringList::ConstIterator it = list.constBegin(),
                              end = list.constEnd();
    for ( ; it != end; ++it) {
        const qsizetype pos = it->indexOf(u'=', 1);
        if (pos < 1)
            continue;

        QString value = it->mid(pos + 1);
        QString name = *it;
        name.truncate(pos);
        env.insert(name, value);
    }
    return env;
}

QStringList ShellProcessEnvironmentPrivate::keys() const
{
    QStringList result;
    result.reserve(vars.size());
    auto it = vars.constBegin();
    const auto end = vars.constEnd();
    for ( ; it != end; ++it)
        result << nameToString(it.key());
    return result;
}

void ShellProcessEnvironmentPrivate::insert(const ShellProcessEnvironmentPrivate &other)
{
    auto it = other.vars.constBegin();
    const auto end = other.vars.constEnd();
    for ( ; it != end; ++it)
        vars.insert(it.key(), it.value());

#ifdef Q_OS_UNIX
    const OrderedNameMapMutexLocker locker(this, &other);
    auto nit = other.nameMap.constBegin();
    const auto nend = other.nameMap.constEnd();
    for ( ; nit != nend; ++nit)
        nameMap.insert(nit.key(), nit.value());
#endif
}

ShellProcessEnvironment::ShellProcessEnvironment() : d(new ShellProcessEnvironmentPrivate) { }


ShellProcessEnvironment::ShellProcessEnvironment(ShellProcessEnvironment::Initialization) noexcept { }

/*!
    Frees the resources associated with this QProcessEnvironment object.
*/
ShellProcessEnvironment::~ShellProcessEnvironment()
{
}

/*!
    Creates a QProcessEnvironment object that is a copy of \a other.
*/
ShellProcessEnvironment::ShellProcessEnvironment(const ShellProcessEnvironment &other)
    : d(other.d)
{
}

/*!
    Copies the contents of the \a other QProcessEnvironment object into this
    one.
*/


bool comparesEqual(const ShellProcessEnvironment &lhs, const ShellProcessEnvironment &rhs)
{
    if (lhs.d == rhs.d)
        return true;

    return lhs.d && rhs.d && lhs.d->vars == rhs.d->vars;
}


bool ShellProcessEnvironment::isEmpty() const
{
    // Needs no locking, as no hash nodes are accessed
    return d ? d->vars.isEmpty() : true;
}


bool ShellProcessEnvironment::inheritsFromParent() const
{
    return !d;
}


void ShellProcessEnvironment::clear()
{
    if (d.constData())
        d->vars.clear();
    // Unix: Don't clear d->nameMap, as the environment is likely to be
    // re-populated with the same keys again.
}

bool ShellProcessEnvironment::contains(const QString &name) const
{
    if (!d)
        return false;
    return d->vars.contains(d->prepareName(name));
}


void ShellProcessEnvironment::insert(const QString &name, const QString &value)
{
    // our re-impl of detach() detaches from null
    d.detach(); // detach before prepareName()
    d->vars.insert(d->prepareName(name), d->prepareValue(value));
}

void ShellProcessEnvironment::remove(const QString &name)
{
    if (d.constData()) {
        ShellProcessEnvironmentPrivate *p = d.data();
        p->vars.remove(p->prepareName(name));
    }
}

QString ShellProcessEnvironment::value(const QString &name, const QString &defaultValue) const
{
    if (!d)
        return defaultValue;

    const auto it = d->vars.constFind(d->prepareName(name));
    if (it == d->vars.constEnd())
        return defaultValue;

    return d->valueToString(it.value());
}

QStringList ShellProcessEnvironment::toStringList() const
{
    if (!d)
        return QStringList();
    return d->toList();
}


QStringList ShellProcessEnvironment::keys() const
{
    if (!d)
        return QStringList();
    return d->keys();
}


void ShellProcessEnvironment::insert(const ShellProcessEnvironment &e)
{
    if (!e.d)
        return;

    // our re-impl of detach() detaches from null
    d->insert(*e.d);
}


void ShellProcessPrivate::Channel::clear()
{
    switch (type) {
    case PipeSource:
        Q_ASSERT(process);
        process->stdinChannel.type = Normal;
        process->stdinChannel.process = nullptr;
        break;
    case PipeSink:
        Q_ASSERT(process);
        process->stdoutChannel.type = Normal;
        process->stdoutChannel.process = nullptr;
        break;
    default:
        break;
    }

    type = Normal;
    file.clear();
    process = nullptr;
}

ShellProcessPrivate::ShellProcessPrivate()
{
    readBufferChunkSize = QRINGBUFFER_CHUNKSIZE;
#ifndef Q_OS_WIN
    writeBufferChunkSize = QRINGBUFFER_CHUNKSIZE;
#endif
}

/*!
    \internal
*/
ShellProcessPrivate::~ShellProcessPrivate()
{
    if (stdinChannel.process)
        stdinChannel.process->stdoutChannel.clear();
    if (stdoutChannel.process)
        stdoutChannel.process->stdinChannel.clear();
}

/*!
    \internal
*/
void ShellProcessPrivate::setError(QProcess::ProcessError error, const QString &description)
{
    processError = error;
    if (description.isEmpty()) {
        switch (error) {
        case QProcess::FailedToStart:
            errorString = QProcess::tr("Process failed to start");
            break;
        case QProcess::Crashed:
            errorString = QProcess::tr("Process crashed");
            break;
        case QProcess::Timedout:
            errorString = QProcess::tr("Process operation timed out");
            break;
        case QProcess::ReadError:
            errorString = QProcess::tr("Error reading from process");
            break;
        case QProcess::WriteError:
            errorString = QProcess::tr("Error writing to process");
            break;
        case QProcess::UnknownError:
            errorString.clear();
            break;
        }
    } else {
        errorString = description;
    }
}

/*!
    \internal
*/
void ShellProcessPrivate::setErrorAndEmit(QProcess::ProcessError error, const QString &description)
{
    Q_Q(ShellProcess);
    Q_ASSERT(error != QProcess::UnknownError);
    setError(error, description);
    emit q->errorOccurred(QProcess::ProcessError(processError));
}

/*!
    \internal
*/
bool ShellProcessPrivate::openChannels()
{
    // stdin channel.
    if (inputChannelMode == QProcess::ForwardedInputChannel) {
        if (stdinChannel.type != Channel::Normal)
            qWarning("QProcess::openChannels: Inconsistent stdin channel configuration");
    } else if (!openChannel(stdinChannel)) {
        return false;
    }

    // stdout channel.
    if (processChannelMode == QProcess::ForwardedChannels
            || processChannelMode == QProcess::ForwardedOutputChannel) {
        if (stdoutChannel.type != Channel::Normal)
            qWarning("QProcess::openChannels: Inconsistent stdout channel configuration");
    } else if (!openChannel(stdoutChannel)) {
        return false;
    }

    // stderr channel.
    if (processChannelMode == QProcess::ForwardedChannels
            || processChannelMode == QProcess::ForwardedErrorChannel
            || processChannelMode == QProcess::MergedChannels) {
        if (stderrChannel.type != Channel::Normal)
            qWarning("QProcess::openChannels: Inconsistent stderr channel configuration");
    } else if (!openChannel(stderrChannel)) {
        return false;
    }

    return true;
}

/*!
    \internal
*/
void ShellProcessPrivate::closeChannels()
{
    closeChannel(&stdoutChannel);
    closeChannel(&stderrChannel);
    closeChannel(&stdinChannel);
}

/*!
    \internal
*/
bool ShellProcessPrivate::openChannelsForDetached()
{
    // stdin channel.
    bool needToOpen = (stdinChannel.type == Channel::Redirect
                       || stdinChannel.type == Channel::PipeSink);
    if (stdinChannel.type != Channel::Normal
            && (!needToOpen
                || inputChannelMode == QProcess::ForwardedInputChannel)) {
        qWarning("QProcess::openChannelsForDetached: Inconsistent stdin channel configuration");
    }
    if (needToOpen && !openChannel(stdinChannel))
        return false;

    // stdout channel.
    needToOpen = (stdoutChannel.type == Channel::Redirect
                  || stdoutChannel.type == Channel::PipeSource);
    if (stdoutChannel.type != Channel::Normal
            && (!needToOpen
                || processChannelMode == QProcess::ForwardedChannels
                || processChannelMode == QProcess::ForwardedOutputChannel)) {
        qWarning("QProcess::openChannelsForDetached: Inconsistent stdout channel configuration");
    }
    if (needToOpen && !openChannel(stdoutChannel))
        return false;

    // stderr channel.
    needToOpen = (stderrChannel.type == Channel::Redirect);
    if (stderrChannel.type != Channel::Normal
            && (!needToOpen
                || processChannelMode == QProcess::ForwardedChannels
                || processChannelMode == QProcess::ForwardedErrorChannel
                || processChannelMode == QProcess::MergedChannels)) {
        qWarning("QProcess::openChannelsForDetached: Inconsistent stderr channel configuration");
    }
    if (needToOpen && !openChannel(stderrChannel))
        return false;

    return true;
}

/*!
    \internal
    Returns \c true if we emitted readyRead().
*/
bool ShellProcessPrivate::tryReadFromChannel(Channel *channel)
{
    Q_Q(ShellProcess);
    if (channel->pipe[0] == INVALID_Q_PIPE)
        return false;

    qint64 available = bytesAvailableInChannel(channel);
    if (available == 0)
        available = 1;      // always try to read at least one byte

    QProcess::ProcessChannel channelIdx = (channel == &stdoutChannel
                                           ? QProcess::StandardOutput
                                           : QProcess::StandardError);
    Q_ASSERT(readBuffers.size() > int(channelIdx));
    QRingBuffer &readBuffer = readBuffers[int(channelIdx)];
    char *ptr = readBuffer.reserve(available);
    qint64 readBytes = readFromChannel(channel, ptr, available);
    if (readBytes <= 0)
        readBuffer.chop(available);
    if (readBytes == -2) {
        // EWOULDBLOCK
        return false;
    }
    if (readBytes == -1) {
        setErrorAndEmit(QProcess::ReadError);
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::tryReadFromChannel(%d), failed to read from the process",
               int(channel - &stdinChannel));
#endif
        return false;
    }
    if (readBytes == 0) {
        // EOF
        closeChannel(channel);
#if defined QPROCESS_DEBUG
        qDebug("QProcessPrivate::tryReadFromChannel(%d), 0 bytes available",
               int(channel - &stdinChannel));
#endif
        return false;
    }
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::tryReadFromChannel(%d), read %lld bytes from the process' output",
           int(channel - &stdinChannel), readBytes);
#endif

    if (channel->closed) {
        readBuffer.chop(readBytes);
        return false;
    }

    readBuffer.chop(available - readBytes);

    bool didRead = false;
    if (currentReadChannel == channelIdx) {
        didRead = true;
        if (!emittedReadyRead) {
            QScopedValueRollback<bool> guard(emittedReadyRead, true);
            emit q->readyRead();
        }
    }
    emit q->channelReadyRead(int(channelIdx));
    if (channelIdx == QProcess::StandardOutput)
        emit q->readyReadStandardOutput(ShellProcess::QPrivateSignal());
    else
        emit q->readyReadStandardError(ShellProcess::QPrivateSignal());
    return didRead;
}

/*!
    \internal
*/
bool ShellProcessPrivate::_q_canReadStandardOutput()
{
    return tryReadFromChannel(&stdoutChannel);
}

/*!
    \internal
*/
bool ShellProcessPrivate::_q_canReadStandardError()
{
    return tryReadFromChannel(&stderrChannel);
}

/*!
    \internal
*/
void ShellProcessPrivate::_q_processDied()
{
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::_q_processDied()");
#endif

    // in case there is data in the pipeline and this slot by chance
    // got called before the read notifications, call these functions
    // so the data is made available before we announce death.
#ifdef Q_OS_WIN
    drainOutputPipes();
#else
    _q_canReadStandardOutput();
    _q_canReadStandardError();
#endif

    // Slots connected to signals emitted by the functions called above
    // might call waitFor*(), which would synchronously reap the process.
    // So check the state to avoid trying to reap a second time.
    if (processState != QProcess::NotRunning)
        processFinished();
}

/*!
    \internal
*/
void ShellProcessPrivate::processFinished()
{
    Q_Q(ShellProcess);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::processFinished()");
#endif

#ifdef Q_OS_UNIX
    waitForDeadChild();
#else
    findExitCode();
#endif

    cleanup();

    if (exitStatus == QProcess::CrashExit)
        setErrorAndEmit(QProcess::Crashed);

    // we received EOF now:
    emit q->readChannelFinished();
    // in the future:
    //emit q->standardOutputClosed();
    //emit q->standardErrorClosed();

    emit q->finished(exitCode, QProcess::ExitStatus(exitStatus));

#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::processFinished(): process is dead");
#endif
}

/*!
    \internal
*/
bool ShellProcessPrivate::_q_startupNotification()
{
    Q_Q(ShellProcess);
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::startupNotification()");
#endif

    QString errorMessage;
    if (processStarted(&errorMessage)) {
        q->setProcessState(QProcess::Running);
        emit q->started(ShellProcess::QPrivateSignal());
        return true;
    }

    q->setProcessState(QProcess::NotRunning);
    setErrorAndEmit(QProcess::FailedToStart, errorMessage);
#ifdef Q_OS_UNIX
    waitForDeadChild();
#endif
    cleanup();
    return false;
}

/*!
    \internal
*/
void ShellProcessPrivate::closeWriteChannel()
{
#if defined QPROCESS_DEBUG
    qDebug("QProcessPrivate::closeWriteChannel()");
#endif

    closeChannel(&stdinChannel);
}

/*!
    Constructs a QProcess object with the given \a parent.
*/
ShellProcess::ShellProcess(QObject *parent)
    : QIODevice(*new ShellProcessPrivate, parent)
{
#if defined QPROCESS_DEBUG
    qDebug("QProcess::QProcess(%p)", parent);
#endif
}

/*!
    Destructs the QProcess object, i.e., killing the process.

    Note that this function will not return until the process is
    terminated.
*/
ShellProcess::~ShellProcess()
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning().nospace()
            << "QProcess: Destroyed while process (" << QDir::toNativeSeparators(program()) << ") is still running.";
        kill();
        waitForFinished();
    }
    d->cleanup();
}


QProcess::ProcessChannelMode ShellProcess::processChannelMode() const
{
    Q_D(const ShellProcess);
    return QProcess::ProcessChannelMode(d->processChannelMode);
}


void ShellProcess::setProcessChannelMode(QProcess::ProcessChannelMode mode)
{
    Q_D(ShellProcess);
    d->processChannelMode = mode;
}

/*!
    \since 5.2

    Returns the channel mode of the QProcess standard input channel.

    \sa setInputChannelMode(), InputChannelMode
*/
QProcess::InputChannelMode ShellProcess::inputChannelMode() const
{
    Q_D(const ShellProcess);
    return QProcess::InputChannelMode(d->inputChannelMode);
}


void ShellProcess::setInputChannelMode(QProcess::InputChannelMode mode)
{
    Q_D(ShellProcess);
    d->inputChannelMode = mode;
}

/*!
    Returns the current read channel of the QProcess.

    \sa setReadChannel()
*/
QProcess::ProcessChannel ShellProcess::readChannel() const
{
    Q_D(const ShellProcess);
    return QProcess::ProcessChannel(d->currentReadChannel);
}

/*!
    Sets the current read channel of the QProcess to the given \a
    channel. The current input channel is used by the functions
    read(), readAll(), readLine(), and getChar(). It also determines
    which channel triggers QProcess to emit readyRead().

    \sa readChannel()
*/
void ShellProcess::setReadChannel(QProcess::ProcessChannel channel)
{
    QIODevice::setCurrentReadChannel(int(channel));
}

/*!
    Closes the read channel \a channel. After calling this function,
    QProcess will no longer receive data on the channel. Any data that
    has already been received is still available for reading.

    Call this function to save memory, if you are not interested in
    the output of the process.

    \sa closeWriteChannel(), setReadChannel()
*/
void ShellProcess::closeReadChannel(QProcess::ProcessChannel channel)
{
    Q_D(ShellProcess);

    if (channel == QProcess::StandardOutput)
        d->stdoutChannel.closed = true;
    else
        d->stderrChannel.closed = true;
}

/*!
    Schedules the write channel of QProcess to be closed. The channel
    will close once all data has been written to the process. After
    calling this function, any attempts to write to the process will
    fail.

    Closing the write channel is necessary for programs that read
    input data until the channel has been closed. For example, the
    program "more" is used to display text data in a console on both
    Unix and Windows. But it will not display the text data until
    QProcess's write channel has been closed. Example:

    \snippet code/src_corelib_io_qprocess.cpp 1

    The write channel is implicitly opened when start() is called.

    \sa closeReadChannel()
*/
void ShellProcess::closeWriteChannel()
{
    Q_D(ShellProcess);
    d->stdinChannel.closed = true; // closing
    if (bytesToWrite() == 0)
        d->closeWriteChannel();
}

void ShellProcess::setStandardInputFile(const QString &fileName)
{
    Q_D(ShellProcess);
    d->stdinChannel = fileName;
}

void ShellProcess::setStandardOutputFile(const QString &fileName, OpenMode mode)
{
    Q_ASSERT(mode == QProcess::Append || mode == QProcess::Truncate);
    Q_D(ShellProcess);

    d->stdoutChannel = fileName;
    d->stdoutChannel.append = mode == Append;
}


void ShellProcess::setStandardErrorFile(const QString &fileName, QProcess::OpenMode mode)
{
    Q_ASSERT(mode == QProcess::Append || mode == QProcess::Truncate);
    Q_D(ShellProcess);

    d->stderrChannel = fileName;
    d->stderrChannel.append = mode == Append;
}

void ShellProcess::setStandardOutputProcess(ShellProcess *destination)
{
    ShellProcessPrivate *dfrom = d_func();
    ShellProcessPrivate *dto = destination->d_func();
    dfrom->stdoutChannel.pipeTo(dto);
    dto->stdinChannel.pipeFrom(dfrom);
}



#if defined(Q_OS_UNIX) || defined(Q_QDOC)

std::function<void(void)> ShellProcess::childProcessModifier() const
{
    Q_D(const ShellProcess);
    return d->unixExtras ? d->unixExtras->childProcessModifier : std::function<void(void)>();
}

void ShellProcess::setChildProcessModifier(const std::function<void(void)> &modifier)
{
    Q_D(ShellProcess);
    if (!d->unixExtras)
        d->unixExtras.reset(new ShellProcessPrivate::UnixExtras);
    d->unixExtras->childProcessModifier = modifier;
}

auto ShellProcess::unixProcessParameters() const noexcept -> UnixProcessParameters
{
    Q_D(const ShellProcess);
    return d->unixExtras ? d->unixExtras->processParameters : UnixProcessParameters{};
}


void ShellProcess::setUnixProcessParameters(const UnixProcessParameters &params)
{
    Q_D(ShellProcess);
    if (!d->unixExtras)
        d->unixExtras.reset(new ShellProcessPrivate::UnixExtras);
    d->unixExtras->processParameters = params;
}

void ShellProcess::setUnixProcessParameters(UnixProcessFlags flagsOnly)
{
    Q_D(ShellProcess);
    if (!d->unixExtras)
        d->unixExtras.reset(new ShellProcessPrivate::UnixExtras);
    d->unixExtras->processParameters = { flagsOnly };
}
#endif

QString ShellProcess::workingDirectory() const
{
    Q_D(const ShellProcess);
    return d->workingDirectory;
}

/*!
    Sets the working directory to \a dir. QProcess will start the
    process in this directory. The default behavior is to start the
    process in the working directory of the calling process.

    \sa workingDirectory(), start()
*/
void ShellProcess::setWorkingDirectory(const QString &dir)
{
    Q_D(ShellProcess);
    d->workingDirectory = dir;
}

/*!
    \since 5.3

    Returns the native process identifier for the running process, if
    available. If no process is currently running, \c 0 is returned.
 */
qint64 ShellProcess::processId() const
{
    Q_D(const ShellProcess);
#ifdef Q_OS_WIN
    return d->pid ? d->pid->dwProcessId : 0;
#else
    return d->pid;
#endif
}

/*!
    Closes all communication with the process and kills it. After calling this
    function, QProcess will no longer emit readyRead(), and data can no
    longer be read or written.
*/
void ShellProcess::close()
{
    Q_D(ShellProcess);
    emit aboutToClose();
    while (waitForBytesWritten(-1))
        ;
    kill();
    waitForFinished(-1);
    d->setWriteChannelCount(0);
    QIODevice::close();
}

/*! \reimp
*/
bool ShellProcess::isSequential() const
{
    return true;
}

/*! \reimp
*/
qint64 ShellProcess::bytesToWrite() const
{
#ifdef Q_OS_WIN
    return d_func()->pipeWriterBytesToWrite();
#else
    return QIODevice::bytesToWrite();
#endif
}

QProcess::ProcessError ShellProcess::error() const
{
    Q_D(const ShellProcess);
    return QProcess::ProcessError(d->processError);
}

/*!
    Returns the current state of the process.

    \sa stateChanged(), error()
*/
QProcess::ProcessState ShellProcess::state() const
{
    Q_D(const ShellProcess);
    return QProcess::ProcessState(d->processState);
}


void ShellProcess::setEnvironment(const QStringList &environment)
{
    setProcessEnvironment(ShellProcessEnvironmentPrivate::fromList(environment));
}


QStringList ShellProcess::environment() const
{
    Q_D(const ShellProcess);
    return d->environment.toStringList();
}


void ShellProcess::setProcessEnvironment(const ShellProcessEnvironment &environment)
{
    Q_D(ShellProcess);
    d->environment = environment;
}


ShellProcessEnvironment ShellProcess::processEnvironment() const
{
    Q_D(const ShellProcess);
    return d->environment;
}


bool ShellProcess::waitForStarted(int msecs)
{
    Q_D(ShellProcess);
    if (d->processState == QProcess::Starting)
        return d->waitForStarted(QDeadlineTimer(msecs));

    return d->processState == QProcess::Running;
}

/*! \reimp
*/
bool ShellProcess::waitForReadyRead(int msecs)
{
    Q_D(ShellProcess);

    if (d->processState == QProcess::NotRunning)
        return false;
    if (d->currentReadChannel == QProcess::StandardOutput && d->stdoutChannel.closed)
        return false;
    if (d->currentReadChannel == QProcess::StandardError && d->stderrChannel.closed)
        return false;

    QDeadlineTimer deadline(msecs);
    if (d->processState == QProcess::Starting) {
        bool started = d->waitForStarted(deadline);
        if (!started)
            return false;
    }

    return d->waitForReadyRead(deadline);
}

/*! \reimp
*/
bool ShellProcess::waitForBytesWritten(int msecs)
{
    Q_D(ShellProcess);
    if (d->processState == QProcess::NotRunning)
        return false;

    QDeadlineTimer deadline(msecs);
    if (d->processState == QProcess::Starting) {
        bool started = d->waitForStarted(deadline);
        if (!started)
            return false;
    }

    return d->waitForBytesWritten(deadline);
}

bool ShellProcess::waitForFinished(int msecs)
{
    Q_D(ShellProcess);
    if (d->processState == QProcess::NotRunning)
        return false;

    QDeadlineTimer deadline(msecs);
    if (d->processState == QProcess::Starting) {
        bool started = d->waitForStarted(deadline);
        if (!started)
            return false;
    }

    return d->waitForFinished(deadline);
}

/*!
    Sets the current state of the QProcess to the \a state specified.

    \sa state()
*/
void ShellProcess::setProcessState(QProcess::ProcessState state)
{
    Q_D(ShellProcess);
    if (d->processState == state)
        return;
    d->processState = state;
    emit stateChanged(state, QPrivateSignal());
}

#if QT_VERSION < QT_VERSION_CHECK(7,0,0)
/*!
    \internal
*/
auto ShellProcess::setupChildProcess() -> Use_setChildProcessModifier_Instead
{
    //Q_UNREACHABLE_RETURN({});
    return {};
}
#endif

/*! \reimp
*/
qint64 ShellProcess::readData(char *data, qint64 maxlen)
{
    Q_D(ShellProcess);
    Q_UNUSED(data);
    if (!maxlen)
        return 0;
    if (d->processState == QProcess::NotRunning)
        return -1;              // EOF
    return 0;
}

QByteArray ShellProcess::readAllStandardOutput()
{
    QProcess::ProcessChannel tmp = readChannel();
    setReadChannel(QProcess::StandardOutput);
    QByteArray data = readAll();
    setReadChannel(tmp);
    return data;
}


QByteArray ShellProcess::readAllStandardError()
{
    Q_D(ShellProcess);
    QByteArray data;
    if (d->processChannelMode == QProcess::MergedChannels) {
        qWarning("QProcess::readAllStandardError: Called with MergedChannels");
    } else {
        QProcess::ProcessChannel tmp = readChannel();
        setReadChannel(QProcess::StandardError);
        data = readAll();
        setReadChannel(tmp);
    }
    return data;
}

void ShellProcess::start(const QString &program, const QStringList &arguments, OpenMode mode)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::start: Process is already running");
        return;
    }
    if (program.isEmpty()) {
        d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
        return;
    }

    d->program = program;
    d->arguments = arguments;

    d->start(mode);
}

void ShellProcess::start(OpenMode mode)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::start: Process is already running");
        return;
    }
    if (d->program.isEmpty()) {
        d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
        return;
    }

    d->start(mode);
}

void ShellProcess::startCommand(const QString &command, OpenMode mode)
{
    QStringList args = splitCommand(command);
    if (args.isEmpty()) {
        qWarning("QProcess::startCommand: empty or whitespace-only command was provided");
        return;
    }
    const QString program = args.takeFirst();
    start(program, args, mode);
}

bool ShellProcess::startDetached(qint64 *pid)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::startDetached: Process is already running");
        return false;
    }
    if (d->program.isEmpty()) {
        d->setErrorAndEmit(QProcess::FailedToStart, tr("No program defined"));
        return false;
    }
    return d->startDetached(pid);
}

/*!
    Starts the program set by setProgram() with arguments set by setArguments().
    The OpenMode is set to \a mode.

    This method is an alias for start(), and exists only to fully implement
    the interface defined by QIODevice.

    Returns \c true if the program has been started.

    \sa start(), setProgram(), setArguments()
*/
bool ShellProcessPrivate::open(QProcess::OpenMode mode)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::start: Process is already running");
        return false;
    }
    if (d->program.isEmpty()) {
        qWarning("QProcess::start: program not set");
        return false;
    }

    d->start(mode);
    return true;
}

void ShellProcessPrivate::start(QIODevice::OpenMode mode)
{
    Q_Q(ShellProcess);
#if defined QPROCESS_DEBUG
    qDebug() << "QProcess::start(" << program << ',' << arguments << ',' << mode << ')';
#endif

    if (stdinChannel.type != ShellProcessPrivate::Channel::Normal)
        mode &= ~QIODevice::WriteOnly;     // not open for writing
    if (stdoutChannel.type != ShellProcessPrivate::Channel::Normal &&
        (stderrChannel.type != ShellProcessPrivate::Channel::Normal ||
         processChannelMode == QProcess::MergedChannels))
        mode &= ~QIODevice::ReadOnly;      // not open for reading
    if (mode == 0)
        mode = QIODevice::Unbuffered;
    if ((mode & QIODevice::ReadOnly) == 0) {
        if (stdoutChannel.type == ShellProcessPrivate::Channel::Normal)
            q->setStandardOutputFile(q->nullDevice());
        if (stderrChannel.type == ShellProcessPrivate::Channel::Normal
            && processChannelMode != QProcess::MergedChannels)
            q->setStandardErrorFile(q->nullDevice());
    }

    q->QIODevice::open(mode);

    if (q->isReadable() && processChannelMode != QProcess::MergedChannels)
        setReadChannelCount(2);

    stdinChannel.closed = false;
    stdoutChannel.closed = false;
    stderrChannel.closed = false;

    exitCode = 0;
    exitStatus = QProcess::NormalExit;
    processError = QProcess::UnknownError;
    errorString.clear();
    startProcess();
}

QStringList ShellProcess::splitCommand(QStringView command)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < command.size(); ++i) {
        if (command.at(i) == u'"') {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += command.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && command.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += command.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}


QString ShellProcess::program() const
{
    Q_D(const ShellProcess);
    return d->program;
}


void ShellProcess::setProgram(const QString &program)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::setProgram: Process is already running");
        return;
    }
    d->program = program;
}


QStringList ShellProcess::arguments() const
{
    Q_D(const ShellProcess);
    return d->arguments;
}


void ShellProcess::setArguments(const QStringList &arguments)
{
    Q_D(ShellProcess);
    if (d->processState != QProcess::NotRunning) {
        qWarning("QProcess::setProgram: Process is already running");
        return;
    }
    d->arguments = arguments;
}

void ShellProcess::terminate()
{
    Q_D(ShellProcess);
    d->terminateProcess();
}


void ShellProcess::kill()
{
    Q_D(ShellProcess);
    d->killProcess();
}

int ShellProcess::exitCode() const
{
    Q_D(const ShellProcess);
    return d->exitCode;
}

QProcess::ExitStatus ShellProcess::exitStatus() const
{
    Q_D(const ShellProcess);
    return QProcess::ExitStatus(d->exitStatus);
}

int ShellProcess::execute(const QString &program, const QStringList &arguments)
{
    ShellProcess process;
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start(program, arguments);
    if (!process.waitForFinished(-1) || process.error() == QProcess::FailedToStart)
        return -2;
    return process.exitStatus() == QProcess::NormalExit ? process.exitCode() : -1;
}

bool ShellProcess::startDetached(const QString &program,
                             const QStringList &arguments,
                             const QString &workingDirectory,
                             qint64 *pid)
{
    ShellProcess process;
    process.setProgram(program);
    process.setArguments(arguments);
    process.setWorkingDirectory(workingDirectory);
    return process.startDetached(pid);
}

QStringList ShellProcess::systemEnvironment()
{
    return QProcessEnvironment::systemEnvironment().toStringList();
}

QString ShellProcess::nullDevice()
{
#ifdef Q_OS_WIN
    return QStringLiteral("\\\\.\\NUL");
#elif defined(_PATH_DEVNULL)
    return QStringLiteral(_PATH_DEVNULL);
#else
    return QStringLiteral("/dev/null");
#endif
}


QT_END_NAMESPACE

#include "moc_qprocess.cpp"
