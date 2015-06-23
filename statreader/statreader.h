#include "vtkTableReader.h"

class statreader : public vtkTableReader
{
public:
  static statreader *New();
  vtkTypeMacro(statreader,vtkTableReader);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  statreader();
  ~statreader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  statreader(const statreader&);  // Not implemented.
  void operator=(const statreader&);  // Not implemented.

};

