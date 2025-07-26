#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
终极构建脚本 - 构建项目并自动解决所有DLL依赖问题
"""

import os
import sys
import subprocess
import shutil
import glob

def run_command(cmd, cwd=None, show_output=True):
    """运行命令并返回结果"""
    print(f"执行命令: {cmd}")
    try:
        result = subprocess.run(cmd, shell=True, cwd=cwd, 
                              capture_output=True, text=True, encoding='utf-8')
        
        # 总是显示输出，便于调试
        if result.stdout.strip() and show_output:
            print("标准输出:")
            print(result.stdout)
        
        if result.stderr.strip():
            print("错误输出:")
            print(result.stderr)
        
        if result.returncode != 0:
            print(f"命令执行失败，返回码: {result.returncode}")
            return False
        
        return True
    except Exception as e:
        print(f"执行命令失败: {e}")
        return False

def build_project():
    """构建项目"""
    print("=" * 30)
    print("步骤1: 构建项目")
    print("=" * 30)
    
    # 检查当前目录
    if not os.path.exists("CMakeLists.txt"):
        print("错误: 请在项目根目录运行此脚本")
        return False
    
    # 清理之前的构建
    build_dir = "build"
    if os.path.exists(build_dir):
        print("清理之前的构建目录...")
        shutil.rmtree(build_dir)
    
    # 创建构建目录
    os.makedirs(build_dir)
    print(f"创建构建目录: {build_dir}")
    
    # 配置CMake
    print("\n配置CMake...")
    cmake_cmd = 'cmake .. -G "Visual Studio 17 2022" -A x64'
    if not run_command(cmake_cmd, cwd=build_dir):
        print("CMake配置失败!")
        return False
    
    # 构建项目
    print("\n开始构建...")
    build_cmd = "cmake --build . --config Release"
    if not run_command(build_cmd, cwd=build_dir):
        print("构建失败!")
        return False
    
    # 检查可执行文件
    exe_path = os.path.join(build_dir, "Release", "FEMResultViewer.exe")
    if os.path.exists(exe_path):
        print(f"构建成功: {exe_path}")
        return True
    else:
        print("构建失败: 找不到可执行文件")
        return False

def deploy_all_dependencies():
    """部署所有依赖库"""
    print("\n" + "=" * 30)
    print("步骤2: 部署所有依赖库")
    print("=" * 30)
    
    build_dir = os.path.join("build", "Release")
    qt_bin_dir = r"H:\Programs\Qt\6.9.1\msvc2022_64\bin"
    vtk_bin_dir = r"H:\SourceCode\vtk-9.5.0\install\bin"
    
    success_count = 0
    total_count = 0
    
    # 1. 复制所有VTK DLL
    print("\n复制VTK DLL文件...")
    if os.path.exists(vtk_bin_dir):
        vtk_dll_pattern = os.path.join(vtk_bin_dir, "vtk*-9.5.dll")
        vtk_dlls = glob.glob(vtk_dll_pattern)
        
        print(f"找到 {len(vtk_dlls)} 个VTK DLL文件")
        for dll_path in vtk_dlls:
            dll_name = os.path.basename(dll_path)
            dst_path = os.path.join(build_dir, dll_name)
            total_count += 1
            
            try:
                shutil.copy2(dll_path, dst_path)
                print(f"  [OK] {dll_name}")
                success_count += 1
            except Exception as e:
                print(f"  [错误] {dll_name}: {e}")
    else:
        print(f"警告: VTK bin目录不存在: {vtk_bin_dir}")
    
    # 2. 复制Qt DLL
    print("\n复制Qt DLL文件...")
    qt_dlls = [
        "Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", 
        "Qt6OpenGL.dll", "Qt6OpenGLWidgets.dll"
    ]
    
    for dll_name in qt_dlls:
        src_path = os.path.join(qt_bin_dir, dll_name)
        dst_path = os.path.join(build_dir, dll_name)
        total_count += 1
        
        if os.path.exists(src_path):
            try:
                shutil.copy2(src_path, dst_path)
                print(f"  [OK] {dll_name}")
                success_count += 1
            except Exception as e:
                print(f"  [错误] {dll_name}: {e}")
        else:
            print(f"  [警告] 找不到: {dll_name}")
    
    # 3. 复制Qt平台插件
    print("\n复制Qt平台插件...")
    platforms_src = os.path.join(qt_bin_dir, "..", "plugins", "platforms")
    platforms_dst = os.path.join(build_dir, "platforms")
    
    if os.path.exists(platforms_src):
        os.makedirs(platforms_dst, exist_ok=True)
        qwindows_src = os.path.join(platforms_src, "qwindows.dll")
        qwindows_dst = os.path.join(platforms_dst, "qwindows.dll")
        total_count += 1
        
        if os.path.exists(qwindows_src):
            try:
                shutil.copy2(qwindows_src, qwindows_dst)
                print("  [OK] platforms/qwindows.dll")
                success_count += 1
            except Exception as e:
                print(f"  [错误] platforms/qwindows.dll: {e}")
    
    # 4. 使用windeployqt
    print("\n使用windeployqt自动部署...")
    windeployqt = os.path.join(qt_bin_dir, "windeployqt.exe")
    exe_path = os.path.join(build_dir, "FEMResultViewer.exe")
    
    if os.path.exists(windeployqt) and os.path.exists(exe_path):
        cmd = f'"{windeployqt}" --release --no-translations "{exe_path}"'
        if run_command(cmd):
            print("  [OK] windeployqt执行成功")
        else:
            print("  [警告] windeployqt执行失败")
    
    print(f"\n依赖库部署完成: {success_count}/{total_count} 个文件")
    return success_count > 0

def main():
    print("=" * 60)
    print("终极构建脚本 - 一键解决所有问题")
    print("=" * 60)
    
    # 步骤1: 构建项目
    if not build_project():
        print("构建失败，终止执行")
        return False
    
    # 步骤2: 部署依赖
    if not deploy_all_dependencies():
        print("依赖部署失败")
        return False
    
    # 完成
    exe_path = os.path.join("build", "Release", "FEMResultViewer.exe")
    print("\n" + "=" * 60)
    print("🎉 构建和部署完成!")
    print(f"可执行文件: {exe_path}")
    print("所有依赖库已自动复制，现在应该可以正常运行了")
    print("=" * 60)
    
    # 询问是否立即运行
    try:
        choice = input("\n是否立即运行程序？(y/n): ").strip().lower()
        if choice in ['y', 'yes', '是']:
            print("启动程序...")
            os.startfile(exe_path)
    except KeyboardInterrupt:
        print("\n用户取消")
    
    return True

if __name__ == "__main__":
    success = main()
    input("\n按回车键退出...")
    sys.exit(0 if success else 1)