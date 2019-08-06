GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(centermodify modifycenter.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(centermodify hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(centersearch searchcenter.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(centersearch hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(centerdelete deletecenter.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(centerdelete hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS centermodify centersearch centerdelete
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
