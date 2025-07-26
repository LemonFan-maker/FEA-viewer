#ifndef CONTOURWIDGET_H
#define CONTOURWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QListWidget>

#include <vtkSmartPointer.h>
#include <vtkContourFilter.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <QList>
#include <cmath>

class ContourWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContourWidget(QWidget *parent = nullptr);
    ~ContourWidget();

    void setData(vtkUnstructuredGrid *data);
    void setRenderer(vtkRenderer *renderer);
    void setDataRange(double min, double max);
    void setActiveScalarArray(const QString &arrayName, bool isPointData);
    vtkActor* getContourActor() const { return m_contourActor; }

signals:
    void contoursChanged();

private slots:
    void onContourEnabledChanged(bool enabled);
    void onAddContour();
    void onRemoveContour();
    void onClearContours();
    void onAutoContoursChanged();

private:
    void setupUI();
    void setupVTK();
    void updateContours();
    void generateAutoContours();

    // UI组件
    QCheckBox *m_enableContourCheckBox;
    QDoubleSpinBox *m_contourValueSpinBox;
    QPushButton *m_addContourButton;
    QPushButton *m_removeContourButton;
    QPushButton *m_clearContoursButton;
    QListWidget *m_contourListWidget;
    
    // 自动等值线
    QCheckBox *m_autoContoursCheckBox;
    QSpinBox *m_numContoursSpinBox;
    
    QLabel *m_rangeLabel;

    // VTK组件
    vtkSmartPointer<vtkContourFilter> m_contourFilter;
    vtkSmartPointer<vtkDataSetMapper> m_contourMapper;
    vtkSmartPointer<vtkActor> m_contourActor;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkUnstructuredGrid> m_inputData;

    // 数据
    double m_dataMin;
    double m_dataMax;
    bool m_contourEnabled;
    QList<double> m_contourValues;
};

#endif // CONTOURWIDGET_H