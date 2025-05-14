 
cmake_minimum_required(VERSION 3.28.3)
project(AbyssFT)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_DISABLE_PREDEFINED_TARGETS TRUE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

function(set_vendor_properties library folder)
    set_target_properties(${library} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
        FOLDER ${folder}
    )
endfunction()

set(STB_IMAGE_INCLUDE_DIR "Vendor/stb")

set(ZLIB_DIR "Vendor/zlib")
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "Enable Zlib Examples as tests")
add_subdirectory(${ZLIB_DIR})
set(ZLIB_INCLUDE_DIR ${ZLIB_DIR})
set(ZLIB_LIBRARY ${ZLIB_DIR}/build/${CMAKE_BUILD_TYPE})
set_vendor_properties(zlib "Dependencies/zlib")
set_vendor_properties(zlibstatic "Dependencies/zlib")

set(PNG_DIR "Vendor/libpng")
add_subdirectory(${PNG_DIR})
set(PNG_PNG_INCLUDE_DIR ${PNG_DIR})
set(PNG_LIBRARY ${PNG_DIR}/build/${CMAKE_BUILD_TYPE})
set_vendor_properties(png_shared "Dependencies/png")
set_vendor_properties(png_static "Dependencies/png")
set_vendor_properties(png_genfiles "Dependencies/png")
set_vendor_properties(png_shared "Dependencies/png")
set_vendor_properties(pngfix "Dependencies/png")
set_vendor_properties(png-fix-itxt "Dependencies/png")
set_vendor_properties(pngimage "Dependencies/png")
set_vendor_properties(pngstest "Dependencies/png")
set_vendor_properties(pngtest "Dependencies/png")
set_vendor_properties(pngunknown "Dependencies/png")
set_vendor_properties(pngvalid "Dependencies/png")

set(BZIP2_DIR "Vendor/bzip2")
add_subdirectory(${BZIP2_DIR})
set(BZIP2_INCLUDE_DIR ${BZIP2_DIR})
set(BZIP2_LIBRARIES ${BZIP2_DIR}/build/${CMAKE_BUILD_TYPE})
set_vendor_properties(bzip2 "Dependencies/bzip2")
set_vendor_properties(bz2 "Dependencies/bzip2")
set_vendor_properties(bz2_ObjLib "Dependencies/bzip2")
set_vendor_properties(bzip2recover "Dependencies/bzip2")
set_vendor_properties(check "Dependencies/bzip2")

set(BROTLI_DIR "Vendor/brotli")
add_subdirectory(${BROTLI_DIR})
set(BROTLIDEC_INCLUDE_DIRS ${BROTLI_DIR}/c/include)
set(BROTLI_LIB_DIR ${BROTLI_DIR}/build/${CMAKE_BUILD_TYPE})
set(BROTLIDEC_LIBRARIES brotlicommon brotlidec brotlienc)
set_vendor_properties(brotli "Dependencies/brotli")
set_vendor_properties(brotlicommon "Dependencies/brotli")
set_vendor_properties(brotlidec "Dependencies/brotli")
set_vendor_properties(brotlienc "Dependencies/brotli")

set(FREETYPE_DIR "Vendor/freetype")
set(FREETYPE_INCLUDE_DIR "${FREETYPE_DIR}/include")
set(FREETYPE_DEVEL_DIR "${FREETYPE_DIR}/devel")
set(FREETYPE_LIBRARY_DIR "${FREETYPE_DIR}/objs")
set(FREETYPE_INCLUDE_DIRS ${FREETYPE_DIR} ${FREETYPE_DEVEL_DIR})
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "" FORCE)
set(FT_REQUIRE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_REQUIRE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_REQUIRE_PNG ON CACHE BOOL "" FORCE)
set(FT_REQUIRE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(SKIP_INSTALL_ALL ON CACHE BOOL "Skip installation of all targets" FORCE)
add_subdirectory(${FREETYPE_DIR})
target_include_directories(freetype PRIVATE ${BZIP2_DIR} ${BROTLIDEC_INCLUDE_DIRS})
set_vendor_properties(freetype "Dependencies/freetype")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
add_dependencies(freetype zlib bzip2 brotli png_static)

if (MSVC)
    # W4                    warn level 4
    # wd4100                unreferenced formal parameter
    # Zc:preprocessor       standard conformant preprocssor (allow __VA_OPT__)
    set(COMPILE_OPTS /W4 /wd4100 /Zc:preprocessor)
else()
    set(COMPILE_OPTS -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-unused-function)
endif()



set(CPP_SOURCES
    Source/Private/abyft.cpp
    Source/Private/serializer.cpp
    Vendor/stb/stb/stb_image_write.cpp
)

set(CPP_HEADERS
    Source/Public/FT/abyft.h
    Source/Public/FT/serializer.h
    Vendor/stb/stb/stb_image_write.h
)

set(TEST_SOURCES Source/Tests/Test.cpp)


source_group("Private" FILES ${CPP_SOURCES})
source_group("Public" FILES ${CPP_HEADERS})
source_group("Vendor" FILES Vendor/stb/stb/stb_image_write.cpp Vendor/stb/stb/stb_image_write.h)
source_group("Tests" FILES ${TEST_SOURCES})

add_library(${PROJECT_NAME} STATIC ${CPP_SOURCES} ${CPP_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC "Source/Public" ${FREETYPE_INCLUDE_DIRS} ${STB_IMAGE_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE freetype)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTS})
add_dependencies(${PROJECT_NAME} freetype)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Abyss")

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/Source/Tests/Fonts"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Fonts"
    COMMENT "Copy fonts used by tests"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIG_POSTFIX "d")
else()
    set(CONFIG_POSTFIX "")
endif()


if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(FREETYPE_DLL_IN  "${CMAKE_BINARY_DIR}/Vendor/freetype/${CMAKE_BUILD_TYPE}/freetype${CONFIG_POSTFIX}.dll")
    set(FREETYPE_DLL_OUT "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/freetype${CONFIG_POSTFIX}.dll")
    set(ZLIB_DLL_IN  "${CMAKE_BINARY_DIR}/Vendor/zlib/${CMAKE_BUILD_TYPE}/z${CONFIG_POSTFIX}.dll")
    set(ZLIB_DLL_OUT "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/z${CONFIG_POSTFIX}.dll")
    set(BZIP2_DLL_IN  "${CMAKE_BINARY_DIR}/Vendor/bz2/${CMAKE_BUILD_TYPE}/bz2.dll")
    set(BZIP2_DLL_OUT "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bz2-1.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_DLL_IN} ${FREETYPE_DLL_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZLIB_DLL_IN} ${ZLIB_DLL_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BZIP2_DLL_IN} ${BZIP2_DLL_OUT}
        COMMENT "Copy necessary DLLs"
    )
endif()


add_executable(${PROJECT_NAME}Test ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}Test PRIVATE "Source/Public")
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${PROJECT_NAME})
target_compile_options(${PROJECT_NAME}Test PRIVATE ${COMPILE_OPTS})
add_dependencies(${PROJECT_NAME}Test ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}Test PROPERTIES FOLDER "Abyss/Tests")
