#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class cvvels : public vtkUnstructuredGridAlgorithm
{
public:
  static cvvels* New();
  vtkTypeRevisionMacro(cvvels,vtkUnstructuredGridAlgorithm);
 protected:

  cvvels();
  ~cvvels();

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
