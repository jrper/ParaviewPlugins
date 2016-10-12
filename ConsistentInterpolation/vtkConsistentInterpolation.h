#include "vtkUnstructuredGridAlgorithm.h"

class vtkConsistentInterpolation : public vtkUnstructuredGridAlgorithm
{
  
 public:
  static vtkConsistentInterpolation *New();
  vtkTypeRevisionMacro(vtkConsistentInterpolation,vtkUnstructuredGridAlgorithm);

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
