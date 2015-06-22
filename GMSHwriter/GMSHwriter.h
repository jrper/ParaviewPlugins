#include "vtkDataWriter.h"
#include "vtkUnstructuredGrid.h"

class GMSHwriter : public vtkDataWriter
{
public:
  static GMSHwriter *New();
  vtkTypeMacro(GMSHwriter,vtkDataWriter);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the input to this writer.
  vtkUnstructuredGrid* GetInput();
  vtkUnstructuredGrid* GetInput(int port);

  void SetBinaryWriteMode(int isBinary);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  GMSHwriter();
  ~GMSHwriter();

  void WriteData();

  virtual int FillInputPortInformation(int,vtkInformation *info);

  char* FileName;
  int isBinary;
 private:
  GMSHwriter(const GMSHwriter&);  // Not implemented.
  void operator=(const GMSHwriter&);  // Not implemented.

};

