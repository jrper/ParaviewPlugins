#include "SplitBcs.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTriangle.h"
#include "vtkTetra.h"
#include "vtkQuad.h"
#include "vtkHexahedron.h"
#include "vtkPolygon.h"
#include <iostream>
#include <map>
#include <set>



vtkCxxRevisionMacro(SplitBcs, "$Revision: 0.0$");
vtkStandardNewMacro(SplitBcs);

SplitBcs::SplitBcs(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(2);
};
SplitBcs::~SplitBcs(){};

int SplitBcs::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  // output0 will be a copy of the high dimensional cells and 
  // output1 will be the rest
  vtkInformation* outInfo0=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output0= vtkUnstructuredGrid::SafeDownCast(outInfo0->Get(vtkDataObject::DATA_OBJECT() ) );
  vtkInformation* outInfo1=outputVector->GetInformationObject(1);
  vtkUnstructuredGrid* output1= vtkUnstructuredGrid::SafeDownCast(outInfo1->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[0]);

vtkSmartPointer<vtkPoints> outpoints0= vtkSmartPointer<vtkPoints>::New();
vtkSmartPointer<vtkPoints> outpoints1= vtkSmartPointer<vtkPoints>::New();
vtkSmartPointer<vtkCellTypes> cellTypeList= vtkSmartPointer<vtkCellTypes>::New();
 outpoints0->DeepCopy(input->GetPoints());
 output0->SetPoints(outpoints0);
outpoints1->DeepCopy(input->GetPoints());
 output1->SetPoints(outpoints1);

 input->GetCellTypes(cellTypeList);

 int dimension=0;

 if (cellTypeList->IsType(VTK_TETRA) || cellTypeList->IsType(VTK_HEXAHEDRON) || cellTypeList->IsType(VTK_WEDGE) || cellTypeList->IsType(VTK_QUADRATIC_TETRA) || cellTypeList->IsType(VTK_QUADRATIC_HEXAHEDRON) || cellTypeList->IsType(VTK_QUADRATIC_WEDGE) ) { dimension=3;} else if ( cellTypeList->IsType(VTK_TRIANGLE) || cellTypeList->IsType(VTK_QUAD) || cellTypeList->IsType(VTK_POLYGON) || cellTypeList->IsType(VTK_QUADRATIC_TRIANGLE) || cellTypeList->IsType(VTK_BIQUADRATIC_QUAD)) { dimension=2;}
 else if (cellTypeList->IsType(VTK_LINE) or cellTypeList->IsType(VTK_QUADRATIC_EDGE)) {dimension=1;}

 vtkIdType N=input->GetNumberOfCells();
 output0->Allocate(N);
 output0->GetPointData()->DeepCopy(input->GetPointData());
 output1->Allocate(N);
 output1->GetPointData()->DeepCopy(input->GetPointData());

   
 int NCDA=input->GetCellData()->GetNumberOfArrays();
 output0->GetCellData()->CopyStructure(input->GetCellData());
 output1->GetCellData()->CopyStructure(input->GetCellData());
 for (vtkIdType i=0;i<NCDA;i++)
   {
     output0->GetCellData()->GetArray(i)->SetName(input->GetCellData()->GetArray(i)->GetName());
     output1->GetCellData()->GetArray(i)->SetName(input->GetCellData()->GetArray(i)->GetName());
   }
     
 
 for (vtkIdType i=0;i<N;i++)
   {
     vtkCell* C=input->GetCell(i);
     if (dimension==input->GetCell(i)->GetCellDimension()) {
       output0->InsertNextCell(C->GetCellType(),
			       C->GetPointIds());
       for (vtkIdType j=0;j<NCDA;j++) {
	 output0->GetCellData()->GetArray(j)->InsertNextTuple(input->GetCellData()->GetArray(j)->GetTuple(i)); }
     } else {
       output1->InsertNextCell(C->GetCellType(),
			       C->GetPointIds());
       for (vtkIdType j=0;j<NCDA;j++) {
	 output1->GetCellData()->GetArray(j)->InsertNextTuple(input->GetCellData()->GetArray(j)->GetTuple(i)); }

     }
   }

  return 1;
}

int SplitBcs::RequestUpdateExtent(
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


int SplitBcs::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
