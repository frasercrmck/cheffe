#include "CheffeDiagnosticHandler.h"

namespace cheffe
{

std::ostream &CheffeDiagnosticHandler::errs()
{
  return std::cerr;
}

void CheffeDiagnosticHandler::Report(const std::string &Msg,
                                     const unsigned LineNo,
                                     const unsigned ColumnNo)
{
  std::string Message = Msg;
  const std::size_t MessageLength = Message.length();
  // Strip off the final trailing newline
  if (Message.length() != 0 && Message[MessageLength - 1] == '\n')
  {
    Message = Message.substr(0, MessageLength - 1);
  }

  errs() << "Error while processing source file ";
  PrintFileAndLineNumberInformation(LineNo, ColumnNo);
  errs() << std::endl;

  errs() << Message << std::endl;
}

void CheffeDiagnosticHandler::PrintLine(const unsigned LineNo,
                                        const std::size_t Begin,
                                        const std::size_t End)
{
  unsigned LineCount = 1;
  std::size_t FilePos = 0;
  for (FilePos = 0; FilePos < File.Source.size() && LineCount != LineNo;
       ++FilePos)
  {
    if (File.Source[FilePos] == '\n')
    {
      LineCount++;
    }
  }

  const std::size_t TokenLength = End - Begin;
  const std::size_t TokenOffsetFromLineBegin = Begin - FilePos;

  std::string Line =
      File.Source.substr(FilePos, File.Source.find('\n', FilePos) - FilePos);

  errs() << Line << std::endl;

  const std::string Padding = std::string(TokenOffsetFromLineBegin, ' ');
  const std::string UnderlineToken = std::string(TokenLength, '~');

  errs() << Padding << UnderlineToken << std::endl;
}

void CheffeDiagnosticHandler::PrintFileAndLineNumberInformation(
    const unsigned LineNo, const unsigned ColumnNo)
{
  errs() << File.Name << ":" << LineNo << ":" << ColumnNo;
}

} // end namespace cheffe
