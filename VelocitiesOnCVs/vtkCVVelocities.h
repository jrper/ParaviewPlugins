#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkSetGet.h"
#include "vtkVersion.h"

class vtkCVVelocities : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkCVVelocities* New();
#if VTK_MAJOR_VERSION < 6
  vtkTypeRevisionMacro(vtkCVVelocities,vtkUnstructuredGridAlgorithm);
#else
  vtkTypeMacro(vtkCVVelocities,vtkUnstructuredGridAlgorithm);
#endif
  int Degree, Continuity, inputContinuity;

  vtkSetMacro(Degree, int);
  vtkGetMacro(Degree, int);
  vtkSetMacro(Continuity, int);
  vtkGetMacro(Continuity, int);

 protected:

  vtkCVVelocities();
  ~vtkCVVelocities();

  int isContinuous();

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
