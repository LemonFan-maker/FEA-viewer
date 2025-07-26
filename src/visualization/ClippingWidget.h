#ifndef CLIPPINGWIDGET_H
#define CLIPPINGWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>

#include <vtkSmartPointer.h>
#include <vtkPlane.h>
#include <vtkClipDataSet.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <cmath>

class ClippingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClippingWidget(QWidget *parent = nullptr);
    ~ClippingWidget();

    void setData(vtkUnstructuredGrid *data);
    void setRenderer(vtkRenderer *renderer);
    vtkActor* getClippedActor() const { return m_clippedActor; }

signals:
    void clippingChanged();

private slots:
    void onClippingEnabledChanged(bool enabled);
    void onPlanePositionChanged();
    void onPlaneNormalChanged();

private:
    void setupUI();
    void setupVTK();
    void updateClipping();

    // UI组件
    QCheckBox *m_enableClippingCheckBox;
    QSlider *m_planePositionSlider;
    QSlider *m_normalXSlider;
    QSlider *m_normalYSlider;
    QSlider *m_normalZSlider;
    QLabel *m_positionLabel;
    QLabel *m_normalLabel;

    // VTK组件
    vtkSmartPointer<vtkPlane> m_clippingPlane;
    vtkSmartPointer<vtkClipDataSet> m_clipFilter;
    vtkSmartPointer<vtkDataSetMapper> m_clippedMapper;
    vtkSmartPointer<vtkActor> m_clippedActor;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkUnstructuredGrid> m_inputData;

    bool m_clippingEnabled;
};

#endif // CLIPPINGWIDGET_H