#include "CheffeDiagnosticHandler.h"

namespace cheffe
{

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

  std::cerr << "Error while processing source file ";
  PrintFileAndLineNumberInformation(LineNo, ColumnNo);
  std::cerr << std::endl;

  std::cerr << Message << std::endl;
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

  std::cerr << Line << std::endl;

  const std::string Padding = std::string(TokenOffsetFromLineBegin, ' ');
  const std::string UnderlineToken = std::string(TokenLength, '~');

  std::cerr << Padding << UnderlineToken << std::endl;
}

void CheffeDiagnosticHandler::PrintFileAndLineNumberInformation(
    const unsigned LineNo, const unsigned ColumnNo)
{
  std::cerr << File.Name << ":" << LineNo << ":" << ColumnNo;
}

} // end namespace cheffe
