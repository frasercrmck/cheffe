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

  CheffeScope(const std::string &BeginVerb,
              std::shared_ptr<CheffeMethodStep> BeginMethodStep)
      : BeginVerb(BeginVerb), BeginScope(BeginMethodStep), EndScope(nullptr)
  {
  }

  std::string BeginVerb;
  std::shared_ptr<CheffeMethodStep> BeginScope;
  std::shared_ptr<CheffeMethodStep> EndScope;
  std::vector<std::shared_ptr<CheffeMethodStep>> BreakList;
};

class CheffeScopeInfo
{
public:
  CheffeScopeInfo()
  {
  }

  void clearInfo();

  void addScope(const std::string &BeginVerb,
                std::shared_ptr<CheffeMethodStep> BeginMethodStep);

  bool empty() const;

  bool popScope(std::shared_ptr<CheffeScope> &ScopeInfo);

  bool addBreak(std::shared_ptr<CheffeMethodStep> MethodStep);

  CheffeErrorCode fixupScopeMethodSteps(
      const std::vector<std::shared_ptr<CheffeMethodStep>> &MethodList);

  void dumpInfo(std::ostream &OS) const;

private:
  std::stack<unsigned> ScopeStack;
  std::vector<std::shared_ptr<CheffeScope>> ScopeList;
};

} // end namespace cheffe

#endif // CHEFFE_NEST_INFO
