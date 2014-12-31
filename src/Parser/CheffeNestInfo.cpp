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

// This method fixes up the Method Steps inside each recorded Scope with the
// offsets to their counterparts.
// * The method step that marks the beginning of the scope gets an offset to
//   the end of the scope
// * The method step that marks the end of the scope gets an offset to the
//   beginning of the scope
// * Each Set Aside method scope in the nest get an offset to the end of the
//   scope.
CheffeErrorCode CheffeNestInfo::fixupNestMethodSteps(
    const std::vector<std::shared_ptr<CheffeMethodStep>> &MethodList)
{
  for (auto &Nest : NestList)
  {
    if (!Nest)
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    if (!Nest->BeginNest || !Nest->EndNest)
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    const auto BeginNestMethodStep = Nest->BeginNest;

    auto BeginFound = std::find(std::begin(MethodList), std::end(MethodList),
                                BeginNestMethodStep);
    if (BeginFound == std::end(MethodList))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    const auto EndNestMethodStep = Nest->EndNest;
    auto EndFound = std::find(std::begin(MethodList), std::end(MethodList),
                              EndNestMethodStep);
    if (EndFound == std::end(MethodList))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    const auto BeginningToEnd = std::distance(BeginFound, EndFound);

    BeginNestMethodStep->addNumber(BeginningToEnd);
    EndNestMethodStep->addNumber(-BeginningToEnd);

    for (auto &Break : Nest->BreakList)
    {
      auto BreakFound =
          std::find(std::begin(MethodList), std::end(MethodList), Break);
      if (BreakFound == std::end(MethodList))
      {
        return CheffeErrorCode::CHEFFE_ERROR;
      }
      const auto BreakToEnd = std::distance(BreakFound, EndFound);

      Break->addNumber(BreakToEnd);
    }
  }
  return CheffeErrorCode::CHEFFE_SUCCESS;
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
