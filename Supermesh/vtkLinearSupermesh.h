#include "vtkUnstructuredGridAlgorithm.h"

class vtkLinearSupermesh : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkLinearSupermesh *New();
  vtkTypeRevisionMacro(vtkLinearSupermesh,vtkUnstructuredGridAlgorithm);
  
  double Coefficients[2];

  vtkSetVector2Macro(Coefficients, double);
  vtkGetVector2Macro(Coefficients, double);

 protected:

  vtkLinearSupermesh();
  ~vtkLinearSupermesh();

  virtual int AddInputToData(vtkUnstructuredGrid *output,
			     vtkUnstructuredGrid *input,
			     vtkDataArray *ele_map,
			     int n);

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
