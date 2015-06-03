#include "FLMLreader.h"

#include "GMSHreader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
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
#include <fstream>
#include <map>
#include <cstdlib>

#include <sstream>
#include <spud>


vtkCxxRevisionMacro(FLMLreader, "$Revision: 0.0$");
vtkStandardNewMacro(FLMLreader);

FLMLreader::FLMLreader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
FLMLreader::~FLMLreader(){
 this->SetFileName(0);
};

int FLMLreader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkUnstructuredGrid* output= vtkUnstructuredGrid::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

this->DebugOn();

  // try to open the FLML file

Spud::load_options(this->FileName);

// get the filename of a mesh


std::string fname;
std::string lhs ("/geometry/mesh[");
std::string rhs ("]/from_file");
for (int i=0;i<Spud::option_count("/geometry/mesh");i++){
std::stringstream a;
a<<i;
std::string key=lhs+a.str()+rhs;    
if ( Spud::have_option(key)) {
Spud::get_option(key+"/file_name",fname);
break;
}
}

fname=fname+(".msh");

vtkDebugMacro(<<fname);

vtkSmartPointer<GMSHreader> gr = 
    vtkSmartPointer<GMSHreader>::New();

gr->SetFileName(fname.c_str());

vtkDebugMacro(<<"File name set");

gr->Update();

vtkDebugMacro(<<"Updated");

output->DeepCopy(gr->GetOutput());

vtkDebugMacro(<<"Linked to output");

  return 1;
}

void FLMLreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
