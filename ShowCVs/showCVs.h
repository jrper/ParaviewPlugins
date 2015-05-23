#include "vtkDataObjectAlgorithm.h"

#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"

class showCVs : public vtkUnstructuredGridAlgorithm
{
public:
  static showCVs* New();
  vtkTypeRevisionMacro(showCVs,vtkUnstructuredGridAlgorithm);
 protected:

  showCVs();
  ~showCVs();

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
