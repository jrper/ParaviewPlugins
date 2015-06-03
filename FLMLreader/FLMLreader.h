#include "vtkUnstructuredGridAlgorithm.h"

class FLMLreader : public vtkUnstructuredGridAlgorithm
{
public:
  static FLMLreader *New();
  vtkTypeMacro(FLMLreader,vtkUnstructuredGridAlgorithm);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  FLMLreader();
  ~FLMLreader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  FLMLreader(const FLMLreader&);  // Not implemented.
  void operator=(const FLMLreader&);  // Not implemented.

};

