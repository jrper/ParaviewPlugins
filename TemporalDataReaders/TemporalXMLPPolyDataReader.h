#include "vtkXMLPPolyDataReader.h"
#include "vtkPVConfig.h"

class TemporalXMLPPolyDataReader : public vtkXMLPPolyDataReader
{
public:
  static TemporalXMLPPolyDataReader *New();
  vtkTypeMacro(TemporalXMLPPolyDataReader,vtkXMLPPolyDataReader);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  TemporalXMLPPolyDataReader();
  ~TemporalXMLPPolyDataReader();

  virtual int RequestInformation(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* OutputVector);

 private:
  TemporalXMLPPolyDataReader(const TemporalXMLPPolyDataReader&);  // Not implemented.
  void operator=(const TemporalXMLPPolyDataReader&);  // Not implemented.

};

