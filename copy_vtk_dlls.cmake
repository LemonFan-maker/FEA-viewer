# CMake脚本：自动复制所有VTK DLL文件

# 检查参数
if(NOT DEFINED VTK_BIN_DIR)
    message(FATAL_ERROR "VTK_BIN_DIR not defined")
endif()

if(NOT DEFINED OUTPUT_DIR)
    message(FATAL_ERROR "OUTPUT_DIR not defined")
endif()

# 查找所有VTK DLL文件
file(GLOB VTK_ALL_DLLS "${VTK_BIN_DIR}/vtk*-9.5.dll")

if(NOT VTK_ALL_DLLS)
    message(WARNING "没有找到VTK DLL文件在: ${VTK_BIN_DIR}")
else()
    message(STATUS "找到 ${CMAKE_MATCH_COUNT} 个VTK DLL文件")
    
    # 复制每个DLL文件
    foreach(DLL_PATH ${VTK_ALL_DLLS})
        get_filename_component(DLL_NAME ${DLL_PATH} NAME)
        set(DEST_PATH "${OUTPUT_DIR}/${DLL_NAME}")
        
        # 检查文件是否需要更新
        if(NOT EXISTS "${DEST_PATH}" OR "${DLL_PATH}" IS_NEWER_THAN "${DEST_PATH}")
            message(STATUS "复制VTK DLL: ${DLL_NAME}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DLL_PATH}" "${DEST_PATH}"
                RESULT_VARIABLE COPY_RESULT
            )
            
            if(NOT COPY_RESULT EQUAL 0)
                message(WARNING "复制失败: ${DLL_NAME}")
            endif()
        endif()
    endforeach()
endif()

# 同时复制一些可能需要的系统DLL
set(SYSTEM_DLLS
    "msvcp140.dll"
    "vcruntime140.dll"
    "vcruntime140_1.dll"
)

foreach(SYS_DLL ${SYSTEM_DLLS})
    set(SYS_DLL_PATH "${VTK_BIN_DIR}/${SYS_DLL}")
    if(EXISTS "${SYS_DLL_PATH}")
        set(DEST_PATH "${OUTPUT_DIR}/${SYS_DLL}")
        if(NOT EXISTS "${DEST_PATH}")
            message(STATUS "复制系统DLL: ${SYS_DLL}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SYS_DLL_PATH}" "${DEST_PATH}"
                RESULT_VARIABLE COPY_RESULT
            )
        endif()
    endif()
endforeach()