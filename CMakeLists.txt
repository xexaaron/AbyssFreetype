 
cmake_minimum_required(VERSION 3.28.3)
project(AbyssFT)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_DISABLE_PREDEFINED_TARGETS TRUE)

set(ABY_FT_VER_MAJOR 1)
set(ABY_FT_VER_MINOR 0)
set(ABY_FT_VER_PATCH 0)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CONFIG_POSTFIX "d")
else()
    set(CONFIG_POSTFIX "")
endif()

if (NOT WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    set(SHARED_LIB_EXT ".so")
    set(SHARED_LIB_PREFIX "lib")
    set(CMAKE_LIB_BUILD_DIR "")

    function(set_vendor_properties library folder)
        set_target_properties(${library} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}/${CMAKE_BUILD_TYPE}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}/${CMAKE_BUILD_TYPE}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            FOLDER ${folder}
        )
    endfunction()
else() # WIN_32
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(SHARED_LIB_EXT ".dll")
    set(SHARED_LIB_PREFIX "")
    set(CMAKE_LIB_BUILD_DIR ${CMAKE_BUILD_TYPE})

    function(set_vendor_properties library folder)
        set_target_properties(${library} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Vendor/${library}"
            FOLDER ${folder}
        )
    endfunction()
endif()

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor")
set(STB_IMAGE_INCLUDE_DIR "Vendor/stb")

set(ZLIB_DIR "${VENDOR_DIR}/zlib")
set(ZLIB_BUILD_TESTING OFF CACHE BOOL "Enable Zlib Examples as tests")
add_subdirectory(${ZLIB_DIR})
set(ZLIB_INCLUDE_DIR ${ZLIB_DIR})
set(ZLIB_LIBRARY ${ZLIB_DIR}/build/${CMAKE_BUILD_TYPE})
set_vendor_properties(zlib "Dependencies/zlib")
set_vendor_properties(zlibstatic "Dependencies/zlib")

set(PNG_DIR "${VENDOR_DIR}/libpng")
add_subdirectory(${PNG_DIR})
target_include_directories(png_shared PRIVATE ${ZLIB_INCLUDE_DIR})
target_include_directories(png_static PRIVATE ${ZLIB_INCLUDE_DIR})
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
if (NOT WIN32)
    set_target_properties(png_static PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set(BZIP2_DIR "${VENDOR_DIR}/bzip2")
add_subdirectory(${BZIP2_DIR})
set(BZIP2_INCLUDE_DIR ${BZIP2_DIR})
set(BZIP2_LIBRARIES ${BZIP2_DIR}/build/${CMAKE_BUILD_TYPE})
set_vendor_properties(bzip2 "Dependencies/bzip2")
set_vendor_properties(bz2 "Dependencies/bzip2")
set_vendor_properties(bz2_ObjLib "Dependencies/bzip2")
set_vendor_properties(bzip2recover "Dependencies/bzip2")
set_vendor_properties(check "Dependencies/bzip2")

set(BROTLI_DIR "${VENDOR_DIR}/brotli")
add_subdirectory(${BROTLI_DIR})
set(BROTLIDEC_INCLUDE_DIRS ${BROTLI_DIR}/c/include)
set(BROTLI_LIB_DIR ${BROTLI_DIR}/build/${CMAKE_BUILD_TYPE})
set(BROTLIDEC_LIBRARIES brotlicommon brotlidec brotlienc)
set_vendor_properties(brotli "Dependencies/brotli")
set_vendor_properties(brotlicommon "Dependencies/brotli")
set_vendor_properties(brotlidec "Dependencies/brotli")
set_vendor_properties(brotlienc "Dependencies/brotli")

set(FREETYPE_DIR "${VENDOR_DIR}/freetype")
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

set(CMDLINE_DIR "${VENDOR_DIR}/CmdLine")
add_subdirectory(${CMDLINE_DIR})
set(CMDLINE_INCLUDE_DIR "${CMDLINE_DIR}/Source/Public")
set(ABY_PP_INCLUDE_DIR "${CMDLINE_DIR}/Vendor/AbyssPrettyPrint/Source/Public")

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

source_group("Private" FILES ${CPP_SOURCES} Source/Private/main.cpp)
source_group("Public" FILES ${CPP_HEADERS})
source_group("Vendor" FILES Vendor/stb/stb/stb_image_write.cpp Vendor/stb/stb/stb_image_write.h)
source_group("Tests" FILES ${TEST_SOURCES})

add_library(${PROJECT_NAME}Lib STATIC ${CPP_SOURCES} ${CPP_HEADERS})
target_include_directories(${PROJECT_NAME}Lib PUBLIC "Source/Public" ${FREETYPE_INCLUDE_DIRS} ${STB_IMAGE_INCLUDE_DIR} ${ABY_PP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}Lib PRIVATE freetype PrettyPrint)
target_compile_options(${PROJECT_NAME}Lib PRIVATE ${COMPILE_OPTS})
add_dependencies(${PROJECT_NAME}Lib freetype)
set_target_properties(${PROJECT_NAME}Lib PROPERTIES FOLDER "Abyss")
target_compile_definitions(${PROJECT_NAME}Lib PUBLIC 
    ABY_FT_VER_MAJOR=${ABY_FT_VER_MAJOR}
    ABY_FT_VER_MINOR=${ABY_FT_VER_MINOR}
    ABY_FT_VER_PATCH=${ABY_FT_VER_PATCH}
)

add_custom_command(
    TARGET ${PROJECT_NAME}Lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Source/Tests/Fonts"
            "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/AbyssFreetypeTests/Fonts"
    COMMENT "Copy fonts used by tests"
)

set(FREETYPE_DLL_IN     "${CMAKE_BINARY_DIR}/Vendor/freetype/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}freetype${CONFIG_POSTFIX}${SHARED_LIB_EXT}")
set(FREETYPE_DLL_OUT    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}freetype${CONFIG_POSTFIX}${SHARED_LIB_EXT}")
set(ZLIB_DLL_IN         "${CMAKE_BINARY_DIR}/Vendor/zlib/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}z${CONFIG_POSTFIX}${SHARED_LIB_EXT}")
set(ZLIB_DLL_OUT        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}z${CONFIG_POSTFIX}${SHARED_LIB_EXT}")
set(BZIP2_DLL_IN        "${CMAKE_BINARY_DIR}/Vendor/bz2/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}bz2${SHARED_LIB_EXT}")
set(BZIP2_DLL_OUT       "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}bz2-1${SHARED_LIB_EXT}")
set(BROTLI_COMMON_IN    "${CMAKE_BINARY_DIR}/Vendor/brotlicommon/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}brotlicommon${SHARED_LIB_EXT}")
set(BROTLI_COMMON_OUT   "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}brotlicommon${SHARED_LIB_EXT}")
set(BROTLI_DEC_IN       "${CMAKE_BINARY_DIR}/Vendor/brotlidec/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}brotlidec${SHARED_LIB_EXT}")
set(BROTLI_DEC_OUT      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}brotlidec${SHARED_LIB_EXT}")
set(BROTLI_ENC_IN       "${CMAKE_BINARY_DIR}/Vendor/brotlienc/${CMAKE_BUILD_TYPE}/${SHARED_LIB_PREFIX}brotlienc${SHARED_LIB_EXT}")
set(BORTLI_ENC_OUT      "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SHARED_LIB_PREFIX}brotlienc${SHARED_LIB_EXT}")

add_custom_command(TARGET ${PROJECT_NAME}Lib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FREETYPE_DLL_IN} ${FREETYPE_DLL_OUT}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZLIB_DLL_IN} ${ZLIB_DLL_OUT}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BZIP2_DLL_IN} ${BZIP2_DLL_OUT}
    COMMENT "Copy necessary DLLs"
    USES_TERMINAL
)

if (EXISTS ${BROTLI_COMMON_IN} AND EXISTS ${BROTLI_DEC_IN} AND EXISTS ${BROTLI_ENC_IN})
    add_custom_command(TARGET ${PROJECT_NAME}Lib POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BROTLI_COMMON_IN} ${BROTLI_COMMON_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BROTLI_DEC_IN} ${BROTLI_DEC_OUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BROTLI_ENC_IN} ${BORTLI_ENC_OUT}
        COMMENT "Copy necessary DLLs"
        USES_TERMINAL
    )
endif()

add_executable(${PROJECT_NAME}Test ${TEST_SOURCES})
target_include_directories(${PROJECT_NAME}Test PRIVATE "Source/Public")
target_link_libraries(${PROJECT_NAME}Test PRIVATE ${PROJECT_NAME}Lib)
target_compile_options(${PROJECT_NAME}Test PRIVATE ${COMPILE_OPTS})
add_dependencies(${PROJECT_NAME}Test ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME}Test PROPERTIES FOLDER "Abyss/Tests")

add_executable(${PROJECT_NAME} Source/Private/main.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE "Source/Public" ${CMDLINE_INCLUDE_DIR} ${ABY_PP_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Lib CmdLine)
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTS})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}Lib)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Abyss")