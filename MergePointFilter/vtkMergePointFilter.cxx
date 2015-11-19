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
#include "vtkStreamingDemandDrivenPipeline.h"





vtkCxxRevisionMacro(vtkMergePointFilter, "$Revision: 0.0$");
vtkStandardNewMacro(vtkMergePointFilter);

vtkMergePointFilter::vtkMergePointFilter(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
};
vtkMergePointFilter::~vtkMergePointFilter(){};

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

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();

  mergePoints->SetTolerance(1e-16);
  mergePoints->InitPointInsertion(outpoints, input->GetBounds());

  vtkIdType* id_map;
  id_map = new vtkIdType [input->GetNumberOfPoints()];

  for (int i=0; i<input->GetNumberOfPoints();i++) {
    mergePoints->InsertUniquePoint(input->GetPoints()->GetPoint(i),id_map[i]);
  }
  output->SetPoints(outpoints);
  output->Allocate(input->GetNumberOfCells());

  for (int i=0; i<input->GetNumberOfCells();i++) {
    vtkCell* cell;
    cell = input->GetCell(i);
    vtkSmartPointer<vtkIdList> cellIds= vtkSmartPointer<vtkIdList>::New();

    for (int j=0; j<cell->GetNumberOfPoints();j++) {
      cellIds->InsertNextId(id_map[cell->GetPointIds()->GetId(j)]);
    }

    output->InsertNextCell(cell->GetCellType(),cellIds);
   
  }

  for (int i=0; i<input->GetPointData()->GetNumberOfArrays();i++) {
    vtkSmartPointer<vtkDoubleArray> data= vtkSmartPointer<vtkDoubleArray>::New();
    data->SetName(input->GetPointData()->GetArray(i)->GetName());
    data->SetNumberOfComponents(input->GetPointData()->GetArray(i)->GetNumberOfComponents());
    data->SetNumberOfTuples(output->GetNumberOfPoints());

    for (int j=0; j<input->GetNumberOfPoints();j++) {
      data->InsertTuple(id_map[j], input->GetPointData()->GetArray(i)->GetTuple(j));
    }
    output->GetPointData()->AddArray(data);
    
  }

  output->GetCellData()->DeepCopy(input->GetCellData());

  delete[] id_map;

  return 1;
}

int vtkMergePointFilter::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
