set(DIR ${PROJECTDIR}/src)

set(MODULE_IFLAGS ${DIR})
set(INCLUDE_DIR ${DIR})
message(STATUS "INCLUDE_DIR: ${INCLUDE_DIR}")

include_directories(${INCLUDE_DIR})
# set(MODULE_OPTIONS -Wno-multichar -Wno-unused-parameter -Wno-unused-const-variable)
# set(MODULE_OPTIONS -w)

add_compile_definitions(C_BW20_5G)

set(CSRC ${DIR}/main.c ${DIR}/YE-DevAuth.c)


ameba_internal_library(project_src)
target_include_directories(${CURRENT_LIB_NAME} PRIVATE ${MODULE_IFLAGS})
target_sources(${CURRENT_LIB_NAME} PRIVATE ${CSRC})

