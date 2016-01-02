#include "vtkTemplate.h"

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





vtkCxxRevisionMacro(vtkTemplate, "$Revision: 0.0$");
vtkStandardNewMacro(vtkTemplate);

vtkTemplate::vtkTemplate(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
};
vtkTemplate::~vtkTemplate(){};

int vtkTemplate::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{

  vtkInformation* outInfo0=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output0= vtkUnstructuredGrid::SafeDownCast(outInfo0->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo0=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input0= vtkUnstructuredGrid::GetData(inputVector[0]);

  vtkSmartPointer<vtkPoints> outpoints0= vtkSmartPointer<vtkPoints>::New();  

#ifndef NDEBUG
  this->DebugOn();
#endif


  outpoints0->DeepCopy(input0->GetPoints());
  output0->SetPoints(outpoints0);
  output0->GetCellData()->DeepCopy(input0->GetCellData());
  output0->GetPointData()->DeepCopy(input0->GetPointData());

  return 1;
}

int vtkTemplate::RequestUpdateExtent(
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


int vtkTemplate::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
