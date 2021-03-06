#include "TemporalXMLPUnstructuredGridReader.h"
#include "vtkPVConfig.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLUnstructuredGridReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataReader.h"
#include "vtkXMLDataParser.h"

#include <iostream>

#if PARAVIEW_VERSION_MAJOR < 5
vtkCxxRevisionMacro(TemporalXMLPUnstructuredGridReader, "$Revision: 0.0$");
#endif
vtkStandardNewMacro(TemporalXMLPUnstructuredGridReader);

TemporalXMLPUnstructuredGridReader::TemporalXMLPUnstructuredGridReader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
TemporalXMLPUnstructuredGridReader::~TemporalXMLPUnstructuredGridReader(){
 this->SetFileName(0);
};

int TemporalXMLPUnstructuredGridReader::RequestInformation(
		      vtkInformation* request,
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  {
    this->Superclass::RequestInformation(
		      request,
		      inputVector,
		      outputVector );


    double time;
    int ntimes;
;
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
    for(i=0; i < ePrimary->GetNumberOfNestedElements(); ++i)
      {
	vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
	if(strcmp(eNested->GetName(), "Piece") == 0)
	  {
	    ePrimary = eNested;
	    break;
	  }
      }
    const char* fileName = ePrimary->GetAttribute("Source");
    vtkSmartPointer<vtkXMLUnstructuredGridReader> reader = vtkXMLUnstructuredGridReader::New();
    reader->SetFileName(fileName);
    reader->UpdateInformation();
    eVTKFile=reader->GetXMLParser()->GetRootElement();
    for(i=0; i < eVTKFile->GetNumberOfNestedElements(); ++i)
      {
    vtkXMLDataElement* eNested = eVTKFile->GetNestedElement(i);
    if(strcmp(eNested->GetName(), "UnstructuredGrid") == 0)
      {
      ePrimary = eNested;
      break;
      }
    }
  ntimes=ePrimary->GetScalarAttribute("Time",time);
  if (ntimes==0) {
    for(i=0; i < ePrimary->GetNumberOfNestedElements(); ++i) {
      vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
      if(strcmp(eNested->GetName(), "Piece") == 0) {
	ePrimary = eNested;
	break;
      }
    }
    for(i=0; i < ePrimary->GetNumberOfNestedElements(); ++i) {
      vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
      if(strcmp(eNested->GetName(), "PointData") == 0) {
	ePrimary = eNested;
	break;
      }
    }
    for(i=0; i < ePrimary->GetNumberOfNestedElements(); ++i) {
      vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
      const char* name=eNested->GetAttribute("Name");
      if(strcmp(eNested->GetName(), "DataArray") == 0
	   && (strcmp(name,"Time") == 0 
		 || (strlen(name)>=6
		       && strcmp(&name[strlen(name)-6],"::Time") == 0))){
	ePrimary = eNested;
	break;
      } 
    }
    ntimes=ePrimary->GetScalarAttribute("RangeMax",time);
  }
if (ntimes>0)  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),&time,1);
  }

  return 1;
}

void TemporalXMLPUnstructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
