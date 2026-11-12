# Qt6 要求 CMake >= 3.16
cmake_minimum_required(VERSION 3.16)

project(App)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)




set(COMPILER_INFO "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} ${CMAKE_SYSTEM_PROCESSOR}")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Xml Sql Gui Network Concurrent LinguistTools Svg Qml Quick QuickWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Xml Sql Gui Network Concurrent LinguistTools Svg Qml Quick QuickWidgets)

# Qt6 中 QSvgWidget 位于独立的 Qt::SvgWidgets 模块，Qt5 则包含在 Qt::Svg 内
# Qt6 中 QTextCodec 迁移到 Qt::Core5Compat 模块
set(APP_EXTRA_QT_LIBS "")
if(QT_VERSION_MAJOR EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS SvgWidgets Core5Compat)
    list(APPEND APP_EXTRA_QT_LIBS Qt6::SvgWidgets Qt6::Core5Compat)
endif()



# 第三方库的头文件目录全部指向 3party 内的源码，配合下面的 in-tree 目标使用，
# 不再依赖 ../3party/bin 下预先编译好的头文件（版本与源码保持一致）
include_directories(SYSTEM "../3party/curl/include")
include_directories(SYSTEM "../3party/libssh2/include")
# libgit2 的目标只提供 PRIVATE 头文件目录，这里手工补上；
# 构建目录下的 include 中有 configure_file 生成的 git2/experimental.h
include_directories(SYSTEM "${CMAKE_BINARY_DIR}/3party/libgit2/include")
include_directories(SYSTEM "../3party/libgit2/include")
# 加密库改用 3party 内源码构建的 mbedtls（替代原 openssl）
include_directories(SYSTEM "../3party/mbedtls/include")

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

#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#message(STATUS "Include directories: ${dirs}")


# 加密库不再使用预编译的 openssl，改由 3party/CMakeLists.txt 中
# add_subdirectory(mbedtls) 提供的 mbedtls / mbedx509 / mbedcrypto 目标
set(MBEDTLS_LIBS mbedtls mbedx509 mbedcrypto)

# ---------------------------------------------------------------------------
# 其余第三方库同样由 3party/CMakeLists.txt 就地（in-tree）编译：
# 这里直接链接 CMake 目标，既自动带上头文件目录，又自动建立构建顺序依赖，
# 不再需要先用脚本把 curl / libssh2 / zlib / libgit2 预编译到 ../3party/bin 下
# ---------------------------------------------------------------------------

# curl：3party 中只开启了 BUILD_SHARED_LIBS，实际产物是 libcurl_shared；
# CURL::libcurl 是该目标（或静态版）的别名，会跟随 3party 的开关自动选择
set(CURL_LIB "")
if(TARGET CURL::libcurl)
    set(CURL_LIB CURL::libcurl)
elseif(TARGET libcurl_shared)
    set(CURL_LIB libcurl_shared)
elseif(TARGET libcurl_static)
    set(CURL_LIB libcurl_static)
else()
    message(FATAL_ERROR "找不到 libcurl 目标：请确认顶层已 add_subdirectory(3party) 并参与构建")
endif()

# libssh2 在 3party 中是静态编译进 libcurl 的，只有 App 直接静态链接 libcurl 时才需要
set(SSH2_LIB "")
if(CURL_LIB STREQUAL "libcurl_static" AND TARGET libssh2_static)
    set(SSH2_LIB libssh2_static)
endif()

# zlib：3party 中只构建静态库 zlibstatic（zlib.h 与生成的 zconf.h 由该目标带出）
if(TARGET zlibstatic)
    set(Z_LIB zlibstatic)
elseif(TARGET zlib)
    set(Z_LIB zlib)
else()
    message(FATAL_ERROR "找不到 zlib 目标：请确认顶层已 add_subdirectory(3party) 并参与构建")
endif()

# libgit2：3party 中的目标名是 libgit2package，产物为 git2.dll / git2.lib
if(TARGET libgit2package)
    set(GIT2_LIB libgit2package)
elseif(TARGET git2)
    set(GIT2_LIB git2)
else()
    message(FATAL_ERROR "找不到 libgit2 目标：请确认顶层已 add_subdirectory(3party) 并参与构建")
endif()


string(TIMESTAMP BUILD_TIME "")
add_definitions(-DBUILD_TIME="${BUILD_TIME}")


if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DQ_DEBUG)
endif()








set(RESOURCES
    r.qrc
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_resources(RESOURCE_ADDED ${RESOURCES})
else()
    qt5_add_resources(RESOURCE_ADDED ${RESOURCES})
endif()



set(TS_FILES
    translations/App_zh_CN.ts
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_translation(QM_FILES ${TS_FILES})
    set(APP_LUPDATE_EXECUTABLE ${Qt6_LUPDATE_EXECUTABLE})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    set(APP_LUPDATE_EXECUTABLE ${Qt5_LUPDATE_EXECUTABLE})
endif()

# 添加自定义目标用于手动更新翻译（不会在每次构建时自动运行）
add_custom_target(update_translations
    COMMAND ${APP_LUPDATE_EXECUTABLE}
        -source-language zh_CN
        -no-obsolete
        ${CMAKE_CURRENT_SOURCE_DIR}
        -ts ${TS_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Updating translations..."
)


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
        ${QM_FILES}


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
        cvs/diff_content.h cvs/diff_content.cpp
        cvs/repository.h cvs/repository.cpp
        cvs/error.h cvs/error.cpp
        cvs/git/git_repository.h cvs/git/git_repository.cpp
        interface/form_panel.h interface/form_panel.cpp

        interface/remote_file_item_model.h interface/remote_file_item_model.cpp
        interface/file_item.h interface/file_item.cpp



        network/network_listen.h network/network_listen.cpp
        network/network_manager.h network/network_manager.cpp
        network/network_request.h network/network_request.cpp
        network/network_response.h network/network_response.cpp
        network/http/http_client.h network/http/http_client.cpp
        network/http/http_response.h network/http/http_response.cpp
        network/gateway_http_server.h network/gateway_http_server.cpp
        network/mcp/mcp_handler.h network/mcp/mcp_handler.cpp

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
        panes/code_editor/diff_editor_widget.h panes/code_editor/diff_editor_widget.cpp
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
        components/file_selector.h components/file_selector.cpp
        modules/options/language_settings.h modules/options/language_settings.cpp
        modules/options/language_option_widget.h modules/options/language_option_widget.cpp
        modules/options/language/php_tab.h modules/options/language/php_tab.cpp
        modules/options/language_option_widget.ui
        modules/options/language/php_tab.ui
        panes/image_editor/image_editor_pane.h panes/image_editor/image_editor_pane.cpp
        panes/image_editor/image_editor_pane.ui
        panes/image_editor/image_view.h panes/image_editor/image_view.cpp
        tools/ssl_query/ssl_query_dialog.h tools/ssl_query/ssl_query_dialog.cpp tools/ssl_query/ssl_query_dialog.ui
        tools/ssl_query/ssl_query_task.h tools/ssl_query/ssl_query_task.cpp
        tools/ssl_query/ssl_querier.h tools/ssl_query/ssl_querier.cpp
        tools/ssl_query/query_result_model.h tools/ssl_query/query_result_model.cpp
        panes/sql/sql_pane.h panes/sql/sql_pane.cpp panes/sql/sql_pane.ui
        interface/sql_querier.h interface/sql_querier.cpp
        panes/notification/notification_pane.h panes/notification/notification_pane.cpp panes/notification/notification_pane.ui
        panes/notification/notification_card.h panes/notification/notification_card.cpp
        panes/notification/notification_model.h panes/notification/notification_model.cpp
        panes/notification/notification_card.ui
        project/import_site_dialog.h project/import_site_dialog.cpp project/import_site_dialog.ui
        modules/ai/ai_request.h modules/ai/ai_request.cpp
        modules/options/ai_settings.h modules/options/ai_settings.cpp
        modules/options/ai_option_widget.h modules/options/ai_option_widget.cpp modules/options/ai_option_widget.ui
        modules/options/network_settings.h modules/options/network_settings.cpp
        modules/options/network_option_widget.h modules/options/network_option_widget.cpp modules/options/network_option_widget.ui
        modules/options/network/proxy_tab.h modules/options/network/proxy_tab.cpp modules/options/network/proxy_tab.ui
        modules/options/agent_settings.h modules/options/agent_settings.cpp
        modules/options/agent_option_widget.h modules/options/agent_option_widget.cpp modules/options/agent_option_widget.ui
        modules/options/agent/local_tab.h modules/options/agent/local_tab.cpp modules/options/agent/local_tab.ui
        components/select_model.h
        panes/code_editor/ai_client.h panes/code_editor/ai_client.cpp
        panes/code_editor/agent_diff_service.h panes/code_editor/agent_diff_service.cpp
        modules/ai/ai_response.h modules/ai/ai_response.cpp
        modules/ai/gateway_request.h modules/ai/gateway_request.cpp
        modules/ai/gateway_response.h modules/ai/gateway_response.cpp
        panes/db/dbms_pane.h panes/db/dbms_pane.cpp panes/db/dbms_pane.ui
        panes/db/dbms_model.cpp panes/db/dbms_model.h
        panes/db/dbms_thread.cpp panes/db/dbms_thread.h
        panes/db/db_driver.h panes/db/db_driver.cpp
        storage/db_storage.h storage/db_storage.cpp
        storage/template_command_storage.h storage/template_command_storage.cpp
        storage/template_command_history.h storage/template_command_history.cpp
        panes/db/sqlite_driver.h panes/db/sqlite_driver.cpp
        panes/db/sqlite_connect_dialog.h panes/db/sqlite_connect_dialog.cpp panes/db/sqlite_connect_dialog.ui
        components/tree_item_delegate.h components/tree_item_delegate.cpp
        panes/svg_editor/svg_editor_pane.h panes/svg_editor/svg_editor_pane.cpp panes/svg_editor/svg_editor_pane.ui
        panes/svg_editor/svg_viewer.h panes/svg_editor/svg_viewer.cpp
        panes/db/table_pane.h panes/db/table_pane.cpp panes/db/table_pane.ui
        panes/db/table_list_widget.h panes/db/table_list_widget.cpp panes/db/table_list_widget.ui
        panes/db/table_scheme_widget.h panes/db/table_scheme_widget.cpp panes/db/table_scheme_widget.ui
        panes/db/table_field_widget.h panes/db/table_field_widget.cpp panes/db/table_field_widget.ui
        panes/db/table_index_widget.h panes/db/table_index_widget.cpp panes/db/table_index_widget.ui
        panes/db/table_option_widget.h panes/db/table_option_widget.cpp panes/db/table_option_widget.ui
        panes/db/table_field_model.h panes/db/table_field_model.cpp
        panes/db/table_field.h panes/db/table_field.cpp
        panes/db/table_data_model.h panes/db/table_data_model.cpp
        panes/db/sqlite/field_form.h panes/db/sqlite/field_form.cpp panes/db/sqlite/field_form.ui
        panes/db/field_form_impl.h panes/db/field_form_impl.cpp
        core/debug_log.h core/debug_log.cpp
        panes/db/index_form_impl.h panes/db/index_form_impl.cpp
        panes/db/sqlite/index_form.h panes/db/sqlite/index_form.cpp panes/db/sqlite/index_form.ui
        panes/db/table_index_model.h panes/db/table_index_model.cpp
        tools/color/color_dialog.h tools/color/color_dialog.cpp tools/color/color_dialog.ui
        tools/color/color_picker.h tools/color/color_picker.cpp
        tools/image_clip/image_clip_dialog.h tools/image_clip/image_clip_dialog.cpp tools/image_clip/image_clip_dialog.ui
        components/dir_selector.h components/dir_selector.cpp
        tools/image_clip/scale_tab.h tools/image_clip/scale_tab.cpp tools/image_clip/scale_tab.ui
        tools/image_clip/resize_tab.h tools/image_clip/resize_tab.cpp tools/image_clip/resize_tab.ui
        tools/image_clip/image_process_thread.h tools/image_clip/image_process_thread.cpp
        panes/frames_viewer/frames_viewer_pane.h panes/frames_viewer/frames_viewer_pane.cpp panes/frames_viewer/frames_viewer_pane.ui
        panes/frames_viewer/animation_frames_player.h panes/frames_viewer/animation_frames_player.cpp
        panes/frames_viewer/frames_model.h panes/frames_viewer/frames_model.cpp
        panes/frames_viewer/frames_merge_dialog.h panes/frames_viewer/frames_merge_dialog.cpp panes/frames_viewer/frames_merge_dialog.ui
        panes/ai_chat/ai_chat_pane.h panes/ai_chat/ai_chat_pane.cpp panes/ai_chat/ai_chat_pane.ui
        panes/ai_chat/chat_service.h panes/ai_chat/chat_service.cpp
        panes/ai_chat/chat_message_widget.h panes/ai_chat/chat_message_widget.cpp panes/ai_chat/chat_message_widget.ui
        panes/ai_chat/chat_message_bubble.h panes/ai_chat/chat_message_bubble.cpp
        panes/ai_chat/chat_message_view.h
        panes/ai_chat/session_page_widget.h panes/ai_chat/session_page_widget.cpp panes/ai_chat/session_page_widget.ui
        panes/ai_chat/session_config_dialog.h panes/ai_chat/session_config_dialog.cpp panes/ai_chat/session_config_dialog.ui
        panes/ai_chat/file_diff_list_widget.h panes/ai_chat/file_diff_list_widget.cpp panes/ai_chat/file_diff_list_widget.ui
        panes/ai_chat/file_diff_item_widget.h panes/ai_chat/file_diff_item_widget.cpp panes/ai_chat/file_diff_item_widget.ui
        panes/ai_chat/session_list_popup.h panes/ai_chat/session_list_popup.cpp
        panes/ai_chat/session_list_popup_v2.h panes/ai_chat/session_list_popup_v2.cpp
        panes/ai_chat/message_model.h panes/ai_chat/message_model.cpp
        panes/ai_chat/message_list_view.h panes/ai_chat/message_list_view.cpp
        panes/ai_chat/message_delegate.h panes/ai_chat/message_delegate.cpp
        panes/ai_chat/qml_message_model.h panes/ai_chat/qml_message_model.cpp
        tools/image_clip/cut_tab.h tools/image_clip/cut_tab.cpp tools/image_clip/cut_tab.ui
        tools/image_clip/workflow_tab.h tools/image_clip/workflow_tab.cpp tools/image_clip/workflow_tab.ui
        tools/image_clip/workflow_item.h tools/image_clip/workflow_item.cpp tools/image_clip/workflow_item.ui
        tools/image_clip/workflow_model.h tools/image_clip/workflow_model.cpp
        tools/template_command/template_command_dialog.h tools/template_command/template_command_dialog.cpp tools/template_command/template_command_dialog.ui
        panes/ai_chat/mdstyler.h panes/ai_chat/mdstyler.cpp
        addon_loader_helper.h addon_loader_helper.cpp




)




target_link_libraries(App PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Concurrent Qt${QT_VERSION_MAJOR}::Svg Qt${QT_VERSION_MAJOR}::Quick Qt${QT_VERSION_MAJOR}::QuickWidgets  ${APP_EXTRA_QT_LIBS}  ${MBEDTLS_LIBS} ${SSH2_LIB} ${Z_LIB} ${CURL_LIB} ${GIT2_LIB} dockingpane wRect texteditor libvterm ptyqt
    gatewap
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
    tree-sitter-php_only
    tree-sitter-embedded-template

)





if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(App)
endif()

target_compile_definitions(App PRIVATE APP_LIBRARY COMPILER_INFO="${COMPILER_INFO}")



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

# in-tree 共同编译出来的共享库（DLL）也要放到可执行文件目录，否则运行时会找不到依赖
set(APP_RUNTIME_DLL_TARGETS
    libcurl_shared
    libgit2package
    mbedtls
    mbedx509
    mbedcrypto
)
foreach(_dll_target ${APP_RUNTIME_DLL_TARGETS})
    if(TARGET ${_dll_target})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:${_dll_target}>
                ${CMAKE_BINARY_DIR}/src
            COMMENT "Copying ${_dll_target} runtime to binary directory"
        )
    endif()
endforeach()
else()


endif()

#copy i18n files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/App_zh_CN.qm        ${CMAKE_BINARY_DIR}/src/translations/App_zh_CN.qm
    COMMENT "Copying file to binary directory"
)

