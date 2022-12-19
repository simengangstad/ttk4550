include_guard()
message("component_osa component is included.")

if(CONFIG_USE_middleware_baremetal_MIMXRT1166_cm4)
target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_os_abstraction_bm.c
)
elseif(CONFIG_USE_middleware_freertos-kernel_MIMXRT1166_cm4)
target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/fsl_os_abstraction_free_rtos.c
)
else()
    message(WARNING "please config middleware.baremetal_MIMXRT1166_cm4 or middleware.freertos-kernel_MIMXRT1166_cm4 first.")
endif()


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/.
)


include(driver_common_MIMXRT1166_cm4)

include(component_lists_MIMXRT1166_cm4)
