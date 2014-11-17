#include "Driver/CheffeDriver.h"

namespace cheffe
{
void CheffeDriver::addSourceFile(const CheffeSourceFile &File)
{
  SourceFiles.push_back(File);
}
} // end namespace cheffe
