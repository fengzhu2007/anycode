#ifndef COMMON_H
#define COMMON_H
#include <QObject>
const QString APP_NAME = QObject::tr("Any Deploy");
const QString APP_VERSION = QObject::tr("0.9.0 Beta");
const int APP_VERSIONID = 1;
const QString APP_URL = QLatin1String("https://www.anydeploy.xyz/");
constexpr const static char MM_DOWNLOAD[] = "download/any-fileitem";
constexpr const static char MM_UPLOAD[] = "upload/any-fileitem";

#endif // COMMON_H
