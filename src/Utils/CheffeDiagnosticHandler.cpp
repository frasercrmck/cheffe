#include "CheffeDiagnosticHandler.h"

#include <cassert>

namespace cheffe
{

std::ostream &CheffeDiagnosticHandler::errs()
{
  return std::cerr;
}

void CheffeDiagnosticHandler::printLine(const unsigned LineNo,
                                        const std::size_t Begin,
                                        const std::size_t End)
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

  const std::size_t TokenLength = End - Begin;
  const std::size_t TokenOffsetFromLineBegin = Begin - FilePos;

  const std::string Line =
      File.Source.substr(FilePos, File.Source.find('\n', FilePos) - FilePos);

  errs() << Line << std::endl;

  const std::string Padding = std::string(TokenOffsetFromLineBegin, ' ');
  const std::string UnderlineToken = std::string(TokenLength, '~');

  errs() << Padding << UnderlineToken << std::endl;
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

void CheffeDiagnosticHandler::printFileAndLineNumberInformation(
    const unsigned LineNo, const unsigned ColumnNo)
{
  errs() << File.Name << ":" << LineNo << ":" << ColumnNo;
}

} // end namespace cheffe
