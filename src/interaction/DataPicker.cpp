#include "DataPicker.h"
#include <QDebug>
#include <vtkRenderWindow.h>

DataPicker::DataPicker(QObject *parent)
    : QObject(parent)
    , m_renderer(nullptr)
    , m_interactor(nullptr)
    , m_data(nullptr)
    , m_isPointData(true)
    , m_pickingEnabled(false)
{
    setupPickers();
}

DataPicker::~DataPicker()
{
}

void DataPicker::setupPickers()
{
    // 创建单元拾取器
    m_cellPicker = vtkSmartPointer<vtkCellPicker>::New();
    m_cellPicker->SetTolerance(0.005); // 设置拾取容差
    
    // 创建点拾取器
    m_pointPicker = vtkSmartPointer<vtkPointPicker>::New();
    m_pointPicker->SetTolerance(0.005);
}

void DataPicker::setRenderer(vtkRenderer *renderer)
{
    m_renderer = renderer;
}

void DataPicker::setInteractor(vtkRenderWindowInteractor *interactor)
{
    m_interactor = interactor;
}

void DataPicker::setData(vtkUnstructuredGrid *data)
{
    m_data = data;
}

void DataPicker::setActiveScalarArray(const QString &arrayName, bool isPointData)
{
    m_activeArrayName = arrayName;
    m_isPointData = isPointData;
}

void DataPicker::enablePicking(bool enabled)
{
    m_pickingEnabled = enabled;
}

void DataPicker::onMouseClick(int x, int y)
{
    if (!m_pickingEnabled || !m_renderer || !m_data) {
        return;
    }

    // 获取渲染窗口尺寸
    int *size = m_renderer->GetRenderWindow()->GetSize();
    
    // VTK使用左下角为原点，Qt使用左上角，需要转换Y坐标
    int vtkY = size[1] - y - 1;
    
    bool pickSuccessful = false;
    double position[3] = {0, 0, 0};
    double value = 0.0;
    vtkIdType cellId = -1;
    vtkIdType pointId = -1;

    if (m_isPointData) {
        // 使用点拾取器
        if (m_pointPicker->Pick(x, vtkY, 0, m_renderer)) {
            pickSuccessful = true;
            m_pointPicker->GetPickPosition(position);
            pointId = m_pointPicker->GetPointId();
            
            // 获取点数据值
            if (pointId >= 0 && !m_activeArrayName.isEmpty()) {
                vtkDataArray *array = m_data->GetPointData()->GetArray(m_activeArrayName.toStdString().c_str());
                if (array) {
                    value = array->GetTuple1(pointId);
                }
            }
        }
    } else {
        // 使用单元拾取器
        if (m_cellPicker->Pick(x, vtkY, 0, m_renderer)) {
            pickSuccessful = true;
            m_cellPicker->GetPickPosition(position);
            cellId = m_cellPicker->GetCellId();
            
            // 获取单元数据值
            if (cellId >= 0 && !m_activeArrayName.isEmpty()) {
                vtkDataArray *array = m_data->GetCellData()->GetArray(m_activeArrayName.toStdString().c_str());
                if (array) {
                    value = array->GetTuple1(cellId);
                }
            }
        }
    }

    if (pickSuccessful) {
        QString info = formatPickInfo(position, value, cellId, pointId);
        emit pointPicked(info);
    } else {
        emit pointPicked("未拾取到数据点");
    }
}

QString DataPicker::formatPickInfo(double position[3], double value, vtkIdType cellId, vtkIdType pointId)
{
    QString info;
    
    // 坐标信息
    info += QString("坐标: (%.3f, %.3f, %.3f)").arg(position[0]).arg(position[1]).arg(position[2]);
    
    // 数据值信息
    if (!m_activeArrayName.isEmpty()) {
        info += QString(" | %1: %.6f").arg(m_activeArrayName).arg(value);
    }
    
    // ID信息
    if (m_isPointData && pointId >= 0) {
        info += QString(" | 点ID: %1").arg(pointId);
    } else if (!m_isPointData && cellId >= 0) {
        info += QString(" | 单元ID: %1").arg(cellId);
    }
    
    return info;
}

