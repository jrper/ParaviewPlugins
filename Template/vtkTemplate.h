#include "vtkUnstructuredGridAlgorithm.h"

class vtkTemplate : public vtkUnstructuredGridAlgorithm
{
  
 public:
  static vtkTemplate *New();
  vtkTypeRevisionMacro(vtkTemplate,vtkUnstructuredGridAlgorithm);
  
 protected:

  vtkTemplate();
  ~vtkTemplate();

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
