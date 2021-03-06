//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <vtkActor.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCompositeDataDisplayAttributes.h>
#include <vtkCompositePolyDataMapper2.h>
#include <vtkGlyph3DMapper.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include <vtkPVCacheKeeper.h>
#include <vtkPVRenderView.h>
#include <vtkPVTrivialProducer.h>

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "vtkSMTKModelRepresentation.h"

vtkStandardNewMacro(vtkSMTKModelRepresentation);

vtkSMTKModelRepresentation::vtkSMTKModelRepresentation()
  : EntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , SelectedEntityMapper(vtkSmartPointer<vtkCompositePolyDataMapper2>::New())
  , GlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , SelectedGlyphMapper(vtkSmartPointer<vtkGlyph3DMapper>::New())
  , Entities(vtkSmartPointer<vtkActor>::New())
  , SelectedEntities(vtkSmartPointer<vtkActor>::New())
  , GlyphEntities(vtkSmartPointer<vtkActor>::New())
  , SelectedGlyphEntities(vtkSmartPointer<vtkActor>::New())
{
  this->SetupDefaults();
  this->SetNumberOfInputPorts(3);
}

vtkSMTKModelRepresentation::~vtkSMTKModelRepresentation() = default;

void vtkSMTKModelRepresentation::SetupDefaults()
{
  auto compAtt = vtkCompositeDataDisplayAttributes::New();
  this->EntityMapper->SetCompositeDataDisplayAttributes(compAtt);

  this->Entities->SetMapper(this->EntityMapper);
  this->SelectedEntities->SetMapper(this->SelectedEntityMapper);
  this->GlyphEntities->SetMapper(this->GlyphMapper);
  this->SelectedGlyphEntities->SetMapper(this->SelectedGlyphMapper);
}

int vtkSMTKModelRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inVec, vtkInformationVector* outVec)
{
  vtkMath::UninitializeBounds(this->DataBounds);

  if (inVec[0]->GetNumberOfInformationObjects() == 1)
  {
    vtkInformation* inInfo = inVec[0]->GetInformationObject(0);
    this->SetOutputExtent(this->GetInternalOutputPort(0), inInfo);

    // Model entities
    this->CacheKeeper->SetInputConnection(this->GetInternalOutputPort(0));

    // Glyph points (2) and prototypes (1)
    this->GlyphMapper->SetInputConnection(this->GetInternalOutputPort(2));
    this->GlyphMapper->SetInputConnection(1, this->GetInternalOutputPort(1));
    this->ConfigureGlyphMapper(this->GlyphMapper.GetPointer());
  }
  this->CacheKeeper->Update();

  this->EntityMapper->Modified();
  this->GlyphMapper->Modified();

  //  // Determine data bounds
  //  this->GetBounds(this->GetOutputDataObject(0), this->DataBounds,
  //    this->EntityMapper->GetCompositeDataDisplayAttributes());

  return vtkPVDataRepresentation::RequestData(request, inVec, outVec);
}

int vtkSMTKModelRepresentation::ProcessViewRequest(
  vtkInformationRequestKey* request_type, vtkInformation* inInfo, vtkInformation* outInfo)
{
  if (!vtkPVDataRepresentation::ProcessViewRequest(request_type, inInfo, outInfo))
  {
    // i.e. this->GetVisibility() == false, hence nothing to do.
    return 0;
  }

  if (request_type == vtkPVView::REQUEST_UPDATE())
  {
    // provide the "geometry" to the view so the view can delivery it to the
    // rendering nodes as and when needed.
    // When this process doesn't have any valid input, the cache-keeper is setup
    // to provide a place-holder dataset of the right type. This is essential
    // since the vtkPVRenderView uses the type specified to decide on the
    // delivery mechanism, among other things.
    vtkPVRenderView::SetPiece(inInfo, this, this->CacheKeeper->GetOutputDataObject(0), 0, 0);

    // Since we are rendering polydata, it can be redistributed when ordered
    // compositing is needed. So let the view know that it can feel free to
    // redistribute data as and when needed.
    vtkPVRenderView::MarkAsRedistributable(inInfo, this);

    // Tell the view if this representation needs ordered compositing. We need
    // ordered compositing when rendering translucent geometry. We need to extend
    // this condition to consider translucent LUTs once we start supporting them.
    if (this->Entities->HasTranslucentPolygonalGeometry())
    {
      outInfo->Set(vtkPVRenderView::NEED_ORDERED_COMPOSITING(), 1);
      // Pass partitioning information to the render view.
      if (this->UseDataPartitions == true)
      {
        vtkPVRenderView::SetOrderedCompositingInformation(inInfo, this->DataBounds);
      }
    }

    // Finally, let the view know about the geometry bounds. The view uses this
    // information for resetting camera and clip planes. Since this
    // representation allows users to transform the geometry, we need to ensure
    // that the bounds we report include the transformation as well.
    vtkNew<vtkMatrix4x4> matrix;
    this->Entities->GetMatrix(matrix.GetPointer());
    vtkPVRenderView::SetGeometryBounds(inInfo, this->DataBounds, matrix.GetPointer());
  }
  else if (request_type == vtkPVView::REQUEST_UPDATE_LOD())
  {
    /// TODO Add LOD Mappers
  }
  else if (request_type == vtkPVView::REQUEST_RENDER())
  {
    auto producerPort = vtkPVRenderView::GetPieceProducer(inInfo, this, 0);
    this->EntityMapper->SetInputConnection(0, producerPort);

    auto data = producerPort->GetProducer()->GetOutputDataObject(0);
    if (this->BlockAttributeTime < data->GetMTime() || this->BlockAttrChanged)
    {
      this->UpdateBlockAttributes(this->EntityMapper.GetPointer());
      this->BlockAttributeTime.Modified();
      this->BlockAttrChanged = false;
    }
  }

  return 1;
}

void vtkSMTKModelRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkPVDataRepresentation::PrintSelf(os, indent);
}

void vtkSMTKModelRepresentation::SetOutputExtent(vtkAlgorithmOutput* output, vtkInformation* inInfo)
{
  if (inInfo->Has(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()))
  {
    vtkPVTrivialProducer* prod = vtkPVTrivialProducer::SafeDownCast(output->GetProducer());
    if (prod)
    {
      prod->SetWholeExtent(inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()));
    }
  }
}

bool vtkSMTKModelRepresentation::AddToView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->AddActor(this->Entities);
    rview->GetRenderer()->AddActor(this->GlyphEntities);

    // Indicate that this is prop that we are rendering when hardware selection
    // is enabled.
    rview->RegisterPropForHardwareSelection(this, this->Entities);
    rview->RegisterPropForHardwareSelection(this, this->GlyphEntities);
    return this->vtkPVDataRepresentation::AddToView(view);
  }
  return false;
}

bool vtkSMTKModelRepresentation::RemoveFromView(vtkView* view)
{
  vtkPVRenderView* rview = vtkPVRenderView::SafeDownCast(view);
  if (rview)
  {
    rview->GetRenderer()->RemoveActor(this->Entities);
    rview->GetRenderer()->RemoveActor(this->GlyphEntities);
    rview->UnRegisterPropForHardwareSelection(this, this->Entities);
    rview->UnRegisterPropForHardwareSelection(this, this->GlyphEntities);
    return this->vtkPVDataRepresentation::RemoveFromView(view);
  }
  return false;
}

void vtkSMTKModelRepresentation::SetVisibility(bool val)
{
  this->Entities->SetVisibility(val);
  this->GlyphEntities->SetVisibility(val);
  this->vtkPVDataRepresentation::SetVisibility(val);
}

int vtkSMTKModelRepresentation::FillInputPortInformation(int port, vtkInformation* info)
{
  // Saying INPUT_IS_OPTIONAL() is essential, since representations don't have
  // any inputs on client-side (in client-server, client-render-server mode) and
  // render-server-side (in client-render-server mode).
  if (port == 0)
  {
    // Entity tessellations
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCompositeDataSet");
    return 1;
  }
  if (port == 1)
  {
    // Glyph vertices
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkCompositeDataSet");
    return 1;
  }
  else if (port == 2)
  {
    // Glyph sources
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObjectTree");
    info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }

  return 0;
}

void vtkSMTKModelRepresentation::ConfigureGlyphMapper(vtkGlyph3DMapper* mapper)
{
  mapper->SetUseSourceTableTree(true);

  mapper->SetSourceIndexArray(VTK_INSTANCE_SOURCE);
  mapper->SetSourceIndexing(true);

  mapper->SetScaleArray(VTK_INSTANCE_SCALE);
  mapper->SetScaling(true);

  mapper->SetOrientationArray(VTK_INSTANCE_ORIENTATION);
  mapper->SetOrientationMode(vtkGlyph3DMapper::ROTATION);

  mapper->SetMaskArray(VTK_INSTANCE_VISIBILITY);
  mapper->SetMasking(true);
}

void vtkSMTKModelRepresentation::SetMapScalars(int val)
{
  if (val < 0 || val > 1)
  {
    vtkWarningMacro(<< "Invalid parameter for vtkSMTKModelRepresentation::SetMapScalars: " << val);
    val = 0;
  }

  int mapToColorMode[] = { VTK_COLOR_MODE_DIRECT_SCALARS, VTK_COLOR_MODE_MAP_SCALARS };
  this->EntityMapper->SetColorMode(mapToColorMode[val]);
  this->GlyphMapper->SetColorMode(mapToColorMode[val]);
}
