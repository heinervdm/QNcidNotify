project(QNcidNotify)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtSql REQUIRED)

# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR})

set(DATADIR "${CMAKE_INSTALL_PREFIX}/share")

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
qt4_add_resources(QNcidNotify_RCC QNcidNotify.qrc)
   
set(QNcidNotify_SRCS main.cpp QNcidNotify.cpp ${QNcidNotify_RCC})
qt4_automoc(${QNcidNotify_SRCS})
add_executable(qncidnotify ${QNcidNotify_SRCS} )
target_link_libraries(qncidnotify 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})
 
install(TARGETS qncidnotify
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-qncidnotify.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
