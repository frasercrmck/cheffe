#include "CheffeDiagnosticHandler.h"

namespace cheffe
{

void CheffeDiagnosticHandler::Report(const std::string& Msg)
{
  std::string Message = Msg;
  const std::size_t MessageLength = Message.length();
  // Strip off the final trailing newline
  if (Message.length() != 0 && Message[MessageLength - 1] == '\n')
  {
    Message = Message.substr(0, MessageLength - 1);
  }
  std::cerr << Message << std::endl;
}

} // end namespace cheffe
