#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkPVConfig.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

class FLMLreader : public vtkUnstructuredGridAlgorithm
{
public:
  static FLMLreader *New();
  vtkTypeMacro(FLMLreader,vtkUnstructuredGridAlgorithm);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  void PrintSelf(ostream& os, vtkIndent indent);
 protected:

  FLMLreader();
  ~FLMLreader();

  int FillOutputPortInformation(int port, 
				vtkInformation* info);

  virtual int RequestData(
			  vtkInformation* request,
			  vtkInformationVector** InputVector,
			  vtkInformationVector* outputVector);

  char* FileName;
 private:
  FLMLreader(const FLMLreader&);  // Not implemented.
  void operator=(const FLMLreader&);  // Not implemented.

};

#include "spud.h"

namespace spud {

int load_options(char* a) {
return spud_load_options(a, strlen(a));
}

int option_count(const std::string &s){
return spud_option_count(s.c_str(), s.length());
}

bool have_option(const std::string &s){
return spud_have_option(s.c_str(), s.length());
}

int get_option(const std::string &k, std::string &v){
char buffer[8192];
int flag = spud_get_option(k.c_str(), k.length(), buffer);
v.assign(buffer);
return flag;
}

template <class T> int get_option(const std::string &k, std::vector<T> &v){
int shape[2];
spud_get_option_shape(k.c_str(), k.length(), shape);
T* tmp = new T[shape[1]];
int flag = spud_get_option(k.c_str(), k.length(), tmp);
for (int i=0; i<shape[1]; ++i) {
v.push_back(tmp[i]);
}
delete[] tmp;
return flag;
}

int get_option(const std::string &k, double &v){
return spud_get_option(k.c_str(), k.length(), &v);
}


}

