#include "FLMLreader.h"

#include "GMSHreader.h"
#include "SplitBcs.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellType.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkCellArray.h"
#include "vtkDataArray.h"
#include "vtkIntArray.h"
#include "vtkDoubleArray.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
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
#include "vtkMath.h"

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
  this->SetNumberOfOutputPorts(2);
};
FLMLreader::~FLMLreader(){
 this->SetFileName(0);
};

int FLMLreader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
vtkInformation* outInfo0=outputVector->GetInformationObject(0);
vtkUnstructuredGrid* output0= vtkUnstructuredGrid::SafeDownCast(outInfo0->Get(vtkDataObject::DATA_OBJECT() ) );
vtkInformation* outInfo1=outputVector->GetInformationObject(1);
vtkUnstructuredGrid* output1= vtkUnstructuredGrid::SafeDownCast(outInfo1->Get(vtkDataObject::DATA_OBJECT() ) );
 outInfo1->Set(vtkUnstructuredGrid::FIELD_NAME(),"Boundary");

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

 vtkSmartPointer<SplitBcs> bcs = 
   vtkSmartPointer<SplitBcs>::New();

 bcs->SetInputConnection(gr->GetOutputPort());
 bcs->Update();

vtkDebugMacro(<<"Updated");

output0->DeepCopy(bcs->GetOutput(0));
output1->DeepCopy(bcs->GetOutput(1));

vtkDebugMacro(<<"Linked to output");

 vtkIntArray* boundaryIds=vtkIntArray::SafeDownCast(output1->GetCellData()->GetArray("PhysicalIds"));

lhs="/material_phase[";
rhs="]/";
for (int i=0;i<Spud::option_count("/material_phase");i++){
  std::stringstream a;
  a<<i;
  std::string key=lhs+a.str()+rhs;
  std::string state_name;
  Spud::get_option(key+"name",state_name);
  for (int j=0;j<Spud::option_count(key+"scalar_field");j++){
    vtkSmartPointer<vtkDoubleArray> scalar_field= vtkSmartPointer<vtkDoubleArray>::New();
    std::stringstream b;
    b<<j;
    std::string key2=key+"scalar_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    vtkDebugMacro(<<i<<" "<<j<<" "<<key2.c_str());

    std::map<int,int> id2bc;
    for (int k=0;k<Spud::option_count(key2+"prognostic/boundary_conditions");k++){
      std::stringstream c;
      c<<k;
      std::string key3=key2+"prognostic/boundary_conditions["+c.str()+"]/surface_ids";
      std::vector<int> surface_ids;
      Spud::get_option(key3,surface_ids);
      for (std::vector<int>::iterator it = surface_ids.begin(); it!=surface_ids.end(); ++it){
	id2bc.insert(std::pair<int,int>(*it,k));
vtkDebugMacro(<<"BC pair: "<< *it <<" "<< k);
      }
    }

    scalar_field->SetName((state_name+"::"+field_name).c_str());
    scalar_field->Allocate(output0->GetNumberOfPoints());
    output0->GetPointData()->AddArray(scalar_field);
    scalar_field= vtkSmartPointer<vtkDoubleArray>::New();
    scalar_field->SetName((state_name+"::"+field_name).c_str());
    scalar_field->Allocate(output1->GetNumberOfPoints());
    output1->GetPointData()->AddArray(scalar_field);
    for (int k=0;k<output1->GetNumberOfCells();k++){
      int id=boundaryIds->GetValue(k);
      std::map<int,int>::iterator it;
      it = id2bc.find(id);
vtkDebugMacro(<<"BC val: "<< id <<" "<< k<<" "<<it->first);
      if (it != id2bc.end()) {
	for (int n=0;n<output1->GetCell(k)->GetNumberOfPoints();n++){
scalar_field->SetValue(output1->GetCell(k)->GetPointId(n),0.0);
	}
      } else {
	for (int n=0;n<output1->GetCell(k)->GetNumberOfPoints();n++){
	  scalar_field->SetValue(output1->GetCell(k)->GetPointId(n),vtkMath::Nan());
	}
      }
    }
  }
  for (int j=0;j<Spud::option_count(key+"vector_field");j++){
    vtkSmartPointer<vtkDoubleArray> vector_field= vtkSmartPointer<vtkDoubleArray>::New();
    std::stringstream b;
    b<<j;
    std::string key2=key+"vector_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    vtkDebugMacro(<<i<<" "<<j<<" "<<key2.c_str());
    vector_field->SetName((state_name+"::"+field_name).c_str());
    vector_field->SetNumberOfComponents(3);
    vector_field->Allocate(output0->GetNumberOfPoints());
    output0->GetPointData()->AddArray(vector_field);
  }
  for (int j=0;j<Spud::option_count(key+"tensor_field");j++){
    vtkSmartPointer<vtkDoubleArray> tensor_field= vtkSmartPointer<vtkDoubleArray>::New();
    std::stringstream b;
    b<<j;
    std::string key2=key+"tensor_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    vtkDebugMacro(<<i<<" "<<j<<" "<<key2.c_str());
    tensor_field->SetName((state_name+"::"+field_name).c_str());
    tensor_field->SetNumberOfComponents(9);
    tensor_field->Allocate(output0->GetNumberOfPoints());
    output0->GetPointData()->AddArray(tensor_field);
  }
 }


  return 1;
}

void FLMLreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}

