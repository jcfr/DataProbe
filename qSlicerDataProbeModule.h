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

==============================================================================*/

#ifndef __qSlicerDataProbeModule_h
#define __qSlicerDataProbeModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "qSlicerDataProbeModuleExport.h"

class qSlicerDataProbeModulePrivate;

/// \ingroup Slicer_QtModules_DataProbe
class Q_SLICER_QTMODULES_DATAPROBE_EXPORT qSlicerDataProbeModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerDataProbeModule(QObject *parent=0);
  virtual ~qSlicerDataProbeModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  virtual QString helpText()const;

  /// Return acknowledgements
  virtual QString acknowledgementText()const;

  /// Return the authors of the module
  virtual QStringList  contributors()const;

  /// Return a custom icon for the module
  virtual QIcon icon()const;

  /// Return the categories for the module
  virtual QStringList categories()const;

  virtual bool isHidden()const;

public slots:
  virtual void setMRMLScene(vtkMRMLScene* mrmlScene);

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerDataProbeModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDataProbeModule);
  Q_DISABLE_COPY(qSlicerDataProbeModule);

};

#endif
