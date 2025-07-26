#include "ContourWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>

ContourWidget::ContourWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderer(nullptr)
    , m_inputData(nullptr)
    , m_dataMin(0.0)
    , m_dataMax(1.0)
    , m_contourEnabled(false)
{
    setupUI();
    setupVTK();
}

ContourWidget::~ContourWidget()
{
}

void ContourWidget::setupUI()
{
    setWindowTitle("等值面/等值线控制");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 等值面启用控制
    m_enableContourCheckBox = new QCheckBox("启用等值面", this);
    connect(m_enableContourCheckBox, &QCheckBox::toggled,
            this, &ContourWidget::onContourEnabledChanged);
    mainLayout->addWidget(m_enableContourCheckBox);
    
    // 数据范围显示
    m_rangeLabel = new QLabel("数据范围: [0.0, 1.0]", this);
    mainLayout->addWidget(m_rangeLabel);
    
    // 手动添加等值面
    QGroupBox *manualGroup = new QGroupBox("手动等值面", this);
    QVBoxLayout *manualLayout = new QVBoxLayout(manualGroup);
    
    // 等值面数值输入
    QHBoxLayout *valueLayout = new QHBoxLayout();
    valueLayout->addWidget(new QLabel("等值:", this));
    
    m_contourValueSpinBox = new QDoubleSpinBox(this);
    m_contourValueSpinBox->setDecimals(6);
    m_contourValueSpinBox->setRange(-1e6, 1e6);
    m_contourValueSpinBox->setValue(0.5);
    m_contourValueSpinBox->setEnabled(false);
    valueLayout->addWidget(m_contourValueSpinBox);
    
    m_addContourButton = new QPushButton("添加", this);
    m_addContourButton->setEnabled(false);
    connect(m_addContourButton, &QPushButton::clicked,
            this, &ContourWidget::onAddContour);
    valueLayout->addWidget(m_addContourButton);
    
    manualLayout->addLayout(valueLayout);
    
    // 等值面列表
    m_contourListWidget = new QListWidget(this);
    m_contourListWidget->setMaximumHeight(100);
    manualLayout->addWidget(m_contourListWidget);
    
    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_removeContourButton = new QPushButton("删除选中", this);
    m_removeContourButton->setEnabled(false);
    connect(m_removeContourButton, &QPushButton::clicked,
            this, &ContourWidget::onRemoveContour);
    buttonLayout->addWidget(m_removeContourButton);
    
    m_clearContoursButton = new QPushButton("清空全部", this);
    m_clearContoursButton->setEnabled(false);
    connect(m_clearContoursButton, &QPushButton::clicked,
            this, &ContourWidget::onClearContours);
    buttonLayout->addWidget(m_clearContoursButton);
    
    manualLayout->addLayout(buttonLayout);
    mainLayout->addWidget(manualGroup);
    
    // 自动等值面
    QGroupBox *autoGroup = new QGroupBox("自动等值面", this);
    QVBoxLayout *autoLayout = new QVBoxLayout(autoGroup);
    
    m_autoContoursCheckBox = new QCheckBox("自动生成等值面", this);
    connect(m_autoContoursCheckBox, &QCheckBox::toggled,
            this, &ContourWidget::onAutoContoursChanged);
    autoLayout->addWidget(m_autoContoursCheckBox);
    
    QHBoxLayout *numLayout = new QHBoxLayout();
    numLayout->addWidget(new QLabel("数量:", this));
    
    m_numContoursSpinBox = new QSpinBox(this);
    m_numContoursSpinBox->setRange(1, 20);
    m_numContoursSpinBox->setValue(5);
    m_numContoursSpinBox->setEnabled(false);
    connect(m_numContoursSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ContourWidget::onAutoContoursChanged);
    numLayout->addWidget(m_numContoursSpinBox);
    
    autoLayout->addLayout(numLayout);
    mainLayout->addWidget(autoGroup);
    
    mainLayout->addStretch();
}

void ContourWidget::setupVTK()
{
    // 创建等值面过滤器
    m_contourFilter = vtkSmartPointer<vtkContourFilter>::New();
    
    // 创建映射器和演员
    m_contourMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_contourMapper->SetInputConnection(m_contourFilter->GetOutputPort());
    m_contourMapper->ScalarVisibilityOff(); // 不使用标量着色
    
    m_contourActor = vtkSmartPointer<vtkActor>::New();
    m_contourActor->SetMapper(m_contourMapper);
    
    // 设置等值面外观
    m_contourActor->GetProperty()->SetColor(1.0, 0.0, 0.0); // 红色，更醒目
    m_contourActor->GetProperty()->SetLineWidth(3.0); // 更粗的线条
    m_contourActor->GetProperty()->SetOpacity(1.0); // 完全不透明
    m_contourActor->GetProperty()->SetRepresentationToWireframe(); // 确保显示为线框
}

void ContourWidget::setData(vtkUnstructuredGrid *data)
{
    m_inputData = data;
    if (m_inputData) {
        m_contourFilter->SetInputData(m_inputData);
        
        // 详细调试信息
        qDebug() << "ContourWidget: 设置数据，点数:" << m_inputData->GetNumberOfPoints() 
                 << "单元数:" << m_inputData->GetNumberOfCells();
        
        // 检查点数据数组
        vtkPointData* pointData = m_inputData->GetPointData();
        qDebug() << "点数据数组数量:" << pointData->GetNumberOfArrays();
        for (int i = 0; i < pointData->GetNumberOfArrays(); ++i) {
            vtkDataArray* array = pointData->GetArray(i);
            if (array && array->GetName()) {
                double range[2];
                array->GetRange(range);
                qDebug() << "  点数据数组" << i << ":" << array->GetName() 
                         << "范围:[" << range[0] << "," << range[1] << "]";
            }
        }
        
        // 检查单元数据数组
        vtkCellData* cellData = m_inputData->GetCellData();
        qDebug() << "单元数据数组数量:" << cellData->GetNumberOfArrays();
        for (int i = 0; i < cellData->GetNumberOfArrays(); ++i) {
            vtkDataArray* array = cellData->GetArray(i);
            if (array && array->GetName()) {
                double range[2];
                array->GetRange(range);
                qDebug() << "  单元数据数组" << i << ":" << array->GetName() 
                         << "范围:[" << range[0] << "," << range[1] << "]";
            }
        }
        
        // 检查活动标量
        vtkDataArray* activeScalars = m_inputData->GetPointData()->GetScalars();
        if (activeScalars) {
            double range[2];
            activeScalars->GetRange(range);
            qDebug() << "活动点标量:" << (activeScalars->GetName() ? activeScalars->GetName() : "无名")
                     << "范围:[" << range[0] << "," << range[1] << "]";
        } else {
            qDebug() << "没有活动点标量";
        }
        
        activeScalars = m_inputData->GetCellData()->GetScalars();
        if (activeScalars) {
            double range[2];
            activeScalars->GetRange(range);
            qDebug() << "活动单元标量:" << (activeScalars->GetName() ? activeScalars->GetName() : "无名")
                     << "范围:[" << range[0] << "," << range[1] << "]";
        } else {
            qDebug() << "没有活动单元标量";
        }
    }
}

void ContourWidget::setRenderer(vtkRenderer *renderer)
{
    m_renderer = renderer;
}

void ContourWidget::setDataRange(double min, double max)
{
    m_dataMin = min;
    m_dataMax = max;
    
    m_rangeLabel->setText(QString("数据范围: [%1, %2]")
                         .arg(m_dataMin, 0, 'f', 3)
                         .arg(m_dataMax, 0, 'f', 3));
    
    // 更新输入框范围
    m_contourValueSpinBox->setRange(m_dataMin, m_dataMax);
    m_contourValueSpinBox->setValue((m_dataMin + m_dataMax) / 2.0);
}

void ContourWidget::setActiveScalarArray(const QString &arrayName, bool isPointData)
{
    if (!m_inputData || arrayName.isEmpty()) return;
    
    qDebug() << "ContourWidget: 设置活动标量数组:" << arrayName << "是否为点数据:" << isPointData;
    
    if (isPointData) {
        m_inputData->GetPointData()->SetActiveScalars(arrayName.toStdString().c_str());
    } else {
        m_inputData->GetCellData()->SetActiveScalars(arrayName.toStdString().c_str());
    }
    
    // 重新获取数据范围
    double range[2];
    m_inputData->GetScalarRange(range);
    setDataRange(range[0], range[1]);
}

void ContourWidget::onContourEnabledChanged(bool enabled)
{
    m_contourEnabled = enabled;
    
    // 设置属性供外部查询
    setProperty("contourEnabled", enabled);
    
    // 启用/禁用控件
    m_contourValueSpinBox->setEnabled(enabled);
    m_addContourButton->setEnabled(enabled);
    m_removeContourButton->setEnabled(enabled && m_contourListWidget->currentRow() >= 0);
    m_clearContoursButton->setEnabled(enabled && m_contourListWidget->count() > 0);
    m_autoContoursCheckBox->setEnabled(enabled);
    m_numContoursSpinBox->setEnabled(enabled && m_autoContoursCheckBox->isChecked());
    
    emit contoursChanged();
}

void ContourWidget::onAddContour()
{
    double value = m_contourValueSpinBox->value();
    
    // 检查是否已存在相同值
    for (double existingValue : m_contourValues) {
        if (qAbs(existingValue - value) < 1e-6) {
            QMessageBox::information(this, "提示", "该等值面已存在");
            return;
        }
    }
    
    // 添加到列表
    m_contourValues.append(value);
    m_contourListWidget->addItem(QString::number(value, 'f', 6));
    
    // 更新等值面
    updateContours();
    
    // 启用删除和清空按钮
    m_removeContourButton->setEnabled(true);
    m_clearContoursButton->setEnabled(true);
}

void ContourWidget::onRemoveContour()
{
    int currentRow = m_contourListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_contourValues.size()) {
        m_contourValues.removeAt(currentRow);
        delete m_contourListWidget->takeItem(currentRow);
        
        updateContours();
        
        // 如果没有等值面了，禁用删除和清空按钮
        if (m_contourValues.isEmpty()) {
            m_removeContourButton->setEnabled(false);
            m_clearContoursButton->setEnabled(false);
        }
    }
}

void ContourWidget::onClearContours()
{
    m_contourValues.clear();
    m_contourListWidget->clear();
    
    updateContours();
    
    // 禁用删除和清空按钮
    m_removeContourButton->setEnabled(false);
    m_clearContoursButton->setEnabled(false);
}

void ContourWidget::onAutoContoursChanged()
{
    bool autoEnabled = m_autoContoursCheckBox->isChecked();
    m_numContoursSpinBox->setEnabled(m_contourEnabled && autoEnabled);
    
    if (autoEnabled) {
        generateAutoContours();
    } else {
        // 如果关闭自动模式，保持当前手动添加的等值面
        updateContours();
    }
}

void ContourWidget::generateAutoContours()
{
    if (!m_autoContoursCheckBox->isChecked()) return;
    
    int numContours = m_numContoursSpinBox->value();
    
    // 清空现有等值面
    m_contourValues.clear();
    m_contourListWidget->clear();
    
    // 生成均匀分布的等值面
    for (int i = 0; i < numContours; ++i) {
        double t = static_cast<double>(i + 1) / (numContours + 1);
        double value = m_dataMin + t * (m_dataMax - m_dataMin);
        
        m_contourValues.append(value);
        m_contourListWidget->addItem(QString::number(value, 'f', 6));
    }
    
    updateContours();
    
    // 启用删除和清空按钮
    m_removeContourButton->setEnabled(true);
    m_clearContoursButton->setEnabled(true);
}

void ContourWidget::updateContours()
{
    if (!m_inputData || !m_contourEnabled) {
        setProperty("hasContours", false);
        emit contoursChanged();
        return;
    }
    
    // 清空现有等值面
    m_contourFilter->SetNumberOfContours(0);
    
    // 添加所有等值面
    for (int i = 0; i < m_contourValues.size(); ++i) {
        m_contourFilter->SetValue(i, m_contourValues[i]);
    }
    
    bool hasContours = !m_contourValues.isEmpty();
    setProperty("hasContours", hasContours);
    
    if (hasContours) {
        // 显示等值面数值
        qDebug() << "ContourWidget: 等值面数值:";
        for (int i = 0; i < m_contourValues.size(); ++i) {
            qDebug() << "  等值面" << i << ":" << m_contourValues[i];
        }
        
        m_contourFilter->Update();
        
        // 调试信息
        qDebug() << "ContourWidget: 更新等值面，数量:" << m_contourValues.size()
                 << "输出点数:" << m_contourFilter->GetOutput()->GetNumberOfPoints()
                 << "输出单元数:" << m_contourFilter->GetOutput()->GetNumberOfCells();
        
        // 如果没有输出，检查可能的原因
        if (m_contourFilter->GetOutput()->GetNumberOfPoints() == 0) {
            qDebug() << "警告: 等值面没有输出，可能原因:";
            qDebug() << "1. 等值面数值超出数据范围";
            qDebug() << "2. 没有正确设置活动标量数组";
            qDebug() << "3. 数据类型不匹配";
        }
    }
    
    emit contoursChanged();
}

