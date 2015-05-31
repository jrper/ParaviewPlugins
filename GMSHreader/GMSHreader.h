#include "vtkUnstructuredGridAlgorithm.h"

class GMSHreader : public vtkUnstructuredGridAlgorithm
{
public:
  static GMSHreader *New();
  vtkTypeMacro(GMSHreader,vtkUnstructuredGridAlgorithm);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  GMSHreader();
  ~GMSHreader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  GMSHreader(const GMSHreader&);  // Not implemented.
  void operator=(const GMSHreader&);  // Not implemented.

};

