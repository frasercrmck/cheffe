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

  std::ostream& errs();

public:
  CheffeDiagnosticHandler(const CheffeSourceFile &File) : File(File)
  {
  }

  ~CheffeDiagnosticHandler()
  {
  }

  void report(const std::string &Message, const unsigned LineNo,
              const unsigned ColumnNo);

  void printLine(const unsigned LineNo, const std::size_t Begin,
                 const std::size_t End);
  void printFileAndLineNumberInformation(const unsigned LineNo,
                                         const unsigned ColumnNo);
};

} // end namespace cheffe

#endif
