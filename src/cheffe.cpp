#include "cheffe.h"
#include "Driver/CheffeDriver.h"
#include "Parser/CheffeParser.h"
#include "Utils/CheffeDebugUtils.h"
#include "Utils/CheffeFileHandler.h"
#include "Utils/CheffeDiagnosticHandler.h"

#include <string>
#include <iostream>

using namespace cheffe;

int main(int argc, char **argv)
{
  std::string FileName;
  for (int i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "-debug-only"))
    {
      if (i != argc - 1)
      {
        setCurrentDebugType(argv[++i]);
        continue;
      }
    }
    // Input file is last in argument list
    if (i == argc - 1)
    {
      FileName = argv[i];
    }
  }

  if (FileName.empty())
  {
    std::cerr << "Error: no input file\n";
    return 1;
  }

  CheffeSourceFile InFile = {FileName, ""};
  const CheffeErrorCode Ret = CheffeFileHandler::readFile(InFile);

  if (Ret != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    std::cerr << "Error: could not read input file '" << InFile.Name << "'\n";
    return 1;
  }

  if (InFile.Source.empty())
  {
    std::cerr << "Error: empty input file '" << InFile.Name << "'\n";
    return 1;
  }

  CheffeDriver Driver;
  Driver.setSourceFile(InFile);

  auto Diagnostics = std::make_shared<CheffeDiagnosticHandler>();

  Driver.setDiagnosticHandler(Diagnostics);

  const CheffeErrorCode Success = Driver.compileRecipe();

  if (Success != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    std::cerr << "Error: could not parse input file\n";
  }
  else
  {
    std::cout << "File parsed successfully\n";
  }

  return 0;
}
