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
        components/message_dialog.cpp
        components/Notification.cpp
        components/TreeView.cpp
        core/des.cpp
        cvs/branch.h cvs/branch.cpp
        cvs/commit.h cvs/commit.cpp
        cvs/commit_model.h cvs/commit_model.cpp
        cvs/diff_file.h cvs/diff_file.cpp
        cvs/diff_file_model.h cvs/diff_file_model.cpp
        cvs/repository.h cvs/repository.cpp
        cvs/git/git_repository.h cvs/git/git_repository.cpp
        cvs/svn/svn_repository.h cvs/svn/svn_repository.cpp
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
        common/utils.cpp common/utils.h
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
        project/project_select_model.h project/project_select_model.cpp
        panes/resource_manager/resource_manager_tree_item_delegate.h panes/resource_manager/resource_manager_tree_item_delegate.cpp
        panes/code_editor/code_editor_pane.h panes/code_editor/code_editor_pane.cpp
        panes/code_editor/code_editor.h panes/code_editor/code_editor.cpp
        panes/code_editor/language/php_high_lighter.h panes/code_editor/language/php_high_lighter.cpp
        core/backend_thread.h core/backend_thread.cpp
        panes/resource_manager/resource_manage_read_folder_task.h panes/resource_manager/resource_manage_read_folder_task.cpp
        panes/code_editor/code_high_lighter.h panes/code_editor/code_high_lighter.cpp
        panes/code_editor/auto_completion_view.h panes/code_editor/auto_completion_view.cpp
        panes/code_editor/language.h panes/code_editor/language.cpp
        panes/code_editor/language/php_language.h panes/code_editor/language/php_language.cpp
        panes/code_editor/language_manager.h panes/code_editor/language_manager.cpp
        panes/code_editor/language/js_language.h panes/code_editor/language/js_language.cpp
        panes/code_editor/language/html_language.h panes/code_editor/language/html_language.cpp
        panes/code_editor/language/css_language.h panes/code_editor/language/css_language.cpp
        panes/code_editor/code_editor_manager.h panes/code_editor/code_editor_manager.cpp
        panes/code_editor/code_editor_read_file_task.h panes/code_editor/code_editor_read_file_task.cpp
        panes/version_control/version_control_pane.h panes/version_control/version_control_pane.cpp
        panes/version_control/version_control_pane.ui
        panes/version_control/version_control_query_commit_task.h panes/version_control/version_control_query_commit_task.cpp
        panes/version_control/version_control_query_diff_task.h panes/version_control/version_control_query_diff_task.cpp

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


