cmake_minimum_required(VERSION 3.10)                                                          

include(cmake/modules.cmake)

set(PERIDYNO_GPU_BACKEND "Vulkan" CACHE STRING "GPU Backend")
set_property(CACHE PERIDYNO_GPU_BACKEND PROPERTY STRINGS "Vulkan")

set(SUPPORTED_LANGUAGES "CXX;C")
set(GPU_BACKEND "NO_BACKEND")

if("${PERIDYNO_GPU_BACKEND}" STREQUAL "Vulkan")
	set(GPU_BACKEND "VK_BACKEND")
else()
	set(GPU_BACKEND "NO_BACKEND")
endif()


project(peridyno LANGUAGES ${SUPPORTED_LANGUAGES})

include_directories(external)
include_directories(external/glm-0.9.9.7)
include_directories(external/gli)
include_directories(external/imgui)
include_directories(external/tinygltf)
include_directories(external/ktx/include)
include_directories(external/ktx/other_include)
include_directories(Source/base)

# define library version (update: apparently you can also do it in project()!)
set(PERIDYNO_LIBRARY_VERSION_MAJOR 0 CACHE STRING "peridyno major version" FORCE)
set(PERIDYNO_LIBRARY_VERSION_MINOR 8 CACHE STRING "peridyno minor version" FORCE)
set(PERIDYNO_LIBRARY_VERSION_PATCH 1 CACHE STRING "patch version of peridyno" FORCE)
set(PERIDYNO_LIBRARY_VERSION "${PERIDYNO_LIBRARY_VERSION_MAJOR}.${PERIDYNO_LIBRARY_VERSION_MINOR}.${PERIDYNO_LIBRARY_VERSION_PATCH}" CACHE STRING "Peridyno version" FORCE)

# define library version (update: apparently you can also do it in project()!)
set(PHOENIX_PHYSICS_VERSION_MAJOR 0 CACHE STRING "px major version" FORCE)
set(PHOENIX_PHYSICS_VERSION_MINOR 0 CACHE STRING "px minor version" FORCE)
set(PHOENIX_PHYSICS_VERSION_PATCH 1 CACHE STRING "patch version of px" FORCE)
set(PHOENIX_PHYSICS_VERSION "${PHOENIX_PHYSICS_VERSION_MAJOR}.${PHOENIX_PHYSICS_VERSION_MINOR}.${PHOENIX_PHYSICS_VERSION_PATCH}" CACHE STRING "px version" FORCE)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 11 CACHE STRING "CXX STANDARD VERSION 11,14,17")

set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/data" DST_DATA_DIR)
file(TO_NATIVE_PATH "${PROJECT_SOURCE_DIR}/data" SRC_DATA_DIR)
if (WIN32)
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DST_DATA_DIR})
	execute_process(COMMAND cmd /C mklink /J ${DST_DATA_DIR} ${SRC_DATA_DIR})
else()
	execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DST_DATA_DIR})
	execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SRC_DATA_DIR} ${DST_DATA_DIR})
endif()



#set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

IF(MSVC)
    SET( CMAKE_DEBUG_POSTFIX "d" )
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
ENDIF(MSVC)

# installation destinations
if(UNIX)
    include(GNUInstallDirs)

    set(PERIDYNO_INC_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/peridyno")
    set(PERIDYNO_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
    set(PERIDYNO_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
    set(PERIDYNO_ARCHIVE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
    set(PERIDYNO_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")

    set(PERIDYNO_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/peridyno/cmake")
    set(PERIDYNO_ADDITIONAL_FILES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/peridyno")

    set(PERIDYNO_RUNTIME_INSTALL_DIR "bin") # for the nodesize_dbg, just ignore version and the like
    set(PERIDYNO_INC_INSTALL_DIR "include/peridyno") # header filestree

elseif(WIN32)
    set(PERIDYNO_INC_INSTALL_DIR "include/peridyno")
    set(PERIDYNO_RUNTIME_INSTALL_DIR   "bin")
    set(PERIDYNO_LIBRARY_INSTALL_DIR   "bin")
    set(PERIDYNO_ARCHIVE_INSTALL_DIR   "lib")
    set(PERIDYNO_FRAMEWORK_INSTALL_DIR "bin")

    set(PERIDYNO_CMAKE_CONFIG_INSTALL_DIR "share/peridyno/cmake")
    set(PERIDYNO_ADDITIONAL_FILES_INSTALL_DIR "share/peridyno")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
else()
    message(FATAL_ERROR "Could not set install folders for this platform!")
endif()

if("${PERIDYNO_GPU_BACKEND}" STREQUAL "Vulkan")

	set(PERIDYNO_ASSET_PATH "${CMAKE_SOURCE_DIR}/data" CACHE PATH "" FORCE)
	option(PERIDYNO_LIBRARY_FRAMEWORK "Enable binding the framework library" ON)
	option(PERIDYNO_LIBRARY_IO "Enable binding the io library" ON)
	option(PERIDYNO_LIBRARY_RENDERING "Enable binding the rendering library" ON)
	option(PERIDYNO_LIBRARY_PLUGIN "Enable binding the plugin libraries" OFF)
	
	add_subdirectory(external/glfw-3.3.0)
	add_subdirectory(external/glad-4.6)
	add_subdirectory(external/imgui)

	# Use FindVulkan module added with CMAKE 3.7
	if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
		message(STATUS "Using module to find Vulkan")
		find_package(Vulkan)
	endif()

	IF(UNIX AND NOT APPLE)
		set(LINUX TRUE)
	ENDIF()

	IF(WIN32)
		IF (NOT Vulkan_FOUND)
			find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
			IF (Vulkan_LIBRARY)
				set(Vulkan_FOUND ON)
				MESSAGE("Using bundled Vulkan library version")
			ENDIF()
		ENDIF()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
	ELSEIF(LINUX)
		IF (NOT Vulkan_FOUND)
			find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
			IF (Vulkan_LIBRARY)
				set(Vulkan_FOUND ON)
				MESSAGE("Using bundled Vulkan library version")
			ENDIF()
		ENDIF()
		find_package(Threads REQUIRED)
		IF(USE_D2D_WSI)
			MESSAGE("Using direct to display extension...")
			add_definitions(-D_DIRECT2DISPLAY)
		ELSEIF(USE_DIRECTFB_WSI)
			find_package(DirectFB REQUIRED)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
			include_directories(${DIRECTFB_INCLUDE_DIR})
		ELSEIF(USE_WAYLAND_WSI)
			find_program(PKG_CONFIG pkg-config)
			if (NOT PKG_CONFIG)
				message(FATAL_ERROR "pkg-config binary not found")
			endif ()
			find_package(Wayland REQUIRED)
			if (NOT WAYLAND_FOUND)
				message(FATAL_ERROR "Wayland development package not found")
			endif ()
			pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
			if (NOT WAYLAND_PROTOCOLS_FOUND)
				message(FATAL_ERROR "Wayland protocols package not found")
			endif ()
			find_program(WAYLAND_SCANNER wayland-scanner)
			if (NOT WAYLAND_SCANNER)
				message(FATAL_ERROR "wayland-scanner binary not found")
			endif ()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
			include_directories(${WAYLAND_INCLUDE_DIR})
			execute_process(COMMAND ${PKG_CONFIG} --variable=pkgdatadir wayland-protocols OUTPUT_VARIABLE protocol_dir OUTPUT_STRIP_TRAILING_WHITESPACE)
			execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
					COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
			include_directories(${CMAKE_BINARY_DIR})
		ELSE(USE_D2D_WSI)
			find_package(XCB REQUIRED)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
		ENDIF(USE_D2D_WSI)
	ELSEIF(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_MACOS_MVK -DVK_EXAMPLE_XCODE_GENERATED")
		# Todo : android?
	ENDIF(WIN32)

	IF (NOT Vulkan_FOUND)
		message(FATAL_ERROR "Could not find Vulkan library!")
	ELSE()
		message(STATUS ${Vulkan_LIBRARY})
	ENDIF()

	# Set preprocessor defines
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

	# Clang specific stuff
	if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
	endif()

	if("${PERIDYNO_GPU_BACKEND}" STREQUAL "Vulkan")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glslang)
		
		# for cmake version 3.19 and later, we can directly use system's glslc executable
		if(EXISTS ${Vulkan_GLSLC_EXECUTABLE})

			file(GLOB_RECURSE SHADER_SOURCES 
				"${PROJECT_SOURCE_DIR}/data/shaders/glsl/*.vert"
				"${PROJECT_SOURCE_DIR}/data/shaders/glsl/*.frag"
				#"${PROJECT_SOURCE_DIR}/data/shaders/glsl/*.glsl"
				"${PROJECT_SOURCE_DIR}/data/shaders/glsl/*.comp"
				)		
			source_group(TREE "${PROJECT_SOURCE_DIR}/data/shaders/glsl" FILES ${SHADER_SOURCES})

			set(SHADER_OUTPUT_FILES "")
		
			foreach(file IN ITEMS ${SHADER_SOURCES})
				
				set(SHADER_OUTPUT "${file}.spv")
				add_custom_command(
					OUTPUT "${SHADER_OUTPUT}"
					COMMAND "${Vulkan_GLSLC_EXECUTABLE}" "${file}" -O --target-env=vulkan1.2 -o "${SHADER_OUTPUT}"
					DEPENDS "${file}"
				)
				list(APPEND SHADER_OUTPUT_FILES ${SHADER_OUTPUT})

			endforeach()

			# maybe we prefer to use a 'Shaders' project...
			add_custom_target(CompileShader ALL SOURCES ${SHADER_SOURCES} DEPENDS ${SHADER_OUTPUT_FILES})

		else()
			# use python script...
			add_custom_target(CompileShader ALL COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/data/shaders/glsl/compileshaders.py)
			set_property(TARGET CompileShader PROPERTY FOLDER glslang)
		endif()
		
	endif()


	option(PHOENIX_Examples "Enable building examples" ON)
	if(PHOENIX_Examples)
	    add_subdirectory(examples)
	endif()
else()
	option(PERIDYNO_LIBRARY_FRAMEWORK "Enable binding the framework library" ON)
	option(PERIDYNO_LIBRARY_IO "Enable binding the io library" ON)
	option(PERIDYNO_LIBRARY_RENDERING "Enable binding the rendering library" ON)
	option(PERIDYNO_LIBRARY_PLUGIN "Enable binding the plugin libraries" OFF)
	
	if(PERIDYNO_TESTS)
		add_subdirectory(external/gtest)
	endif()

	add_subdirectory(external/glfw-3.3.0)
	add_subdirectory(external/glad-4.6)
	add_subdirectory(external/imgui)

	option(PERIDYNO_EXAMPLE "Enable building examples" ON)
	if(PERIDYNO_EXAMPLE)
		add_subdirectory(examples)
	endif()
endif()
add_subdirectory(src)


