#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPVConfig.h"

class vtkConsistentInterpolation : public vtkUnstructuredGridAlgorithm
{
  
 public:
  static vtkConsistentInterpolation *New();
  vtkTypeMacro(vtkConsistentInterpolation,vtkUnstructuredGridAlgorithm);

  vtkGetMacro(Radius, double);
  vtkSetMacro(Radius, double);

  
 protected:

  double Radius;

  vtkConsistentInterpolation();
  ~vtkConsistentInterpolation();

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
