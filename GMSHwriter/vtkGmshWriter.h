#include "vtkDataWriter.h"
#include "vtkUnstructuredGrid.h"
#include "vtkPVConfig.h"

class vtkGmshWriter : public vtkDataWriter
{
public:
  static vtkGmshWriter *New();
#if PARAVIEW_VERSION_MAJOR < 5
  vtkTypeRevisionMacro(vtkGmshWriter,vtkDataWriter);
#else
  vtkTypeMacro(vtkGmshWriter,vtkDataWriter);
#endif

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the input to this writer.
  vtkUnstructuredGrid* GetInput();
  vtkUnstructuredGrid* GetInput(int port);

  void SetBinaryWriteMode(int isBinary);

  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  vtkGmshWriter();
  ~vtkGmshWriter();

  void WriteData();

  virtual int FillInputPortInformation(int,vtkInformation *info);

  char* FileName;
  int isBinary;
 private:
  vtkGmshWriter(const vtkGmshWriter&);  // Not implemented.
  void operator=(const vtkGmshWriter&);  // Not implemented.

};

