#include "detectorreader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include <vtkVariantArray.h>
#include <vtkDoubleArray.h>
#include <vtkCellArray.h>
#include <vtkVariant.h>
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkVertex.h"
#include "vtkTable.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include <limits>

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef enum {
  UNINITIALISED,
  METADATA,
  DETECTOR_POSITION,
  DETECTOR_DATA,
} col_type;

struct columnData {
  col_type isA;
  std::string name;
  std::string detector;
  std::string material_phase;
  int components;
}

vtkCxxRevisionMacro(detectorreader, "$Revision: 0.0$");
vtkStandardNewMacro(detectorreader);





detectorreader::detectorreader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
detectorreader::~detectorreader(){
 this->SetFileName(0);
};

int add_field(xmlAttribute * a_node,struct columnData& field)
{
    xmlAttribute *cur_node = NULL;
    std::string columnName="";
 
    for (cur_node = a_node; cur_node; cur_node = (xmlAttribute *)cur_node->next) {
        if (cur_node->type == XML_ATTRIBUTE_NODE) {
	  std::string attrName=(const char*)(cur_node->name);
	  std::string attrContent=(const char*)(cur_node->children->content);
	  if(attrName.compare("statistic")==0) {
	    if(attrContent.compare("value")==0) {
	      field.isA=METADATA;
	    } else if (attrContent.compare("position")==0) {
	      field.isA=DETECTOR_POSITION;
	    } else {
	      field.isA=DETECTOR_DATA;
	      field.detector=(const char*)(cur_node->children->content);
	    }
	  } else if (attrName.compare("name")==0) {
	    field.name=(const char*)(cur_node->children->content);
	  } else if (attrName.compare("material_phase")==0) {
	    field.material_phase=(const char*)(cur_node->children->content);
	  } else if (attrName.compare("components")==0) {
	    std::stringstream b((const char*)(cur_node->children->content));
	    b >> field.components;
	  }
	}
    }
    return 1;
}

int check_format(xmlAttribute * a_node,int& isBinary)
{
    xmlAttribute *cur_node = NULL;
    std::string columnName="";

    int flag=0;
 
    for (cur_node = a_node; cur_node;
	 cur_node = (xmlAttribute *)cur_node->next) {
      if (cur_node->type == XML_ATTRIBUTE_NODE) {
	  std::string attrName=(const char*)(cur_node->name);
	  std::string attrContent=(const char*)(cur_node->children->content);
	  if(attrName.compare("name")==0 &&  
	     attrContent.compare("format")!=0 ) {
	    flag=0;
	    break;
	  } else if (attrName.compare("value")==0 &&
		     attrContent.compare("binary")==0 ) {
	    flag=1;
	  }
      }
    }   
	   isBinary=isBinary || flag;
    return 1;
}

	    

int walk_elements(xmlNode * a_node,int& isBinary,
		  std::vector<struct columnData>& fields,
		  std::map<std::string,int>& detectors_lookup)
{
  xmlNode *cur_node = NULL;
  for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
	  std::string attrName=(const char*)(cur_node->name);
	  if (attrName.compare("field")==0) {
            struct columnData field={.isA=UNINITIALISED,.name="",.detector="",.material_phase="",.components=1};
	    add_field(((xmlElement*)cur_node)->attributes,field);
            fields.push_back(field);
	  } else if (attrName.compare("constant")==0) {
	    check_format(((xmlElement*)cur_node)->attributes,isBinary);
	  }
        }
	walk_elements(cur_node->children,isBinary,fields,detectors_lookup);
  }
  return 1;
}


int readDetectorHeader(std::ifstream& detectorFile,int& isBinary,std::vector<struct columnData>& fields,std::map<std::string,int>& detectors_lookup)
{
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  std::stringstream header;

  while(true) {
    std::string line;
    detectorFile>>line;
    header<<line<<std::endl;
    if (line.compare("</header>")==0) break;
  }
  
  doc = xmlParseDoc( reinterpret_cast<const xmlChar*>(header.str().c_str()));
  root_element = xmlDocGetRootElement(doc);

  walk_elements(root_element,isBinary,fields,detectors_lookup);

  int n=0;
  for(std::vector<struct columnData>::iterator it = fields.begin();
      it != fields.end(); ++it) {
    if (it->isA==DETECTOR_POSITION) {
      detectors_lookup.insert(std::pair<std::string,int>(it->name,n));
      n++;
    }
  }

  xmlFreeDoc(doc);
  xmlCleanupParser();

  return 1;
}

int readDetectorData(std::ifstream& detectorFile,
		     std::vector<struct columnData>& fields,
		     std::map<std::string,int>& detector_lookup,
		     vtkPolyData* output) {
  vtkSmartPointer<vtkPoints> points
    = vtkSmartPointer<vtkPoints>::New();

  std::string sline;

  vtkSmartPointer<vtkPointData> pd =
      vtkSmartPointer<vtkPointData>::New();
  for(std::vector<struct columnData>::iterator it = fields.begin();
	it != fields.end(); ++it) {
    switch (it->isA) {
    case DETECTOR_POSITION:
      { break;}
    case METADATA:
      { 
	vtkSmartPointer<vtkDoubleArray> data =
	  vtkSmartPointer<vtkDoubleArray>::New();
	data->SetName((it->name).c_str());
	data->SetNumberOfComponents(it->components);
	pd->AddArray(data);
	break;
      }
      case DETECTOR_DATA:
      { 
	if (pd->HasArray((it->material_phase+"::"+it->name).c_str())==0) {
	  vtkSmartPointer<vtkDoubleArray> data =
	    vtkSmartPointer<vtkDoubleArray>::New();
	  data->SetName((it->material_phase+"::"+it->name).c_str());
	  data->SetNumberOfComponents(it->components);
	  pd->AddArray(data);
	  std::cout << (it->material_phase+"::"+it->name);
	}
	break;
      }
	  
    }
  }

  points->Allocate(0);
  int i=0;
  while (getline(detectorFile,sline)) {
    if (sline == "") continue;
    std::stringstream line(sline);
    std::cout << "loop1 " << line.str()<< std::endl;
    for(std::vector<struct columnData>::iterator it = fields.begin();
	it != fields.end(); ++it) {
     std::cout << "loop2" << std::endl;
      switch (it->isA) {
      case DETECTOR_POSITION:
	{
	  double x[3]={0.0,0.0,0.0};
	  for (int j=0;j<it->components;++j) {
	    line>>x[j];
	    std::cout<<"x_"<<j+1<<":"<< x[j] <<std::endl;
	  }
	  points->InsertNextPoint(x);
	  break;
	}
      case DETECTOR_DATA:
	{
	  double val[it->components];
	  for (int j=0;j<it->components;++j) {
	    std::string s;
	    line>>s;
	    if (s.compare("NaN")==0) {
	      val[j]=0.0;
	      //   val[j]=std::numeric_limits<double>::quiet_NaN();
	    } else {
	      std::stringstream ss(s);
	      ss>>val[j];
	    }
	    std::cout<<"data:"<<s<<std::endl;
	  }
	  pd->GetArray((it->material_phase+"::"+it->name).c_str())->InsertTuple(detector_lookup[it->detector]+i*detector_lookup.size(),val);
	  break;
	}
      case METADATA:
	{
	  double mval[it->components];
	  for (int j=0;j<it->components;++j) {
	    line>>mval[j];
	    std::cout<< "metadata "<<mval[j]<<std::endl;
	  }
	  for (int k=0;k<detector_lookup.size();++k) {
	    pd->GetArray(it->name.c_str())->InsertNextTuple(mval);
	  }
	  break;
	}
      }
//      row->InsertNextValue ( vtkVariant ( val ) );
    }
    ++i;
  }
  output->SetPoints(points);
  output->GetPointData()->DeepCopy(pd);

  vtkSmartPointer<vtkCellArray> cellarray =
      vtkSmartPointer<vtkCellArray>::New();
  if (output->GetNumberOfPoints()>detector_lookup.size()) {
    for (int i=0;i<detector_lookup.size();++i) {
      vtkSmartPointer<vtkPolyLine> line =
	vtkSmartPointer<vtkPolyLine>::New();
      for (int j=i;j<output->GetNumberOfPoints();j=j+detector_lookup.size()) {
	line->GetPointIds()->InsertNextId(j);
      }
      cellarray->InsertNextCell(line);
    }
    output->SetLines(cellarray);
  } else {
    for (int i=0;i<detector_lookup.size();++i) {
      vtkSmartPointer<vtkVertex> p =
	vtkSmartPointer<vtkVertex>::New();
      p->GetPointIds()->InsertNextId(i);
      cellarray->InsertNextCell(p);
    }
    output->SetVerts(cellarray);
  }
 
  
  return 1;
}


int readBinaryDetectorData(std::ifstream& detectorFile,
		     std::vector<struct columnData>& fields,
		     std::map<std::string,int>& detector_lookup,
		     vtkPolyData* output) {
  vtkSmartPointer<vtkPoints> points
    = vtkSmartPointer<vtkPoints>::New();
  points->Allocate(0);

  vtkSmartPointer<vtkPointData> pd =
      vtkSmartPointer<vtkPointData>::New();
  for(std::vector<struct columnData>::iterator it = fields.begin();
	it != fields.end(); ++it) {
    switch (it->isA) {
    case DETECTOR_POSITION:
      { break;}
    case METADATA:
      { 
	vtkSmartPointer<vtkDoubleArray> data =
	  vtkSmartPointer<vtkDoubleArray>::New();
	data->SetName((it->name).c_str());
	data->SetNumberOfComponents(it->components);
	pd->AddArray(data);
	break;
      }
      case DETECTOR_DATA:
      { 
	if (pd->HasArray((it->material_phase+"::"+it->name).c_str())==0) {
	  vtkSmartPointer<vtkDoubleArray> data =
	    vtkSmartPointer<vtkDoubleArray>::New();
	  data->SetName((it->material_phase+"::"+it->name).c_str());
	  data->SetNumberOfComponents(it->components);
	  pd->AddArray(data);
	  std::cout << (it->material_phase+"::"+it->name);
	}
	break;
      }
	  
    }
  }

  int i=0;
  while (!detectorFile.eof()) {
    detectorFile>>std::ws;
    if (detectorFile.eof()) break;
    for(std::vector<struct columnData>::iterator it = fields.begin();
	it != fields.end(); ++it) {
      switch (it->isA) {
      case DETECTOR_POSITION:
	{
	  double x[3]={0.0,0.0,0.0};
	  for (int j=0;j<it->components;++j) {
	    detectorFile.read((char*)&(x[j]),sizeof(double));
	  }
	  points->InsertNextPoint(x);
	  break;
	}
      case DETECTOR_DATA:
	{
	  double val[it->components];
	  for (int j=0;j<it->components;++j) {
	    detectorFile.read((char*)&(val[j]),sizeof(double));
	  }
	  pd->GetArray((it->material_phase+"::"+it->name).c_str())->InsertTuple(detector_lookup[it->detector]+i*detector_lookup.size(),val);
	  break;
	}
      case METADATA:
	{
	  double mval[it->components];
	  for (int j=0;j<it->components;++j) {
	    detectorFile.read((char*)&(mval[j]),sizeof(double));
	  }
	  for (int k=0;k<detector_lookup.size();++k) {
	    pd->GetArray(it->name.c_str())->InsertNextTuple(mval);
	  }
	  break;
	}
      }
//      row->InsertNextValue ( vtkVariant ( val ) );
    }
    ++i;
  }
  output->SetPoints(points);
  output->GetPointData()->DeepCopy(pd);

  vtkSmartPointer<vtkCellArray> cellarray =
      vtkSmartPointer<vtkCellArray>::New();
  if (output->GetNumberOfPoints()>detector_lookup.size()) {
    for (int i=0;i<detector_lookup.size();++i) {
      vtkSmartPointer<vtkPolyLine> line =
	vtkSmartPointer<vtkPolyLine>::New();
      for (int j=i;j<output->GetNumberOfPoints();j=j+detector_lookup.size()) {
	line->GetPointIds()->InsertNextId(j);
      }
      cellarray->InsertNextCell(line);
    }
    output->SetLines(cellarray);
  } else {
    for (int i=0;i<detector_lookup.size();++i) {
      vtkSmartPointer<vtkVertex> p =
	vtkSmartPointer<vtkVertex>::New();
      p->GetPointIds()->InsertNextId(i);
      cellarray->InsertNextCell(p);
    }
    output->SetVerts(cellarray);
  }
 
  
  return 1;
}

int detectorreader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkPolyData* output= vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

  // try to open the detector file

  std::ifstream detectorFile(this->FileName,std::ifstream::binary);

  int isBinary=0;

  // try to read the header
  
  std::vector<struct columnData> fields;
  std::map<std::string,int> detectors_lookup;
  readDetectorHeader(detectorFile,isBinary,fields,detectors_lookup);

  this->DebugOn();
  vtkDebugMacro(<<detectors_lookup.size());

  // read the data
  
  if (isBinary==1) {
    std::string fname=this->FileName;
    std::ifstream detectorDatFile((fname+".dat").c_str(),
				  std::ifstream::binary);
    readBinaryDetectorData(detectorDatFile,fields,detectors_lookup,output);
    detectorDatFile.close();
  } else {
    readDetectorData(detectorFile,fields,detectors_lookup,output);
  }

  // try to close the detectorfile
  detectorFile.close();

  return 1;
}

void detectorreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
