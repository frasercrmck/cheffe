#include "Parser/CheffeParser.h"

#include <string>
#include <fstream>
#include <cerrno>
#include <stdio.h>
#include <iostream>

using namespace cheffe;

static CheffeErrorCode readFile(CheffeSourceFile& File)
{
  std::ifstream InStream(File.Name, std::ios::in | std::ios::binary);

  if (!InStream)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  InStream.seekg(0, std::ios::end);

  File.Source.resize(InStream.tellg());

  InStream.seekg(0, std::ios::beg);
  InStream.read(&File[0], File.Source.size());

  InStream.close();

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

int main(int argc, char** argv)
{
  std::string FileName;
  for (int i = 1; i < argc; ++i)
  {
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

  CheffeSourceFile InFile = { FileName, "" };
  const CheffeErrorCode Ret = readFile(InFile);

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

  CheffeParser Parser(InFile);

  CheffeErrorCode Success = Parser.parseRecipe();

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
