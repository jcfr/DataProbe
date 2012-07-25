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

  This file was developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1.

  Based on the scripted module "DataProbe.py" developed by Steve Pieper (Isomics).

==============================================================================*/

#ifndef __vtkSlicerDataProbeLogic_h
#define __vtkSlicerDataProbeLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerDataProbeModuleLogicExport.h"

class vtkMRMLVolumeNode;

/// \ingroup Slicer_QtModules_DataProbe
class VTK_SLICER_DATAPROBE_MODULE_LOGIC_EXPORT vtkSlicerDataProbeLogic :
  public vtkSlicerModuleLogic
{
public:

  typedef vtkSlicerDataProbeLogic Self;

  static vtkSlicerDataProbeLogic *New();
  vtkTypeMacro(vtkSlicerDataProbeLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  enum DataProbeStatus
  {
    UNKNOWN = 0x0,

    PROBE_SUCCESS = 0x1,
    PROBE_ERROR   = 0x2,

    SCALAR_VOLUME = 0x4,
    LABEL_VOLUME  = 0x8  | SCALAR_VOLUME,
    DTI_VOLUME    = 0x10 | SCALAR_VOLUME,

    PROBE_WARNING_LABEL_VOLUME_UNKNOWN_LABELNAME = 0x100,

    PROBE_SUCCESS_DTI_VOLUME    = DTI_VOLUME    | PROBE_SUCCESS,
    PROBE_SUCCESS_SCALAR_VOLUME = SCALAR_VOLUME | PROBE_SUCCESS,
    PROBE_SUCCESS_LABEL_VOLUME  = LABEL_VOLUME  | PROBE_SUCCESS,
    PROBE_SUCCESS_LABEL_VOLUME_UNKNOWN_LABELNAME = PROBE_WARNING_LABEL_VOLUME_UNKNOWN_LABELNAME | LABEL_VOLUME | PROBE_SUCCESS,

    PROBE_ERROR_NO_SCALAR_VOLUME   = 0x1  * 10000 | PROBE_ERROR,
    PROBE_ERROR_NO_IMAGE_DATA      = 0x2  * 10000 | SCALAR_VOLUME | PROBE_ERROR,
    PROBE_ERROR_OUT_OF_FRAME       = 0x4  * 10000 | SCALAR_VOLUME | PROBE_ERROR,
    PROBE_ERROR_DTI_NO_POINT_DATA  = 0x8  * 10000 | DTI_VOLUME | PROBE_ERROR,
    PROBE_ERROR_DTI_NO_TENSOR_DATA = 0x10 * 10000 | DTI_VOLUME | PROBE_ERROR
  };

  /// Return a descriptive string associated with given \a probeStatus
  static const char* GetDataProbeStatusEnumAsString(int probeStatus);

  int ProbePixel(vtkMRMLVolumeNode* volumeNode, double ijk[3]);
  int ProbePixel(vtkMRMLVolumeNode* volumeNode, double i, double j, double k);

  /// Return the number of components associated with the probed pixel.
  /// It will return 0 if the probe status is set to DataProbeStatus::FAILURE
  /// \sa ProbePixel, GetProbeStatus, GetPixelProbeStatusAsString
  int GetPixelNumberOfComponents()const;

  /// Return the maximum number of pixel values that will be stored after a successful probing.
  /// \sa ProbePixel, GetPixelValue
  int GetMaxGetNumberOfPixelValues()const;

  /// Return the current number of pixel values that have been probed and stored.
  int GetNumberOfPixelValues() const;

  /// Return the Nth pixel value stored aften a successfull probing.
  /// If probe status is set to DataProbeStatus::SUCCESS, retrieving the first (nth = 0)
  /// is guaranted to return a valid result. It case of error, vtkMath::Nan() will be returned.
  /// \sa ProbePixel, GetMaxGetNumberOfPixelValues, GetNumberOfPixelValues
  double GetPixelValue(int nth)const;

  /// Return the probe status set after function ProbePixel is called.
  /// \sa ProbePixel, \sa DataProbeStatus
  int GetPixelProbeStatus()const;
  std::string GetPixelProbeStatusAsString()const;

  /// Return additional information associated with the probed pixel.
  /// For example, in case the probed pixel is associated with a \a LABEL_VOLUME,
  /// the name of the lable could be retrived using this method.
  std::string GetPixelDescription()const;

  ///
  double CalculateTensorScalars(float tensor[9], int operation);

protected:
  vtkSlicerDataProbeLogic();
  virtual ~vtkSlicerDataProbeLogic();

private:

  vtkSlicerDataProbeLogic(const vtkSlicerDataProbeLogic&); // Not implemented
  void operator=(const vtkSlicerDataProbeLogic&);          // Not implemented

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
