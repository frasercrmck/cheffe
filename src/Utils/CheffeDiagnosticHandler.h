#ifndef CHEFFE_DIAGNOSTIC_HANDLER
#define CHEFFE_DIAGNOSTIC_HANDLER

#include <iostream>

namespace cheffe
{

class CheffeDiagnosticHandler
{
private:
public:
  CheffeDiagnosticHandler()
  {
  }
  ~CheffeDiagnosticHandler()
  {
  }

  void Report(const std::string& Message);
};

} // end namespace cheffe

#endif
