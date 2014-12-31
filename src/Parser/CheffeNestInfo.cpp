#include "Parser/CheffeNestInfo.h"

namespace cheffe
{

void CheffeNestInfo::clearInfo()
{
  NestList.clear();
  while (!NestStack.empty())
  {
    NestStack.pop();
  }
}

void CheffeNestInfo::addNest(const std::string &BeginVerb,
                             std::shared_ptr<CheffeMethodStep> BeginMethodStep)
{
  NestStack.push(NestList.size());
  NestList.push_back(std::make_shared<CheffeNest>(BeginVerb, BeginMethodStep));
}

bool CheffeNestInfo::empty() const
{
  return NestStack.empty();
}

bool CheffeNestInfo::popNest(std::shared_ptr<CheffeNest> &NestInfo)
{
  if (NestStack.empty())
  {
    return true;
  }

  const auto LastNestId = NestStack.top();
  NestStack.pop();

  assert(LastNestId < NestList.size() && "Invalid nest index");
  NestInfo = NestList[LastNestId];
  return false;
}

bool CheffeNestInfo::addBreak(std::shared_ptr<CheffeMethodStep> MethodStep)
{
  if (NestStack.empty())
  {
    return true;
  }

  const auto LastNestId = NestStack.top();
  assert(LastNestId < NestList.size() && "Invalid nest index");

  if (!NestList[LastNestId])
  {
    return true;
  }

  NestList[LastNestId]->BreakList.push_back(MethodStep);
  return false;
}

void CheffeNestInfo::dumpInfo(std::ostream &OS) const
{
  for (auto &Nest : NestList)
  {
    OS << "=================" << std::endl;
    OS << "Begin Of Nest:" << std::endl << "\t";
    if (Nest->BeginNest)
    {
      OS << *Nest->BeginNest;
    }
    else
    {
      OS << "undef" << std::endl;
    }
    OS << "End Of Nest:" << std::endl << "\t";
    if (Nest->EndNest)
    {
      OS << *Nest->EndNest;
    }
    else
    {
      OS << "undef" << std::endl;
    }
    OS << "Break List:" << std::endl;
    for (auto &Break : Nest->BreakList)
    {
      OS << "\t";
      if (Break)
      {
        OS << *Break;
      }
      else
      {
        OS << "undef" << std::endl;
      }
    }
    OS << "=================" << std::endl;
    OS << std::endl;
  }
}

} // end namespace cheffe
