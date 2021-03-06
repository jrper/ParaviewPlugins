#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPVConfig.h"

class vtkSupermesh : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkSupermesh *New();
  vtkTypeMacro(vtkSupermesh,vtkUnstructuredGridAlgorithm);

 protected:

  vtkSupermesh();
  ~vtkSupermesh();

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
