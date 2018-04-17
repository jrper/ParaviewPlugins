#include "vtkFiltersHybridModule.h" // For export macro
#include "vtkTemporalInterpolator.h"
#include "vtkMultiTimeStepAlgorithm.h"

class vtkDataSet;
class VTKFILTERSHYBRID_EXPORT vtkSpatialTemporalInterpolation : public vtkTemporalInterpolator
{
  
 public:
  static vtkSpatialTemporalInterpolation *New();
    vtkTypeMacro(vtkSpatialTemporalInterpolation,vtkTemporalInterpolator);
  
 protected:

  vtkSpatialTemporalInterpolation();
  ~vtkSpatialTemporalInterpolation();

  vtkDataSet *InterpolateDataSet(vtkDataSet *in1, vtkDataSet *in2, double ratio);

};
