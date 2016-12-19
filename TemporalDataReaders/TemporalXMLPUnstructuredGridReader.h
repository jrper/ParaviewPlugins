#include "vtkXMLPUnstructuredGridReader.h"
#include "vtkPVConfig.h"

class TemporalXMLPUnstructuredGridReader : public vtkXMLPUnstructuredGridReader
{
public:
  static TemporalXMLPUnstructuredGridReader *New();
  vtkTypeMacro(TemporalXMLPUnstructuredGridReader,vtkXMLPUnstructuredGridReader);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  TemporalXMLPUnstructuredGridReader();
  ~TemporalXMLPUnstructuredGridReader();

  virtual int RequestInformation(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* OutputVector);

 private:
  TemporalXMLPUnstructuredGridReader(const TemporalXMLPUnstructuredGridReader&);  // Not implemented.
  void operator=(const TemporalXMLPUnstructuredGridReader&);  // Not implemented.

};

