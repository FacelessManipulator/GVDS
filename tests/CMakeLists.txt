cmake_policy(SET CMP0057 NEW) # if IN_LIST


SET(CONFIG_FILE_IN ${PROJECT_SOURCE_DIR}/tests/data/example.cfg.in)
#SET(CONFIG_FILE_OUT ${PROJECT_BINARY_DIR}/tests/data/example.cfg)
SET(CONFIG_FILE_OUT /tmp/hvs/tests/data/example.cfg)


configure_file(${CONFIG_FILE_IN} ${CONFIG_FILE_OUT} @ONLY)

MESSAGE("已经生成配置文件！")

ADD_SUBDIRECTORY(datastore)
ADD_SUBDIRECTORY(common)


