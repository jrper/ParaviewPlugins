#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class vtkGmshReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGmshReader* New();
  vtkTypeRevisionMacro(vtkGmshReader,vtkUnstructuredGridAlgorithm);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  vtkGmshReader();
  ~vtkGmshReader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;

 private:
  vtkGmshReader(const vtkGmshReader&);  // Not implemented.
  void operator=(const vtkGmshReader&);  // Not implemented.

};
