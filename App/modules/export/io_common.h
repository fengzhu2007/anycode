#ifndef IO_COMMON_H
#define IO_COMMON_H


/*
<ADY>
    <INFO>
        <NAME>AnyDeploy Export Data</NAME>
        <VERSION></VERSION>
        <CREATOR></CREATOR>
        <DATE></DATE>
    </INFO>
    <DATA>
        <PROJECTS>
            <PROJECT>
                <NAME></NAME>
                <PATH></PATH>
                <GROUPS>
                    <GROUP system="0,1,2,3">
                        <NAME ></NAME>
                        <SITES>
                            <SITE>
                                <NAME></NAME>
                                <HOST></HOST>
                                <PORT></PORT>
                                <USERNAME><USERNAME>
                                <PASSWORD></PASSWORD>
                                <PATH></PATH>
                                <TYPE></TYPE>
                                <STATUS></STATUS>
                                <LISTORDER></LISTORDER>
                                <SETTINGS></SETTINGS>
                            </SITE>
                        </SITES>
                    </GROUP>
                </GROUPS>
            </PROJECT>

        </PROJECTS>

    </DATA>

</ADY>
*/

#include <QString>

const QString ADY_TAG = "ADY";
const QString INFO_TAG = "INFO";
const QString DATA_TAG = "DATA";
const QString NAME_TAG = "NAME";
//const QString VERSION_TAG = "VERSION";
const QString CREATOR_TAG = "CREATOR";
const QString DATE_TAG = "DATE";
const QString PROJECTS_TAG = "PROJECTS";
const QString PROJECT_TAG = "PROJECT";
const QString PROJECT_NAME_TAG = "NAME";
const QString PROJECT_PATH_TAG = "PATH";
const QString GROUPS_TAG = "GROUPS";
const QString GROUP_TAG = "GROUP";
const QString GROUP_NAME_TAG = "NAME";
const QString SITES_TAG = "SITES";
const QString SITE_TAG = "SITE";
const QString SITE_NAME_TAG = "NAME";
const QString SITE_HOST_TAG = "HOST";
const QString SITE_PORT_TAG = "PORT";
const QString SITE_USERNAME_TAG = "USERNAME";
const QString SITE_PASSWORD_TAG = "PASSWORD";
const QString SITE_PATH_TAG = "PATH";
const QString SITE_TYPE_TAG = "TYPE";
const QString SITE_STATUS_TAG = "STATUS";
const QString SITE_LISTORDER_TAG = "LISTORDER";
const QString SITE_SETTINGS_TAG = "SETTINGS";

const QString ADY_ATTR_VERSION = "version";
const QString GROUP_ATTR_SYSTEM = "system";

#endif // IO_COMMON_H
