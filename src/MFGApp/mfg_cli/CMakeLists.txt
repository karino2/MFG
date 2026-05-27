cmake_minimum_required(VERSION 3.10.0)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(mfg_cli VERSION 0.1.0)


include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# asan: AddressSanitizer
# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Add libpng
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})

find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})


# linkやqrcなどではbuildがcwdになる事があるので、
# 以下の記述よりもCMakeLists.txtの場所からのパスとする方が良さそう。
# set(REPO_ROOT "..")

set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)


#
# Platfrom depend part.
#

set(PLATFORM_LIB "")

if(CMAKE_HOST_APPLE)

find_library(METAL_LIBRARY Metal)
if (NOT METAL_LIBRARY)
	message(AUTHOR_WARNING "Metal framework dependency not found on system.")
endif ()

# MTLCreateSystemDefaultDevice need CoreGraphics.
# https://developer.apple.com/documentation/metal/1433401-mtlcreatesystemdefaultdevice
find_library(COREGRAPHICS_LIBRARY CoreGraphics)
if (NOT COREGRAPHICS_LIBRARY)
	message(AUTHOR_WARNING "Metal default device require CoreGraphics, but not found on system.")
endif ()


find_library(FOUNDATION_LIBRARY Foundation)
if (NOT FOUNDATION_LIBRARY)
	message(AUTHOR_WARNING "Foundation framework dependency not found on system.")
endif ()

set(GPU_LIB
	${METAL_LIBRARY}
	${COREGRAPHICS_LIBRARY}
	${FOUNDATION_LIBRARY}
)

set(MFG_BACKEND_CPP "")
set(MFG_BACKEND_TEST_CPP mfg_metal_test.cpp)

elseif (WIN32)

set(GPU_LIB "")
set(MFG_BACKEND_CPP ${REPO_ROOT}/_pgn/_mfg/mfg_runtime_d3d.cpp)
set(MFG_BACKEND_TEST_CPP mfg_d3d_test.cpp)
add_compile_options(/bigobj /utf-8)

else ()

set(GPU_LIB "")
set(MFG_BACKEND_CPP "")
set(MFG_BACKEND_TEST_CPP "")

endif ()

if (WIN32)
set(PLATFORM_LIB Winmm.lib)
endif ()

set(COMMON_FILES
		${REPO_ROOT}/_pgn/_mfg/mfg.cpp
		${REPO_ROOT}/_pgn/_mfg/mfg_resource_gen.cpp
		${REPO_ROOT}/_pgn/_mfg/mfg_resource_trans_str.cpp
		${REPO_ROOT}/_pgn/_mfg/mfg_resource_ui_trans_str.cpp
		${REPO_ROOT}/_pgn/_mfg/mfg_samples.cpp
		${MFG_BACKEND_CPP}
	    ${REPO_ROOT}/MFGApp/OpenCommon/open_pal.cpp
		mfg_pal/pal.cpp
		mfg_pal/libpng_codec.cpp
)

set(COMMON_HEADERS
	${REPO_ROOT}/_pgn/_mfg/mfg.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_binary.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_elem.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_ir_util.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_parser.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_printer.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_resource.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_runtime_common.hpp
	${REPO_ROOT}/_pgn/_mfg/mfg_tool.hpp
	${REPO_ROOT}/_pgn/_mfg/forest.hpp
)



add_library(common ${COMMON_FILES})
target_sources(common PRIVATE ${COMMON_HEADERS})

set(COMMON_INCLUDES
    "${REPO_ROOT}/MFGApp/OpenCommon/"
    "${REPO_ROOT}/MFGApp/mfg_cli/"
    "${REPO_ROOT}/_pgn/_mfg"
    "${REPO_ROOT}/_pgn/_tools"
    "${REPO_ROOT}/_pgn/_picojson"
)

target_include_directories(common BEFORE PUBLIC
    ${COMMON_INCLUDES}
 )

 set(COMMON_LINK
 common
 ${ZLIB_LIBRARIES}
 ${PNG_LIBRARY}
 ${GPU_LIB}
 ${PLATFORM_LIB}
)



##
# unit test
##

add_executable(unit_test unit_test_main.cpp forest_test.cpp mfg_test.cpp mfg_script_test.cpp ${MFG_BACKEND_TEST_CPP})

target_include_directories(unit_test BEFORE PUBLIC
${COMMON_INCLUDES}
)

target_link_libraries(unit_test PUBLIC ${COMMON_LINK})


##
# mfg_cli
##
add_executable(mfg_cli cli_main.cpp)

target_include_directories(mfg_cli BEFORE PUBLIC
${COMMON_INCLUDES}
)

target_link_libraries(mfg_cli PUBLIC ${COMMON_LINK})

###
### VS用設定
###
if (MSVC)

  # 複数プロセッサの使用、VisualStudioからビルドする場合
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  # VSからデバッグ実行したいことのあるターゲットはworking directoryを設定しておく。
  set_target_properties(unit_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
