#include "vtkUnstructuredGridAlgorithm.h"

class vtkAustraliaFilter : public vtkUnstructuredGridAlgorithm
{
  
 public:
  static vtkAustraliaFilter *New();
  vtkTypeRevisionMacro(vtkAustraliaFilter,vtkUnstructuredGridAlgorithm);
  
 protected:

  vtkAustraliaFilter();
  ~vtkAustraliaFilter();

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
