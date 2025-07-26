#include "ClippingWidget.h"

ClippingWidget::ClippingWidget(QWidget *parent)
    : QWidget(parent)
    , m_renderer(nullptr)
    , m_inputData(nullptr)
    , m_clippingEnabled(false)
{
    setupUI();
    setupVTK();
}

ClippingWidget::~ClippingWidget()
{
}

void ClippingWidget::setupUI()
{
    setWindowTitle("剖切控制");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 剖切启用控制
    m_enableClippingCheckBox = new QCheckBox("启用剖切", this);
    connect(m_enableClippingCheckBox, &QCheckBox::toggled, 
            this, &ClippingWidget::onClippingEnabledChanged);
    mainLayout->addWidget(m_enableClippingCheckBox);
    
    // 剖切平面位置控制
    QGroupBox *positionGroup = new QGroupBox("剖切位置", this);
    QVBoxLayout *positionLayout = new QVBoxLayout(positionGroup);
    
    m_positionLabel = new QLabel("位置: 50%", this);
    positionLayout->addWidget(m_positionLabel);
    
    m_planePositionSlider = new QSlider(Qt::Horizontal, this);
    m_planePositionSlider->setRange(0, 100);
    m_planePositionSlider->setValue(50);
    m_planePositionSlider->setEnabled(false);
    connect(m_planePositionSlider, &QSlider::valueChanged,
            this, &ClippingWidget::onPlanePositionChanged);
    positionLayout->addWidget(m_planePositionSlider);
    
    mainLayout->addWidget(positionGroup);
    
    // 剖切平面法向量控制
    QGroupBox *normalGroup = new QGroupBox("剖切方向", this);
    QVBoxLayout *normalLayout = new QVBoxLayout(normalGroup);
    
    m_normalLabel = new QLabel("法向量: (1.0, 0.0, 0.0)", this);
    normalLayout->addWidget(m_normalLabel);
    
    // X方向
    QHBoxLayout *xLayout = new QHBoxLayout();
    xLayout->addWidget(new QLabel("X:", this));
    m_normalXSlider = new QSlider(Qt::Horizontal, this);
    m_normalXSlider->setRange(-100, 100);
    m_normalXSlider->setValue(100);
    m_normalXSlider->setEnabled(false);
    connect(m_normalXSlider, &QSlider::valueChanged,
            this, &ClippingWidget::onPlaneNormalChanged);
    xLayout->addWidget(m_normalXSlider);
    normalLayout->addLayout(xLayout);
    
    // Y方向
    QHBoxLayout *yLayout = new QHBoxLayout();
    yLayout->addWidget(new QLabel("Y:", this));
    m_normalYSlider = new QSlider(Qt::Horizontal, this);
    m_normalYSlider->setRange(-100, 100);
    m_normalYSlider->setValue(0);
    m_normalYSlider->setEnabled(false);
    connect(m_normalYSlider, &QSlider::valueChanged,
            this, &ClippingWidget::onPlaneNormalChanged);
    yLayout->addWidget(m_normalYSlider);
    normalLayout->addLayout(yLayout);
    
    // Z方向
    QHBoxLayout *zLayout = new QHBoxLayout();
    zLayout->addWidget(new QLabel("Z:", this));
    m_normalZSlider = new QSlider(Qt::Horizontal, this);
    m_normalZSlider->setRange(-100, 100);
    m_normalZSlider->setValue(0);
    m_normalZSlider->setEnabled(false);
    connect(m_normalZSlider, &QSlider::valueChanged,
            this, &ClippingWidget::onPlaneNormalChanged);
    zLayout->addWidget(m_normalZSlider);
    normalLayout->addLayout(zLayout);
    
    mainLayout->addWidget(normalGroup);
    
    mainLayout->addStretch();
}

void ClippingWidget::setupVTK()
{
    // 创建剖切平面
    m_clippingPlane = vtkSmartPointer<vtkPlane>::New();
    m_clippingPlane->SetOrigin(0, 0, 0);
    m_clippingPlane->SetNormal(1, 0, 0);
    
    // 创建剖切过滤器
    m_clipFilter = vtkSmartPointer<vtkClipDataSet>::New();
    m_clipFilter->SetClipFunction(m_clippingPlane);
    m_clipFilter->GenerateClippedOutputOn();
    
    // 创建映射器和演员
    m_clippedMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    m_clippedMapper->SetInputConnection(m_clipFilter->GetOutputPort());
    
    m_clippedActor = vtkSmartPointer<vtkActor>::New();
    m_clippedActor->SetMapper(m_clippedMapper);
}

void ClippingWidget::setData(vtkUnstructuredGrid *data)
{
    m_inputData = data;
    if (m_inputData) {
        m_clipFilter->SetInputData(m_inputData);
        updateClipping();
    }
}

void ClippingWidget::setRenderer(vtkRenderer *renderer)
{
    m_renderer = renderer;
}

void ClippingWidget::onClippingEnabledChanged(bool enabled)
{
    m_clippingEnabled = enabled;
    
    // 设置属性供外部查询
    setProperty("clippingEnabled", enabled);
    
    // 启用/禁用控件
    m_planePositionSlider->setEnabled(enabled);
    m_normalXSlider->setEnabled(enabled);
    m_normalYSlider->setEnabled(enabled);
    m_normalZSlider->setEnabled(enabled);
    
    emit clippingChanged();
}

void ClippingWidget::onPlanePositionChanged()
{
    if (!m_inputData) return;
    
    int value = m_planePositionSlider->value();
    m_positionLabel->setText(QString("位置: %1%").arg(value));
    
    // 计算模型边界
    double bounds[6];
    m_inputData->GetBounds(bounds);
    
    // 根据当前法向量计算位置
    double nx = m_normalXSlider->value() / 100.0;
    double ny = m_normalYSlider->value() / 100.0;
    double nz = m_normalZSlider->value() / 100.0;
    
    // 归一化法向量
    double length = sqrt(nx*nx + ny*ny + nz*nz);
    if (length > 0) {
        nx /= length;
        ny /= length;
        nz /= length;
    }
    
    // 计算中心点
    double center[3] = {
        (bounds[0] + bounds[1]) / 2.0,
        (bounds[2] + bounds[3]) / 2.0,
        (bounds[4] + bounds[5]) / 2.0
    };
    
    // 计算模型尺寸
    double size = sqrt(
        (bounds[1] - bounds[0]) * (bounds[1] - bounds[0]) +
        (bounds[3] - bounds[2]) * (bounds[3] - bounds[2]) +
        (bounds[5] - bounds[4]) * (bounds[5] - bounds[4])
    );
    
    // 根据滑块值计算平面位置
    double t = (value - 50) / 50.0; // -1 到 1
    double origin[3] = {
        center[0] + t * size * 0.5 * nx,
        center[1] + t * size * 0.5 * ny,
        center[2] + t * size * 0.5 * nz
    };
    
    m_clippingPlane->SetOrigin(origin);
    updateClipping();
}

void ClippingWidget::onPlaneNormalChanged()
{
    double nx = m_normalXSlider->value() / 100.0;
    double ny = m_normalYSlider->value() / 100.0;
    double nz = m_normalZSlider->value() / 100.0;
    
    // 归一化法向量
    double length = sqrt(nx*nx + ny*ny + nz*nz);
    if (length > 0) {
        nx /= length;
        ny /= length;
        nz /= length;
    } else {
        nx = 1.0; ny = 0.0; nz = 0.0; // 默认法向量
    }
    
    m_normalLabel->setText(QString("法向量: (%1, %2, %3)")
                          .arg(nx, 0, 'f', 2)
                          .arg(ny, 0, 'f', 2)
                          .arg(nz, 0, 'f', 2));
    
    m_clippingPlane->SetNormal(nx, ny, nz);
    
    // 更新位置（因为法向量改变了）
    onPlanePositionChanged();
}

void ClippingWidget::updateClipping()
{
    if (!m_inputData || !m_renderer) return;
    
    if (m_clippingEnabled) {
        m_clipFilter->Update();
        emit clippingChanged();
    }
}

