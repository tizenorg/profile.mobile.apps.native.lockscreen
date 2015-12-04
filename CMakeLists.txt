CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lockscreen)
# FIND ALL SOURCE IN A SOURCE DIRECTORY
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/src SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_SOURCE_DIR}/src/daemon SRCS)

#INCLUDE FILE
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
SET(PACKAGE "${PROJECT_NAME}")
SET(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/res")
SET(EDJDIR "${RESOURCE_DIR}/edje/${PACKAGE}")
SET(LOCALEDIR "/usr/apps/org.tizen.lockscreen/res/locale")

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	bundle
	appcore-efl
	evas
	edje
	elementary
	ecore
	eina
	capi-system-sensor
	capi-system-info
	capi-system-runtime-info
	capi-system-system-settings
	capi-system-device
	capi-appfw-application
	capi-appfw-preference
	capi-appfw-app-manager
	aul
	ail
	notification
	minicontrol-viewer
	security-server
	minicontrol-monitor
	ui-gadget-1
	feedback
	msg-service
	tapi
	efl-extension
	key-manager
	accounts-svc
	dbus-1
	dbus-glib-1
	deviced
)

IF(X11_SUPPORT)
pkg_check_modules(pkgs REQUIRED
	ecore-x
	utilX
	)
ENDIF(X11_SUPPORT)

IF(WAYLAND_SUPPORT)
pkg_check_modules(pkgs REQUIRED
	ecore-wayland
	)
ENDIF(WAYLAND_SUPPORT)

ADD_DEFINITIONS("-DPROJECT=\"${PROJECT_NAME}\"")

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIE")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fPIE")
SET(CMAKE_C_FLAGS_PROFILING    "-O0 -g -pg -fPIE")
SET(CMAKE_CXX_FLAGS_PROFILING  "-O0 -std=c++0x -g -pg -fPIE")
SET(CMAKE_C_FLAGS_DEBUG        "-O0 -g -fPIE")
SET(CMAKE_CXX_FLAGS_DEBUG      "-O0 -std=c++0x -g -fPIE")
SET(CMAKE_C_FLAGS_RELEASE      "-O2 -g -fPIE")
SET(CMAKE_CXX_FLAGS_RELEASE    "-O2 -std=c++0x -g -fPIE")
SET(CMAKE_C_FLAGS_CCOV         "-O2 -g --coverage -fPIE")
SET(CMAKE_CXX_FLAGS_CCOV       "-O2 -std=c++0x -g --coverage -fPIE")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

#Install images
INSTALL(DIRECTORY data/images/ DESTINATION res/images)

# Install Manifest File
INSTALL(FILES org.tizen.lockscreen.xml DESTINATION /usr/share/packages)

# End of a file
ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(po)
