#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"
#include "vtkPVConfig.h"

class vtkGmshReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGmshReader* New();
#if PARAVIEW_VERSION_MAJOR < 5
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
