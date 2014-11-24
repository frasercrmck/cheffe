#include "CheffeDiagnosticHandler.h"

#include <cassert>

namespace cheffe
{

std::ostream &CheffeDiagnosticHandler::errs()
{
  return std::cerr;
}

std::string CheffeDiagnosticHandler::getLineAsString(const unsigned LineNo)
{
  assert(LineNo != 0 && "Lines must be indexed from 1");
  unsigned LineCount = 1;
  std::size_t FilePos = 0;
  for (; FilePos < File.Source.size() && LineCount != LineNo; ++FilePos)
  {
    if (File.Source[FilePos] == '\n')
    {
      LineCount++;
    }
  }

  const std::string Line =
      File.Source.substr(FilePos, File.Source.find('\n', FilePos) - FilePos);

  return Line;
}

std::string CheffeDiagnosticHandler::getContextAsString(const unsigned ColumnNo,
                                                        const unsigned Length)
{
  assert(ColumnNo != 0 && "Columns must be indexed from 1");
  const std::string Padding = std::string(ColumnNo - 1, ' ');
  const std::string UnderlineToken = std::string(Length, '~');

  std::stringstream ss;
  ss << Padding << UnderlineToken;
  return ss.str();
}

std::string CheffeDiagnosticHandler::getFileAndLineNumberInfoAsString(
    const unsigned LineNo, const unsigned ColumnNo)
{
  std::stringstream ss;
  ss << File.Name << ":" << LineNo << ":" << ColumnNo;
  return ss.str();
}

} // end namespace cheffe
