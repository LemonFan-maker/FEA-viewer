#!/usr/bin/env python3
"""
创建优美、对称的三维体积数据
专门用于展示变形图和流线的美观效果
"""

import vtk
import numpy as np
import math

def create_twisted_cylinder():
    """创建扭转圆柱体 - 展示扭转变形和螺旋流线"""
    
    # 创建圆柱形网格
    nr, ntheta, nz = 12, 24, 30
    radius = 2.0
    height = 6.0
    
    points = vtk.vtkPoints()
    
    for k in range(nz):
        z = (k / (nz-1)) * height - height/2
        for j in range(ntheta):
            theta = (j / ntheta) * 2 * math.pi
            for i in range(nr):
                r = (i / (nr-1)) * radius
                x = r * math.cos(theta)
                y = r * math.sin(theta)
                points.InsertNextPoint(x, y, z)
    
    # 创建网格
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(points)
    
    # 创建六面体单元
    for k in range(nz-1):
        for j in range(ntheta):
            j_next = (j + 1) % ntheta
            for i in range(nr-1):
                hex_cell = vtk.vtkHexahedron()
                
                # 底面四个点
                hex_cell.GetPointIds().SetId(0, k * ntheta * nr + j * nr + i)
                hex_cell.GetPointIds().SetId(1, k * ntheta * nr + j * nr + i + 1)
                hex_cell.GetPointIds().SetId(2, k * ntheta * nr + j_next * nr + i + 1)
                hex_cell.GetPointIds().SetId(3, k * ntheta * nr + j_next * nr + i)
                
                # 顶面四个点
                hex_cell.GetPointIds().SetId(4, (k+1) * ntheta * nr + j * nr + i)
                hex_cell.GetPointIds().SetId(5, (k+1) * ntheta * nr + j * nr + i + 1)
                hex_cell.GetPointIds().SetId(6, (k+1) * ntheta * nr + j_next * nr + i + 1)
                hex_cell.GetPointIds().SetId(7, (k+1) * ntheta * nr + j_next * nr + i)
                
                ugrid.InsertNextCell(hex_cell.GetCellType(), hex_cell.GetPointIds())
    
    num_points = points.GetNumberOfPoints()
    
    # 扭转位移场
    twist_displacement = vtk.vtkFloatArray()
    twist_displacement.SetName("Twist_Displacement")
    twist_displacement.SetNumberOfComponents(3)
    twist_displacement.SetNumberOfTuples(num_points)
    
    # 螺旋速度场
    spiral_velocity = vtk.vtkFloatArray()
    spiral_velocity.SetName("Spiral_Velocity")
    spiral_velocity.SetNumberOfComponents(3)
    spiral_velocity.SetNumberOfTuples(num_points)
    
    # 扭转应力
    shear_stress = vtk.vtkFloatArray()
    shear_stress.SetName("Shear_Stress")
    shear_stress.SetNumberOfComponents(1)
    shear_stress.SetNumberOfTuples(num_points)
    
    # Von Mises应力
    von_mises = vtk.vtkFloatArray()
    von_mises.SetName("VonMises_Stress")
    von_mises.SetNumberOfComponents(1)
    von_mises.SetNumberOfTuples(num_points)
    
    twist_angle_per_unit = math.pi / height  # 每单位高度的扭转角
    
    for i in range(num_points):
        point = points.GetPoint(i)
        x, y, z = point
        r = math.sqrt(x*x + y*y)
        
        if r > 0:
            theta = math.atan2(y, x)
            
            # 扭转位移：沿Z轴的扭转
            twist_angle = twist_angle_per_unit * z
            
            # 扭转后的位置
            new_theta = theta + twist_angle * (r / radius)
            new_x = r * math.cos(new_theta)
            new_y = r * math.sin(new_theta)
            
            # 位移矢量
            dx = new_x - x
            dy = new_y - y
            dz = 0.0
            
            twist_displacement.SetTuple3(i, dx, dy, dz)
            
            # 螺旋速度场
            vx = -y * (1 + z / height)
            vy = x * (1 + z / height)
            vz = r * 0.5
            
            spiral_velocity.SetTuple3(i, vx, vy, vz)
            
            # 扭转剪应力
            tau = r * twist_angle_per_unit * 50000  # 剪切模量 * 剪应变
            shear_stress.SetValue(i, tau)
            
            # Von Mises应力（主要是剪应力）
            vm = tau * math.sqrt(3)
            von_mises.SetValue(i, vm)
        else:
            twist_displacement.SetTuple3(i, 0, 0, 0)
            spiral_velocity.SetTuple3(i, 0, 0, 0)
            shear_stress.SetValue(i, 0)
            von_mises.SetValue(i, 0)
    
    # 添加数据数组
    ugrid.GetPointData().AddArray(twist_displacement)
    ugrid.GetPointData().AddArray(spiral_velocity)
    ugrid.GetPointData().AddArray(shear_stress)
    ugrid.GetPointData().AddArray(von_mises)
    ugrid.GetPointData().SetActiveScalars("VonMises_Stress")
    ugrid.GetPointData().SetActiveVectors("Twist_Displacement")
    
    return ugrid

def create_sphere_with_radial_field():
    """创建球体径向场 - 展示径向变形和发散流线"""
    
    # 创建球形网格
    nr, ntheta, nphi = 15, 24, 16
    max_radius = 3.0
    
    points = vtk.vtkPoints()
    
    for i in range(nr):
        r = (i / (nr-1)) * max_radius
        for j in range(ntheta):
            theta = (j / ntheta) * 2 * math.pi  # 方位角
            for k in range(nphi):
                phi = (k / (nphi-1)) * math.pi  # 极角
                
                x = r * math.sin(phi) * math.cos(theta)
                y = r * math.sin(phi) * math.sin(theta)
                z = r * math.cos(phi)
                points.InsertNextPoint(x, y, z)
    
    # 创建网格
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(points)
    
    # 创建六面体单元（简化版）
    for i in range(nr-1):
        for j in range(ntheta):
            j_next = (j + 1) % ntheta
            for k in range(nphi-1):
                # 创建六面体单元
                hex_cell = vtk.vtkHexahedron()
                
                # 内层四个点
                hex_cell.GetPointIds().SetId(0, i * ntheta * nphi + j * nphi + k)
                hex_cell.GetPointIds().SetId(1, i * ntheta * nphi + j * nphi + k + 1)
                hex_cell.GetPointIds().SetId(2, i * ntheta * nphi + j_next * nphi + k + 1)
                hex_cell.GetPointIds().SetId(3, i * ntheta * nphi + j_next * nphi + k)
                
                # 外层四个点
                hex_cell.GetPointIds().SetId(4, (i+1) * ntheta * nphi + j * nphi + k)
                hex_cell.GetPointIds().SetId(5, (i+1) * ntheta * nphi + j * nphi + k + 1)
                hex_cell.GetPointIds().SetId(6, (i+1) * ntheta * nphi + j_next * nphi + k + 1)
                hex_cell.GetPointIds().SetId(7, (i+1) * ntheta * nphi + j_next * nphi + k)
                
                ugrid.InsertNextCell(hex_cell.GetCellType(), hex_cell.GetPointIds())
    
    num_points = points.GetNumberOfPoints()
    
    # 径向位移场
    radial_displacement = vtk.vtkFloatArray()
    radial_displacement.SetName("Radial_Displacement")
    radial_displacement.SetNumberOfComponents(3)
    radial_displacement.SetNumberOfTuples(num_points)
    
    # 径向速度场
    radial_velocity = vtk.vtkFloatArray()
    radial_velocity.SetName("Radial_Velocity")
    radial_velocity.SetNumberOfComponents(3)
    radial_velocity.SetNumberOfTuples(num_points)
    
    # 径向应力
    radial_stress = vtk.vtkFloatArray()
    radial_stress.SetName("Radial_Stress")
    radial_stress.SetNumberOfComponents(1)
    radial_stress.SetNumberOfTuples(num_points)
    
    # 压力场
    pressure = vtk.vtkFloatArray()
    pressure.SetName("Pressure")
    pressure.SetNumberOfComponents(1)
    pressure.SetNumberOfTuples(num_points)
    
    for i in range(num_points):
        point = points.GetPoint(i)
        x, y, z = point
        r = math.sqrt(x*x + y*y + z*z)
        
        if r > 0.01:
            # 单位径向矢量
            nx, ny, nz = x/r, y/r, z/r
            
            # 径向位移（球体膨胀）
            displacement_magnitude = r * 0.2 * (1 - r / max_radius)
            dx = nx * displacement_magnitude
            dy = ny * displacement_magnitude
            dz = nz * displacement_magnitude
            
            radial_displacement.SetTuple3(i, dx, dy, dz)
            
            # 径向速度场（发散流）
            velocity_magnitude = 2.0 * (1 + math.sin(r * math.pi / max_radius))
            vx = nx * velocity_magnitude
            vy = ny * velocity_magnitude
            vz = nz * velocity_magnitude
            
            radial_velocity.SetTuple3(i, vx, vy, vz)
            
            # 径向应力
            sigma_r = 100 * (max_radius - r) / max_radius
            radial_stress.SetValue(i, sigma_r)
            
            # 压力场
            p = 50 * math.exp(-r / max_radius * 2)
            pressure.SetValue(i, p)
        else:
            radial_displacement.SetTuple3(i, 0, 0, 0)
            radial_velocity.SetTuple3(i, 0, 0, 0)
            radial_stress.SetValue(i, 100)
            pressure.SetValue(i, 50)
    
    # 添加数据数组
    ugrid.GetPointData().AddArray(radial_displacement)
    ugrid.GetPointData().AddArray(radial_velocity)
    ugrid.GetPointData().AddArray(radial_stress)
    ugrid.GetPointData().AddArray(pressure)
    ugrid.GetPointData().SetActiveScalars("Pressure")
    ugrid.GetPointData().SetActiveVectors("Radial_Displacement")
    
    return ugrid

def create_torus_vortex():
    """创建环形涡流 - 展示环形变形和涡流线"""
    
    # 厚环面参数（创建体积网格）
    R = 3.0  # 大半径
    r_inner = 0.8  # 内小半径
    r_outer = 1.2  # 外小半径
    nu, nv, nr = 24, 16, 6  # 添加径向分层
    
    points = vtk.vtkPoints()
    
    # 创建厚环面的体积点
    for i in range(nu):
        u = (i / nu) * 2 * math.pi  # 大圆角度
        for j in range(nv):
            v = (j / nv) * 2 * math.pi  # 小圆角度
            for k in range(nr):
                # 在小圆的径向上分层
                r_local = r_inner + (k / (nr-1)) * (r_outer - r_inner)
                
                x = (R + r_local * math.cos(v)) * math.cos(u)
                y = (R + r_local * math.cos(v)) * math.sin(u)
                z = r_local * math.sin(v)
                points.InsertNextPoint(x, y, z)
    
    # 创建网格
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(points)
    
    # 创建六面体单元
    for i in range(nu):
        i_next = (i + 1) % nu
        for j in range(nv):
            j_next = (j + 1) % nv
            for k in range(nr-1):
                hex_cell = vtk.vtkHexahedron()
                
                # 内层四个点
                hex_cell.GetPointIds().SetId(0, i * nv * nr + j * nr + k)
                hex_cell.GetPointIds().SetId(1, i * nv * nr + j * nr + k + 1)
                hex_cell.GetPointIds().SetId(2, i * nv * nr + j_next * nr + k + 1)
                hex_cell.GetPointIds().SetId(3, i * nv * nr + j_next * nr + k)
                
                # 外层四个点
                hex_cell.GetPointIds().SetId(4, i_next * nv * nr + j * nr + k)
                hex_cell.GetPointIds().SetId(5, i_next * nv * nr + j * nr + k + 1)
                hex_cell.GetPointIds().SetId(6, i_next * nv * nr + j_next * nr + k + 1)
                hex_cell.GetPointIds().SetId(7, i_next * nv * nr + j_next * nr + k)
                
                ugrid.InsertNextCell(hex_cell.GetCellType(), hex_cell.GetPointIds())
    
    num_points = points.GetNumberOfPoints()
    
    # 环形变形场
    torus_displacement = vtk.vtkFloatArray()
    torus_displacement.SetName("Torus_Displacement")
    torus_displacement.SetNumberOfComponents(3)
    torus_displacement.SetNumberOfTuples(num_points)
    
    # 涡流速度场
    vortex_velocity = vtk.vtkFloatArray()
    vortex_velocity.SetName("Vortex_Velocity")
    vortex_velocity.SetNumberOfComponents(3)
    vortex_velocity.SetNumberOfTuples(num_points)
    
    # 涡量
    vorticity = vtk.vtkFloatArray()
    vorticity.SetName("Vorticity")
    vorticity.SetNumberOfComponents(1)
    vorticity.SetNumberOfTuples(num_points)
    
    # 动压
    dynamic_pressure = vtk.vtkFloatArray()
    dynamic_pressure.SetName("Dynamic_Pressure")
    dynamic_pressure.SetNumberOfComponents(1)
    dynamic_pressure.SetNumberOfTuples(num_points)
    
    for i in range(num_points):
        point = points.GetPoint(i)
        x, y, z = point
        
        # 到Z轴的距离
        rho = math.sqrt(x*x + y*y)
        
        if rho > 0:
            # 环形坐标
            phi = math.atan2(y, x)
            
            # 计算在小圆上的位置
            r_from_center = math.sqrt((rho - R)**2 + z**2)
            
            # 环形变形（径向脉动）
            pulsation = 0.1 * math.sin(4 * phi) * math.cos(2 * math.pi * z / (r_outer - r_inner))
            dx = (x / rho) * pulsation
            dy = (y / rho) * pulsation
            dz = 0.1 * math.sin(2 * phi) * math.sin(math.pi * r_from_center / (r_outer - r_inner))
            
            torus_displacement.SetTuple3(i, dx, dy, dz)
            
            # 涡流速度场
            # 主涡流（沿环形方向）
            circulation_strength = 3.0
            v_phi = circulation_strength * (r_outer - r_from_center) / (r_outer - r_inner)
            vx = -v_phi * y / rho
            vy = v_phi * x / rho
            
            # 次级涡流（沿小圆方向）
            if r_from_center > 0.01:
                # 小圆上的切向速度
                small_circle_phi = math.atan2(z, rho - R)
                v_secondary = 1.0 * math.sin(2 * phi + small_circle_phi)
                
                # 转换为笛卡尔坐标的Z分量
                vz = v_secondary
                
                # 添加径向分量
                vr = 0.2 * math.cos(2 * phi + small_circle_phi)
                vx += vr * (rho - R) / r_from_center * x / rho
                vy += vr * (rho - R) / r_from_center * y / rho
            else:
                vz = 0
            
            vortex_velocity.SetTuple3(i, vx, vy, vz)
            
            # 涡量大小
            omega = abs(v_phi / rho) + abs(v_secondary) if 'v_secondary' in locals() else abs(v_phi / rho)
            vorticity.SetValue(i, omega)
            
            # 动压
            v_mag = math.sqrt(vx*vx + vy*vy + vz*vz)
            q = 0.5 * 1.0 * v_mag * v_mag  # 假设密度为1
            dynamic_pressure.SetValue(i, q)
        else:
            torus_displacement.SetTuple3(i, 0, 0, 0)
            vortex_velocity.SetTuple3(i, 0, 0, 0)
            vorticity.SetValue(i, 0)
            dynamic_pressure.SetValue(i, 0)
    
    # 添加数据数组
    ugrid.GetPointData().AddArray(torus_displacement)
    ugrid.GetPointData().AddArray(vortex_velocity)
    ugrid.GetPointData().AddArray(vorticity)
    ugrid.GetPointData().AddArray(dynamic_pressure)
    ugrid.GetPointData().SetActiveScalars("Dynamic_Pressure")
    ugrid.GetPointData().SetActiveVectors("Torus_Displacement")
    
    return ugrid

def create_helical_structure():
    """创建螺旋结构 - 展示螺旋变形和螺旋流线"""
    
    # 螺旋参数
    nr, ntheta, nz = 8, 32, 40
    max_radius = 2.0
    height = 8.0
    pitch = 2.0  # 螺距
    
    points = vtk.vtkPoints()
    
    for k in range(nz):
        z = (k / (nz-1)) * height - height/2
        helix_angle = 2 * math.pi * z / pitch
        
        for j in range(ntheta):
            theta = (j / ntheta) * 2 * math.pi + helix_angle
            for i in range(nr):
                r = (i / (nr-1)) * max_radius
                
                x = r * math.cos(theta)
                y = r * math.sin(theta)
                points.InsertNextPoint(x, y, z)
    
    # 创建网格
    ugrid = vtk.vtkUnstructuredGrid()
    ugrid.SetPoints(points)
    
    # 创建六面体单元
    for k in range(nz-1):
        for j in range(ntheta):
            j_next = (j + 1) % ntheta
            for i in range(nr-1):
                hex_cell = vtk.vtkHexahedron()
                
                # 底面四个点
                hex_cell.GetPointIds().SetId(0, k * ntheta * nr + j * nr + i)
                hex_cell.GetPointIds().SetId(1, k * ntheta * nr + j * nr + i + 1)
                hex_cell.GetPointIds().SetId(2, k * ntheta * nr + j_next * nr + i + 1)
                hex_cell.GetPointIds().SetId(3, k * ntheta * nr + j_next * nr + i)
                
                # 顶面四个点
                hex_cell.GetPointIds().SetId(4, (k+1) * ntheta * nr + j * nr + i)
                hex_cell.GetPointIds().SetId(5, (k+1) * ntheta * nr + j * nr + i + 1)
                hex_cell.GetPointIds().SetId(6, (k+1) * ntheta * nr + j_next * nr + i + 1)
                hex_cell.GetPointIds().SetId(7, (k+1) * ntheta * nr + j_next * nr + i)
                
                ugrid.InsertNextCell(hex_cell.GetCellType(), hex_cell.GetPointIds())
    
    num_points = points.GetNumberOfPoints()
    
    # 螺旋位移场
    helical_displacement = vtk.vtkFloatArray()
    helical_displacement.SetName("Helical_Displacement")
    helical_displacement.SetNumberOfComponents(3)
    helical_displacement.SetNumberOfTuples(num_points)
    
    # 螺旋速度场
    helical_velocity = vtk.vtkFloatArray()
    helical_velocity.SetName("Helical_Velocity")
    helical_velocity.SetNumberOfComponents(3)
    helical_velocity.SetNumberOfTuples(num_points)
    
    # 螺旋应力
    helical_stress = vtk.vtkFloatArray()
    helical_stress.SetName("Helical_Stress")
    helical_stress.SetNumberOfComponents(1)
    helical_stress.SetNumberOfTuples(num_points)
    
    # 温度场
    temperature = vtk.vtkFloatArray()
    temperature.SetName("Temperature")
    temperature.SetNumberOfComponents(1)
    temperature.SetNumberOfTuples(num_points)
    
    for i in range(num_points):
        point = points.GetPoint(i)
        x, y, z = point
        r = math.sqrt(x*x + y*y)
        
        if r > 0:
            theta = math.atan2(y, x)
            
            # 螺旋位移
            helix_factor = math.sin(2 * math.pi * z / pitch + theta)
            dx = 0.1 * helix_factor * x / r
            dy = 0.1 * helix_factor * y / r
            dz = 0.05 * math.cos(theta + 2 * math.pi * z / pitch)
            
            helical_displacement.SetTuple3(i, dx, dy, dz)
            
            # 螺旋速度场
            vx = -y + 0.2 * z * x / max_radius
            vy = x + 0.2 * z * y / max_radius
            vz = 1.0 + 0.5 * math.sin(theta + 2 * math.pi * z / pitch)
            
            helical_velocity.SetTuple3(i, vx, vy, vz)
            
            # 螺旋应力
            stress = 100 * (1 + 0.5 * math.sin(4 * theta + 4 * math.pi * z / pitch)) * (max_radius - r) / max_radius
            helical_stress.SetValue(i, stress)
            
            # 温度场（螺旋分布）
            temp = 300 + 100 * math.sin(theta + 2 * math.pi * z / pitch) * (1 - r / max_radius)
            temperature.SetValue(i, temp)
        else:
            helical_displacement.SetTuple3(i, 0, 0, 0)
            helical_velocity.SetTuple3(i, 0, 0, 1.0)
            helical_stress.SetValue(i, 100)
            temperature.SetValue(i, 300)
    
    # 添加数据数组
    ugrid.GetPointData().AddArray(helical_displacement)
    ugrid.GetPointData().AddArray(helical_velocity)
    ugrid.GetPointData().AddArray(helical_stress)
    ugrid.GetPointData().AddArray(temperature)
    ugrid.GetPointData().SetActiveScalars("Temperature")
    ugrid.GetPointData().SetActiveVectors("Helical_Displacement")
    
    return ugrid

def save_beautiful_3d_data():
    """保存所有美观的3D数据"""
    
    test_cases = [
        ("twisted_cylinder.vtu", create_twisted_cylinder, "扭转圆柱体 - 螺旋变形和涡流"),
        ("radial_sphere.vtu", create_sphere_with_radial_field, "径向球体 - 径向变形和发散流"),
        ("torus_vortex.vtu", create_torus_vortex, "环形涡流 - 环形变形和涡流线"),
        ("helical_structure.vtu", create_helical_structure, "螺旋结构 - 螺旋变形和螺旋流")
    ]
    
    print("=" * 70)
    print("创建优美的三维体积数据")
    print("=" * 70)
    
    for filename, create_func, description in test_cases:
        print(f"\n🎨 正在创建: {filename}")
        print(f"📝 描述: {description}")
        
        ugrid = create_func()
        
        writer = vtk.vtkXMLUnstructuredGridWriter()
        writer.SetFileName(filename)
        writer.SetInputData(ugrid)
        writer.Write()
        
        print(f"✅ 已保存: {filename}")
        print(f"   📊 点数: {ugrid.GetNumberOfPoints():,}")
        print(f"   🔷 单元数: {ugrid.GetNumberOfCells():,}")
        
        # 显示数据数组信息
        pointData = ugrid.GetPointData()
        print("   📋 数据数组:")
        for i in range(pointData.GetNumberOfArrays()):
            array = pointData.GetArray(i)
            array_type = "🔄 矢量" if array.GetNumberOfComponents() == 3 else "📈 标量"
            print(f"      - {array.GetName()}: {array_type}")
    
    print("\n" + "=" * 70)
    print("🎉 所有美观的3D数据创建完成！")
    print("=" * 70)
    
    print("\n🎯 推荐测试顺序:")
    print("1. 🌪️  twisted_cylinder.vtu - 观察扭转变形，流线呈螺旋状")
    print("2. 🌟 radial_sphere.vtu - 观察径向膨胀，流线从中心发散")
    print("3. 🍩 torus_vortex.vtu - 观察环形变形，流线呈涡流状")
    print("4. 🌀 helical_structure.vtu - 观察螺旋变形，流线呈螺旋上升")
    
    print("\n� 最佳观观察设置:")
    print("📐 变形图:")
    print("   - 缩放: 0.5-2.0倍（根据效果调节）")
    print("   - 显示原始几何体: 开启（对比效果）")
    print("   - 透明度: 自动调节到10%")
    
    print("🌊 流线:")
    print("   - 流线数量: 100-500条")
    print("   - 积分步长: 0.01-0.1")
    print("   - 种子点: 随机分布")
    
    print("🎨 颜色映射:")
    print("   - 彩虹色谱: 适合温度和压力场")
    print("   - 蓝白红: 适合应力场")
    print("   - 热力图: 适合涡量场")
    
    print("\n💡 观察要点:")
    print("- 扭转圆柱: 注意螺旋变形的对称性")
    print("- 径向球体: 观察从中心向外的完美对称")
    print("- 环形涡流: 注意环形结构的复杂流动")
    print("- 螺旋结构: 观察螺旋上升的优美曲线")

if __name__ == "__main__":
    save_beautiful_3d_data()