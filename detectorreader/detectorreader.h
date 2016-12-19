#include "vtkPolyDataAlgorithm.h"
#include "vtkPVConfig.h"

class detectorreader : public vtkPolyDataAlgorithm
{
public:
  static detectorreader *New();
  vtkTypeMacro(detectorreader,vtkPolyDataAlgorithm);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  detectorreader();
  ~detectorreader();

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  detectorreader(const detectorreader&);  // Not implemented.
  void operator=(const detectorreader&);  // Not implemented.

};

