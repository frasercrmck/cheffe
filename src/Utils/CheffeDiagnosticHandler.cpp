#include "CheffeDiagnosticHandler.h"

#include <cassert>
#include <map>

#if !defined(_WIN32) && !defined(__WIN64) && defined(__unix__) ||              \
    (defined(__APPLE__) && defined(__MACH__))
#define CHEFFE_POSIX 1
#else
#define CHEFFE_POSIX 0
#endif

#if CHEFFE_POSIX
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#endif

namespace cheffe
{

std::ostream &CheffeDiagnosticHandler::errs()
{
  return std::cerr;
}

// A conservative method to determine whether to emit colour diagnostics:
// * if it's a unix system
// * if stderr is a TTY
// * if $TERM is an xterm
static bool shouldUseColour()
{
#ifndef CHEFFE_POSIX
  // Be conversative: don't support colour
  return false;
#endif
  const char *const TermVar = std::getenv("TERM");
  const bool TerminalSupportsColour = !std::strcmp(TermVar, "xterm") ||
                                      !std::strcmp(TermVar, "xterm-color") ||
                                      !std::strcmp(TermVar, "xterm-256color");
  const bool IsATTY = isatty(fileno(stderr)) != 0;
  return IsATTY && TerminalSupportsColour;
}

enum class ColourKind
{
  RedBold,
  YellowBold,
  MagentaBold
};

const std::map<ColourKind, std::string> ColourCodes = {
    {ColourKind::RedBold, "\x1b[31;1m"},
    {ColourKind::YellowBold, "\x1b[33;1m"},
    {ColourKind::MagentaBold, "\x1b[35;1m"}};

struct ColourText
{
public:
  ColourText(const std::string &Message, const ColourKind Colour)
      : Message(Message)
  {
    assert(ColourCodes.find(Colour) != std::end(ColourCodes) &&
           "Invalid Colour Code");
    ColourOn = shouldUseColour() ? ColourCodes.find(Colour)->second : "";
    ColourOff = shouldUseColour() ? "\x1b[0m" : "";
  }

private:
  const std::string &Message;
  std::string ColourOn;
  std::string ColourOff;

  friend std::ostream &operator<<(std::ostream &os, const ColourText &Text)
  {
    os << Text.ColourOn << Text.Message << Text.ColourOff;
    return os;
  }
};

void CheffeDiagnosticHandler::formatAndLogMessage(
    const std::string &Message, const SourceLocation SourceLoc,
    const DiagnosticKind Kind, const LineContext Context)
{
  std::stringstream ss;
  ss << getFileAndLineNumberInfoAsString(SourceLoc.getLineNo(),
                                         SourceLoc.getColumnNo()) << ": ";
  if (Kind == DiagnosticKind::Error)
  {
    ss << ColourText("error", ColourKind::RedBold) << ": ";
  }
  else if (Kind == DiagnosticKind::Warning)
  {
    ss << ColourText("warning", ColourKind::YellowBold) << ": ";
  }
  ss << Message << std::endl;
  if (Context == LineContext::WithContext)
  {
    ss << getLineAsString(SourceLoc) << std::endl;
    ss << ColourText(getContextAsString(SourceLoc), ColourKind::MagentaBold)
       << std::endl;
  }

  switch (Kind)
  {
  default:
    cheffe_unreachable("Impossible enum value");
    break;
  case DiagnosticKind::Error:
    Errors.push_back(ss.str());
    break;
  case DiagnosticKind::Warning:
    Warnings.push_back(ss.str());
    break;
  }
}

void CheffeDiagnosticHandler::flushDiagnostics()
{
  for (auto &Message : Warnings)
  {
    errs() << Message << std::endl;
  }
  for (auto &Message : Errors)
  {
    errs() << Message << std::endl;
  }
}

unsigned CheffeDiagnosticHandler::getErrorCount() const
{
  return Errors.size();
}

unsigned CheffeDiagnosticHandler::getWarningCount() const
{
  return Warnings.size();
}

std::string
CheffeDiagnosticHandler::getLineAsString(const SourceLocation SourceLoc)
{
  assert(SourceLoc.getLineNo() != 0 && "Lines must be indexed from 1");
  unsigned LineCount = 1;
  std::size_t FilePos = 0;
  const std::size_t LineNo = SourceLoc.getLineNo();
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

std::string
CheffeDiagnosticHandler::getContextAsString(const SourceLocation SourceLoc)
{
  assert(SourceLoc.getColumnNo() != 0 && "Columns must be indexed from 1");
  const std::string Padding = std::string(SourceLoc.getColumnNo() - 1, ' ');
  const std::string UnderlineToken = std::string(SourceLoc.getLength(), '~');

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
