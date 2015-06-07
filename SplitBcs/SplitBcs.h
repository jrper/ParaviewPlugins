#include "vtkDataObjectAlgorithm.h"

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class SplitBcs : public vtkUnstructuredGridAlgorithm
{
public:
  static SplitBcs* New();
  vtkTypeRevisionMacro(SplitBcs,vtkUnstructuredGridAlgorithm);
 protected:

  SplitBcs();
  ~SplitBcs();

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
