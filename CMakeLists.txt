project(QNcidNotify)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtSql QtDBus REQUIRED)

add_definitions (${QT_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${QT_INCLUDES}
   ${CMAKE_CURRENT_BINARY_DIR})

set(DATADIR "${CMAKE_INSTALL_PREFIX}/share")

configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
qt4_add_resources(QNcidNotify_RCC QNcidNotify.qrc)
   
set(QNcidNotify_SRCS QNcidLogDialog.cpp QNcidSocket.cpp QNcidOptionsDialog.cpp main.cpp QNcidNotify.cpp ${QNcidNotify_RCC})
qt4_automoc(${QNcidNotify_SRCS})
add_executable(qncidnotify ${QNcidNotify_SRCS} )
target_link_libraries(qncidnotify ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTSQL_LIBRARY} ${QT_QTDBUS_LIBRARY})
 
install(TARGETS qncidnotify DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
 
install(FILES plasma-applet-qncidnotify.desktop DESTINATION ${DATADIR}/applications/)
