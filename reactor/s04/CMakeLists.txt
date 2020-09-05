
file(GLOB SRC "*.cpp")
file(GLOB IgnoreFiles "test[0-9]*.cpp")
list(REMOVE_ITEM SRC ${IgnoreFiles})
aux_source_directory(../../thread/ THREAD_SRC)
aux_source_directory(../../datetime/ DATETIME_SRC)
aux_source_directory(../../logging/ LOG_SRC)

add_executable(test7 test7.cpp ${SRC} ${THREAD_SRC} ${DATETIME_SRC} ${LOG_SRC})

find_package(Threads)
target_link_libraries(test7 ${CMAKE_THREAD_LIBS_INIT})
