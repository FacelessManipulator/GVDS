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