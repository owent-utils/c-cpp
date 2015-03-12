
# µ¼ÈëGTest  
add_subdirectory("${PROJECT_SOURCE_DIR}/test/3rd_party/src/gtest-1.7.0")
include_directories ("${PROJECT_SOURCE_DIR}/test/3rd_party/src/gtest-1.7.0/include")

aux_source_directory(src SRC_LIST_TEST)

add_executable(owent_utils_test ${SRC_LIST_TEST})

target_link_libraries(owent_utils_test owent_utils gtest ${EXTENTION_LINK_LIB})

			
