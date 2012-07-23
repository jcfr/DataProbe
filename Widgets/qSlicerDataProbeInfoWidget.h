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

#ifndef __qSlicerDataProbeInfoWidget_h
#define __qSlicerDataProbeInfoWidget_h

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerWidget.h"

#include "qSlicerDataProbeModuleWidgetsExport.h"

class qSlicerDataProbeInfoWidgetPrivate;
class qSlicerLayoutManager;
class vtkSlicerDataProbeLogic;

/// \ingroup Slicer_QtModules_DataProbe
class Q_SLICER_MODULE_DATAPROBE_WIDGETS_EXPORT qSlicerDataProbeInfoWidget : public qSlicerWidget
{ 
  Q_OBJECT
  QVTK_OBJECT
public:
  typedef qSlicerDataProbeInfoWidgetPrivate Pimpl;
  typedef qSlicerWidget Superclass;
  qSlicerDataProbeInfoWidget(QWidget *parent=0);
  virtual ~qSlicerDataProbeInfoWidget();

  qSlicerLayoutManager * layoutManager()const;
  void setLayoutManager(qSlicerLayoutManager* layoutManager);

  vtkSlicerDataProbeLogic * dataProbeLogic()const;
  void setDataProbeLogic(vtkSlicerDataProbeLogic * dataProbeLogic);

protected slots:

  void onLayoutChanged();

  void processEvent(vtkObject* sender, void* callData, unsigned long eventId, void* clientData);

protected:
  QScopedPointer<qSlicerDataProbeInfoWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDataProbeInfoWidget);
  Q_DISABLE_COPY(qSlicerDataProbeInfoWidget);
};

#endif

