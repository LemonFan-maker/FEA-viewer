#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
快速启动FEM结果查看器
"""

import os
import sys

def main():
    """快速启动程序"""
    exe_path = os.path.join("build", "Release", "FEMResultViewer.exe")
    
    if os.path.exists(exe_path):
        print(f"启动FEM结果查看器: {exe_path}")
        try:
            os.startfile(exe_path)
            print("程序已启动")
        except Exception as e:
            print(f"启动失败: {e}")
            input("按回车键退出...")
            sys.exit(1)
    else:
        print("错误: 找不到可执行文件")
        print("请先运行构建脚本:")
        print("  python build_and_deploy.py")
        print("或使用主控制面板:")
        print("  python main.py")
        input("按回车键退出...")
        sys.exit(1)

if __name__ == "__main__":
    main()