#include "vtkConsistentInterpolation.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkProbeFilter.h"
#include "vtkCellLocator.h"
#include "vtkPointLocator.h"
#include "vtkGenericCell.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"




#if PARAVIEW_VERSION_MAJOR < 5
vtkCxxRevisionMacro(vtkConsistentInterpolation, "$Revision: 0.0$");
#endif
vtkStandardNewMacro(vtkConsistentInterpolation);

vtkConsistentInterpolation::vtkConsistentInterpolation(){
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
  this->Radius=1.0e-1;
};
vtkConsistentInterpolation::~vtkConsistentInterpolation(){};

int vtkConsistentInterpolation::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{

  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[1]);
  vtkUnstructuredGrid* source= vtkUnstructuredGrid::GetData(inputVector[0]);

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New(); 
  // vtkSmartPointer<vtkUnstructuredGrid> output= vtkSmartPointer<vtkUnstructuredGrid>::New(); 

#ifndef NDEBUG
  this->DebugOn();
#endif

  outpoints->DeepCopy(input->GetPoints());
  output->SetPoints(outpoints);
  output->Allocate(input->GetNumberOfCells());
  output->SetCells(input->GetCellTypesArray(),
		   input->GetCellLocationsArray(),
		   input->GetCells());

  vtkSmartPointer<vtkCellLocator> locator= vtkSmartPointer<vtkCellLocator>::New();
  locator->SetDataSet(source);
  locator->BuildLocator();

  vtkSmartPointer<vtkPointLocator> plocator= vtkSmartPointer<vtkPointLocator>::New();
  plocator->SetDataSet(source);
  plocator->BuildLocator();

  output->GetPointData()->CopyStructure(source->GetPointData());
  for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j){
    output->GetPointData()->GetArray(j)->SetNumberOfTuples(input->GetNumberOfPoints());
  }

  double w[10];
  double p[10];
  double val[50];
  

  vtkSmartPointer<vtkGenericCell> cell= vtkSmartPointer<vtkGenericCell>::New();

  for (vtkIdType i=0;i<input->GetNumberOfPoints();++i){
    
    vtkIdType cell_id = locator->FindCell(input->GetPoint(i), 0.0, cell, p, w);

    if (cell_id<0) {
      double dist2;
      vtkIdType id = plocator->FindClosestPointWithinRadius(this->Radius, input->GetPoint(i), dist2);
      for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j) {
	vtkDataArray* data =source->GetPointData()->GetArray(j);
	int n = output->GetPointData()->GetArray(j)->GetNumberOfComponents();
	switch (data->GetDataType()) 
	case VTK_DOUBLE:
	  {
	    double val[10], val_in[10];
	    output->GetPointData()->GetArray(j)->GetTuple(i,val_in);
            if ( dist2<=this->Radius*this->Radius ) {
	      source->GetPointData()->GetArray(j)->GetTuple(id,val);
	      for (int k=0; k<n; ++k) {
		val_in[k]=val[k];	
	      }
	    } else {
	      for (int k=0; k<n; ++k) {
		val_in[k]=vtkMath::Nan();	
	      }
	    }
	    vtkDoubleArray::SafeDownCast(output->GetPointData()->GetArray(j))->SetTuple(i,val_in);
	    break;
	  }
      }
    }else {
      int N = cell->GetNumberOfPoints();
      for (vtkIdType j=0;j<source->GetPointData()->GetNumberOfArrays();++j) {
	vtkDataArray* data =source->GetPointData()->GetArray(j);
	int n = output->GetPointData()->GetArray(j)->GetNumberOfComponents();
	switch (data->GetDataType()) 
	case VTK_DOUBLE:
	  {
	    double val[10], val_in[10];
	    output->GetPointData()->GetArray(j)->GetTuple(i,val_in);
	    for (int k=0; k<n; ++k) {
              val_in[k]=0;	
	    }
	    for (int a=0; a<N; ++a) {
	      vtkIdType id = cell->GetPointIds()->GetId(a);
	      data->GetTuple(id, val);
	      for (int k=0; k<n; ++k) {
                val_in[k]=val_in[k]+w[a]*val[k];	
	      }
	    }
	    vtkDoubleArray::SafeDownCast(output->GetPointData()->GetArray(j))->SetTuple(i,val_in);
	    break;
	  }
      }
    }	  
  }

  //  vtkSmartPointer<vtkProbeFilter> filter = vtkSmartPointer<vtkProbeFilter>::New();
  //filter->SetInputData(input);
  //filter->SetSourceData(source);
  //filter->SetTolerance(0.0);
  //filter->Update();
  //out->DeepCopy(filter->GetOutput());

  return 1;
}

int vtkConsistentInterpolation::RequestUpdateExtent(
			  vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector)
 {

  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkInformation *sourceInfo=inputVector[1]->GetInformationObject(0);

  //  this->DebugOn();

  int piece, numPieces, ghostLevels;
  
  piece=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
  numPieces=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
  ghostLevels=outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());

  if (numPieces > 1)
  {
    ++ghostLevels;
  }

  vtkDebugMacro(<<"Running Update Extent"<<piece<<numPieces<<ghostLevels);

  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),piece);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),numPieces);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),ghostLevels);
  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(),1);

  sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(),piece);
  sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(),numPieces);
  sourceInfo->Set(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(),ghostLevels);
  sourceInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(),1);


  return 1;

};


int vtkConsistentInterpolation::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
