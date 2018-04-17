#include "vtkSpatialTemporalInterpolation.h"

#include "vtkObjectFactory.h"
#include "vtkPointSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPoints.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkPVConfig.h"
#include "vtkSmartPointer.h"
#include "vtkConsistentInterpolation.h"

//vtkCxxRevisionMacro(vtkSpatialTemporalInterpolation, "$Revision: 0.0$");
vtkStandardNewMacro(vtkSpatialTemporalInterpolation);

vtkSpatialTemporalInterpolation::vtkSpatialTemporalInterpolation(){
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
};
vtkSpatialTemporalInterpolation::~vtkSpatialTemporalInterpolation(){};

vtkDataSet *vtkSpatialTemporalInterpolation::InterpolateDataSet(vtkDataSet *in1, vtkDataSet *in2, double ratio)
{
  vtkDataSet *input[2];
  input[0] = in1;
  input[1] = in2;

  //
  vtkDataSet *output = input[0]->NewInstance();
  output->CopyStructure(input[0]);
  //
  // Interpolate points if the dataset is a vtkUnstructuredGrid
  //
  vtkUnstructuredGrid *inUGrid1 = vtkUnstructuredGrid::SafeDownCast(input[0]);
  vtkUnstructuredGrid *inUGrid2 = vtkUnstructuredGrid::SafeDownCast(input[1]);
  vtkPointSet *outPointSet = vtkPointSet::SafeDownCast(output);
  vtkConsistentInterpolation *interpolator;
  if (inUGrid1 && inUGrid2)
    {
    vtkDataArray *outarray = NULL;
    vtkPoints *outpoints;

    interpolator = vtkConsistentInterpolation::New();

    interpolator->SetInputData(0,inUGrid1);
    interpolator->SetInputData(1,inUGrid2);
    interpolator->SetRadius(1.0e-8);
    interpolator->Update();
    inUGrid1=interpolator->GetOutput();
    input[0]=interpolator->GetOutput();

    if (inUGrid1->GetNumberOfPoints()>0 && inUGrid2->GetNumberOfPoints()>0)
    {
      vtkDataArray *arrays[2];
      arrays[0] = inUGrid1->GetPoints()->GetData();
      arrays[1] = inUGrid2->GetPoints()->GetData();

      // allocate double for output if input is double - otherwise float
      // do a quick check to see if all arrays have the same number of tuples
      if (!this->VerifyArrays(arrays, 2))
      {
        vtkWarningMacro
          ("Interpolation aborted for points because the number of "
           "tuples/components in each time step are different");
      }
      outarray = this->InterpolateDataArray(
          ratio, arrays,arrays[0]->GetNumberOfTuples());
      // Do not shallow copy points from either input, because otherwise when
      // we set the actual point coordinate data we overwrite the original
      // we must instantiate a new points object
      // (ie we override the copystrucure above)
      vtkPoints *inpoints = inUGrid1->GetPoints();
      outpoints = inpoints->NewInstance();
      outPointSet->SetPoints(outpoints);

    }
    else
    {
      // not much we can do really
      outpoints = vtkPoints::New();
      outPointSet->SetPoints(outpoints);
    }

    outpoints->SetDataTypeToDouble();
    outpoints->SetNumberOfPoints(inUGrid1->GetNumberOfPoints());
    outpoints->SetData(inUGrid2->GetPoints()->GetData());
    outpoints->Delete();
    if (outarray)
    {
      outarray->Delete();
    }
  }
  //
  // Interpolate pointdata if present
  //
  output->GetPointData()->ShallowCopy(input[0]->GetPointData());
  for (int s=0; s < input[0]->GetPointData()->GetNumberOfArrays(); ++s)
  {
    std::vector<vtkDataArray*> arrays;
    char *scalarname = NULL;
    for (int i=0; i<2; ++i)
    {
      //
      // On some data, the scalar arrays are consistent but ordered
      // differently on each time step, so we will fetch them by name if
      // possible.
      //
      if (i==0 || (scalarname==NULL))
      {
        vtkDataArray *dataarray = input[i]->GetPointData()->GetArray(s);
        scalarname = dataarray->GetName();
        arrays.push_back(dataarray);
      }
      else
      {
        vtkDataArray *dataarray =
          input[i]->GetPointData()->GetArray(scalarname);
        arrays.push_back(dataarray);
      }
    }
    if (arrays[1])
    {
      // do a quick check to see if all arrays have the same number of tuples
      if (!this->VerifyArrays(&arrays[0], 2))
      {
        vtkWarningMacro(<<"Interpolation aborted for array "
                        << (scalarname ? scalarname : "(unnamed array)")
                        << " because the number of tuples/components"
                        << " in each time step are different");
      }
      else
      {
        // allocate double for output if input is double - otherwise float
        vtkDataArray *outarray =
          this->InterpolateDataArray(ratio, &arrays[0],
                                     arrays[0]->GetNumberOfTuples());
        output->GetPointData()->AddArray(outarray);
        outarray->Delete();
      }
    }
    else
    {
      vtkDebugMacro(<<"Interpolation aborted for point array "
                    << (scalarname ? scalarname : "(unnamed array)")
                    << " because the array was not found"
                    << " in the second time step");
    }
  }
  //
  // Interpolate celldata if present
  //
  output->GetCellData()->ShallowCopy(input[0]->GetCellData());
  for (int s=0; s<input[0]->GetCellData()->GetNumberOfArrays(); ++s)
  {
    // copy the structure
    std::vector<vtkDataArray*> arrays;
    char *scalarname = NULL;
    for (int i=0; i<2; ++i)
    {
      //
      // On some data, the scalar arrays are consistent but ordered
      // differently on each time step, so we will fetch them by name if
      // possible.
      //
      if (i==0 || (scalarname==NULL))
      {
        vtkDataArray *dataarray = input[i]->GetCellData()->GetArray(s);
        scalarname = dataarray->GetName();
        arrays.push_back(dataarray);
      }
      else
      {
        vtkDataArray *dataarray =
          input[i]->GetCellData()->GetArray(scalarname);
        arrays.push_back(dataarray);
      }
    }
    if (arrays[1])
    {
      // do a quick check to see if all arrays have the same number of tuples
      if (!this->VerifyArrays(&arrays[0], 2))
      {
        vtkWarningMacro(<<"Interpolation aborted for array "
                        << (scalarname ? scalarname : "(unnamed array)")
                        << " because the number of tuples/components"
                        << " in each time step are different");
      }
      // allocate double for output if input is double - otherwise float
      vtkDataArray *outarray =
        this->InterpolateDataArray(ratio, &arrays[0],
                                   arrays[0]->GetNumberOfTuples());
      output->GetCellData()->AddArray(outarray);
      outarray->Delete();
      interpolator->Delete();
    }
    else
    {
      vtkDebugMacro(<<"Interpolation aborted for cell array "
                    << (scalarname ? scalarname : "(unnamed array)")
                    << " because the array was not found"
                    << " in the second time step");
    }

  }
  return output;
}
