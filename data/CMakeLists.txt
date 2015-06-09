ADD_CUSTOM_TARGET(index.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/edje/images
                ${CMAKE_SOURCE_DIR}/data/edje/index.edc ${CMAKE_BINARY_DIR}/data/edje/index.edj
                DEPENDS ${CMAKE_SOURCE_DIR}/data/edje/index.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} index.edj)
INSTALL(FILES edje/index.edj DESTINATION res/edje)

ADD_CUSTOM_TARGET(lockscreen.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/edje/images
                ${CMAKE_SOURCE_DIR}/data/edje/lockscreen.edc ${CMAKE_BINARY_DIR}/data/edje/lockscreen.edj
                DEPENDS ${CMAKE_SOURCE_DIR}/data/edje/lockscreen.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} lockscreen.edj)
INSTALL(FILES edje/lockscreen.edj DESTINATION res/edje)

