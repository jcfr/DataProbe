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

// DataProbe includes
#include "vtkSlicerDataProbeLogic.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>

// vtkTeem includes
#include "vtkDiffusionTensorMathematics.h"

// VTK includes
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPointData.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
class vtkSlicerDataProbeLogic::vtkInternal
{
public:
  vtkInternal(vtkSlicerDataProbeLogic* external);
  ~vtkInternal();

  void ResetProbe();

  vtkSmartPointer<vtkDiffusionTensorMathematics> DTIMath;
  vtkSmartPointer<vtkImageData> SinglePixelImage;
  vtkSmartPointer<vtkFloatArray> TensorData;

  int PixelProbeStatus;
  std::string PixelDescription;

  int PixelNumberOfComponents;

  int NumberOfPixelValues;
  static const int MAX_NUMBER_OF_PIXEL_VALUES = 3;
  double PixelValues[MAX_NUMBER_OF_PIXEL_VALUES];

  vtkSlicerDataProbeLogic*      External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerDataProbeLogic::vtkInternal::vtkInternal(
    vtkSlicerDataProbeLogic* _external)
{
  this->External = _external;

  this->DTIMath = vtkSmartPointer<vtkDiffusionTensorMathematics>::New();

  this->SinglePixelImage = vtkSmartPointer<vtkImageData>::New();
  this->SinglePixelImage->SetExtent(0, 0, 0, 0, 0, 0);
  this->SinglePixelImage->AllocateScalars();

  this->TensorData = vtkSmartPointer<vtkFloatArray>::New();
  this->TensorData->SetNumberOfComponents(9);
  this->TensorData->SetNumberOfTuples(this->SinglePixelImage->GetNumberOfPoints());
  this->SinglePixelImage->GetPointData()->SetTensors(this->TensorData);

  this->DTIMath->SetInput(this->SinglePixelImage);

  this->ResetProbe();
}

//---------------------------------------------------------------------------
vtkSlicerDataProbeLogic::vtkInternal::~vtkInternal()
{
}

//----------------------------------------------------------------------------
void vtkSlicerDataProbeLogic::vtkInternal::ResetProbe()
{
  this->NumberOfPixelValues = 0;
  this->PixelNumberOfComponents = 0;
  for (int pixelValueIdx = 0; pixelValueIdx < MAX_NUMBER_OF_PIXEL_VALUES; ++pixelValueIdx)
    {
    this->PixelValues[pixelValueIdx] = vtkMath::Nan();
    }
  this->PixelDescription.clear();
  this->PixelProbeStatus = vtkSlicerDataProbeLogic::UNKNOWN;
}

//----------------------------------------------------------------------------
// vtkSlicerDataProbeLogic methods

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDataProbeLogic);

//----------------------------------------------------------------------------
vtkSlicerDataProbeLogic::vtkSlicerDataProbeLogic()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkSlicerDataProbeLogic::~vtkSlicerDataProbeLogic()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkSlicerDataProbeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::ProbePixel(vtkMRMLVolumeNode* volumeNode, double ijk[3])
{
  return this->ProbePixel(volumeNode, ijk[0], ijk[1], ijk[2]);
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::ProbePixel(vtkMRMLVolumeNode* volumeNode, double i, double j, double k)
{
  this->Internal->ResetProbe();

  vtkMRMLScalarVolumeNode * scalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
  if (!scalarVolumeNode)
    {
    this->Internal->PixelProbeStatus = ERROR_NO_SCALAR_VOLUME;
    return this->Internal->PixelProbeStatus;
    }

  vtkImageData * imageData = scalarVolumeNode->GetImageData();
  if(!imageData)
    {
    this->Internal->PixelProbeStatus = ERROR_NO_IMAGE_DATA;
    return this->Internal->PixelProbeStatus;
    }

  int dims[3] = {-1, -1, -1};
  double ijk[3] = {i, j, k};
  imageData->GetDimensions(dims);
  for (int dimIdx = 0; dimIdx < 3; ++dimIdx)
    {
    if(ijk[dimIdx] < 0 || ijk[dimIdx] >= dims[dimIdx])
      {
      this->Internal->PixelProbeStatus = ERROR_OUT_OF_FRAME;
      return this->Internal->PixelProbeStatus;
      }
    }

  if (scalarVolumeNode->GetLabelMap())
    {
    std::string labelName;
    int labelProbeStatus = LABEL_VOLUME;
    double labelIndex = imageData->GetScalarComponentAsDouble(i, j, k, 0);
    vtkMRMLColorNode * colorNode = scalarVolumeNode->GetDisplayNode()->GetColorNode();
    if (colorNode)
      {
      labelName = colorNode->GetColorName(static_cast<int>(labelIndex));
      }
    else
      {
      labelProbeStatus = WARNING_LABEL_VOLUME_UNKNOWN_LABELNAME;
      }
    this->Internal->PixelNumberOfComponents = 1;
    this->Internal->PixelValues[0] = labelIndex;
    this->Internal->PixelDescription = labelName;
    this->Internal->PixelProbeStatus = labelProbeStatus | SUCCESS;
    return this->Internal->PixelProbeStatus;
    }
  else if(vtkMRMLDiffusionTensorVolumeNode * dtiVolumeNode =
     vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(scalarVolumeNode))
    {
    vtkIdType pointIdx = imageData->FindPoint(i, j, k);
    if (pointIdx == -1)
      {
      this->Internal->PixelProbeStatus = ERROR_OUT_OF_FRAME;
      return this->Internal->PixelProbeStatus;
      }
    vtkPointData* pointData = imageData->GetPointData();
    if (!pointData)
      {
      this->Internal->PixelProbeStatus = ERROR_DTI_NO_POINT_DATA;
      return this->Internal->PixelProbeStatus;
      }
    vtkDataArray * tensors = pointData->GetTensors();
    if (!tensors)
      {
      this->Internal->PixelProbeStatus = ERROR_DTI_NO_TENSOR_DATA;
      return this->Internal->PixelProbeStatus;
      }
    double tensor[9] = {0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0,
                        0.0, 0.0, 0.0};
    tensors->GetTuple(pointIdx, tensor);

    float tensorAsFloat[9];
    for(int idx = 0; idx < 9; ++idx)
      {
      tensorAsFloat[idx] = static_cast<float>(tensor[idx]);
      }

    vtkMRMLDiffusionTensorVolumeDisplayNode * dtiVolumeDisplayNode =
        vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(dtiVolumeNode->GetScalarVolumeDisplayNode());

    std::string scalarInvariant = "ColorOrientation";
    int operation = vtkDiffusionTensorMathematics::VTK_TENS_COLOR_ORIENTATION;
    if (dtiVolumeDisplayNode)
      {
      operation = dtiVolumeDisplayNode->GetScalarInvariant();
      scalarInvariant = dtiVolumeDisplayNode->GetScalarInvariantAsString();
      }
    this->Internal->PixelNumberOfComponents = 1;
    this->Internal->PixelValues[0] = this->CalculateTensorScalars(tensorAsFloat, operation);
    this->Internal->PixelDescription = scalarInvariant;
    this->Internal->PixelProbeStatus = SUCCESS_DTI_VOLUME;
    return this->Internal->PixelProbeStatus;
    }
  else
    {
    int numberOfComponents = imageData->GetNumberOfScalarComponents();
    int numberOfPixelValues = vtkInternal::MAX_NUMBER_OF_PIXEL_VALUES;
    if (numberOfComponents < vtkInternal::MAX_NUMBER_OF_PIXEL_VALUES)
      {
      numberOfPixelValues = numberOfComponents;
      }
    for (int componentIdx = 0; componentIdx < numberOfPixelValues; ++componentIdx)
      {
      this->Internal->PixelValues[componentIdx] = imageData->GetScalarComponentAsDouble(
            vtkMath::Round(i), vtkMath::Round(j), vtkMath::Round(k), componentIdx);
      }
    this->Internal->PixelNumberOfComponents = numberOfComponents;
    this->Internal->PixelProbeStatus = SUCCESS_SCALAR_VOLUME;
    return this->Internal->PixelProbeStatus;
    }
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::GetPixelNumberOfComponents() const
{
  return this->Internal->PixelNumberOfComponents;
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::GetMaxGetNumberOfPixelValues()const
{
  return this->Internal->MAX_NUMBER_OF_PIXEL_VALUES;
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::GetNumberOfPixelValues() const
{
  return this->Internal->PixelNumberOfComponents;
}

//---------------------------------------------------------------------------
double vtkSlicerDataProbeLogic::GetPixelValue(int nth)const
{
  if (nth >= 0 || nth < vtkInternal::MAX_NUMBER_OF_PIXEL_VALUES)
    {
    return this->Internal->PixelValues[nth];
    }
  else
    {
    return vtkMath::Nan();
    }
}

//---------------------------------------------------------------------------
int vtkSlicerDataProbeLogic::GetPixelProbeStatus()const
{
  return this->Internal->PixelProbeStatus;
}

//---------------------------------------------------------------------------
std::string vtkSlicerDataProbeLogic::GetPixelProbeStatusAsString()const
{
  return Self::GetDataProbeStatusEnumAsString(this->Internal->PixelProbeStatus);
}

//---------------------------------------------------------------------------
std::string vtkSlicerDataProbeLogic::GetPixelDescription()const
{
  return this->Internal->PixelDescription;
}

//---------------------------------------------------------------------------
double vtkSlicerDataProbeLogic::CalculateTensorScalars(float tensor[9], int operation)
{
  this->Internal->TensorData->SetTupleValue(0, tensor);
  this->Internal->TensorData->Modified();
  this->Internal->SinglePixelImage->Modified();

  this->Internal->DTIMath->SetOperation(operation);
  this->Internal->DTIMath->Update();

  vtkImageData * output = this->Internal->DTIMath->GetOutput();

  double value = vtkMath::Nan();
  if (output && output->GetNumberOfScalarComponents() > 0)
    {
    value = output->GetScalarComponentAsDouble(0, 0, 0, 0);
    }
  return value;
}

//----------------------------------------------------------------------------
const char* vtkSlicerDataProbeLogic::GetDataProbeStatusEnumAsString(int probeStatus)
{
  if (probeStatus ==  SUCCESS_DTI_VOLUME)
    {
    return "Successfully probed DTI volume";
    }
  else if (probeStatus ==  SUCCESS_SCALAR_VOLUME)
    {
    return "Successfully probed  Scalar volume";
    }
  else if (probeStatus ==  SUCCESS_LABEL_VOLUME)
    {
    return "Successfully probed  Label volume";
    }
  else if (probeStatus == SUCCESS_LABEL_VOLUME_UNKNOWN_LABELNAME)
    {
    return "Unknown label name";
    }
  else if (probeStatus ==  ERROR_NO_SCALAR_VOLUME)
    {
    return "No scalar volume";
    }
  else if (probeStatus ==  ERROR_NO_IMAGE_DATA)
    {
    return "No image data";
    }
  else if (probeStatus ==  ERROR_OUT_OF_FRAME)
    {
    return "Out of frame";
    }
  else if (probeStatus ==  ERROR_DTI_NO_POINT_DATA)
    {
    return "No point data";
    }
  else if (probeStatus ==  ERROR_DTI_NO_TENSOR_DATA)
    {
    return "No tensor data";
    }

  return "Unknown";
}
