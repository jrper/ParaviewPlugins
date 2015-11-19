#include "vtkUnstructuredGridAlgorithm.h"

class vtkMergePointFilter : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMergePointFilter *New();
  vtkTypeRevisionMacro(vtkMergePointFilter,vtkUnstructuredGridAlgorithm);

 protected:

  vtkMergePointFilter();
  ~vtkMergePointFilter();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);
  virtual int FillInputPortInformation(int port,vtkInformation *info);

};
