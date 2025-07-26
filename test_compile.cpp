// 简单的编译测试文件
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkSTLReader.h>

int main() {
    // 测试VTK读取器是否可用
    auto objReader = vtkSmartPointer<vtkOBJReader>::New();
    auto plyReader = vtkSmartPointer<vtkPLYReader>::New();
    auto stlReader = vtkSmartPointer<vtkSTLReader>::New();
    
    return 0;
}