requires(qtHaveModule(widgets))

TEMPLATE = subdirs
SUBDIRS += \
    FTP \
    SFTP \
    OSS \
    COS

TRANSLATIONS = \
    language.zh_CN.ts
