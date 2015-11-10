#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class vtkCVVelocities : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkCVVelocities* New();
  vtkTypeRevisionMacro(vtkCVVelocities,vtkUnstructuredGridAlgorithm);
 protected:

  vtkCVVelocities();
  ~vtkCVVelocities();

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
