#include "vtkXMLPolyDataReader.h"
#include "vtkPVConfig.h"

class TemporalXMLPolyDataReader : public vtkXMLPolyDataReader
{
public:
  static TemporalXMLPolyDataReader *New();
  vtkTypeMacro(TemporalXMLPolyDataReader,vtkXMLPolyDataReader);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  TemporalXMLPolyDataReader();
  ~TemporalXMLPolyDataReader();

  virtual int RequestInformation(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* OutputVector);

 private:
  TemporalXMLPolyDataReader(const TemporalXMLPolyDataReader&);  // Not implemented.
  void operator=(const TemporalXMLPolyDataReader&);  // Not implemented.

};

