#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPVConfig.h"

class vtkBoundingSurface : public vtkUnstructuredGridAlgorithm
{
  
 public:
  static vtkBoundingSurface *New();
  vtkTypeMacro(vtkBoundingSurface,vtkUnstructuredGridAlgorithm);
  
 protected:

  vtkBoundingSurface();
  ~vtkBoundingSurface();

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
