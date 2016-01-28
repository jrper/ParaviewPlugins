#include "vtkAustraliaFilter.h"

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

vtkCxxRevisionMacro(vtkAustraliaFilter, "$Revision: 0.0$");
vtkStandardNewMacro(vtkAustraliaFilter);

vtkAustraliaFilter::vtkAustraliaFilter(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
};
vtkAustraliaFilter::~vtkAustraliaFilter(){};

int vtkAustraliaFilter::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{

  vtkUnstructuredGrid* input=this->GetUnstructuredGridInput(0);
  vtkUnstructuredGrid* output=this->GetOutput();
  vtkSmartPointer<vtkPoints> points= vtkSmartPointer<vtkPoints>::New();  

#ifndef NDEBUG
  this->DebugOn();
#endif

  points->DeepCopy(input->GetPoints());
  double x[3];
  for (vtkIdType j=0; j<points->GetNumberOfPoints(); j++) {
    points->GetPoint(j,x);
    x[1]=-x[1];
    points->SetPoint(j,x);
  }
  output->SetCells(input->GetCellTypesArray(),
		   input->GetCellLocationsArray(),
		   input->GetCells());
  output->SetPoints(points);
  output->GetCellData()->DeepCopy(input->GetCellData());
  output->GetPointData()->DeepCopy(input->GetPointData());

  return 1;
}

int vtkAustraliaFilter::RequestUpdateExtent(
			  vtkInformation* request,
			  vtkInformationVector** inputVector,
			  vtkInformationVector* outputVector)
 {

  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);


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


  return 1;

};


int vtkAustraliaFilter::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
