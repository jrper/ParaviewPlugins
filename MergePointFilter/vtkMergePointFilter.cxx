#include "vtkMergePointFilter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellTypes.h"
#include "vtkCellData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkMergePoints.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkStreamingDemandDrivenPipeline.h"





vtkCxxRevisionMacro(vtkMergePointFilter, "$Revision: 0.0$");
vtkStandardNewMacro(vtkMergePointFilter);

int LinearType(int cellType) {
  switch (cellType) {
  case VTK_QUADRATIC_HEXAHEDRON:
    return VTK_HEXAHEDRON;
  case VTK_QUADRATIC_PYRAMID:
    return VTK_PYRAMID;
  case VTK_QUADRATIC_QUAD:
    return VTK_QUAD;
  case VTK_QUADRATIC_TETRA:
    return VTK_TETRA;
  case VTK_QUADRATIC_TRIANGLE:
    return VTK_TRIANGLE;
  case VTK_QUADRATIC_WEDGE:
    return VTK_WEDGE;
  }
  return cellType;
}

int QuadraticType(int cellType) {
  switch (cellType) {
  case VTK_HEXAHEDRON:
    return VTK_QUADRATIC_HEXAHEDRON;
  case VTK_PYRAMID:
    return VTK_QUADRATIC_PYRAMID;
  case VTK_QUAD:
    return VTK_QUADRATIC_QUAD;
  case VTK_TETRA:
    return VTK_QUADRATIC_TETRA;
  case VTK_TRIANGLE:
    return VTK_QUADRATIC_TRIANGLE;
  case VTK_WEDGE:
    return VTK_QUADRATIC_WEDGE;
  }
  return cellType;
}

void AddPoints(vtkCell* cell, vtkPoints* input_points, int id1, int id2, double* result) {
  double p1[3], p2[3];
  input_points->GetPoint(cell->GetPointId(id1),p1);
  input_points->GetPoint(cell->GetPointId(id2),p2);
  for (int i=0;i<3;i++){
    result[i]=(p1[i]+p2[i])/2.0;
  }
}

void AddData(vtkDataArray* output_data,vtkDataArray* input_data,vtkCell* output_cell,vtkCell* input_cell,int i,int j,int k) {
  double v1[input_data->GetNumberOfTuples()];
  double v2[input_data->GetNumberOfTuples()];
  double vo[input_data->GetNumberOfTuples()];

  input_data->GetTuple(input_cell->GetPointId(i),v1);
  input_data->GetTuple(input_cell->GetPointId(j),v2);

  for (int n=0; n<input_data->GetNumberOfTuples(); n++) {
    vo[n]=(v1[n]+v2[n])/2.0;
  }

  output_data->SetTuple(output_cell->GetPointId(k),vo);
}


vtkMergePointFilter::vtkMergePointFilter(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
  this->Degree=-1; // -1 will mean don't change, 1 Linear, 2 Quadratic.
  this->Continuity=0; // 0 will mean don't change, 1 continuous, -1 discontinuous.

#ifndef NDEBUG
  this->DebugOn();
#endif
};
vtkMergePointFilter::~vtkMergePointFilter(){};

void vtkMergePointFilter::AddQuadraticData(vtkDataArray* output_data,vtkDataArray* input_data,vtkCell* input_cell,vtkCell* output_cell) {

  int N = output_cell->GetNumberOfPoints()-input_cell->GetNumberOfPoints();
  int N_in = input_cell->GetNumberOfPoints();

  AddData(output_data,input_data,output_cell,input_cell,0,1,N_in);
  if (N>1) {
    AddData(output_data,input_data,output_cell,input_cell,1,2,N_in+1);
    AddData(output_data,input_data,output_cell,input_cell,2,0,N_in+2);
  }
  if (N>3) {
    AddData(output_data,input_data,output_cell,input_cell,0,3,N_in+3);
    AddData(output_data,input_data,output_cell,input_cell,1,3,N_in+4);
    AddData(output_data,input_data,output_cell,input_cell,2,3,N_in+5);
  }
}

 void vtkMergePointFilter::AddQuadraticPoints(vtkMergePoints* mergePoints, vtkPoints* input_points, vtkCell* cell,vtkIdList* cellIds, int input_continuity) {
  if (cell->GetCellType() == QuadraticType(cell->GetCellType())) {
    return;
  }
  int N = cell->GetNumberOfPoints();
  N = N*(N-1)/2;
  double points[3*N];
  vtkDebugMacro(<<"bob "<< cell->GetPointId(0)<<","<<cell->GetPointId(1)<<","<<cell->GetPointId(2));
  AddPoints(cell,input_points,0,1,&(points[0]));
  vtkDebugMacro(<<"janet "<< input_points->GetPoint(cell->GetPointId(0))[0]<<","<<input_points->GetPoint(cell->GetPointId(1))[0]<<","<<points[0]);
  if (N>1) {
    AddPoints(cell,input_points,1,2,&(points[3]));
    AddPoints(cell,input_points,2,0,&(points[6]));
  }
  if (N>3) {
    AddPoints(cell,input_points,0,3,&(points[9]));
    AddPoints(cell,input_points,1,3,&(points[12]));
    AddPoints(cell,input_points,2,3,&(points[15]));
  }	 
  for (int j=0; j<N; j++) {
    vtkIdType id;
    if (this->Continuity<0 || ( this->Continuity==0 && input_continuity<0)) {
      id=mergePoints->GetPoints()->InsertNextPoint(&(points[3*j]));
    } else { 
      mergePoints->InsertUniquePoint(&(points[3*j]),id);
    }
    cellIds->InsertNextId(id);
  }
}

int vtkMergePointFilter::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[0]);

  vtkSmartPointer<vtkMergePoints> mergePoints= vtkSmartPointer<vtkMergePoints>::New();

  int input_continuity;
  
  if (input->GetNumberOfPoints() == input->GetNumberOfCells() * input->GetCell(0)->GetNumberOfPoints()) {
    input_continuity=-1;
  } else {
    input_continuity=1;
  }

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();

  mergePoints->SetTolerance(1.e-34);
  mergePoints->InitPointInsertion(outpoints, input->GetBounds());

  output->Allocate(input->GetNumberOfCells());

  vtkDebugMacro(<<"Ouput Continuity "<< this->GetContinuity());
  vtkDebugMacro(<<"Ouput Degree "<< this->GetDegree());

  for (int i=0; i<input->GetNumberOfCells();i++) {
    vtkCell* cell;
    cell = input->GetCell(i);
    vtkSmartPointer<vtkIdList> cellIds= vtkSmartPointer<vtkIdList>::New();
    vtkIdType id_map;

    for (int j=0; j<cell->GetNumberOfPoints();j++) {
      vtkIdType id=cell->GetPointIds()->GetId(j);
      if (this->Continuity<0 || (this->Continuity==0 && input_continuity<0)) {
	id_map=mergePoints->GetPoints()->InsertNextPoint(input->GetPoints()->GetPoint(id));
      } else {
	mergePoints->InsertUniquePoint(input->GetPoints()->GetPoint(id),id_map);
      }
      cellIds->InsertNextId(id_map);
    }

    switch (this->GetDegree())
      {
      case -1:
	output->InsertNextCell(cell->GetCellType(),cellIds);
	break;
      case 1:
	output->InsertNextCell(LinearType(cell->GetCellType()),cellIds);
	break;
      case 2:	
	AddQuadraticPoints(mergePoints,input->GetPoints(),cell,cellIds, input_continuity);
	vtkDebugMacro(<<"Cell Type : "<<cell->GetCellType());
	vtkDebugMacro(<<"Quadratic Cell Type : "<<QuadraticType(cell->GetCellType()));
	vtkDebugMacro(<<"cellIds : "<< cellIds->GetNumberOfIds());
	output->InsertNextCell(QuadraticType(cell->GetCellType()),cellIds);
	break;
      }    
  }

  vtkSmartPointer<vtkPoints> tmppoints= vtkSmartPointer<vtkPoints>::New();
  tmppoints->DeepCopy(mergePoints->GetPoints());
  output->SetPoints(tmppoints);

  vtkDebugMacro(<<"Ouput No. of Points "<< output->GetNumberOfPoints());

  for (int i=0; i<input->GetPointData()->GetNumberOfArrays();i++) {
    vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
    data->SetName(input->GetPointData()->GetArray(i)->GetName());
    data->SetNumberOfComponents(input->GetPointData()->GetArray(i)->GetNumberOfComponents());
    data->SetNumberOfTuples(output->GetNumberOfPoints());

    if (this->Continuity==0 && input->GetCell(0)->GetCellType() == output->GetCell(0)->GetCellType()) {
      data->DeepCopy(input->GetPointData()->GetArray(i));
    } else {
      for (int j=0; j<input->GetNumberOfCells();j++) {
	vtkCell* input_cell=input->GetCell(j);
	vtkCell* output_cell=output->GetCell(j);
	if (LinearType(input_cell->GetCellType())==output_cell->GetCellType()
	    || input_cell->GetCellType()==output_cell->GetCellType()) {
	  for (int k=0;k<output_cell->GetNumberOfPoints();k++) {
	    vtkIdType input_id=input_cell->GetPointId(k);
	    vtkIdType output_id=output_cell->GetPointId(k);
	    data->SetTuple(output_id, input->GetPointData()->GetArray(i)->GetTuple(input_id));
	  }
	} else { 
	  // Need to remap the data
	  for (int k=0;k<input_cell->GetNumberOfPoints();k++) {
	    vtkIdType input_id=input_cell->GetPointId(k);
	    vtkIdType output_id=output_cell->GetPointId(k);
	    data->SetTuple(output_id, input->GetPointData()->GetArray(i)->GetTuple(input_id));
	  }
	  AddQuadraticData(data,input->GetPointData()->GetArray(i),input_cell,output_cell);
	} 
      }
    }
    output->GetPointData()->AddArray(data);
    
  }

  output->GetCellData()->DeepCopy(input->GetCellData());
  return 1;
  }

int vtkMergePointFilter::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
