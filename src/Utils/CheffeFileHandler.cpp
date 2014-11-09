#include "CheffeFileHandler.h"

#include <fstream>

using namespace cheffe;

CheffeErrorCode CheffeFileHandler::readFile(CheffeSourceFile &File)
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
