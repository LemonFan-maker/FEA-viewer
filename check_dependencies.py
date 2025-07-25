#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
检查项目依赖
"""

import os
import subprocess
import sys

def check_path_exists(path, name):
    """检查路径是否存在"""
    if os.path.exists(path):
        print(f"[OK] {name}路径存在: {path}")
        return True
    else:
        print(f"[错误] {name}路径不存在: {path}")
        return False

def check_command(cmd, name):
    """检查命令是否可用"""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"[OK] {name}可用")
            return True
        else:
            print(f"[警告] {name}不可用或有问题")
            return False
    except Exception:
        print(f"[错误] {name}未安装或不在PATH中")
        return False

def get_version_info(cmd, name):
    """获取版本信息"""
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if result.returncode == 0:
            print(f"{name}版本信息:")
            print(result.stdout.strip())
        else:
            print(f"无法获取{name}版本信息")
    except Exception:
        print(f"获取{name}版本信息失败")

def main():
    print("检查项目依赖...")
    print("=" * 50)
    
    # 定义路径
    qt_dir = r"H:\Programs\Qt\6.9.1\msvc2022_64"
    vtk_dir = r"H:\SourceCode\vtk-9.5.0\install"
    eigen_dir = r"H:\SourceCode\eigen-3.4.0\install"
    
    all_ok = True
    
    # 检查Qt
    print("检查Qt安装...")
    qt_ok = check_path_exists(qt_dir, "Qt")
    if qt_ok:
        qmake_path = os.path.join(qt_dir, "bin", "qmake.exe")
        if os.path.exists(qmake_path):
            get_version_info(f'"{qmake_path}" -version', "Qt")
        else:
            print("[警告] qmake.exe不存在")
            qt_ok = False
    all_ok &= qt_ok
    
    print("\n" + "=" * 50)
    
    # 检查VTK
    print("检查VTK安装...")
    vtk_cmake_path = os.path.join(vtk_dir, "lib", "cmake", "vtk-9.5")
    vtk_ok = check_path_exists(vtk_cmake_path, "VTK CMake配置")
    
    if vtk_ok:
        # 检查关键DLL文件
        vtk_dll_path = os.path.join(vtk_dir, "bin", "vtkCommonCore-9.5.dll")
        if os.path.exists(vtk_dll_path):
            print("[OK] VTK DLL文件存在")
        else:
            print("[警告] VTK DLL文件不存在，可能需要重新编译VTK")
            vtk_ok = False
    all_ok &= vtk_ok
    
    print("\n" + "=" * 50)
    
    # 检查Eigen
    print("检查Eigen安装...")
    eigen_cmake_path = os.path.join(eigen_dir, "share", "eigen3", "cmake")
    eigen_ok = check_path_exists(eigen_cmake_path, "Eigen")
    all_ok &= eigen_ok
    
    print("\n" + "=" * 50)
    
    # 检查Visual Studio
    print("检查Visual Studio...")
    vs_ok = check_command("cl.exe", "Visual Studio编译器")
    if vs_ok:
        try:
            result = subprocess.run("cl.exe", shell=True, capture_output=True, text=True)
            # cl.exe的版本信息通常在stderr中
            version_info = result.stderr
            if "Microsoft" in version_info:
                print("Visual Studio版本信息:")
                for line in version_info.split('\n'):
                    if "Microsoft" in line or "版本" in line:
                        print(line.strip())
        except Exception:
            pass
    else:
        print("请确保已安装Visual Studio 2022并设置了环境变量")
    all_ok &= vs_ok
    
    print("\n" + "=" * 50)
    
    # 检查CMake
    print("检查CMake...")
    cmake_ok = check_command("cmake --version", "CMake")
    if cmake_ok:
        get_version_info("cmake --version", "CMake")
    all_ok &= cmake_ok
    
    print("\n" + "=" * 50)
    print("依赖检查完成!")
    
    if all_ok:
        print("[OK] 所有依赖都已正确安装")
    else:
        print("[警告] 部分依赖存在问题，可能影响构建")
    
    return all_ok

if __name__ == "__main__":
    success = main()
    input("\n按回车键退出...")
    sys.exit(0 if success else 1)