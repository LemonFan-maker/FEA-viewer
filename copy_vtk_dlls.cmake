# VTK DLL复制脚本
# 自动复制所有必需的VTK DLL文件到输出目录

message(STATUS "开始复制VTK DLL文件...")
message(STATUS "VTK_BIN_DIR: ${VTK_BIN_DIR}")
message(STATUS "OUTPUT_DIR: ${OUTPUT_DIR}")

# 检查VTK_BIN_DIR是否存在
if(NOT EXISTS "${VTK_BIN_DIR}")
    message(WARNING "VTK bin目录不存在: ${VTK_BIN_DIR}")
    # 尝试其他可能的路径
    get_filename_component(VTK_PARENT_DIR "${VTK_BIN_DIR}" DIRECTORY)
    set(ALT_VTK_BIN_DIR "${VTK_PARENT_DIR}/lib/bin")
    if(EXISTS "${ALT_VTK_BIN_DIR}")
        set(VTK_BIN_DIR "${ALT_VTK_BIN_DIR}")
        message(STATUS "使用替代VTK bin目录: ${VTK_BIN_DIR}")
    else()
        message(WARNING "无法找到VTK DLL文件，跳过复制")
        return()
    endif()
endif()

# 检查OUTPUT_DIR是否存在
if(NOT EXISTS "${OUTPUT_DIR}")
    message(WARNING "输出目录不存在: ${OUTPUT_DIR}")
    return()
endif()

# VTK DLL文件列表
set(VTK_DLLS
    # 核心库
    vtkCommonCore-9.5.dll
    vtkCommonDataModel-9.5.dll
    vtkCommonExecutionModel-9.5.dll
    vtkCommonMath-9.5.dll
    vtkCommonMisc-9.5.dll
    vtkCommonSystem-9.5.dll
    vtkCommonTransforms-9.5.dll
    
    # 过滤器
    vtkFiltersCore-9.5.dll
    vtkFiltersGeneral-9.5.dll
    vtkFiltersGeometry-9.5.dll
    vtkFiltersSources-9.5.dll
    vtkFiltersExtraction-9.5.dll
    vtkFiltersModeling-9.5.dll
    
    # IO库
    vtkIOCore-9.5.dll
    vtkIOGeometry-9.5.dll
    vtkIOXML-9.5.dll
    vtkIOLegacy-9.5.dll
    vtkIOPLY-9.5.dll
    
    # 渲染库
    vtkRenderingCore-9.5.dll
    vtkRenderingOpenGL2-9.5.dll
    vtkRenderingUI-9.5.dll
    vtkRenderingAnnotation-9.5.dll
    
    # 交互库
    vtkInteractionStyle-9.5.dll
    vtkInteractionWidgets-9.5.dll
    
    # GUI集成
    vtkGUISupportQt-9.5.dll
    
    # 其他常用库
    vtkCommonColor-9.5.dll
    vtkCommonComputationalGeometry-9.5.dll
    vtkRenderingFreeType-9.5.dll
    vtkfreetype-9.5.dll
    vtkglew-9.5.dll
    vtksys-9.5.dll
)

# 复制每个DLL文件
foreach(dll_name ${VTK_DLLS})
    set(source_file "${VTK_BIN_DIR}/${dll_name}")
    set(dest_file "${OUTPUT_DIR}/${dll_name}")
    
    if(EXISTS "${source_file}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${source_file}" "${dest_file}"
            RESULT_VARIABLE copy_result
        )
        
        if(copy_result EQUAL 0)
            message(STATUS "已复制: ${dll_name}")
        else()
            message(WARNING "复制失败: ${dll_name}")
        endif()
    else()
        message(STATUS "跳过不存在的文件: ${dll_name}")
    endif()
endforeach()

# 尝试复制其他可能存在的VTK DLL
file(GLOB additional_vtk_dlls "${VTK_BIN_DIR}/vtk*.dll")
foreach(dll_file ${additional_vtk_dlls})
    get_filename_component(dll_name ${dll_file} NAME)
    set(dest_file "${OUTPUT_DIR}/${dll_name}")
    
    # 避免重复复制
    if(NOT EXISTS "${dest_file}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll_file}" "${dest_file}"
            RESULT_VARIABLE copy_result
        )
        
        if(copy_result EQUAL 0)
            message(STATUS "已复制额外文件: ${dll_name}")
        endif()
    endif()
endforeach()

message(STATUS "VTK DLL复制完成！")