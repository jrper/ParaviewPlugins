#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPVConfig.h"

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

  int FillOutputPortInformation(int port, 
				vtkInformation* info);

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  FLMLreader(const FLMLreader&);  // Not implemented.
  void operator=(const FLMLreader&);  // Not implemented.

};

