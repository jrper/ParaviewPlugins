#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"
#include "vtkVersion.h"

class vtkGmshReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGmshReader* New();
#if VTK_MAJOR_VERSION < 6
  vtkTypeRevisionMacro(vtkGmshReader,vtkUnstructuredGridAlgorithm);
#else
  vtkTypeMacro(vtkGmshReader,vtkUnstructuredGridAlgorithm);
#endif

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
