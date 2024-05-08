cmake_minimum_required(VERSION 3.14)



set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(../libsvnpp/CMakeLists.txt)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(App SHARED
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
        project/OpenProjectWindow.cpp
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
)

target_link_libraries(App PRIVATE Qt${QT_VERSION_MAJOR}::Core libsvnpp)

target_compile_definitions(App PRIVATE APP_LIBRARY)
