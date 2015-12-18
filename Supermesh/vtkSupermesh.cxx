#include "vtkSupermesh.h"

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



vtkCxxRevisionMacro(vtkSupermesh, "$Revision: 0.0$");
vtkStandardNewMacro(vtkSupermesh);

extern"C" {
  void* position_from_vtk_c(double *x1, int n_x1, vtkIdType *c1, int n_c1);
  void position_from_vtk_c_sp(float *x1, int n_x1, vtkIdType *c1, int n_c1, void** ptr);
  void supermesh(void* ptr1, void* ptr2,
		 int* nNodes, int* nEles, int* nEntries, void** context);
  void copy_mesh_to_vtk(void* mesh, double *x,int n_x,  vtkIdType *c,
			int n_c, int* ele_map_0, int* ele_map_1, int nEles);
}

vtkSupermesh::vtkSupermesh(){
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
};
vtkSupermesh::~vtkSupermesh(){};

int vtkSupermesh::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  // output0 will be a copy of the high dimensional cells and 
  // output1 will be the rest
  vtkInformation* outInfo0=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output0= vtkUnstructuredGrid::SafeDownCast(outInfo0->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo0=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input0= vtkUnstructuredGrid::GetData(inputVector[0]);
  vtkInformation *inInfo1=inputVector[1]->GetInformationObject(0);
  vtkUnstructuredGrid* input1= vtkUnstructuredGrid::GetData(inputVector[1]);

vtkSmartPointer<vtkPoints> outpoints0= vtkSmartPointer<vtkPoints>::New();
vtkSmartPointer<vtkCellTypes> cellTypeList= vtkSmartPointer<vtkCellTypes>::New();


 vtkSmartPointer<vtkCellArray> output_connectivity= vtkSmartPointer<vtkCellArray>::New();      

#ifndef NDEBUG
 this->DebugOn();
#endif

 vtkDebugMacro(<< input0->GetPoints()->GetDataType() <<" " << input0->GetNumberOfPoints() << " " << input1->GetNumberOfPoints());
 vtkDebugMacro(<< input0->GetNumberOfCells() << " " << input1->GetNumberOfCells());


 int nNodes, nElements, nEntries;
 void *context, *ptr1, *ptr2;


 switch(input0->GetPoints()->GetDataType()){
 case VTK_FLOAT:
   position_from_vtk_c_sp((float*) input0->GetPoints()->GetData()->GetVoidPointer(0),
	     input0->GetNumberOfPoints(),
	     input0->GetCells()->GetPointer(),
			  input0->GetCells()->GetNumberOfConnectivityEntries(),&ptr1);
   break;
 case VTK_DOUBLE:
   ptr1 = position_from_vtk_c((double*) input0->GetPoints()->GetData()->GetVoidPointer(0),
	     input0->GetNumberOfPoints(),
	     input0->GetCells()->GetPointer(),
			      input0->GetCells()->GetNumberOfConnectivityEntries());
   break;
 }

 switch(input1->GetPoints()->GetDataType()){
 case VTK_FLOAT:
   position_from_vtk_c_sp((float*) input1->GetPoints()->GetData()->GetVoidPointer(0),
	     input1->GetNumberOfPoints(),	     input1->GetCells()->GetPointer(),
			  input1->GetCells()->GetNumberOfConnectivityEntries(),&ptr2);
   break;
 case VTK_DOUBLE:
   ptr2 = position_from_vtk_c((double*) input1->GetPoints()->GetData()->GetVoidPointer(0),
	     input1->GetNumberOfPoints(),
	     input1->GetCells()->GetPointer(),
			      input1->GetCells()->GetNumberOfConnectivityEntries());
   break;
 }
   
 supermesh( ptr1, ptr2, &nNodes,&nElements,&nEntries,&context);

 outpoints0->SetDataTypeToDouble();
 outpoints0->Allocate(nNodes);
 outpoints0->SetNumberOfPoints(nNodes);
 output0->Allocate(nElements);

 vtkSmartPointer<vtkIntArray> ele_map_0= vtkSmartPointer<vtkIntArray>::New();
 vtkSmartPointer<vtkIntArray> ele_map_1= vtkSmartPointer<vtkIntArray>::New();
 ele_map_0->SetName("CellId0");
 ele_map_1->SetName("CellId1");

 copy_mesh_to_vtk(context,(double*) outpoints0->GetData()->GetVoidPointer(0),
		  nNodes,
		  output_connectivity->WritePointer(nElements,
						    nEntries),
		  nEntries,
		  (int *) ele_map_0->WriteVoidPointer(0,nElements),
		  (int *) ele_map_1->WriteVoidPointer(0,nElements),
		  nElements);


 output0->SetPoints(outpoints0);
 output0->SetCells(VTK_TRIANGLE,output_connectivity);
 output0->GetCellData()->AddArray(ele_map_0);
 output0->GetCellData()->AddArray(ele_map_1);

vtkDebugMacro(<< output0->GetNumberOfCells());

  return 1;
}

int vtkSupermesh::RequestUpdateExtent(
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


int vtkSupermesh::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
