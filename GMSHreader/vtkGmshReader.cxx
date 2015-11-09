#include "vtkGmshReader.h"

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
#include <algorithm>

template <class T>
void endswap(T *objp)
{
  unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
  std::reverse(memp, memp + sizeof(T));
}

vtkCxxRevisionMacro(vtkGmshReader, "$Revision: 0.0$");
vtkStandardNewMacro(vtkGmshReader);

vtkGmshReader::vtkGmshReader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
vtkGmshReader::~vtkGmshReader(){
 this->SetFileName(0);
};

int readGMSHheader(std::ifstream& GMSHfile,float &version,int& isBinary,int& isSameEndian)
{
  std::string line;
  GMSHfile >> line;
  if(line.compare("$MeshFormat")) return 0;
  GMSHfile >> version;
  GMSHfile >> isBinary;
  int datasize;
  GMSHfile >> datasize; 
  if (datasize != sizeof(double)) return 0;
  if (isBinary==1) {
    int one;
    GMSHfile.read(( char*) &one,1);
    GMSHfile.read(( char*) &one,4);
isSameEndian=(one==1);
  } 
  GMSHfile >> line;
  if(line.compare("$EndMeshFormat")) return 0;
  return int(version>2.1);
}


      int readGMSHnodes(std::ifstream& GMSHfile,vtkPoints* outpoints,int isBinary,int isSameEndian,std::map<int,vtkIdType>& nodeMap)
{
  std::string line;
  GMSHfile >> line;
  if(line.compare("$Nodes")) return 0;
  int nnodes;
  GMSHfile >> nnodes;
  outpoints->Allocate(nnodes);
  if (isBinary) {
    char tim;
    GMSHfile.read(&tim,1);
    for(vtkIdType i=0;i<nnodes;i++) {
      int nodeno;
      double x,y,z;
      GMSHfile.read(( char*) &nodeno,4);
      GMSHfile.read(( char*) &x,sizeof(double));
      GMSHfile.read(( char*) &y,sizeof(double));
      GMSHfile.read(( char*) &z,sizeof(double));
      if (isSameEndian!=1) {
	endswap(&nodeno);
	endswap(&x);
	endswap(&y);
	endswap(&z);
      }
      nodeMap[nodeno]=i;
      outpoints->InsertNextPoint(x,y,z);
    }
  } else {
    for(vtkIdType i=0;i<nnodes;i++) {
      int nodeno;
      double x,y,z;
      GMSHfile >> nodeno >> x >> y >> z;
      nodeMap[nodeno]=i;
      outpoints->InsertNextPoint(x,y,z);
    }
  }
  GMSHfile >> line;
  if(line.compare("$EndNodes")) return 0;
  return 1;
}

int readGMSHelements(std::ifstream& GMSHfile,vtkUnstructuredGrid* output,int isBinary,int isSameEndian,std::map<int,vtkIdType>& nodeMap)
{ 
  const int nodeCount[20]={-1,2,3,4,4,8,6,5,3,6,9,10,
			   27,18,14,1,8,20,15,13};
  const int VTKtype[20]={-1,3,5,4,10,8,6,5,3,6,9,10,
			   27,18,14,15,8,20,15,13};
  std::string line;
  GMSHfile >> line;
  if(line.compare("$Elements")) return 0;
  vtkIdType nelements;
  GMSHfile >> nelements;
  output->Allocate(nelements);
  vtkSmartPointer<vtkIntArray> ElementaryEntities= vtkSmartPointer<vtkIntArray>::New();
  ElementaryEntities->SetName("ElementaryEntities");
  ElementaryEntities->Allocate(nelements);
  vtkSmartPointer<vtkIntArray> PhysicalIds= vtkSmartPointer<vtkIntArray>::New();
  PhysicalIds->SetName("PhysicalIds");
  PhysicalIds->Allocate(nelements);
  if (isBinary) {
    char tim;
    GMSHfile.read(( char*) &tim,1);
    int n=0;
    while(n<nelements) {
      int eleType,nElements,nTags;
      GMSHfile.read(( char*) &eleType,4);
      GMSHfile.read(( char*) &nElements,4);
      GMSHfile.read(( char*) &nTags,4);
      if (isSameEndian!=1) {
        endswap( &eleType );
        endswap( &nElements );
        endswap( &nTags );
      }
      n+=nElements;
      for (int i=0;i<nElements;i++) {
	int nele;
	GMSHfile.read(( char*) &nele,4);
	for (int j=0;j<nTags;j++) {
	  int tag;
	  GMSHfile.read(( char*) &tag,4);
          if (isSameEndian!=1) {
            endswap(&tag);
          }
	  if (j==1) {
	    ElementaryEntities->InsertNextValue(tag);
	  }
	  if (j==0) {
	    PhysicalIds->InsertNextValue(tag);
	  }    
	}
	std::vector<int> enodes;
	enodes.reserve(eleType);
	int node[nodeCount[eleType]];
	GMSHfile.read(( char*) &node[0],4*nodeCount[eleType]);
	vtkIdType cellPoints[nodeCount[eleType]];
	for (int j=0;j<nodeCount[eleType];j++) {
          if (isSameEndian!=1) {
            endswap(&node[j]);
          }
	  cellPoints[j]=nodeMap[node[j]];
	}
	output->InsertNextCell(VTKtype[eleType],nodeCount[eleType],cellPoints);
      }
    }
  } else {
    int eleNo, CellType,tagNo;
    for (vtkIdType i=0;i<nelements;i++){
      GMSHfile>> eleNo >> CellType >> tagNo;
      for (int j=0;j<tagNo;j++){
	int tag;
	GMSHfile >> tag;
	if (j==1) {
	  ElementaryEntities->InsertNextValue(tag);
	}
	if (j==0) {
	  PhysicalIds->InsertNextValue(tag);
	}      
      }
      switch (CellType)
	{ 
	case 15:
	  {
	    // Point
	    int cellPoints[1];
	    vtkIdType cellPointsVTK[1];
	    GMSHfile>> cellPoints[0];
	    for(int j=0;j<1;j++){
	      cellPointsVTK[j]=nodeMap[cellPoints[j]];
	    }
	    output->InsertNextCell((int)1,(vtkIdType)1,cellPointsVTK);
	    break;
	  }
	case 1:
	  // Line
	  {
	    int cellPoints[2];
	    vtkIdType cellPointsVTK[2];
	    GMSHfile>> cellPoints[0] >> cellPoints[1];
	    for(int j=0;j<2;j++){
	      cellPointsVTK[j]=nodeMap[cellPoints[j]];
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
	      cellPointsVTK[j]=nodeMap[cellPoints[j]];
	    }
	    output->InsertNextCell((int)5,(vtkIdType)3,cellPointsVTK);
	    break;
	  }
	case 3:
	  // Triangle
	  {
	    int cellPoints[4];
	    vtkIdType cellPointsVTK[4];
	    GMSHfile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2] >> cellPoints[3];
	    for(int j=0;j<4;j++){
	      cellPointsVTK[j]=nodeMap[cellPoints[j]];
	    }
	    output->InsertNextCell((int)9,(vtkIdType)4,cellPointsVTK);
	    break;
	  }
	case 4:
	  // Tetrahedron
	  {
	    int cellPoints[4];
	    vtkIdType cellPointsVTK[4];
	    GMSHfile>> cellPoints[0] >> cellPoints[1] >> cellPoints[2] >> cellPoints[3];
	    for(int j=0;j<4;j++){
	      cellPointsVTK[j]=nodeMap[cellPoints[j]];
	    }
	    output->InsertNextCell((int)10,(vtkIdType)4,cellPointsVTK);
	    break;
	  }
	}
    }
  }
  output->GetCellData()->AddArray(ElementaryEntities);
  output->GetCellData()->AddArray(PhysicalIds);
  GMSHfile >> line;
  if(line.compare("$EndElements")) return 0;
  return 1;
}

int vtkGmshReader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

  // try to open the GMSH file

  ifstream GMSHfile;
  GMSHfile.open(this->FileName);

#ifndef NDEBUG.
  this->DebugOn();
#endif

  float version=-66666;
  int isBinary=-66666;
  int isSameEndian=-66666;

  // try to read the header
  // for now we assume that the isSameEndian flag is true, otherwise we'll
  
  readGMSHheader(GMSHfile,version,isBinary,isSameEndian);

  std::map<int,vtkIdType>nodeMap;
  vtkSmartPointer<vtkPoints> outpoints= vtkSmartPointer<vtkPoints>::New();
  // try to read the point data
  readGMSHnodes(GMSHfile,outpoints,isBinary,isSameEndian,nodeMap);
  output->SetPoints(outpoints);

  // try to read the element data
  readGMSHelements(GMSHfile,output,isBinary,isSameEndian,nodeMap);

  // try to close the GMSH file
  GMSHfile.close();

  return 1;
}

void vtkGmshReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
