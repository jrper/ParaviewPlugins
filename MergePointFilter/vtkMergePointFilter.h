#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkMergePoints.h"
#include "vtkCell.h"
#include "vtkPoints.h"
#include "vtkDataArray.h"
#include "vtkIdList.h"
#include "vtkPVConfig.h"

class vtkMergePointFilter : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkMergePointFilter *New();
#if PARAVIEW_VERSION_MAJOR < 5
  vtkTypeRevisionMacro(vtkMergePointFilter,vtkUnstructuredGridAlgorithm);
#else
  vtkTypeMacro(vtkMergePointFilter,vtkUnstructuredGridAlgorithm);
#endif

  int Degree, Continuity;

  vtkSetMacro(Degree, int);
  vtkGetMacro(Degree, int);
  vtkSetMacro(Continuity, int);
  vtkGetMacro(Continuity, int);

 protected:

  vtkMergePointFilter();
  ~vtkMergePointFilter();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);
  virtual int FillInputPortInformation(int port,vtkInformation *info);

  void AddQuadraticData(vtkDataArray* output_data,vtkDataArray* input_data,vtkCell* input_cell,vtkCell* output_cell);

  void AddQuadraticPoints(vtkMergePoints* mergePoints, vtkPoints* input_points, vtkCell* cell,vtkIdList* cellIds, int input_continuity); 

};
