#ifndef CHEFFE_NEST_INFO
#define CHEFFE_NEST_INFO

#include "cheffe.h"
#include "IR/CheffeMethodStep.h"

#include <memory>
#include <vector>
#include <stack>

namespace cheffe
{

class CheffeScope
{
public:
  CheffeScope() : BeginVerb(""), BeginScope(nullptr), EndScope(nullptr)
  {
  }

  CheffeScope(const std::string &BeginVerb, CheffeMethodStep *BeginMethodStep)
      : BeginVerb(BeginVerb), BeginScope(BeginMethodStep), EndScope(nullptr)
  {
  }

  std::string BeginVerb;
  CheffeMethodStep *BeginScope;
  CheffeMethodStep *EndScope;
  std::vector<CheffeMethodStep *> BreakList;
};

class CheffeScopeInfo
{
public:
  CheffeScopeInfo()
  {
  }

  void clearInfo();

  void addScope(const std::string &BeginVerb,
                CheffeMethodStep *BeginMethodStep);

  bool empty() const;

  bool popScope(CheffeScope **ScopeInfo);

  bool addBreak(CheffeMethodStep *MethodStep);

  CheffeErrorCode
  fixupScopeMethodSteps(const std::vector<CheffeMethodStep *> &MethodList);

  void dumpInfo(std::ostream &OS) const;

private:
  std::stack<unsigned> ScopeStack;
  std::vector<std::unique_ptr<CheffeScope>> ScopeList;
};

} // end namespace cheffe

#endif // CHEFFE_NEST_INFO
