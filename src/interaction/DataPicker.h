#ifndef DATAPICKER_H
#define DATAPICKER_H

#include <QObject>
#include <QString>

#include <vtkSmartPointer.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>

class DataPicker : public QObject
{
    Q_OBJECT

public:
    explicit DataPicker(QObject *parent = nullptr);
    ~DataPicker();

    void setRenderer(vtkRenderer *renderer);
    void setInteractor(vtkRenderWindowInteractor *interactor);
    void setData(vtkUnstructuredGrid *data);
    void setActiveScalarArray(const QString &arrayName, bool isPointData);

    void enablePicking(bool enabled);
    bool isPickingEnabled() const { return m_pickingEnabled; }

signals:
    void pointPicked(const QString &info);

public slots:
    void onMouseClick(int x, int y);

private:
    void setupPickers();
    QString formatPickInfo(double position[3], double value, vtkIdType cellId, vtkIdType pointId);

    // VTK组件
    vtkSmartPointer<vtkCellPicker> m_cellPicker;
    vtkSmartPointer<vtkPointPicker> m_pointPicker;
    vtkSmartPointer<vtkRenderer> m_renderer;
    vtkSmartPointer<vtkRenderWindowInteractor> m_interactor;
    vtkSmartPointer<vtkUnstructuredGrid> m_data;

    // 数据信息
    QString m_activeArrayName;
    bool m_isPointData;
    bool m_pickingEnabled;
};

#endif // DATAPICKER_H