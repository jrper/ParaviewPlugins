#include "FLMLreader.h"

#include "vtkGmshReader.h"
#include "vtkSplitBcs.h"

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
#include "vtkMultiBlockDataSet.h"
#include "vtkThreshold.h"

#include <iostream>
#include <fstream>
#include <map>
#include <set>
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
vtkMultiBlockDataSet* output1= vtkMultiBlockDataSet::SafeDownCast(outInfo1->Get(vtkDataObject::DATA_OBJECT() ) );
 outInfo1->Set(vtkMultiBlockDataSet::FIELD_NAME(),"Boundary");

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

vtkGmshReader* gr =  vtkGmshReader::New();

gr->SetFileName(fname.c_str());

vtkDebugMacro(<<"File name set");

gr->Update();

 vtkSplitBcs* bcs = vtkSplitBcs::New();

 bcs->SetInputConnection(gr->GetOutputPort());
 bcs->Update();

 vtkDebugMacro(<<"Updated");

 output0->DeepCopy(bcs->GetOutput(0));
 vtkUnstructuredGrid* ug = 
   vtkUnstructuredGrid::New();
 ug->DeepCopy(bcs->GetOutput(1)); 
 bcs->Delete();
 gr->Delete();
 std::set<int> surface_ids;
 vtkIntArray* boundaryIds= vtkIntArray::SafeDownCast(ug->GetCellData()->GetArray("PhysicalIds"));
 for (vtkIdType i=0;i<boundaryIds->GetNumberOfTuples();++i) {
   surface_ids.insert(boundaryIds->GetTuple1(i));
 }
 for (std::set<int>::iterator it=surface_ids.begin();
      it!=surface_ids.end();++it) {
   vtkSmartPointer<vtkThreshold> thr = 
     vtkSmartPointer<vtkThreshold>::New();
   thr->SetInputData(ug);
   thr->ThresholdBetween(*it,*it);
   thr->SetInputArrayToProcess(0, 0, 0,
			       vtkDataObject::FIELD_ASSOCIATION_CELLS, "PhysicalIds");
   thr->Update();
   vtkUnstructuredGrid* part_ug = vtkUnstructuredGrid::New();
   part_ug->ShallowCopy(thr->GetOutput());
   if (part_ug->GetNumberOfPoints()>0) {
     output1->SetBlock(output1->GetNumberOfBlocks(),part_ug);
     std::stringstream bname;
     bname<<"Surface Id "<<output1->GetNumberOfBlocks();
     output1->GetMetaData(output1->GetNumberOfBlocks()-1)
       ->Set(vtkMultiBlockDataSet::NAME(),bname.str().c_str());
   }
   part_ug->Delete();
 }
 ug->Delete();

 vtkDebugMacro(<<"Linked to output");

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
    scalar_field->SetNumberOfComponents(1);
    std::stringstream b;
    b<<j;
    std::string key2=key+"scalar_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    vtkDebugMacro(<<i<<" "<<j<<" "<<key2.c_str());

    std::map<int,double> id2bc;
    
    for (int k=0;k<Spud::option_count(key2+"prognostic/boundary_conditions");k++){
      std::stringstream c;
      c<<k;
      std::string key3=key2+"prognostic/boundary_conditions["+c.str()+"]/surface_ids";
      std::vector<int> surface_ids;
      Spud::get_option(key3,surface_ids);
      std::string key4=key2+"prognostic/boundary_conditions["+c.str()+"]/type::dirichlet/constant";
      double val;
      Spud::get_option(key4,val);
      for (std::vector<int>::iterator it = surface_ids.begin(); it!=surface_ids.end(); ++it){
	id2bc.insert(std::pair<int,double>(*it,val));
      }
    }

    scalar_field->SetName((state_name+"::"+field_name).c_str());
    scalar_field->Allocate(output0->GetNumberOfPoints());
    for (int n=0;n<output0->GetNumberOfPoints();n++){
      scalar_field->SetValue(n,vtkMath::Nan());
    }
    output0->GetPointData()->AddArray(scalar_field);

    vtkDebugMacro(<<"BCs");
   
    if (!id2bc.empty()) {
      for (int k=0;k<output1->GetNumberOfBlocks();k++){
	vtkUnstructuredGrid* ug_part= vtkUnstructuredGrid::SafeDownCast(output1->GetBlock(k));
	int id=ug_part->GetCellData()->GetArray("PhysicalIds")->GetTuple1(0);
	std::map<int,double>::iterator it = id2bc.find(id);
	vtkSmartPointer<vtkDoubleArray> bscalar_field= vtkSmartPointer<vtkDoubleArray>::New();
	bscalar_field->SetName((state_name+"::"+field_name+"(Dirichlet)").c_str());
	bscalar_field->SetNumberOfComponents(1);
	bscalar_field->Allocate(ug_part->GetNumberOfPoints());
	if (it != id2bc.end()) {
	  for (int n=0;n<ug_part->GetNumberOfPoints();++n){
	    bscalar_field->InsertValue(n,it->second);
	  }
	} else {
	  for (int n=0;n<ug_part->GetNumberOfPoints();++n){
	    bscalar_field->InsertValue(n,vtkMath::Nan());
	  }
	}
	ug_part->GetPointData()->AddArray(bscalar_field);
      }
    }
  }

       vtkDebugMacro(<<"Vector fields")

  for (int j=0;j<Spud::option_count(key+"vector_field");j++){
    vtkSmartPointer<vtkDoubleArray> vector_field= vtkSmartPointer<vtkDoubleArray>::New();
    std::stringstream b;
    b<<j;
    std::string key2=key+"vector_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    vector_field->SetName((state_name+"::"+field_name).c_str());
    vector_field->SetNumberOfComponents(3);
    vector_field->Allocate(output0->GetNumberOfPoints());
    vtkDebugMacro(<<output0->GetNumberOfPoints());
    output0->GetPointData()->AddArray(vector_field);
    std::vector<double> val;
    Spud::get_option(key2+"prognostic/initial_condition/constant",val);
    for (int n=0;n<output0->GetNumberOfPoints();++n){
      vector_field->InsertTuple3(n,val[0],val[1],val[2]);
    }
  }
       
       vtkDebugMacro(<<"Tensor fields")

  for (int j=0;j<Spud::option_count(key+"tensor_field");j++){
    vtkSmartPointer<vtkDoubleArray> tensor_field= vtkSmartPointer<vtkDoubleArray>::New();
    std::stringstream b;
    b<<j;
    std::string key2=key+"tensor_field["+b.str()+rhs;
    std::string field_name;
    Spud::get_option(key2+"name",field_name);
    tensor_field->SetName((state_name+"::"+field_name).c_str());
    tensor_field->SetNumberOfComponents(9);
    tensor_field->Allocate(output0->GetNumberOfPoints());
    output0->GetPointData()->AddArray(tensor_field);
  }
 }

 vtkDebugMacro(<<"All done!")

  return 1;
}

int FLMLreader::FillOutputPortInformation(
  int port, vtkInformation* info)
{
  // now add our info
  if (port==0) {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkUnstructuredGrid");
  } else {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  }
  return 1;
}

void FLMLreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}

