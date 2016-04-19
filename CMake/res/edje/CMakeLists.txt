ADD_CUSTOM_TARGET(lockscreen.edj
	COMMAND edje_cc -id ${PROJECT_EDC_DIR}/images
	${PROJECT_EDC_DIR}/lockscreen.edc lockscreen.edj
	DEPENDS ${PROJECT_EDC_DIR}/lockscreen.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} lockscreen.edj)
INSTALL(FILES lockscreen.edj DESTINATION ${EDJEDIR})

ADD_CUSTOM_TARGET(index.edj
	COMMAND edje_cc -id ${PROJECT_EDC_DIR}/images
	${PROJECT_EDC_DIR}/index.edc index.edj
	DEPENDS ${PROJECT_EDC_DIR}/index.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} index.edj)
INSTALL(FILES index.edj DESTINATION ${EDJEDIR})
