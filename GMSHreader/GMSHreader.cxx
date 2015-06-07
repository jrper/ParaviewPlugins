#include "GMSHreader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include <vtkIntArray.h>
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
#include <fstream>
#include <map>



vtkCxxRevisionMacro(GMSHreader, "$Revision: 0.0$");
vtkStandardNewMacro(GMSHreader);

GMSHreader::GMSHreader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
GMSHreader::~GMSHreader(){
 this->SetFileName(0);
};

int GMSHreader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

  // try to open the GMSH file

  ifstream GMSHfile;
  GMSHfile.open(this->FileName);

  std::string line;


  // skip the header

  this->DebugOff();


for(GMSHfile >> line;line.compare("$Nodes");GMSHfile >> line){
  vtkDebugMacro(<< line);
}
  // try to read the point data
  int n;
  GMSHfile >> n;
  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();
  outpoints->Allocate(n);
  for (int i=0;i<n;i++){
    int nodeno;
    double x,y,z;
    GMSHfile >> nodeno >> x >> y >> z;
    outpoints->InsertNextPoint(x,y,z);
  }

  output->SetPoints(outpoints);

  // try to read the element data
for(GMSHfile >> line;line.compare("$Elements");GMSHfile >> line){
  vtkDebugMacro(<< line);
}
  GMSHfile >> n;
  output->Allocate(n);
  int eleNo, CellType, tagNo;

  vtkSmartPointer<vtkIntArray> ElementaryEntities= vtkSmartPointer<vtkIntArray>::New();
  ElementaryEntities->SetName("ElementaryEntities");
  ElementaryEntities->Allocate(n);
  vtkSmartPointer<vtkIntArray> PhysicalIds= vtkSmartPointer<vtkIntArray>::New();
  PhysicalIds->SetName("PhysicalIds");
  PhysicalIds->Allocate(n);
  for (int i=0;i<n;i++){
    GMSHfile>> eleNo >> CellType >> tagNo;
    for (int j=0;j<tagNo;j++){
      int tag;
      GMSHfile >> tag;
      if (j==0) {
	ElementaryEntities->InsertNextValue(tag);
      }
      if (j==1) {
	PhysicalIds->InsertNextValue(tag);
      }      
    }
    vtkDebugMacro(<< CellType);
    switch (CellType)
    { 
    case 15:
      {
      // Point
      int cellPoints[1];
      vtkIdType cellPointsVTK[1];
      GMSHfile>> cellPoints[0];
for(int j=0;j<1;j++){
       cellPointsVTK[j]=cellPoints[j]-1;
      }
      output->InsertNextCell((int)1,(vtkIdType)1,cellPointsVTK);
      }
 case 1:
// Line
 {int cellPoints[2];
      vtkIdType cellPointsVTK[2];
GMSHfile>> cellPoints[0] >> cellPoints[1];
for(int j=0;j<2;j++){
       cellPointsVTK[j]=cellPoints[j]-1;
    }
      output->InsertNextCell((int)3,(vtkIdType)2,cellPointsVTK);
break;
}
 case 2:
 // Triangle
 {
int cellPoints[3];
      vtkIdType cellPointsVTK[3];
GMSHfile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2];
for(int j=0;j<3;j++){
       cellPointsVTK[j]=cellPoints[j]-1;
    }
      output->InsertNextCell((int)5,(vtkIdType)3,cellPointsVTK);
break;
}
case 4:
// Tetrahedron
{
      int cellPoints[4];
      vtkIdType cellPointsVTK[4];
GMSHfile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2] >> cellPoints[3];
for(int j=0;j<4;j++){
       cellPointsVTK[j]=cellPoints[j]-1;
    }
      output->InsertNextCell((int)10,(vtkIdType)4,cellPointsVTK);
break;
}
}
}

  output->GetCellData()->AddArray(ElementaryEntities);
  output->GetCellData()->AddArray(PhysicalIds);

  // try to close the GMSH file
  GMSHfile.close();

  return 1;
}

void GMSHreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
