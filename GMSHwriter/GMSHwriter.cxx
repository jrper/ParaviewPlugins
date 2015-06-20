#include "GMSHwriter.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include <vtkIntArray.h>
#include "vtkPoints.h"
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
#include <algorithm>

vtkCxxRevisionMacro(GMSHwriter, "$Revision: 0.0$");
vtkStandardNewMacro(GMSHwriter);

GMSHwriter::GMSHwriter(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(0);
};
GMSHwriter::~GMSHwriter(){
 this->SetFileName(0);
};

void GMSHwriter::WriteData()
{
  vtkUnstructuredGrid *input= vtkUnstructuredGrid::SafeDownCast(
    this->GetInput());

  // try to open the GMSH file
  std::ofstream GMSHfile(this->FileName,std::ofstream::binary);

  this->DebugOn();
  // write the header
  GMSHfile << "$MeshFormat" <<std::endl;
  GMSHfile << "2.2 1 8" << std::endl;
  int one=1;
  GMSHfile.write(reinterpret_cast<char*>(&one),4);
  GMSHfile << std::endl;
  GMSHfile << "$EndMeshFormat" <<std::endl;

  // try to write the point data
  GMSHfile << "$Nodes" <<std::endl;
  GMSHfile <<  input->GetNumberOfPoints() <<std::endl;
  for (vtkIdType i=0;i<input->GetNumberOfPoints();i++) {
    double x[3];
    input->GetPoint(i,x);
    int i_int=i+1;
    GMSHfile.write(reinterpret_cast<char*>(&i_int),sizeof(int));
    GMSHfile.write(reinterpret_cast<char*>(&x),3*sizeof(double));
  }
  GMSHfile<<std::endl;
  GMSHfile << "$EndNodes" <<std::endl; 

  // try to write the element data
  GMSHfile << "$Elements" <<std::endl;
  GMSHfile << input->GetNumberOfCells() <<std::endl;

  int writeTypes[4]={VTK_VERTEX,VTK_LINE,VTK_TRIANGLE,VTK_TETRA};
  int gmshTypes[4]={15,1,2,4};
  int nodeCount[4]={1,2,3,4};

  int n=1;
  for (int i=0;i<4;i++){
    vtkSmartPointer<vtkIdTypeArray> Ids= vtkSmartPointer<vtkIdTypeArray>::New();
    input->GetIdsOfCellsOfType(writeTypes[i],Ids);
    int typeData[3]={gmshTypes[i],Ids->GetNumberOfTuples(),2};
    if (typeData[1]>0) {
      GMSHfile.write(reinterpret_cast<char*>(&typeData),3*sizeof(int));
    }
    for(int j=0;j<typeData[1];j++) {
      int eleMetaData[3]={n,n,n};
      GMSHfile.write(reinterpret_cast<char*>(&eleMetaData),3*sizeof(int));
      vtkIdType cellId=Ids->GetTuple1(j);
      for(int k=0;k<nodeCount[i];k++) {
	int pointId=input->GetCell(cellId)->GetPointId(k)+1;
	GMSHfile.write(reinterpret_cast<char*>(&pointId),sizeof(int));
      }
      ++n;
    }
  }
  GMSHfile << std::endl;
  GMSHfile << "$EndElements" <<std::endl;
     
    
  // try to close the GMSH file
  GMSHfile.close();

  return;
}

int GMSHwriter::FillInputPortInformation(int,vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkUnstructuredGrid");
  return 1;
}

vtkUnstructuredGrid* GMSHwriter::GetInput()
{
  return vtkUnstructuredGrid::SafeDownCast(this->Superclass::GetInput());
}

vtkUnstructuredGrid* GMSHwriter::GetInput(int port)
{
  return vtkUnstructuredGrid::SafeDownCast(this->Superclass::GetInput(port));
}

void GMSHwriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
