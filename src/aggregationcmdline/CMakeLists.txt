GET_PROPERTY(HVSCOMMOM_DEPENDS_ON GLOBAL PROPERTY "HVSCOMMOM_DEPENDS_ON")

add_executable(rg resourceregister.cc)
target_link_libraries(rg hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(rq resourcequery.cc)
target_link_libraries(rq hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(rd resourcedelete.cc)
target_link_libraries(rd hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

add_executable(ru resourceupdate.cc)
target_link_libraries(ru hvscommon ${HVSCOMMOM_DEPENDS_ON} managerO)

install(TARGETS rg rq rd ru
RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
