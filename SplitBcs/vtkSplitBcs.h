#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkVersion.h"

class vtkSplitBcs : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkSplitBcs *New();
#if VTK_MAJOR_VERSION < 5
  vtkTypeRevisionMacro(vtkSplitBcs,vtkUnstructuredGridAlgorithm);
#else
  vtkTypeMacro(vtkSplitBcs,vtkUnstructuredGridAlgorithm);
#endif

 protected:

  vtkSplitBcs();
  ~vtkSplitBcs();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);
  virtual int RequestUpdateExtent(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);
  virtual int FillInputPortInformation(int port,vtkInformation *info);




};
