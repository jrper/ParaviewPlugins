#include "vtkXMLUnstructuredGridReader.h"
#include "vtkPVConfig.h"

class TemporalXMLUnstructuredGridReader : public vtkXMLUnstructuredGridReader
{
public:
  static TemporalXMLUnstructuredGridReader *New();
  vtkTypeMacro(TemporalXMLUnstructuredGridReader,vtkXMLUnstructuredGridReader);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  TemporalXMLUnstructuredGridReader();
  ~TemporalXMLUnstructuredGridReader();

  virtual int RequestInformation(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* OutputVector);

 private:
  TemporalXMLUnstructuredGridReader(const TemporalXMLUnstructuredGridReader&);  // Not implemented.
  void operator=(const TemporalXMLUnstructuredGridReader&);  // Not implemented.

};

