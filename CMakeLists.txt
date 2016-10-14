PROJECT(Holograms)

#SET BUILD-TYPE TO RELEASE IF NOT DEFINED
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)
set(ARCH "x86_64")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE MATCHES "^[Dd][Ee][Bb][Uu][Gg]")
        add_definitions(-DQT_NO_DEBUG_OUTPUT)
        set(CMAKE_BUILD_RELEASE TRUE)
else()
        set(CMAKE_BUILD_RELEASE FALSE)
endif()

if(MSVC OR MSVC_IDE) 
  message( "VC11: use Microsoft Visual Studio 2013 " 
      "with Microsoft Visual C++ Compiler Nov 2012 CTP (v120p_xp)" ) 
    set(CMAKE_GENERATOR_TOOLSET   "v120_xp" CACHE STRING "Platform Toolset" FORCE) 
    set(CMAKE_VS_PLATFORM_TOOLSET  "v120_xp" CACHE STRING "Platform Toolset" FORCE)
else()
  message("Set CC and CXX to point to correct gcc and g++ before first invocation of cmake")
  message("FYI -- OpenCV 2.4.0 seems to depend on gcc < 5.0")

endif()

if(APPLE AND CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
        set(CMAKE_INSTALL_PREFIX "/Applications")
endif()

set(CMAKE_C_FLAGS_RELEASE "-O3")


message(STATUS "Building ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")
cmake_minimum_required(VERSION 2.8.8)

#ADD OPENCV SUPPORT
FIND_PACKAGE( OpenCV REQUIRED )

# Set some Win32 Specific Settings
OPTION(WITH_CONSOLE "Build with a command line for debugging" OFF)

IF(WIN32)
	IF(WITH_CONSOLE)
		ADD_DEFINITIONS(-DWITH_CONSOLE)
	ELSE()
		SET(GUI_TYPE WIN32)
	ENDIF()
ENDIF(WIN32)

#ADD_DEFINITIONS(${QT_DEFINITIONS})

SET(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS})

#SET GENERIC HEADER AND SOURCE FILES
SET(HEADERS 
	src/Socket.h
	src/OctopusClient.h
	src/OfflineReader.h
	src/ImageSource.h
	src/ImageCache.h
	src/ReportWriter.h
	src/tinyxml2.h
	src/Settings.h
)

SET(SOURCES
	src/Socket.cpp
	src/OfflineReader.cpp
	src/OctopusClient.cpp
	src/ImageCache.cpp
	src/ReportWriter.cpp
	src/tinyxml2.cpp
	src/Settings.cpp
)

SOURCE_GROUP(_CORE FILES ${HEADERS} ${SOURCES} )

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/bin)


#BUILD AND LINK
ADD_EXECUTABLE(${PROJECT_NAME} ${GUI_TYPE}
		src/main.cpp
		${SOURCES} 
		${HEADERS}
)

ADD_EXECUTABLE(writeImages ${GUI_TYPE}
		src/writeImages.cpp
		${SOURCES} 
		${HEADERS}
)

IF (APPLE)
  add_dependencies(${PROJECT_NAME} osx_bundle_dirs )
ENDIF (APPLE)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(writeImages ${OpenCV_LIBS})
