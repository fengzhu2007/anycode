cmake_minimum_required(VERSION 3.10)

project(App)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)




find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Xml Sql Gui Network Concurrent LinguistTools Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Xml Sql Gui Network Concurrent LinguistTools Svg)



include_directories(SYSTEM "../3party/bin/x64/include")

include_directories(SYSTEM "../libsvnpp/include")
include_directories(SYSTEM "../dockingpane/src")
include_directories(SYSTEM "../wRect/src")
include_directories(SYSTEM "../texteditor")
include_directories(SYSTEM "../react-qt/src")
include_directories(SYSTEM "../3party/qscintilla/src")
include_directories(SYSTEM "../3party/libvterm/include")
include_directories(SYSTEM "../3party/libptyqt")
include_directories(SYSTEM "../3party/quickjs")
include_directories(SYSTEM "../3party/tree-sitter/include")
include_directories(SYSTEM "../3party/tree-sitter/language")

#include_directories(SYSTEM "../3party/scintilla/qt/ScintillaEditBase")
#include_directories(SYSTEM "../3party/scintilla/include")
#include_directories(SYSTEM "../3party/scintilla/src")


if(WIN32)
find_library(CRYPTO_LIB libcrypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB libssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB libcurl_imp PATHS "../3party/bin/x64/lib")
find_library(SSH2_LIB libssh2 PATHS "../3party/bin/x64/lib")
find_library(Z_LIB zlib PATHS "../3party/bin/x64/lib")
find_library(GIT2_LIB git2 PATHS "../3party/bin/x64/lib")

else()
find_library(CRYPTO_LIB crypto PATHS "../3party/bin/x64/lib")
find_library(SSL_LIB ssl PATHS "../3party/bin/x64/lib")
find_library(CURL_LIB curl PATHS "../3party/bin/x64/lib")
find_library(SSH2_LIB ssh2 PATHS "../3party/bin/x64/lib")
find_library(Z_LIB z PATHS "../3party/bin/x64/lib")
find_library(GIT2_LIB git2 PATHS "../3party/bin/x64/lib")

endif()


string(TIMESTAMP BUILD_TIME "")
add_definitions(-DBUILD_TIME="${BUILD_TIME}")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()




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
        ${RESOURCE_ADDED}
        ${SOURCES}


        common.h addon_loader.h addon_loader.cpp
        components/message_dialog.h components/message_dialog.cpp
        components/tree_view.h components/tree_view.cpp
        components/statusbar/status_bar_view.h  components/statusbar/status_bar_view.cpp components/statusbar/status_bar_view.ui
        components/dir_mapping_view.h components/dir_mapping_view.cpp
        core/des.h core/des.cpp
        cvs/branch.h cvs/branch.cpp
        cvs/commit.h cvs/commit.cpp
        cvs/commit_model.h cvs/commit_model.cpp
        cvs/diff_file.h cvs/diff_file.cpp
        cvs/diff_file_model.h cvs/diff_file_model.cpp
        cvs/repository.h cvs/repository.cpp
        cvs/error.h cvs/error.cpp
        cvs/git/git_repository.h cvs/git/git_repository.cpp
        cvs/svn/svn_repository.h cvs/svn/svn_repository.cpp
        interface/form_panel.h interface/form_panel.cpp

        interface/remote_file_item_model.h interface/remote_file_item_model.cpp
        interface/file_item.h interface/file_item.cpp



        network/network_listen.h network/network_listen.cpp
        network/network_manager.h network/network_manager.cpp
        network/network_request.h network/network_request.cpp
        network/network_response.h network/network_response.cpp
        network/http/http_client.h network/http/http_client.cpp
        network/http/http_response.h network/http/http_response.cpp

        project/open_project_window.ui project/open_project_window.h project/open_project_window.cpp

        storage/addon_storage.h storage/addon_storage.cpp
        storage/commit_storage.h storage/commit_storage.cpp

        storage/common_storage.h storage/common_storage.cpp
        storage/database_helper.h storage/database_helper.cpp
        storage/favorite_storage.h storage/favorite_storage.cpp
        storage/group_storage.h storage/group_storage.cpp
        storage/project_storage.h storage/project_storage.cpp
        storage/site_setting.h storage/site_setting.cpp
        storage/site_storage.h storage/site_storage.cpp
        storage/storage.h storage/storage.cpp
        transfer/task.h transfer/task.cpp
        #transfer/TaskPoolModel.cpp
        #transfer/TaskThread.cpp
        common/utils.cpp common/utils.h
        zip/zip_archive.h zip/zip_archive.cpp
        zip/minizip/ioapi.c
        zip/minizip/mztools.c
        zip/minizip/unzip.c
        zip/minizip/zip.c


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
        project/site_group_list_model.h project/site_group_list_model.cpp
        project/site_type_list_model.h project/site_type_list_model.cpp


        components/itd_button.h components/itd_button.cpp
        components/listview/listview.h components/listview/listview.cpp
        components/listview/listview_model.h components/listview/listview_model.cpp
        project/site_list_model.h project/site_list_model.cpp
        project/project_select_model.h project/project_select_model.cpp
        panes/resource_manager/resource_manager_tree_item_delegate.h panes/resource_manager/resource_manager_tree_item_delegate.cpp
        panes/code_editor/code_editor_pane.ui panes/code_editor/code_editor_pane.h panes/code_editor/code_editor_pane.cpp

        core/backend_thread.h core/backend_thread.cpp
        panes/resource_manager/resource_manage_read_folder_task.h panes/resource_manager/resource_manage_read_folder_task.cpp









        panes/code_editor/code_editor_manager.h panes/code_editor/code_editor_manager.cpp
        panes/code_editor/code_editor_read_file_task.h panes/code_editor/code_editor_read_file_task.cpp
        panes/version_control/version_control_pane.h panes/version_control/version_control_pane.cpp
        panes/version_control/version_control_pane.ui
        panes/version_control/version_control_query_commit_task.h panes/version_control/version_control_query_commit_task.cpp
        panes/version_control/version_control_query_diff_task.h panes/version_control/version_control_query_diff_task.cpp
        panes/version_control/version_controller_thread.h panes/version_control/version_controller_thread.cpp
        panes/resource_manager/resource_manage_tree_view.h panes/resource_manager/resource_manage_tree_view.cpp
        core/event_bus/type.h core/event_bus/type.cpp
        core/event_bus/event_data.h
        panes/server_manage/server_manage_pane.h panes/server_manage/server_manage_pane.cpp panes/server_manage/server_manage_pane.ui
        panes/file_transfer/file_transfer_pane.h panes/file_transfer/file_transfer_pane.cpp panes/file_transfer/file_transfer_pane.ui
        panes/file_transfer/file_transfer_model.h panes/file_transfer/file_transfer_model.cpp
        panes/code_editor/code_editor_view.h panes/code_editor/code_editor_view.cpp
        panes/server_manage/server_manage_model.h panes/server_manage/server_manage_model.cpp
        panes/server_manage/server_request_thread.h panes/server_manage/server_request_thread.cpp
        panes/server_manage/new_folder_dialog.h panes/server_manage/new_folder_dialog.cpp
        panes/server_manage/new_folder_dialog.ui
        panes/find_replace/find_replace_dialog.h panes/find_replace/find_replace_dialog.cpp panes/find_replace/find_replace_dialog.ui
        panes/find_replace/find_replace_pane.h panes/find_replace/find_replace_pane.cpp panes/find_replace/find_replace_pane.ui
        panes/find_replace/find_widget.h panes/find_replace/find_widget.cpp panes/find_replace/find_widget.ui
        panes/find_replace/replace_widget.h panes/find_replace/replace_widget.cpp panes/find_replace/replace_widget.ui
        panes/find_replace/find_replace_progress.h panes/find_replace/find_replace_progress.cpp
        panes/find_replace/search_result_model.h panes/find_replace/search_result_model.cpp
        panes/find_replace/search_scope_model.h panes/find_replace/search_scope_model.cpp
        panes/code_editor/goto_line_dialog.h panes/code_editor/goto_line_dialog.cpp panes/code_editor/gotolinedialog.ui
        panes/find_replace/replace_all_progress_dialog.h panes/find_replace/replace_all_progress_dialog.cpp panes/find_replace/replace_all_progress_dialog.ui
        core/layout_settings.h core/layout_settings.cpp
        panes/loader.h panes/loader.cpp
        panes/startup/startup_pane.h panes/startup/startup_pane.cpp
        panes/startup/startup_pane.ui
        panes/file_transfer/job_thread.h panes/file_transfer/job_thread.cpp
        panes/server_manage/server_client_pane.ui panes/server_manage/server_client_pane.h panes/server_manage/server_client_pane.cpp
        panes/server_manage/permission_dialog.ui panes/server_manage/permission_dialog.h panes/server_manage/permission_dialog.cpp
        core/list_model.h core/list_model.cpp

        panes/resource_manager/search_combobox.h panes/resource_manager/search_combobox.cpp
        panes/terminal/terminal_pane.ui panes/terminal/terminal_pane.h panes/terminal/terminal_pane.cpp
        panes/terminal/cell_iterator.cpp panes/terminal/cell_iterator.h panes/terminal/glyph_cache.cpp panes/terminal/glyph_cache.h panes/terminal/keys.cpp panes/terminal/keys.h panes/terminal/scrollback.cpp panes/terminal/scrollback.h panes/terminal/surface_integration.h panes/terminal/terminal_surface.cpp panes/terminal/terminal_surface.h panes/terminal/terminal_view.cpp panes/terminal/terminal_view.h
        panes/terminal/terminal_widget.h panes/terminal/terminal_widget.cpp
        panes/terminal/tab_style.h panes/terminal/tab_style.cpp
        panes/code_editor/zoom_label.h panes/code_editor/zoom_label.cpp


        modules/help/about_dialog.ui modules/help/about_dialog.h modules/help/about_dialog.cpp
        modules/help/update_dialog.h modules/help/update_dialog.cpp modules/help/update_dialog.ui
        storage/recent_storage.h storage/recent_storage.cpp
        panes/output/output_pane.h panes/output/output_pane.cpp panes/output/output_pane.ui
        panes/version_control/version_control_delete_file_task.h panes/version_control/version_control_delete_file_task.cpp
        modules/options/options_dialog.ui modules/options/options_dialog.h modules/options/options_dialog.cpp
        modules/options/option_widget.h modules/options/option_widget.cpp
        modules/options/environment_option_widget.h modules/options/environment_option_widget.cpp modules/options/environment_option_widget.ui
        modules/options/texteditor_option_widget.h modules/options/texteditor_option_widget.cpp modules/options/texteditor_option_widget.ui
        modules/options/options_model.h modules/options/options_model.cpp
        app_oss.h app_oss.cpp
        components/list_item_delegate.h components/list_item_delegate.cpp
        modules/options/option_tab.h modules/options/option_tab.cpp
        modules/options/texteditor/general_tab.h modules/options/texteditor/general_tab.cpp
        modules/options/texteditor/general_tab.ui
        modules/options/texteditor/color_tab.h modules/options/texteditor/color_tab.cpp modules/options/texteditor/color_tab.ui
        modules/options/options_settings.h modules/options/options_settings.cpp
        modules/options/texteditor/simple_editor_view.h modules/options/texteditor/simple_editor_view.cpp
        modules/options/environment_settings.h modules/options/environment_settings.cpp
        panes/error/error_pane.h panes/error/error_pane.cpp panes/error/error_pane.ui
        modules/addon/addon_manager_dialog.h modules/addon/addon_manager_dialog.cpp modules/addon/addon_manager_dialog.ui
        modules/addon/addon_manager_model.h modules/addon/addon_manager_model.cpp
        modules/addon/addon_item_widget.h modules/addon/addon_item_widget.cpp modules/addon/addon_item_widget.ui
        core/schedule/schedule.h core/schedule/schedule.cpp
        core/schedule/schedule_task.h core/schedule/schedule_task.cpp
        core/schedule/file_auto_save_task.h core/schedule/file_auto_save_task.cpp
        core/schedule/network_auto_close_task.h core/schedule/network_auto_close_task.cpp
        core/schedule/network_rate_task.h core/schedule/network_rate_task.cpp
        core/schedule/network_status_task.h core/schedule/network_status_task.cpp
        components/grid_view.h components/grid_view.cpp
        panes/server_manage/site_quick_manager_dialog.h panes/server_manage/site_quick_manager_dialog.cpp panes/server_manage/site_quick_manager_dialog.ui
        components/password_edit.h components/password_edit.cpp
        modules/import_export/import_export_dialog.h modules/import_export/import_export_dialog.cpp modules/import_export/import_export_dialog.ui
        modules/import_export/select_widget.h modules/import_export/select_widget.cpp modules/import_export/select_widget.ui
        modules/import_export/project_widget.h modules/import_export/project_widget.cpp modules/import_export/project_widget.ui
        modules/import_export/export_widget.h modules/import_export/export_widget.cpp modules/import_export/export_widget.ui
        modules/import_export/export_to_widget.h modules/import_export/export_to_widget.cpp modules/import_export/export_to_widget.ui
        modules/import_export/select_tree_model.h modules/import_export/select_tree_model.cpp
        modules/import_export/import_select_widget.h modules/import_export/import_select_widget.cpp modules/import_export/import_select_widget.ui
        modules/import_export/import_widget.h modules/import_export/import_widget.cpp modules/import_export/import_widget.ui
        #core/extension/extension_engine.h core/extension/extension_engine.cpp
        modules/import_export/backup_restore.h modules/import_export/backup_restore.cpp
        core/quick_pane_impl.h core/quick_pane_impl.cpp

        panes/resource_manager/resource_manager_opened_model.h panes/resource_manager/resource_manager_opened_model.cpp
        panes/resource_manager/locate_file_task.h panes/resource_manager/locate_file_task.cpp
        panes/code_editor/editor.h panes/code_editor/editor.cpp
        panes/code_editor/code_lint.h panes/code_editor/code_lint.cpp
        core/theme.h core/theme.cpp
        themes/light_theme.h themes/light_theme.cpp
        themes/dark_theme.h themes/dark_theme.cpp
        ide_application.h ide_application.cpp
        panes/code_editor/cmdlint/php_lint.h panes/code_editor/cmdlint/php_lint.cpp
        #panes/code_editor/parselint/javascript/javascript_lint.h panes/code_editor/parselint/javascript/javascript_lint.cpp
        panes/code_editor/parselint/json_lint.h panes/code_editor/parselint/json_lint.cpp
        panes/code_editor/parselint/javascript_lint.h panes/code_editor/parselint/javascript_lint.cpp
        #panes/code_editor/cmdlint/python_lint.h panes/code_editor/cmdlint/python_lint.cpp
        panes/code_editor/parselint/python_lint.h panes/code_editor/parselint/python_lint.cpp
        interface/code_parse_lint.h interface/code_parse_lint.cpp
        panes/code_editor/parselint/css_lint.h panes/code_editor/parselint/css_lint.cpp
        panes/code_editor/parselint/html_lint.h panes/code_editor/parselint/html_lint.cpp
        panes/code_editor/parselint/typescript_lint.h panes/code_editor/parselint/typescript_lint.cpp
        panes/code_editor/parselint/xml_lint.h panes/code_editor/parselint/xml_lint.cpp
        panes/code_editor/parselint/cpp_lint.h panes/code_editor/parselint/cpp_lint.cpp

















)





target_link_libraries(App PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Svg   ${CRYPTO_LIB} ${SSL_LIB} ${SSH2_LIB} ${Z_LIB} ${CURL_LIB} ${GIT2_LIB} $ libsvnpp dockingpane wRect texteditor libvterm ptyqt
    tree-sitter
    tree-sitter-json
    tree-sitter-javascript
    tree-sitter-python
    tree-sitter-css
    tree-sitter-html
    tree-sitter-typescript
    tree-sitter-cpp
    tree-sitter-xml
    tree-sitter-dtd
    tree-sitter-php

)


if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(App)
endif()

target_compile_definitions(App PRIVATE APP_LIBRARY)



include(GNUInstallDirs)
install(TARGETS App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})




set(TRANSLATIONS
    zh_CN
)

if(WIN32)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}.dll        ${CMAKE_BINARY_DIR}/src/${PROJECT_NAME}.dll
    COMMENT "Copying file to binary directory"
)
else()


endif()

