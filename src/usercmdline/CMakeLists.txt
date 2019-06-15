GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(userlogin userlogin.cc)
target_link_libraries(userlogin hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(usersearch usersearch.cc)
target_link_libraries(usersearch hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(usermodify usermodify.cc)
target_link_libraries(usermodify hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(userexit userexit.cc)
target_link_libraries(userexit hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(usersignup usersignup.cc)
target_link_libraries(usersignup hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(usercancel usercancel.cc)
target_link_libraries(usercancel hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(userlogin_ipc userlogin_ipc.cc ../client/clientUser/clientUser_struct.cc)
target_link_libraries(userlogin_ipc hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)