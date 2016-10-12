#include "statreader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include <vtkVariantArray.h>
#include <vtkDoubleArray.h>
#include <vtkVariant.h>
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTable.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

#include <libxml/parser.h>
#include <libxml/tree.h>

vtkCxxRevisionMacro(statreader, "$Revision: 0.0$");
vtkStandardNewMacro(statreader);

statreader::statreader(){
  this->FileName=NULL;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
};
statreader::~statreader(){
 this->SetFileName(0);
};

int add_field(xmlAttribute * a_node,vtkTable* table)
{
    xmlAttribute *cur_node = NULL;
    std::string columnName="";
 
    for (cur_node = a_node; cur_node; cur_node = (xmlAttribute *)cur_node->next) {
        if (cur_node->type == XML_ATTRIBUTE_NODE) {
	  std::string attrName=(const char*)(cur_node->name);
	  std::string attrContent=(const char*)(cur_node->children->content);
	  if(attrName.compare("name")==0) {
	    columnName=columnName+attrContent;
	  } else if (attrName.compare("statistic")==0) {
	    columnName=columnName+"("+attrContent+")";
	  } else if (attrName.compare("material_phase")==0) {
	    columnName=attrContent+"::"+columnName;
	  }
	}
    }

     vtkSmartPointer<vtkDoubleArray> array
       = vtkSmartPointer<vtkDoubleArray>::New();
     array->SetName(columnName.c_str());
     table->AddColumn(array);
    return 1;
}

int walk_elements(xmlNode * a_node,vtkTable* table)
{
xmlNode *cur_node = NULL;
for (cur_node = a_node; cur_node; cur_node = cur_node->next) {
        if (cur_node->type == XML_ELEMENT_NODE) {
	  std::string attrName=(const char*)(cur_node->name);
	  if (attrName.compare("field")==0) {
add_field(((xmlElement*)cur_node)->attributes,table);
}
        }
walk_elements(cur_node->children,table);
    }
    return 1;
}


int readStatHeader(std::ifstream& statFile,int& isBinary,vtkTable* table)
{
  xmlDoc *doc = NULL;
  xmlNode *root_element = NULL;
  std::stringstream header;

  while(true) {
    std::string line;
    statFile>>line;
    header<<line<<std::endl;
    if (line.compare("</header>")==0) break;
  }
  
  doc = xmlParseDoc( reinterpret_cast<const xmlChar*>(header.str().c_str()));
  root_element = xmlDocGetRootElement(doc);

  walk_elements(root_element,table);

  xmlFreeDoc(doc);
  xmlCleanupParser();

  return 1;
}

int readStatData(std::ifstream& statFile,vtkTable* table) {
  while (!statFile.eof()) {
    vtkSmartPointer<vtkVariantArray> row = 
      vtkSmartPointer<vtkVariantArray>::New();
    for (int i=0;i<table->GetNumberOfColumns();i++) {
      double val;
      statFile>>val;
      row->InsertNextValue ( vtkVariant ( val ) );
    }
    if (!statFile.eof()) { table->InsertNextRow(row); }
    }
  return 1;
}

int statreader::RequestData(
		      vtkInformation* vtkNotUsed(request),
		      vtkInformationVector **inputVector,
		      vtkInformationVector* outputVector )
{
  vtkInformation* outInfo=outputVector->GetInformationObject(0);
  vtkTable* output= vtkTable::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT() ) );

  // try to open the stat file

  std::ifstream statFile(this->FileName,std::ifstream::binary);

  int isBinary=-66666;

  // try to read the header
  
  readStatHeader(statFile,isBinary,output);

  // read the data
  
  readStatData(statFile,output);

  // try to close the statfile
  statFile.close();

  return 1;
}

void statreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "File Name: "
      << (this->FileName ? this->FileName : "(none)") << "\n";
}
