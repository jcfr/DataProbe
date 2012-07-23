/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

  Based on the scripted module "DataProbe.py" developed by Steve Pieper (Isomics).

==============================================================================*/

// Qt includes
#include <QColor>
#include <QDebug>
#include <QLabel>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKSliceView.h>

// SlicerQt includes
#include <qSlicerLayoutManager.h>

// DataProbe includes
#include "qSlicerDataProbeInfoWidget.h"
#include "ui_qSlicerDataProbeInfoWidget.h"
#include "vtkSlicerDataProbeLogic.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

// MRMLWidgets includes
#include <qMRMLSliceWidget.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceLayerLogic.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkInteractorObserver.h>
#include <vtkTransform.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DataProbe
class qSlicerDataProbeInfoWidgetPrivate : public Ui_qSlicerDataProbeInfoWidget
{
  Q_DECLARE_PUBLIC(qSlicerDataProbeInfoWidget);
protected:
  qSlicerDataProbeInfoWidget* const q_ptr;
public:
  qSlicerDataProbeInfoWidgetPrivate(qSlicerDataProbeInfoWidget& object);

  typedef QList<QLabel*> RowOfLayerLabelsType;
  typedef QHash<QString, RowOfLayerLabelsType> RowsOfLayerLabelsType;

  void init();
  void resetLabels();
  qMRMLSliceWidget * slicerWidget(vtkInteractorObserver * interactorStyle) const;
  QList<vtkInteractorObserver*> currentLayoutSliceViewInteractorStyles() const;
  QList<double> convertXYZToIJK(vtkMRMLSliceLayerLogic* slicerLayerLogic, const QList<double>& xyz) const;

  RowsOfLayerLabelsType RowsOfLayerLabels;
  qSlicerLayoutManager * LayoutManager;
  QList<vtkInteractorObserver*> ObservedInteractorStyles;
  vtkSmartPointer<vtkSlicerDataProbeLogic> DataProbeLogic;

};

//-----------------------------------------------------------------------------
// qSlicerDataProbeInfoWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerDataProbeInfoWidgetPrivate::
qSlicerDataProbeInfoWidgetPrivate(qSlicerDataProbeInfoWidget& object)
  : q_ptr(&object), LayoutManager(0)
{
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeInfoWidgetPrivate::init()
{
  Q_Q(qSlicerDataProbeInfoWidget);
  this->setupUi(q);
  this->RowsOfLayerLabels.insert(
        "L", RowOfLayerLabelsType() << this->L_LayerName << this->L_LayerIJK << this->L_LayerValue);
  this->RowsOfLayerLabels.insert(
        "B", RowOfLayerLabelsType() << this->B_LayerName << this->B_LayerIJK << this->B_LayerValue);
  this->RowsOfLayerLabels.insert(
        "F", RowOfLayerLabelsType() << this->F_LayerName << this->F_LayerIJK << this->F_LayerValue);
  this->resetLabels();
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeInfoWidgetPrivate::resetLabels()
{
  this->ViewerColor->clear();
  this->ViewerColor->setStyleSheet(QLatin1String(""));
  this->ViewerName->clear();
  this->ViewerRAS->clear();
  this->ViewerOrient->clear();
  this->ViewerSpacing->clear();
  foreach(RowOfLayerLabelsType row, this->RowsOfLayerLabels)
    {
    foreach(QLabel* label, row)
      {
      label->clear();
      }
    }
}

//-----------------------------------------------------------------------------
qMRMLSliceWidget *
qSlicerDataProbeInfoWidgetPrivate::slicerWidget(vtkInteractorObserver * interactorStyle) const
{
  if (!this->LayoutManager)
    {
    return 0;
    }
  foreach(const QString& sliceViewName, this->LayoutManager->sliceViewNames())
    {
    qMRMLSliceWidget * sliceWidget = this->LayoutManager->sliceWidget(sliceViewName);
    Q_ASSERT(sliceWidget);
    if (sliceWidget->sliceView()->interactorStyle() == interactorStyle)
      {
      return sliceWidget;
      }
    }
  return 0;
}

//-----------------------------------------------------------------------------
QList<vtkInteractorObserver*>
qSlicerDataProbeInfoWidgetPrivate::currentLayoutSliceViewInteractorStyles() const
{
  QList<vtkInteractorObserver*> interactorStyles;
  if (!this->LayoutManager)
    {
    return interactorStyles;
    }
  foreach(const QString& sliceViewName, this->LayoutManager->sliceViewNames())
    {
    qMRMLSliceWidget * sliceWidget = this->LayoutManager->sliceWidget(sliceViewName);
    Q_ASSERT(sliceWidget);
    interactorStyles << sliceWidget->sliceView()->interactorStyle();
    }
  return interactorStyles;
}

//-----------------------------------------------------------------------------
QList<double>
qSlicerDataProbeInfoWidgetPrivate::convertXYZToIJK(vtkMRMLSliceLayerLogic* slicerLayerLogic,
                                                   const QList<double>& xyz) const
{
  vtkMRMLVolumeNode * volumeNode = slicerLayerLogic->GetVolumeNode();
  if (!volumeNode)
    {
    return QList<double>() << 0.0 << 0.0 << 0.0;
    }
  vtkMatrix4x4 * xyToIJK = slicerLayerLogic->GetXYToIJKTransform()->GetMatrix();
  double xyzw[4] = {xyz[0], xyz[1], xyz[2], 1.0};
  double ijkw[4] = {0.0, 0.0, 0.0, 0.0};
  xyToIJK->MultiplyPoint(xyzw, ijkw);
  Q_ASSERT(ijkw[3] == 1.0);
  return QList<double>() << ijkw[0] <<  ijkw[1] <<  ijkw[2];
}

//-----------------------------------------------------------------------------
// qSlicerDataProbeInfoWidget methods

//-----------------------------------------------------------------------------
qSlicerDataProbeInfoWidget::qSlicerDataProbeInfoWidget(QWidget *_parent):
  Superclass(_parent), d_ptr(new qSlicerDataProbeInfoWidgetPrivate(*this))
{
  Q_D(qSlicerDataProbeInfoWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerDataProbeInfoWidget::~qSlicerDataProbeInfoWidget()
{
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerDataProbeInfoWidget, qSlicerLayoutManager*, layoutManager, LayoutManager)

//-----------------------------------------------------------------------------
void qSlicerDataProbeInfoWidget::setLayoutManager(qSlicerLayoutManager* layoutManager)
{
  Q_D(qSlicerDataProbeInfoWidget);

  if (layoutManager == d->LayoutManager)
    {
    return;
    }
  if (d->LayoutManager)
    {
    disconnect(d->LayoutManager, SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
    }
  if (layoutManager)
    {
    connect(layoutManager, SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
    }
  d->LayoutManager = layoutManager;

  this->onLayoutChanged();
}

//-----------------------------------------------------------------------------
CTK_GET_CPP(qSlicerDataProbeInfoWidget, vtkSlicerDataProbeLogic*, dataProbeLogic, DataProbeLogic)
CTK_SET_CPP(qSlicerDataProbeInfoWidget, vtkSlicerDataProbeLogic*, setDataProbeLogic, DataProbeLogic)

//-----------------------------------------------------------------------------
void qSlicerDataProbeInfoWidget::onLayoutChanged()
{
  Q_D(qSlicerDataProbeInfoWidget);

  if (!this->mrmlScene())
    {
    return;
    }

  // Remove observers
  foreach(vtkInteractorObserver * observedInteractorStyle, d->ObservedInteractorStyles)
    {
    foreach(int event, QList<int>()
            << vtkCommand::MouseMoveEvent << vtkCommand::EnterEvent << vtkCommand::LeaveEvent)
      {
      qvtkDisconnect(observedInteractorStyle, event,
                     this, SLOT(processEvent(vtkObject*,void*,ulong,void*)));
      }
    }
  d->ObservedInteractorStyles.clear();

  // Add observers
  foreach(vtkInteractorObserver * interactorStyle, d->currentLayoutSliceViewInteractorStyles())
    {
    foreach(int event, QList<int>()
            << vtkCommand::MouseMoveEvent << vtkCommand::EnterEvent << vtkCommand::LeaveEvent)
      {
      qvtkConnect(interactorStyle, event,
                  this, SLOT(processEvent(vtkObject*,void*,ulong,void*)));
      }
    d->ObservedInteractorStyles << interactorStyle;
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeInfoWidget::
processEvent(vtkObject* sender, void* callData, unsigned long eventId, void* clientData)
{
  Q_D(qSlicerDataProbeInfoWidget);
  Q_UNUSED(callData);
  Q_UNUSED(clientData);
  if (eventId == vtkCommand::LeaveEvent)
    {
    d->resetLabels();
    }
  else if(eventId == vtkCommand::EnterEvent || eventId == vtkCommand::MouseMoveEvent)
    {
    // Compute RAS
    vtkInteractorObserver * interactorStyle = vtkInteractorObserver::SafeDownCast(sender);
    Q_ASSERT(d->ObservedInteractorStyles.indexOf(interactorStyle) != -1);
    vtkRenderWindowInteractor * interactor = interactorStyle->GetInteractor();
    int xy[2] = {-1, -1};
    interactor->GetEventPosition(xy);
    qMRMLSliceWidget * sliceWidget = d->slicerWidget(interactorStyle);
    Q_ASSERT(sliceWidget);
    QList<double> xyz = sliceWidget->convertDeviceToXYZ(QList<int>() << xy[0] << xy[1]);
    QList<double> ras = sliceWidget->convertXYZToRAS(xyz);

    vtkMRMLSliceLogic * sliceLogic = sliceWidget->sliceLogic();
    vtkMRMLSliceNode * sliceNode = sliceWidget->mrmlSliceNode();

    // RAS
    d->ViewerRAS->setText(QString("RAS: (%1, %2, %3)").
                          arg(ras[0], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
                          arg(ras[1], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1).
                          arg(ras[2], /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1));

    // Orientation
    d->ViewerOrient->setText(QString("  %1").arg(sliceWidget->sliceOrientation()));

    // Spacing
    QString spacing = QString("%1").arg(
          sliceLogic->GetLowestVolumeSliceSpacing()[2],
          /* fieldWidth= */ 0, /* format = */ 'f', /* precision= */ 1);
    if (sliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::PrescribedSliceSpacingMode)
      {
      spacing = "(" + spacing + ")";
      }
    d->ViewerSpacing->setText(QString("Sp: %1").arg(spacing));

    // Color
    double layoutColor[3] = {0.0, 0.0, 0.0};
    sliceNode->GetLayoutColor(layoutColor);
    d->ViewerColor->setStyleSheet(
          QString("QLabel {background-color : %1}").arg(
            QColor::fromRgbF(layoutColor[0], layoutColor[1], layoutColor[2]).name()));

    // Name
    d->ViewerName->setText(QString("  %1  ").arg(sliceNode->GetLayoutName()));

    // Layer name, ijk and value
    typedef QPair<QString, vtkMRMLSliceLayerLogic*> LayerIdAndLogicType;
    foreach(LayerIdAndLogicType layerIdAndLogic,
            (QList<LayerIdAndLogicType>()
            << LayerIdAndLogicType("L", sliceLogic->GetLabelLayer())
            << LayerIdAndLogicType("B", sliceLogic->GetBackgroundLayer())
            << LayerIdAndLogicType("F", sliceLogic->GetForegroundLayer())))
      {
      QString sliceLayerId = layerIdAndLogic.first;
      vtkMRMLSliceLayerLogic * sliceLayerLogic = layerIdAndLogic.second;

      vtkMRMLVolumeNode * volumeNode = sliceLayerLogic->GetVolumeNode();
      QString layerName = "None";
      QString ijkAsString;
      QString valueAsString;
      if (volumeNode)
        {
        layerName = volumeNode->GetName();
        QList<double> ijk = d->convertXYZToIJK(sliceLayerLogic, xyz);
        ijkAsString = QString("(%1, %2, %3)").arg(qRound(ijk[0])).arg(qRound(ijk[1])).arg(qRound(ijk[2]));
        if(d->DataProbeLogic)
          {
          int probeStatus = d->DataProbeLogic->ProbePixel(volumeNode, ijk[0], ijk[1], ijk[2]);
          if (probeStatus & vtkSlicerDataProbeLogic::SUCCESS)
            {
            if (d->DataProbeLogic->GetPixelNumberOfComponents() > 3)
              {
              valueAsString = QString("%1 components").arg(d->DataProbeLogic->GetPixelNumberOfComponents());
              }
            else
              {
              QStringList valueAsStrings;
              for(int pixelValueIdx = 0; pixelValueIdx < d->DataProbeLogic->GetNumberOfPixelValues(); ++pixelValueIdx)
                {
                valueAsStrings << QString("%1").
                                  arg(d->DataProbeLogic->GetPixelValue(pixelValueIdx), /* fieldWidth= */ 0, /* format = */ 'g', /* precision= */ 4);
                }
              valueAsString = valueAsStrings.join(", ");
              }
            QString pixelDescription = QString::fromStdString(d->DataProbeLogic->GetPixelDescription());
            if (!pixelDescription.isEmpty())
              {
              pixelDescription.append(" ");
              if (probeStatus == vtkSlicerDataProbeLogic::SUCCESS_LABEL_VOLUME ||
                  probeStatus == vtkSlicerDataProbeLogic::SUCCESS_LABEL_VOLUME_UNKNOWN_LABELNAME)
                {
                valueAsString = QString("(%1)").arg(valueAsString);
                }
              valueAsString.prepend(pixelDescription);
              }
            }
          else
            {
            valueAsString = QString::fromStdString(d->DataProbeLogic->GetPixelProbeStatusAsString());
            }
          }
        }
      d->RowsOfLayerLabels[sliceLayerId].at(0)->setText(QString("<b>%1</b>").arg(layerName));
      d->RowsOfLayerLabels[sliceLayerId].at(1)->setText(ijkAsString);
      d->RowsOfLayerLabels[sliceLayerId].at(2)->setText(QString("<b>%1</b>").arg(valueAsString));
      }

    }
}
