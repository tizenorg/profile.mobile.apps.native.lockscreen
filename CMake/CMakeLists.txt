CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(lockscreen C)

SET(INSTALL_EXEC_PREFIX "${INSTALL_PREFIX}/bin")
SET(INSTALL_RESDIR "${INSTALL_PREFIX}/res")
SET(INSTALL_IMAGEDIR "${INSTALL_RESDIR}/images")
SET(EDJEDIR "${INSTALL_RESDIR}/edje")
SET(LOCALEDIR "${INSTALL_RESDIR}/locale")
SET(CMAKE_VERBOSE_MAKEFILE 0)

SET(PROJECT_ROOT_DIR "${CMAKE_SOURCE_DIR}/..")
SET(PROJECT_RESOURCES_DIR "${PROJECT_ROOT_DIR}/res")
SET(PROJECT_EDC_DIR "${PROJECT_RESOURCES_DIR}/edje")
SET(PROJECT_IMAGES_DIR "${PROJECT_RESOURCES_DIR}/images")
SET(PROJECT_PO_DIR "${PROJECT_RESOURCES_DIR}/po")

INCLUDE(FindPkgConfig)
pkg_check_modules(MENU_PKGS REQUIRED
	dlog
	bundle
	elementary
	capi-system-info
	capi-system-runtime-info
	capi-system-system-settings
	capi-system-device
	capi-appfw-application
	capi-appfw-app-manager
	notification
	minicontrol-viewer
	feedback
	tzsh-lockscreen-service
	capi-base-utils-i18n
	capi-telephony
	vconf
	auth-fw
	dpm
	gio-2.0
)

FOREACH (flag ${MENU_PKGS_CFLAGS})
    SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall -Winline -g -fno-builtin-malloc -fPIE")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

INCLUDE_DIRECTORIES(${PROJECT_ROOT_DIR}/inc)

ADD_EXECUTABLE(${PROJECT_NAME}
	${PROJECT_ROOT_DIR}/src/battery_ctrl.c
	${PROJECT_ROOT_DIR}/src/shortcut_ctrl.c
	${PROJECT_ROOT_DIR}/src/swipe_icon.c
	${PROJECT_ROOT_DIR}/src/lockscreen.c
	${PROJECT_ROOT_DIR}/src/util.c
	${PROJECT_ROOT_DIR}/src/window.c
	${PROJECT_ROOT_DIR}/src/background.c
	${PROJECT_ROOT_DIR}/src/background_ctrl.c
	${PROJECT_ROOT_DIR}/src/battery.c
	${PROJECT_ROOT_DIR}/src/events.c
	${PROJECT_ROOT_DIR}/src/events_view.c
	${PROJECT_ROOT_DIR}/src/events_ctrl.c
	${PROJECT_ROOT_DIR}/src/minicontrollers.c
	${PROJECT_ROOT_DIR}/src/sim.c
	${PROJECT_ROOT_DIR}/src/sim_lock.c
	${PROJECT_ROOT_DIR}/src/sim_lock_ctrl.c
	${PROJECT_ROOT_DIR}/src/shortcut.c
	${PROJECT_ROOT_DIR}/src/main_view.c
	${PROJECT_ROOT_DIR}/src/main_ctrl.c
	${PROJECT_ROOT_DIR}/src/display.c
	${PROJECT_ROOT_DIR}/src/time_format.c
	${PROJECT_ROOT_DIR}/src/util_time.c
	${PROJECT_ROOT_DIR}/src/time_format_ctrl.c
	${PROJECT_ROOT_DIR}/src/sim_ctrl.c
	${PROJECT_ROOT_DIR}/src/device_lock.c
	${PROJECT_ROOT_DIR}/src/device_lock_ctrl.c
	${PROJECT_ROOT_DIR}/src/password_view.c
	${PROJECT_ROOT_DIR}/src/deviced.c
	${PROJECT_ROOT_DIR}/src/call.c
	${PROJECT_ROOT_DIR}/src/call_ctrl.c
)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${MENU_PKGS_LDFLAGS})

# Install
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_EXEC_PREFIX})

INSTALL(FILES ${PROJECT_ROOT_DIR}/tizen-manifest.xml DESTINATION ${SYS_PACKAGES_DIR} RENAME org.tizen.lockscreen.xml)
INSTALL(DIRECTORY DESTINATION ${PREFIX}/data)
INSTALL(FILES ${PROJECT_ROOT_DIR}/shared/res/lockscreen.png DESTINATION ${SYS_ICONS_DIR})

# End of a file
ADD_SUBDIRECTORY(res)
