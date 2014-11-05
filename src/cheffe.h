#ifndef CHEFFE_H
#define CHEFFE_H

#include <string>

namespace cheffe
{
enum class CheffeErrorCode
{
  CHEFFE_SUCCESS,
  CHEFFE_ERROR
};

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

} // end namespace cheffe
#endif
