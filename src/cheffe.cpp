#include "Parser/CheffeParser.h"

#include <string>
#include <fstream>
#include <cerrno>
#include <stdio.h>
#include <iostream>

using namespace cheffe;

static CheffeErrorCode readFile(const std::string& FileName, std::string& File)
{
  std::ifstream InStream(FileName, std::ios::in | std::ios::binary);

  if (!InStream)
  {
    return CheffeErrorCode::CHEFFE_ERROR;
  }

  InStream.seekg(0, std::ios::end);

  File.resize(InStream.tellg());

  InStream.seekg(0, std::ios::beg);
  InStream.read(&File[0], File.size());

  InStream.close();

  return CheffeErrorCode::CHEFFE_SUCCESS;
}

int main(int argc, char** argv)
{
  std::string InFile;
  std::string FileName;
  for (unsigned i = 1; i < argc; ++i)
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

  const CheffeErrorCode Ret = readFile(FileName, InFile);
  if (Ret != CheffeErrorCode::CHEFFE_SUCCESS)
  {
    std::cerr << "Error: could not read input file '" << FileName << "'\n";
    return 1;
  }

  if (InFile.empty())
  {
    std::cerr << "Error: empty input file '" << FileName << "'\n";
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
