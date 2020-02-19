cmake_policy(SET CMP0057 NEW) # if IN_LIST


SET(CONFIG_FILE_IN ${PROJECT_SOURCE_DIR}/tests/data/example.cfg.in)
SET(CONFIG_FILE_OUT /tmp/hvs/tests/data/example.cfg)
configure_file(${CONFIG_FILE_IN} ${CONFIG_FILE_OUT} @ONLY)
MESSAGE("生成配置文件:example.cfg")

# 生成测试文件
SET(CONFIG_Test_IN ${PROJECT_SOURCE_DIR}/tests/data/syncio.txt)
SET(CONFIG_Test_OUT /tmp/hvs/tests/data/syncio.txt)
configure_file(${CONFIG_Test_IN} ${CONFIG_Test_OUT} @ONLY)
MESSAGE("生成测试文件：syncio.txt")


ADD_SUBDIRECTORY(datastore)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(account)
ADD_SUBDIRECTORY(zone)
#ADD_SUBDIRECTORY(space)
#ADD_SUBDIRECTORY(msg)
#ADD_SUBDIRECTORY(ioproxy)
ADD_SUBDIRECTORY(manager)

