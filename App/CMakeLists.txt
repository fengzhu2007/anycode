cmake_minimum_required(VERSION 3.14)

project(App)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Xml Sql Gui Network Concurrent LinguistTools Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Xml Sql Gui Network Concurrent LinguistTools Svg)

include_directories(SYSTEM "../3party/bin/x64/include")

include_directories(SYSTEM "../libsvnpp/include")
include_directories(SYSTEM "../dockingpane/src")
include_directories(SYSTEM "../wRect/src")


find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB libcurl_imp PATHS "../3party/bin/x64/lib")
find_library(SSH2_LIB libssh2 PATHS "../3party/bin/x64/lib")
find_library(Z_LIB zlib PATHS "../3party/bin/x64/lib")
find_library(GIT2_LIB git2 PATHS "../3party/bin/x64/lib")


set(FORMS
    components/NotificationItem.ui
    interface/NewFileDialog.ui
    localdirpanel.ui
    mainwindow.ui
    modules/addon/AddonManageWindow.ui
    modules/favorite/AddDialog.ui
    modules/favorite/ManageDialog.ui
    modules/help/aboutdialog.ui
    modules/help/donationdialog.ui
    modules/log/LogViewer.ui
    project/ManagerWindow.ui
    project/open_project_window.ui
    project/ProjectEditWidget.ui
    project/SiteFrameworkWidget.ui
    project/splitterhandlewidgetui.ui
    view/StatusBarView.ui
    dockwindow.ui
)

qt5_wrap_ui(FORMS_HEADERS ${FORMS})

set(RESOURCES
    r.qrc
)

qt5_add_resources(RESOURCE_ADDED ${RESOURCES})



set(SOURCES

)

if(WIN32)
    list(APPEND SOURCES
        zip/minizip/iowin32.c
    )
endif()

add_library(App SHARED
        ${FORMS_HEADERS}
        ${RESOURCE_ADDED}
        ${SOURCES}
        AddonLoader.cpp
        components/DirComboBox.cpp
        components/DirMappingView.cpp
        components/IconWidget.cpp
        components/MessageDialog.cpp
        components/Notification.cpp
        components/TreeView.cpp
        core/des.cpp
        cvs/Branch.cpp
        cvs/Commit.cpp
        cvs/CommitModel.cpp
        cvs/DiffFile.cpp
        cvs/DiffFileModel.cpp
        cvs/Repository.cpp
        cvs/git/GitRepository.cpp
        cvs/svn/SvnRepository.cpp
        interface/FormPanel.cpp
        interface/NewFileDialog.cpp
        interface/NotificationImpl.cpp
        interface/Panel.cpp
        interface/RemoteFileItemModel.cpp
        local/FileItem.cpp
        local/FileItemModel.cpp
        localdirpanel.cpp
        mainwindow.cpp
        modules/addon/AddonManageWindow.cpp
        modules/addon/AddonModel.cpp
        modules/export/DataExport.cpp
        modules/favorite/AddDialog.cpp
        modules/favorite/FavoriteItemModel.cpp
        modules/favorite/ManageDialog.cpp
        modules/help/AboutDialog.cpp
        modules/help/DonationDialog.cpp
        modules/import/DataImport.cpp
        modules/log/Log.cpp
        modules/log/LogViewer.cpp
        network/NetworkListen.cpp
        network/NetworkManager.cpp
        network/NetworkRequest.cpp
        network/NetworkResponse.cpp
        network/http/HttpClient.cpp
        network/http/HttpResponse.cpp
        project/ManagerTreeItem.cpp
        project/ManagerTreeModel.cpp
        project/ManagerWindow.cpp
        project/open_project_window.cpp
        project/ProjectEditWidget.cpp
        project/ProjectListModel.cpp
        project/SiteFrameworkWidget.cpp
        project/SiteGroupListModel.cpp
        project/SiteTypeListModel.cpp
        project/SplitterHandleWidget.cpp
        storage/AddonStorage.cpp
        storage/CommitStorage.cpp
        storage/CommonStorage.cpp
        storage/DatabaseHelper.cpp
        storage/FavoriteStorage.cpp
        storage/GroupStorage.cpp
        storage/ProjectStorage.cpp
        storage/SiteSetting.cpp
        storage/SiteStorage.cpp
        storage/Storage.cpp
        transfer/Task.cpp
        transfer/TaskPoolModel.cpp
        transfer/TaskThread.cpp
        utils.cpp
        view/StatusBarView.cpp
        zip/ZipArchive.cpp
        zip/minizip/ioapi.c
        zip/minizip/mztools.c
        zip/minizip/unzip.c
        zip/minizip/zip.c
        mainwindow.h
        dockwindow.h dockwindow.cpp
        idewindow.h idewindow.cpp
        idewindow.ui
        panes/resource_manager/resource_manager_pane.h panes/resource_manager/resource_manager_pane.cpp panes/resource_manager/resource_manager_pane.ui
        panes/resource_manager/resource_manager_model.h panes/resource_manager/resource_manager_model.cpp
        panes/resource_manager/resource_manager_model_item.h panes/resource_manager/resource_manager_model_item.cpp
        panes/resource_manager/resource_manage_icon_provider.h panes/resource_manager/resource_manage_icon_provider.cpp
        core/event_bus/subscriber.h core/event_bus/subscriber.cpp
        core/event_bus/publisher.h core/event_bus/publisher.cpp
        core/event_bus/event.h core/event_bus/event.cpp
        project/new_project_window.h project/new_project_window.cpp project/new_project_window.ui
        project/new_project_one_widget.h project/new_project_one_widget.cpp project/new_project_one_widget.ui
        project/new_project_two_widget.h project/new_project_two_widget.cpp project/new_project_two_widget.ui
        project/new_project_zero_widget.h project/new_project_zero_widget.cpp project/new_project_zero_widget.ui
        project/project_manage_model.h project/project_manage_model.cpp
        components/itd_button.h components/itd_button.cpp
        components/listview/listview.h components/listview/listview.cpp
        components/listview/listview_model.h components/listview/listview_model.cpp
        project/site_list_model.h project/site_list_model.cpp


)





target_link_libraries(App PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Svg ${CRYPTO_LIB} ${SSL_LIB} ${SSH2_LIB} ${Z_LIB} ${CURL_LIB} ${GIT2_LIB} libsvnpp dockingpane wRect)

target_compile_definitions(App PRIVATE APP_LIBRARY)


set(TRANSLATIONS
    zh_CN
)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}.dll        ${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.dll
    #COMMAND ${CMAKE_COMMAND} -E copy        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/language.zh_CN.qm        ${CMAKE_BINARY_DIR}/src/langs/${PROJECT_NAME}.zh_CN.qm
    COMMENT "Copying file to binary directory"
)


