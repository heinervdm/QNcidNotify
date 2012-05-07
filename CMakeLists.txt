project(QNcidNotify)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

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
   
set(QNcidNotify_SRCS QNcidNotify.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_qncidnotify ${QNcidNotify_SRCS})
target_link_libraries(plasma_applet_qncidnotify 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY})
 
install(TARGETS plasma_applet_qncidnotify
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-qncidnotify.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
