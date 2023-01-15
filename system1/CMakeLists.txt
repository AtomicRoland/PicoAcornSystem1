aux_source_directory(. DIR_SYSTEM1_SRCS)

include_directories(../lib/config)
include_directories(../lib/lcd)
include_directories(../lib/font)
include_directories(../lib/fatfs)
include_directories(../lib/sdcard)

add_library(system1 ${DIR_SYSTEM1_SRCS})
target_link_libraries(system1 PUBLIC config lcd font fatfs sdcard)
