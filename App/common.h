#ifndef COMMON_H
#define COMMON_H
#include <QObject>
const QString APP_NAME = QObject::tr("AnyCode");
const QString APP_VERSION = QObject::tr("1.0.0");
const int APP_VERSIONID = 2;
const QString APP_URL = QLatin1String("http://dev.anycode.xyz/");
const QString APP_PROJECT_URL = QLatin1String("https://github.com/fengzhu2007/anycode");
const QString APP_DONATE_URL = QLatin1String("http://dev.anycode.xyz/donate");
const QString APP_HELP_URL = QLatin1String("http://dev.anycode.xyz/help");
const QString APP_UPDATE_URL = QLatin1String("http://dev.anycode.xyz/update");

constexpr const static char MM_DOWNLOAD[] = "download/any-fileitem";
constexpr const static char MM_UPLOAD[] = "upload/any-fileitem";

#endif // COMMON_H
