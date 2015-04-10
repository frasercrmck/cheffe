#include "Parser/CheffeScopeInfo.h"

namespace cheffe
{

void CheffeScopeInfo::clearInfo()
{
  ScopeList.clear();
  while (!ScopeStack.empty())
  {
    ScopeStack.pop();
  }
}

void CheffeScopeInfo::addScope(const std::string &BeginVerb,
                               CheffeMethodStep *BeginMethodStep)
{
  ScopeStack.push(ScopeList.size());
  ScopeList.push_back(
      std::make_unique<CheffeScope>(BeginVerb, BeginMethodStep));
}

bool CheffeScopeInfo::empty() const
{
  return ScopeStack.empty();
}

bool CheffeScopeInfo::popScope(CheffeScope **ScopeInfo)
{
  if (ScopeStack.empty())
  {
    return true;
  }

  const auto LastScopeId = ScopeStack.top();
  ScopeStack.pop();

  assert(LastScopeId < ScopeList.size() && "Invalid nest index");
  *ScopeInfo = ScopeList[LastScopeId].get();
  return false;
}

bool CheffeScopeInfo::addBreak(CheffeMethodStep *MethodStep)
{
  if (ScopeStack.empty())
  {
    return true;
  }

  const auto LastScopeId = ScopeStack.top();
  assert(LastScopeId < ScopeList.size() && "Invalid nest index");

  if (!ScopeList[LastScopeId])
  {
    return true;
  }

  ScopeList[LastScopeId]->BreakList.push_back(MethodStep);
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
CheffeErrorCode CheffeScopeInfo::fixupScopeMethodSteps(
    const std::vector<CheffeMethodStep *> &MethodList)
{
  for (auto &Scope : ScopeList)
  {
    if (!Scope)
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    if (!Scope->BeginScope || !Scope->EndScope)
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }
    auto *BeginScopeMethodStep = Scope->BeginScope;

    auto BeginFound = std::find(std::begin(MethodList), std::end(MethodList),
                                BeginScopeMethodStep);
    if (BeginFound == std::end(MethodList))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    auto *EndScopeMethodStep = Scope->EndScope;
    auto EndFound = std::find(std::begin(MethodList), std::end(MethodList),
                              EndScopeMethodStep);
    if (EndFound == std::end(MethodList))
    {
      return CheffeErrorCode::CHEFFE_ERROR;
    }

    const auto BeginningToEnd = std::distance(BeginFound, EndFound);

    BeginScopeMethodStep->addNumber(BeginningToEnd);
    EndScopeMethodStep->addNumber(-BeginningToEnd);

    for (auto &Break : Scope->BreakList)
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

void CheffeScopeInfo::dumpInfo(std::ostream &OS) const
{
  for (auto &Scope : ScopeList)
  {
    OS << "=================" << std::endl;
    OS << "Begin Of Scope:" << std::endl << "\t";
    if (Scope->BeginScope)
    {
      OS << *Scope->BeginScope;
    }
    else
    {
      OS << "undef" << std::endl;
    }
    OS << "End Of Scope:" << std::endl << "\t";
    if (Scope->EndScope)
    {
      OS << *Scope->EndScope;
    }
    else
    {
      OS << "undef" << std::endl;
    }
    OS << "Break List:" << std::endl;
    for (auto &Break : Scope->BreakList)
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
