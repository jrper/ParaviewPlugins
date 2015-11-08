#include "vtkUnstructuredGridAlgorithm.h"

class vtkSplitBcs : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkSplitBcs *New();
  vtkTypeRevisionMacro(vtkSplitBcs,vtkUnstructuredGridAlgorithm);

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
