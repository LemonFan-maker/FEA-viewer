#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
FEM结果查看器主控制脚本
"""

import os
import sys
import subprocess

def show_menu():
    """显示主菜单"""
    print("\n" + "=" * 50)
    print("FEM结果查看器 - 主控制面板")
    print("=" * 50)
    print("1. 检查依赖环境")
    print("2. 测试CMake配置")
    print("3. 🚀 构建和部署项目 ⭐推荐")
    print("4. 快速重新构建 (保留DLL)")
    print("5. 创建测试数据")
    print("6. 运行程序")
    print("0. 退出")
    print("=" * 50)

def run_script(script_name):
    """运行指定的Python脚本"""
    if not os.path.exists(script_name):
        print(f"错误: 找不到脚本文件 {script_name}")
        return False
    
    try:
        result = subprocess.run([sys.executable, script_name])
        return result.returncode == 0
    except Exception as e:
        print(f"运行脚本失败: {e}")
        return False

def run_program():
    """运行FEM结果查看器"""
    exe_path = os.path.join("build", "Release", "FEMResultViewer.exe")
    if os.path.exists(exe_path):
        print(f"启动程序: {exe_path}")
        try:
            os.startfile(exe_path)
            return True
        except Exception as e:
            print(f"启动程序失败: {e}")
            return False
    else:
        print("错误: 找不到可执行文件")
        print("请先构建项目 (选项4)")
        return False

def main():
    """主函数"""
    while True:
        show_menu()
        
        try:
            choice = input("请选择操作 (0-6): ").strip()
        except KeyboardInterrupt:
            print("\n\n用户取消，退出程序")
            break
        
        if choice == "0":
            print("退出程序")
            break
        elif choice == "1":
            print("检查依赖环境...")
            run_script("check_dependencies.py")
        elif choice == "2":
            print("测试CMake配置...")
            run_script("test_cmake.py")
        elif choice == "3":
            print("🚀 构建和部署项目...")
            run_script("ultimate_build.py")
        elif choice == "4":
            print("快速重新构建...")
            run_script("rebuild.py")
        elif choice == "5":
            print("创建测试数据...")
            run_script("create_test_data.py")
        elif choice == "6":
            print("运行程序...")
            run_program()
        else:
            print("无效选择，请重新输入")
        
        if choice != "0":
            input("\n按回车键继续...")

if __name__ == "__main__":
    main()