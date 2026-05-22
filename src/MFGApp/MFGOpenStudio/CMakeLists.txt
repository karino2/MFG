cmake_minimum_required(VERSION 3.10.0)

set(STUDIO_APP_VERSION "1.0.0")
set(STUDIO_APP_SHORT_VERSION "100")


# qtのCMake関連がCMAKE_OSX_DEPLOYMENT_TARGETを12にハードコードしているので、
# デフォルトのvcpkgだとターゲットが違うというワーニングが出る。
# そこでvcpkgも12に揃える。
# CMakePresets.jsonなどはVCPKG_OSX_DEPLOYMENT_TARGETが利かないようなので
# 新たなtripletを作ってそれを使う事にしてる。
# vcpkg.jsonでoverlay-tripletsを指定し、以下のtripletをcustom_vcpkg_triplets下に用意している
#
if(CMAKE_HOST_APPLE)
    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_APPLE_SILICON_PROCESSOR)
        set(VCPKG_TARGET_TRIPLET "arm64-osx-12")
	else()
		set(VCPKG_TARGET_TRIPLET "x64-osx-12")
	endif()
endif()


# qtのqt-cmakeはqtのCMKE_TOOLCHAIN_FILEを使う
# 一方vcpkgも独自のCMAKE_TOOLCHAIN_FILEを前提としている。
# 両方を指定する事は出来ない。
# vcpkgは二番目のCMAKE_TOOLCHAIN_FILEをVCPKG_CHAINLOAD_TOOLCHAIN_FILEとして指定する事が出来るらしいので、
# それを指定する。
# 詳細は以下の「Using Multiple Toolchain Files」を参照: [vcpkg in CMake projects - Microsoft Learn](https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration#using-multiple-toolchain-files)
# これは以下と似た事をしている。 https://github.com/alcroito/qt_world_summit_2019_cmake_vcpkg_app/blob/master/cmake/app_utils.cmake
# なおこのCMakeLists.txtはqt-cmakeから使われる前提なので、
# この時点でqtのCMAKE_TOOLCHAIN_FILEが設定されている前提。
if(DEFINED ENV{VCPKG_ROOT})
		set(vcpkg_toolchain_path "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
		get_filename_component(vcpkg_toolchain_path "${vcpkg_toolchain_path}" ABSOLUTE)		
		get_filename_component(supplied_toolchain_file "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
		if(NOT supplied_toolchain_file STREQUAL vcpkg_toolchain_path)
				set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" CACHE STRING "")
		endif()		
    if(vcpkg_toolchain_path AND EXISTS "${vcpkg_toolchain_path}")
        set(CMAKE_TOOLCHAIN_FILE "${vcpkg_toolchain_path}" CACHE STRING "" FORCE)
        message(STATUS "Using vcpkg from $ENV{VCPKG_ROOT}")
    endif()
endif()

# message(STATUS ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE})

project(MFGOpenStudio VERSION 1.0.0)


include(CTest)
enable_testing()

# Qt6 require C++ 17
# https://doc.qt.io/qt-6/cmake-get-started.html
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# asan: AddressSanitizer
# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
# set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
# add_compile_definitions($<$<CONFIG:Debug>:DEBUG>)

# linkやqrcなどではbuildがcwdになる事があるので、
# 以下の記述よりもCMakeLists.txtの場所からのパスとする方が良さそう。
# set(MYLIB "../..")

set(MYLIB ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(STUDIOLIB ${CMAKE_CURRENT_SOURCE_DIR}/../StudioLib)
set(OPENCOMMON ${CMAKE_CURRENT_SOURCE_DIR}/../OpenCommon)


# Qt6
# find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS
 Core
 Widgets
 )

set(QT_DEPS
	Qt6::Widgets
	Qt6::Core
	)

qt_standard_project_setup()

# ui_scrollbar.cppとかで勝手に動くと困るのでqt_wrap_uiで直接指定
set(CMAKE_AUTOUIC OFF)
# めちゃ遅いので_qtとFilreAlpaca3ディレクトリだけ手でqt_wrap_cppする。
set(CMAKE_AUTOMOC OFF)

# minizip-ng
find_package(minizip-ng REQUIRED)
# if (minizip-ng_FOUND)
#  message(STATUS "minizip found")
# endif()

set(MINIZIP_LIB MINIZIP::minizip-ng)

# find_package(unofficial-minizip REQUIRED)
# set(MINIZIP_LIB unofficial::minizip::minizip)

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
	set(PLATFORM_LIB z)

elseif (WIN32)

	set(GPU_LIB "")
	set(MFG_BACKEND_CPP ${MYLIB}/_pgn/_mfg/mfg_runtime_d3d.cpp)
	set(PLATFORM_LIB Winmm.lib imm32 gdi32 gdiplus dwmapi user32 advapi32)

else ()

	set(GPU_LIB "")
	set(MFG_BACKEND_CPP "")

endif ()

set(NONUI_SOURCES
	${MYLIB}/_pgn/_mfg/mfg.cpp
	${MYLIB}/_pgn/_mfg/mfg_resource_gen.cpp
	${MYLIB}/_pgn/_mfg/mfg_resource_trans_str.cpp
	${MYLIB}/_pgn/_mfg/mfg_resource_ui_trans_str.cpp
	${MYLIB}/_pgn/_mfg/mfg_samples.cpp
	${OPENCOMMON}/open_pal.cpp
	mfg_pal/pal.cpp
	mfg_pal/qtpng_codec.cpp
	${MFG_BACKEND_CPP}
	trans_str_filter.cpp
	trans_str_main.cpp
	trans_str_menu_edit.cpp
	trans_str_menu_file.cpp
	trans_str_menu_help.cpp
	trans_str_menu_view.cpp
	trans_str_menu_window.cpp
	trans_str_mfg.cpp
	trans.cpp
)



set(NONUI_HEADERS
	${MYLIB}/_pgn/_mfg/mfg.hpp
	${MYLIB}/_pgn/_mfg/mfg_binary.hpp
	${MYLIB}/_pgn/_mfg/mfg_elem.hpp
	${MYLIB}/_pgn/_mfg/mfg_ir_util.hpp
	${MYLIB}/_pgn/_mfg/mfg_parser.hpp
	${MYLIB}/_pgn/_mfg/mfg_printer.hpp
	${MYLIB}/_pgn/_mfg/mfg_resource.hpp
	${MYLIB}/_pgn/_mfg/mfg_runtime_common.hpp
	${MYLIB}/_pgn/_mfg/mfg_tool.hpp
	${MYLIB}/_pgn/_mfg/forest.hpp
)

qt_add_library(nonui_part STATIC ${NONUI_SOURCES})
target_sources(nonui_part PRIVATE ${NONUI_HEADERS})
target_link_libraries(nonui_part PRIVATE ${QT_DEPS})

target_include_directories(nonui_part BEFORE PRIVATE
	${PROJECT_SOURCE_DIR}
	${OPENCOMMON}
	${STUDIOLIB}
	${MYLIB}/_pgn/_mfg
	${MYLIB}/_pgn/_picojson
	${MYLIB}/_pgn/_tools
)


if(CMAKE_HOST_APPLE)
	# Releaseだけユニバーサルバイナリに
	set_target_properties(nonui_part PROPERTIES OSX_ARCHITECTURES_RELEASE "arm64;x86_64")
endif ()

# AUTOMOCはめちゃ遅いので手で指定
# Q_OBJECTを使っているヘッダファイルを列挙
set(QT_MOC_TARGET
	${STUDIOLIB}/widget_mfg_editor.h
	${STUDIOLIB}/dialog_new_project.h
	${STUDIOLIB}/mfgstdapplication.h
	${STUDIOLIB}/widget_code_editor.h
	${STUDIOLIB}/widget_output.h
	${STUDIOLIB}/widget_project_explorer.h
	qt_pal/mfg_widget_pal.h
	dialog_filter_mfg.h
	widget_processing.h
	widget_resource.h
	widget_direction.h
	widget_center_pointer.h
	widget_central.h
	widget_simple_color_picker.h
	dialog_about.h
	mainwindow_impl.h
	uiiconmanager.h
	widget_preview.h
	widget_target_view.h
	window_state_manager.h
)



set(QT_SOURCES
	${STUDIOLIB}/widget_mfg_editor.cpp
	${STUDIOLIB}/project.cpp
	${STUDIOLIB}/mfgstdapplication.cpp
	${STUDIOLIB}/widget_code_editor.cpp
	${STUDIOLIB}/widget_output.cpp
	${STUDIOLIB}/widget_project_explorer.cpp
	${STUDIOLIB}/dialog_new_project.cpp
	${STUDIOLIB}/mainwindow_common.cpp
	qt_pal/mfg_target.cpp
	qt_pal/mfg_widget_pal.cpp
	qt_pal/qt_pal.cpp
	qt_pal/studio_pal.cpp
	dialog_filter_mfg.cpp
	widget_processing.cpp
	widget_simple_color_picker.cpp
	widget_direction.cpp
	widget_center_pointer.cpp
	widget_resource.cpp
	widget_central.cpp
	dialog_about.cpp
	main.cpp
	mainwindow.cpp
	man.cpp
	uiiconmanager.cpp
	widget_preview.cpp
	widget_target_view.cpp
	window_state_manager.cpp
	app_qt.cpp
)

# このファイルだけUIC
qt_wrap_ui(QT_SOURCES mainwindow.ui)

# QT_MOC_TARGETをmocで処理、結果をQT_SOURCESに加える
qt_wrap_cpp(QT_SOURCES
 ${QT_MOC_TARGET}
)

# resourceを追加
qt_add_resources(QT_SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/resource.qrc
	${CMAKE_CURRENT_SOURCE_DIR}/resource_images.qrc
)

set(MFG_STUDIO_SOURCE
	${QT_SOURCES}
)

qt_add_executable(MFGOpenStudio 
	${MFG_STUDIO_SOURCE}
)

target_include_directories(MFGOpenStudio BEFORE PRIVATE
	${PROJECT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} #qt_wrap_uiがui_mainwindow.hをbuildのディレクトリに作るので。
	${OPENCOMMON}
	${STUDIOLIB}
	${MYLIB}/_pgn/_mfg
	${MYLIB}/_pgn/_picojson
	${MYLIB}/_pgn/_tools
)


target_link_libraries(MFGOpenStudio PRIVATE
nonui_part
${GPU_LIB}
${PLATFORM_LIB}
${QT_DEPS}
${MINIZIP_LIB}
)


# 異なるビルドバリアントのあるCMakeListsから持ってきたので残っている名残。
# 複数のビルドバリアントで共通の処理
# target: MFGOpenStudio
# macicns: app.icns
# winrc: app.rc
function(common_target_process target macicns winrc)
	target_compile_definitions(${target} PRIVATE
		STUDIO_APP_VERSION="${STUDIO_APP_VERSION}"
	)

	set_target_properties(${target} PROPERTIES
		WIN32_EXECUTABLE ON
		MACOSX_BUNDLE ON
	)

	if(CMAKE_HOST_APPLE)

		set_source_files_properties(${macicns} PROPERTIES
				MACOSX_PACKAGE_LOCATION "Resources"
		)
		target_sources(${target} PRIVATE ${macicns})
		set_target_properties(${target} PROPERTIES
			MACOSX_BUNDLE_BUNDLE_VERSION "${STUDIO_APP_VERSION}"
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${STUDIO_APP_SHORT_VERSION}"
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in")
		# Releaseだけユニバーサルバイナリに
		set_target_properties(${target} PROPERTIES OSX_ARCHITECTURES_RELEASE "arm64;x86_64")

		execute_process(
			COMMAND qmake -query QT_INSTALL_TRANSLATIONS
			OUTPUT_VARIABLE QT_TRANSLATION_DIR
		)	
		string(STRIP "${QT_TRANSLATION_DIR}" QT_TRANSLATION_DIR)
		
		add_custom_command(TARGET ${target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
					${QT_TRANSLATION_DIR}
					$<TARGET_FILE_DIR:${target}>/../Resources
		)
	elseif (WIN32)
		target_sources(${target} PRIVATE ${winrc})

		# editbinへのパスは通しておいて（探し方が分からなかった）
		add_custom_command(TARGET ${target} POST_BUILD
			COMMAND "editbin.exe" "/LARGEADDRESSAWARE $<TARGET_FILE:${target}>")
	else ()
		message(AUTHOR_WARNING "Neither apple nor win32, Unknown target for resource.")
	endif ()
endfunction()

common_target_process(MFGOpenStudio app.icns app.rc)


###
### VS用設定
###

if (MSVC)
	# デバッグ実行の時の環境変数を設定する
	execute_process(
		COMMAND qmake -query QT_INSTALL_LIBEXECS
		OUTPUT_VARIABLE QT6_DLL_DIR
		)	
	string(STRIP "${QT6_DLL_DIR}" QT6_DLL_DIR)

  # message("PATH=${QT6_DLL_DIR};$(VC_ExecutablePath_x64);%PATH%")
  set_target_properties(MFGOpenStudio PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=${QT6_DLL_DIR};$(VC_ExecutablePath_x64);%PATH%")

  # 複数プロセッサの使用、VisualStudioからビルドする場合
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

  # startup projectをMFGOpenStudioにする
  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MFGOpenStudio)
endif()


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
