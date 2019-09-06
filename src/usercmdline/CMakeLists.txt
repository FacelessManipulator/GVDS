GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(userlogin userlogin_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(userlogin hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(usersearch usersearch_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(usersearch hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(usersignup usersignup_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(usersignup hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(usermodify usermodify_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(usermodify hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(userexit userexit_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(userexit hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(usercancel usercancel_ipc.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(usercancel hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adminsignup adminsignup.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(adminsignup hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(listapply listapply.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(listapply hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(suggestion suggestion.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(suggestion hvscommon ${HVSCOMMOM_DEPENDS_ON})

add_executable(adcam adcreateaccountmap.cc ../client/clientuser/ClientUser_struct.cc)
target_link_libraries(adcam hvscommon ${HVSCOMMOM_DEPENDS_ON})

install(TARGETS userlogin usersearch usersignup usermodify userexit usercancel adminsignup suggestion adcam
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
