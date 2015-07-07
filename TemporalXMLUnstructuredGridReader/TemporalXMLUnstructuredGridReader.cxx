#include "TemporalXMLUnstructuredGridReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataParser.h"

vtkCxxRevisionMacro(TemporalXMLUnstructuredGridReader, "$Revision: 0.0$");
vtkStandardNewMacro(TemporalXMLUnstructuredGridReader);

TemporalXMLUnstructuredGridReader::TemporalXMLUnstructuredGridReader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
TemporalXMLUnstructuredGridReader::~TemporalXMLUnstructuredGridReader(){
 this->SetFileName(0);
};

int TemporalXMLUnstructuredGridReader::RequestInformation(
		      vtkInformation* request,
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  //  this->Update();
  {
    this->Superclass::RequestInformation(
		      request,
		      inputVector,
		      outputVector );



 double time;
 int ntimes;
 vtkXMLDataElement* eVTKFile=this->GetXMLParser()->GetRootElement();
 const char* name = this->GetDataSetName();
  int i;
  vtkXMLDataElement* ePrimary = 0;
  for(i=0; i < eVTKFile->GetNumberOfNestedElements(); ++i)
    {
    vtkXMLDataElement* eNested = eVTKFile->GetNestedElement(i);
    if(strcmp(eNested->GetName(), name) == 0)
      {
      ePrimary = eNested;
      break;
      }
    }
  ntimes=ePrimary->GetScalarAttribute("Time",time);
  if (ntimes==0) {
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    reader->SetFileName(this->FileName);
    reader->Update();
    time=reader->GetOutput()->GetPointData()->GetArray("Time")->GetRange()[0];
  }
 outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),&time,1);
  }

  return 1;
}

void TemporalXMLUnstructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
