set(DIR ${PROJECTDIR}/src)

set(MODULE_IFLAGS ${DIR})
set(INCLUDE_DIR ${DIR})
include_directories(${INCLUDE_DIR})
set(MODULE_OPTIONS -Wno-multichar -Wno-unused-parameter -Wno-unused-const-variable)

set(CSRC ${DIR}/main.c ${DIR}/usr_main.c ${DIR}/bt.c)
# set(CSRC ${DIR}/main.c ${DIR}/usr_main.c)


ameba_internal_library(project_src)
target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${MODULE_IFLAGS})
target_sources(${CURRENT_LIB_NAME} PRIVATE ${CSRC})

