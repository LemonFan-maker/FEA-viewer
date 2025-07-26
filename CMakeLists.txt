cmake_minimum_required(VERSION 3.16)
project(FEMResultViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt路径
set(CMAKE_PREFIX_PATH "H:/Programs/Qt/6.9.1/msvc2022_64")

# VTK路径
set(VTK_DIR "H:/SourceCode/vtk-9.5.0/install/lib/cmake/vtk-9.5")

# Eigen路径
set(Eigen3_DIR "H:/SourceCode/eigen-3.4.0/install/share/eigen3/cmake")

# 查找Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

# 查找VTK
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    CommonMath
    CommonMisc
    CommonSystem
    CommonTransforms
    FiltersCore
    IOCore
    IOXML
    IOXMLParser
    IOLegacy
    IOImage
    IOGeometry
    IOPLY
    IOImport
    RenderingCore
    RenderingOpenGL2
    RenderingAnnotation
    RenderingUI
    RenderingHyperTreeGrid
    InteractionStyle
    InteractionWidgets
    GUISupportQt
    RenderingAnnotation
    FiltersGeneral
    FiltersCore
    FiltersSources
    FiltersFlowPaths
)

# 查找Eigen
find_package(Eigen3 REQUIRED)

# 启用Qt的MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/visualization/ClippingWidget.cpp
    src/visualization/ClippingWidget.h
    src/visualization/ContourWidget.cpp
    src/visualization/ContourWidget.h
    src/visualization/VectorFieldWidget.cpp
    src/visualization/VectorFieldWidget.h
    src/interaction/DataPicker.cpp
    src/interaction/DataPicker.h
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
    ${VTK_LIBRARIES}
    Eigen3::Eigen
)

# VTK模块初始化
vtk_module_autoinit(
    TARGETS ${PROJECT_NAME}
    MODULES ${VTK_LIBRARIES}
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${VTK_INCLUDE_DIRS}
)

# 自动部署依赖库
if(WIN32)
    # Qt路径定义
    set(QT_BIN_DIR "${CMAKE_PREFIX_PATH}/bin")
    set(QT_PLUGINS_DIR "${CMAKE_PREFIX_PATH}/plugins")
    
    # VTK路径定义
    get_filename_component(VTK_INSTALL_PREFIX "${VTK_DIR}/../.." ABSOLUTE)
    set(VTK_BIN_DIR "${VTK_INSTALL_PREFIX}/bin")
    
    # 获取输出目录
    set(OUTPUT_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    
    # 定义一个函数来安全复制DLL
    function(copy_dll_if_exists SOURCE_DIR DLL_NAME TARGET_DIR DESCRIPTION)
        set(SOURCE_FILE "${SOURCE_DIR}/${DLL_NAME}")
        if(EXISTS "${SOURCE_FILE}")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SOURCE_FILE}"
                "${TARGET_DIR}/${DLL_NAME}"
                COMMENT "复制${DESCRIPTION}: ${DLL_NAME}"
            )
        else()
            message(WARNING "找不到${DESCRIPTION}: ${SOURCE_FILE}")
        endif()
    endfunction()
    
    # Qt DLL列表
    set(QT_DLLS
        Qt6Core.dll
        Qt6Gui.dll
        Qt6Widgets.dll
        Qt6OpenGL.dll
        Qt6OpenGLWidgets.dll
    )
    
    # VTK DLL列表（根据错误信息不断补充）
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
        # IO库
        vtkIOCore-9.5.dll
        vtkIOXML-9.5.dll
        vtkIOXMLParser-9.5.dll
        vtkIOLegacy-9.5.dll
        vtkIOImage-9.5.dll
        # 渲染库
        vtkRenderingCore-9.5.dll
        vtkRenderingOpenGL2-9.5.dll
        vtkRenderingAnnotation-9.5.dll
        vtkRenderingUI-9.5.dll
        vtkRenderingHyperTreeGrid-9.5.dll
        # 交互库
        vtkInteractionStyle-9.5.dll
        vtkInteractionWidgets-9.5.dll
        # Qt支持
        vtkGUISupportQt-9.5.dll
        # 系统库
        vtksys-9.5.dll
        # 第三方库
        vtkexpat-9.5.dll
        vtklz4-9.5.dll
        vtklzma-9.5.dll
        vtkzlib-9.5.dll
        vtkglew-9.5.dll
        vtkfreetype-9.5.dll
    )
    
    # 复制Qt DLL文件
    foreach(DLL ${QT_DLLS})
        copy_dll_if_exists("${QT_BIN_DIR}" "${DLL}" "${OUTPUT_DIR}" "Qt DLL")
    endforeach()
    
    # 复制VTK DLL文件
    foreach(DLL ${VTK_DLLS})
        copy_dll_if_exists("${VTK_BIN_DIR}" "${DLL}" "${OUTPUT_DIR}" "VTK DLL")
    endforeach()
    
    # 自动复制所有VTK DLL文件（通配符方式）
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "自动复制所有VTK DLL文件..."
        COMMENT "准备复制VTK DLL文件"
    )
    
    # 使用file(GLOB)在构建时查找所有VTK DLL
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} 
        -DVTK_BIN_DIR="${VTK_BIN_DIR}"
        -DOUTPUT_DIR="${OUTPUT_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/copy_vtk_dlls.cmake"
        COMMENT "执行VTK DLL复制脚本"
    )
    
    # 复制Qt平台插件
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        "${OUTPUT_DIR}/platforms"
        COMMENT "创建platforms目录"
    )
    
    set(QWINDOWS_DLL "${QT_PLUGINS_DIR}/platforms/qwindows.dll")
    if(EXISTS "${QWINDOWS_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QWINDOWS_DLL}"
            "${OUTPUT_DIR}/platforms/qwindows.dll"
            COMMENT "复制Qt平台插件: qwindows.dll"
        )
    else()
        message(WARNING "找不到Qt平台插件: ${QWINDOWS_DLL}")
    endif()
    
    # 使用windeployqt自动部署Qt依赖（如果可用）
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${QT_BIN_DIR})
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} --release --no-translations $<TARGET_FILE:${PROJECT_NAME}>
            COMMENT "使用windeployqt部署Qt依赖"
        )
    endif()
    
    # 打印部署信息
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "依赖库部署完成！"
        COMMAND ${CMAKE_COMMAND} -E echo "可执行文件位置: $<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "部署完成提示"
    )
endif()