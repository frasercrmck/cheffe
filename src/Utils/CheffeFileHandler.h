#ifndef CHEFFE_FILE_HANDLER
#define CHEFFE_FILE_HANDLER

#include "cheffe.h"

#include <string>

namespace cheffe
{

struct CheffeSourceFile
{
  std::string Name;
  std::string Source;

  char &operator[](std::size_t Pos)
  {
    return Source[Pos];
  }

  const char &operator[](std::size_t Pos) const
  {
    return Source[Pos];
  }
};

class CheffeFileHandler
{
public:
  static CheffeErrorCode readFile(CheffeSourceFile &File);
};

} // end namespace cheffe

#endif
