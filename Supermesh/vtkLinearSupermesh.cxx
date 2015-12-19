#include "vtkLinearSupermesh.h"

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
#include "vtkDoubleArray.h"
#include "vtkSupermesh.h"
#include <iostream>
#include <map>
#include <set>



vtkCxxRevisionMacro(vtkLinearSupermesh, "$Revision: 0.0$");
vtkStandardNewMacro(vtkLinearSupermesh);

extern"C" {
  void* position_from_vtk_c(double *x1, int n_x1, vtkIdType *c1, int n_c1);
  void position_from_vtk_c_sp(float *x1, int n_x1, vtkIdType *c1, int n_c1, void** ptr);
  void supermesh(void* ptr1, void* ptr2,
		 int* nNodes, int* nEles, int* nEntries, void** context);
  void copy_mesh_to_vtk(void* mesh, double *x,int n_x,  vtkIdType *c,
			int n_c, int* ele_map_0, int* ele_map_1, int nEles);
}

vtkLinearSupermesh::vtkLinearSupermesh(){
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
};
vtkLinearSupermesh::~vtkLinearSupermesh(){};

int vtkLinearSupermesh::RequestData(
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

  vtkSmartPointer<vtkSupermesh> supermesh= vtkSmartPointer<vtkSupermesh>::New();

  supermesh->SetInputDataObject(0,input0);
  supermesh->SetInputDataObject(1,input1);
  supermesh->Update();

  output0->DeepCopy(supermesh->GetOutput());

  for (int i=0;i< input0->GetCellData()->GetNumberOfArrays();i++)
    {
      if (input1->GetCellData()->HasArray(input0->GetCellData()->GetArray(i)->GetName())) {
	vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
	data->SetName(input0->GetCellData()->GetArray(i)->GetName());
	data->SetNumberOfComponents(input0->GetCellData()->GetArray(i)->GetNumberOfComponents());
	data->SetNumberOfTuples(output0->GetNumberOfCells());
	for (int j=0; j<input0->GetCellData()->GetArray(i)->GetNumberOfComponents(); j++) {
	  data->FillComponent(j,0.0);
	}
	output0->GetCellData()->AddArray(data);
      }
    }

for (int i=0;i< input0->GetPointData()->GetNumberOfArrays();i++)
    {
      if (input1->GetPointData()->HasArray(input0->GetPointData()->GetArray(i)->GetName())) {
	vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
	data->SetName(input0->GetPointData()->GetArray(i)->GetName());
	vtkDebugMacro(<<"Field: "<<data->GetName());
	data->SetNumberOfComponents(input0->GetPointData()->GetArray(i)->GetNumberOfComponents());
	data->SetNumberOfTuples(output0->GetNumberOfPoints());
	for (int j=0; j<data->GetNumberOfComponents(); j++) {
	  data->FillComponent(j,0.0);
	}
	output0->GetPointData()->AddArray(data);
      }
    }


 this->AddInputToData(output0,input0,
		      output0->GetCellData()->GetScalars("CellId0"),0);
 this->AddInputToData(output0,input1,
		      output0->GetCellData()->GetScalars("CellId1"),1);

  return 1;
}

int vtkLinearSupermesh::AddInputToData(vtkUnstructuredGrid *output,
				       vtkUnstructuredGrid *input,
				       vtkDataArray *ele_map,
				       int n)
  {
    int unvisited[output->GetNumberOfPoints()];
    for (int i=0; i<output->GetNumberOfPoints(); i++) {
      unvisited[i]=1;
    }

    for (vtkIdType i=0; i<output->GetNumberOfCells(); i++)
      {
	int input_ele = vtkIntArray::SafeDownCast(ele_map)->GetValue(i);
	vtkCell* cell=output->GetCell(i);
	vtkCell* iCell=input->GetCell(input_ele);

	// Cell data is easy:
	for (int da=0; da < output->GetCellData()->GetNumberOfArrays();da++) {
	  vtkDataArray* data=output->GetCellData()->GetArray(da);
	  if(input->GetCellData()->HasArray(data->GetName())){
	    for (int j=0; j<data->GetNumberOfComponents();j++) {
	      double v;
	      v=data->GetComponent(i,j);
	      v=v+this->Coefficients[n]*input->GetCellData()->GetArray(data->GetName())->GetComponent(input_ele,j);
	      data->SetComponent(i,j,v);
	    } 
	  }
	}
	// Point data requires more work
	vtkSmartPointer<vtkDoubleArray> input_data= vtkSmartPointer<vtkDoubleArray>::New();
	for (int j=0; j<cell->GetNumberOfPoints();j++) {
	  vtkIdType id = cell->GetPointId(j);
	  if (unvisited[id]) {
	    double x[3], pcoords[3], dist2, 
	    weights[iCell->GetNumberOfPoints()] ;
	    int subId;
	    output->GetPoints()->GetPoint(id,x);

	    iCell->EvaluatePosition(x,NULL,subId,pcoords,dist2,weights);
	    
	    for (int da=0; da < output->GetPointData()->GetNumberOfArrays();
		 da++) {
	      vtkDataArray* data=output->GetPointData()->GetArray(da);
	      if(input->GetPointData()->HasArray(data->GetName())){
	        input_data->DeepCopy(input->GetPointData()->GetArray(data->GetName()));
		double v[data->GetNumberOfTuples()];
		data->GetTuple(id,v);
		data->InterpolateTuple( id, iCell->GetPointIds(),
					input_data, weights);
		for (int k=0; k<data->GetNumberOfComponents();k++) {
		  double v2;
		  v2=data->GetComponent(id,k);
		  v2=v[k]+this->Coefficients[n]*v2;
		  data->SetComponent(id,k,v2);
		}
	      }
	    }
	    unvisited[id]=0;
	  }
	}
      }
  }

int vtkLinearSupermesh::RequestUpdateExtent(
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


int vtkLinearSupermesh::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
