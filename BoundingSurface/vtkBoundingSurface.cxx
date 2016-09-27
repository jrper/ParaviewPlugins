#include "vtkBoundingSurface.h"

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
#include "vtkCell.h"
#include "vtkLine.h"

#include <utility>
#include <unordered_set>
#include <algorithm>

struct pair_hash {
  inline std::size_t operator()(const std::pair<vtkIdType,vtkIdType> & v) const {
    return v.first*31+v.second;
  }
};

std::pair<vtkIdType,vtkIdType> sort(vtkCell* e) {
  int p0 = e->GetPointIds()->GetId(0);
  int p1 = e->GetPointIds()->GetId(1);
  
  std::pair<vtkIdType,vtkIdType> out;
  
  if (p0<p1) {
    out.first=p0;
    out.second=p1;
  } else
    {  out.first=p1;
      out.second=p0;
    }
  return out;
}



vtkCxxRevisionMacro(vtkBoundingSurface, "$Revision: 0.0$");
vtkStandardNewMacro(vtkBoundingSurface);

vtkBoundingSurface::vtkBoundingSurface(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
};
vtkBoundingSurface::~vtkBoundingSurface(){};

int vtkBoundingSurface::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{

  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );
  
  vtkInformation *inInfo=inputVector[0]->GetInformationObject(0);
  vtkUnstructuredGrid* input= vtkUnstructuredGrid::GetData(inputVector[0]);

  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();  

#ifndef NDEBUG
  this->DebugOn();
#endif

  outpoints->DeepCopy(input->GetPoints());
  output->SetPoints(outpoints);
  
  int dim=0;

  for (vtkIdType i=0; i<input->GetNumberOfCells(); ++i) {
    vtkCell* cell=input->GetCell(i);
    dim = std::max(dim,cell->GetCellDimension());
  }
  switch (dim)
    {
    case 1: 
      {
	std::unordered_set<vtkIdType> boundary;
	for (vtkIdType i=0; i<input->GetNumberOfCells(); ++i) {
	  vtkCell* cell=input->GetCell(i);
	  
	  if (cell->GetCellDimension() != dim) continue;
	  
	  for (int j=0; j<cell->GetNumberOfPoints(); ++j){
	    vtkIdType pt = cell->GetPointIds()->GetId(j);
	    if (boundary.count(pt)) {
	      boundary.erase(pt);
	    } else {
	      boundary.insert(pt);
	    }
	  }

	}
	output->Allocate(boundary.size());

	for (std::unordered_set<vtkIdType>::iterator e = boundary.begin(); e != boundary.end(); ++e) {
	  vtkIdType l[1];
	  l[0] = *e;
	  output->InsertNextCell(VTK_VERTEX,1,l);
	}
      }
      break;
    case 2:
      {
	std::unordered_set<std::pair<vtkIdType,vtkIdType>, pair_hash > boundary;
	
	for (vtkIdType i=0; i<input->GetNumberOfCells(); ++i) {
	  vtkCell* cell=input->GetCell(i);
	  
	  if (cell->GetCellDimension() != dim) continue;
	  
	  
	  for (int j=0; j<cell->GetNumberOfEdges(); ++j){
	    vtkCell* edge = cell->GetEdge(j);
	    
	    std::pair<vtkIdType,vtkIdType> vertices = sort(edge);
	    
	    if (boundary.count(vertices)) {
	      boundary.erase(vertices);
	    } else {
	      boundary.insert(vertices);
	    }
	  }
	}
	output->Allocate(boundary.size());
	  
	for (std::unordered_set<std::pair<vtkIdType,vtkIdType>, pair_hash>::iterator e = boundary.begin(); e != boundary.end(); ++e  ) {
	  vtkIdType l[2];
	  l[0]=e->first;
	  l[1]=e->second;
	  output->InsertNextCell(VTK_LINE,2,l);
	}
      }
      break;
    case 3:
      break;
    }

  output->GetPointData()->DeepCopy(input->GetPointData());

  return 1;
}

int vtkBoundingSurface::RequestUpdateExtent(
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


int vtkBoundingSurface::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}
