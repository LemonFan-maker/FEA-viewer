#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
创建测试用的VTK文件
需要安装: pip install vtk numpy
"""

import sys
import os

try:
    import vtk
    import numpy as np
except ImportError as e:
    print(f"错误: 缺少必要的Python库: {e}")
    print("请运行: pip install vtk numpy")
    input("按回车键退出...")
    sys.exit(1)

def create_test_cube():
    """创建一个简单的立方体网格用于测试"""
    
    # 创建点
    points = vtk.vtkPoints()
    
    # 立方体的8个顶点
    cube_points = [
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [1.0, 1.0, 0.0],
        [0.0, 1.0, 0.0],
        [0.0, 0.0, 1.0],
        [1.0, 0.0, 1.0],
        [1.0, 1.0, 1.0],
        [0.0, 1.0, 1.0]
    ]
    
    for point in cube_points:
        points.InsertNextPoint(point)
    
    # 创建单元（六面体）
    hexahedron = vtk.vtkHexahedron()
    for i in range(8):
        hexahedron.GetPointIds().SetId(i, i)
    
    # 创建网格
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(points)
    ugrid.InsertNextCell(hexahedron.GetCellType(), hexahedron.GetPointIds())
    
    # 添加标量数据 - 应力
    stress_data = vtk.vtkFloatArray()
    stress_data.SetName("Stress")
    stress_data.SetNumberOfComponents(1)
    
    # 为每个点分配应力值
    stress_values = [10.0, 20.0, 30.0, 25.0, 15.0, 35.0, 40.0, 30.0]
    for value in stress_values:
        stress_data.InsertNextValue(value)
    
    ugrid.GetPointData().AddArray(stress_data)
    ugrid.GetPointData().SetActiveScalars("Stress")
    
    # 添加温度数据
    temp_data = vtk.vtkFloatArray()
    temp_data.SetName("Temperature")
    temp_data.SetNumberOfComponents(1)
    
    temp_values = [100.0, 120.0, 110.0, 105.0, 95.0, 125.0, 130.0, 115.0]
    for value in temp_values:
        temp_data.InsertNextValue(value)
    
    ugrid.GetPointData().AddArray(temp_data)
    
    # 添加单元数据 - 密度
    density_data = vtk.vtkFloatArray()
    density_data.SetName("Density")
    density_data.SetNumberOfComponents(1)
    density_data.InsertNextValue(7850.0)  # 钢的密度
    
    ugrid.GetCellData().AddArray(density_data)
    
    return ugrid

def save_test_files():
    """保存测试文件"""
    ugrid = create_test_cube()
    
    # 保存为XML格式 (.vtu)
    writer_xml = vtk.vtkXMLUnstructuredGridWriter()
    writer_xml.SetFileName("test_data.vtu")
    writer_xml.SetInputData(ugrid)
    writer_xml.Write()
    
    # 保存为Legacy格式 (.vtk)
    writer_legacy = vtk.vtkUnstructuredGridWriter()
    writer_legacy.SetFileName("test_data.vtk")
    writer_legacy.SetInputData(ugrid)
    writer_legacy.Write()
    
    print("测试文件已创建:")
    print("- test_data.vtu (XML格式)")
    print("- test_data.vtk (Legacy格式)")
    print("\n数据包含:")
    print("- 点数据: Stress (应力), Temperature (温度)")
    print("- 单元数据: Density (密度)")

if __name__ == "__main__":
    try:
        save_test_files()
        input("\n按回车键退出...")
    except Exception as e:
        print(f"创建测试文件失败: {e}")
        input("按回车键退出...")
        sys.exit(1)