#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
测试CMake配置是否正确
"""

import os
import sys

def check_cmake_paths():
    """检查CMake中定义的路径是否存在"""
    print("检查CMake配置路径...")
    
    # 定义路径（与CMakeLists.txt中相同）
    qt_dir = r"H:\Programs\Qt\6.9.1\msvc2022_64"
    vtk_dir = r"H:\SourceCode\vtk-9.5.0\install"
    eigen_dir = r"H:\SourceCode\eigen-3.4.0\install"
    
    paths_to_check = [
        (qt_dir, "Qt安装目录"),
        (os.path.join(qt_dir, "bin"), "Qt bin目录"),
        (os.path.join(qt_dir, "plugins", "platforms"), "Qt平台插件目录"),
        (vtk_dir, "VTK安装目录"),
        (os.path.join(vtk_dir, "bin"), "VTK bin目录"),
        (os.path.join(vtk_dir, "lib", "cmake", "vtk-9.5"), "VTK CMake配置目录"),
        (eigen_dir, "Eigen安装目录"),
        (os.path.join(eigen_dir, "share", "eigen3", "cmake"), "Eigen CMake配置目录"),
    ]
    
    all_ok = True
    for path, description in paths_to_check:
        if os.path.exists(path):
            print(f"[OK] {description}: {path}")
        else:
            print(f"[错误] {description}不存在: {path}")
            all_ok = False
    
    return all_ok

def check_key_dlls():
    """检查关键DLL文件是否存在"""
    print("\n检查关键DLL文件...")
    
    qt_bin = r"H:\Programs\Qt\6.9.1\msvc2022_64\bin"
    vtk_bin = r"H:\SourceCode\vtk-9.5.0\install\bin"
    
    qt_dlls = [
        "Qt6Core.dll",
        "Qt6Gui.dll", 
        "Qt6Widgets.dll",
        "Qt6OpenGL.dll",
        "Qt6OpenGLWidgets.dll"
    ]
    
    vtk_dlls = [
        "vtkCommonCore-9.5.dll",
        "vtkRenderingCore-9.5.dll",
        "vtkRenderingOpenGL2-9.5.dll",
        "vtkGUISupportQt-9.5.dll",
        "vtkRenderingHyperTreeGrid-9.5.dll",
        "vtkIOImage-9.5.dll",
        "vtkInteractionWidgets-9.5.dll",
        "vtksys-9.5.dll"
    ]
    
    all_ok = True
    
    print("Qt DLL文件:")
    for dll in qt_dlls:
        dll_path = os.path.join(qt_bin, dll)
        if os.path.exists(dll_path):
            print(f"  [OK] {dll}")
        else:
            print(f"  [错误] {dll}")
            all_ok = False
    
    print("VTK DLL文件:")
    for dll in vtk_dlls:
        dll_path = os.path.join(vtk_bin, dll)
        if os.path.exists(dll_path):
            print(f"  [OK] {dll}")
        else:
            print(f"  [错误] {dll}")
            all_ok = False
    
    return all_ok

def main():
    print("=" * 60)
    print("测试CMake配置")
    print("=" * 60)
    
    paths_ok = check_cmake_paths()
    dlls_ok = check_key_dlls()
    
    print("\n" + "=" * 60)
    if paths_ok and dlls_ok:
        print("所有检查通过！CMake应该能够正确处理依赖")
        print("建议使用: python build_simple.py")
    else:
        print("发现一些问题，可能需要手动处理依赖")
        print("建议使用: python build_and_deploy.py")
    
    return paths_ok and dlls_ok

if __name__ == "__main__":
    success = main()
    input("\n按回车键退出...")
    sys.exit(0 if success else 1)