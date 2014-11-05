#ifndef CHEFFE_DIAGNOSTIC_HANDLER
#define CHEFFE_DIAGNOSTIC_HANDLER

#include "cheffe.h"
#include <iostream>

namespace cheffe
{

class CheffeDiagnosticHandler
{
private:
  CheffeSourceFile File;

public:
  CheffeDiagnosticHandler(const CheffeSourceFile &File) : File(File)
  {
  }

  ~CheffeDiagnosticHandler()
  {
  }

  void Report(const std::string &Message, const unsigned LineNo,
              const unsigned ColumnNo);

  void PrintLine(const unsigned LineNo, const std::size_t Begin,
                 const std::size_t End);
  void PrintFileAndLineNumberInformation(const unsigned LineNo,
                                         const unsigned ColumnNo);
};

} // end namespace cheffe

#endif
