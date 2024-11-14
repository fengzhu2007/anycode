#ifndef COMMON_H
#define COMMON_H
#include <QObject>
const QString APP_NAME = QObject::tr("AnyCode");
const QString APP_VERSION = QObject::tr("0.9.0 Beta");
const int APP_VERSIONID = 1;
const QString APP_URL = QLatin1String("https://www.anycode.xyz/");
const QString APP_PROJECT_URL = QLatin1String("https://github.com/fengzhu2007/anycode");
const QString APP_DONATE_URL = QLatin1String("https://www.anycode.xyz/donate");
const QString APP_HELP_URL = QLatin1String("https://www.anycode.xyz/help");

constexpr const static char MM_DOWNLOAD[] = "download/any-fileitem";
constexpr const static char MM_UPLOAD[] = "upload/any-fileitem";

#endif // COMMON_H
