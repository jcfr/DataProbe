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

// Qt includes
#include <QDebug>
#include <QLayout>
#include <QMainWindow>
#include <qSlicerApplication.h>
#include <QtPlugin>

// DataProbe includes
#include "qSlicerDataProbeInfoWidget.h"
#include "qSlicerDataProbeModule.h"
#include "qSlicerDataProbeModuleWidget.h"
#include "vtkSlicerDataProbeLogic.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerDataProbeModule, qSlicerDataProbeModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_DataProbe
class qSlicerDataProbeModulePrivate
{
  Q_DECLARE_PUBLIC(qSlicerDataProbeModule);
protected:
  qSlicerDataProbeModule* const q_ptr;
public:
  qSlicerDataProbeModulePrivate(qSlicerDataProbeModule& object);

  void setupDataProbeInfoWidget();

  qSlicerDataProbeInfoWidget * DataProbeInfoWidget;
};

//-----------------------------------------------------------------------------
// qSlicerDataProbeModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerDataProbeModulePrivate::qSlicerDataProbeModulePrivate(qSlicerDataProbeModule &object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeModulePrivate::setupDataProbeInfoWidget()
{
  foreach(QWidget* widget, qApp->topLevelWidgets())
    {
    if (QMainWindow* mainWindow = qobject_cast<QMainWindow*>(widget))
      {
      if (QWidget * parent = mainWindow->findChild<QWidget*>("DataProbeCollapsibleWidget"))
        {
        this->DataProbeInfoWidget = new qSlicerDataProbeInfoWidget;
        parent->layout()->addWidget(this->DataProbeInfoWidget);
        }
      }
    }
}

//-----------------------------------------------------------------------------
// qSlicerDataProbeModule methods

//-----------------------------------------------------------------------------
qSlicerDataProbeModule::qSlicerDataProbeModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDataProbeModulePrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerDataProbeModule::~qSlicerDataProbeModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerDataProbeModule::helpText()const
{
  QString help =
      "The DataProbe module is used to get information about the "
      "current RAS position being indicated by the mouse position.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerDataProbeModule::acknowledgementText()const
{
  return "This work is supported by NA-MIC, NAC, NCIGT, and the Slicer Community. "
      "See <a>http://www.slicer.org</a> for details. Module implemented by Steve Pieper";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDataProbeModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Steve Pieper (Isomics)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerDataProbeModule::icon()const
{
  return QIcon(":/Icons/DataProbe.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerDataProbeModule::categories()const
{
  return QStringList() << "Quantification";
}

//-----------------------------------------------------------------------------
bool qSlicerDataProbeModule::isHidden()const
{
  return false;
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeModule::setup()
{
  Q_D(qSlicerDataProbeModule);
  this->Superclass::setup();
  d->setupDataProbeInfoWidget();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerDataProbeModule::createWidgetRepresentation()
{
  return new qSlicerDataProbeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDataProbeModule::createLogic()
{
  return vtkSlicerDataProbeLogic::New();
}

//-----------------------------------------------------------------------------
void qSlicerDataProbeModule::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  Q_D(qSlicerDataProbeModule);
  this->Superclass::setMRMLScene(mrmlScene);
  if(d->DataProbeInfoWidget)
    {
    d->DataProbeInfoWidget->setMRMLScene(mrmlScene);
    d->DataProbeInfoWidget->setLayoutManager(qSlicerApplication::application()->layoutManager());
    d->DataProbeInfoWidget->setDataProbeLogic(vtkSlicerDataProbeLogic::SafeDownCast(this->logic()));
    }
}
