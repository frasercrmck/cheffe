#ifndef CHEFFE_NEST_INFO
#define CHEFFE_NEST_INFO

#include "cheffe.h"
#include "IR/CheffeMethodStep.h"

#include <memory>
#include <vector>
#include <stack>

namespace cheffe
{

class CheffeNest
{
public:
  CheffeNest() : BeginVerb(""), BeginNest(nullptr), EndNest(nullptr)
  {
  }

  CheffeNest(const std::string &BeginVerb,
             std::shared_ptr<CheffeMethodStep> BeginMethodStep)
      : BeginVerb(BeginVerb), BeginNest(BeginMethodStep), EndNest(nullptr)
  {
  }

  std::string BeginVerb;
  std::shared_ptr<CheffeMethodStep> BeginNest;
  std::shared_ptr<CheffeMethodStep> EndNest;
  std::vector<std::shared_ptr<CheffeMethodStep>> BreakList;
};

class CheffeNestInfo
{
public:
  CheffeNestInfo()
  {
  }

  void clearInfo();

  void addNest(const std::string &BeginVerb,
               std::shared_ptr<CheffeMethodStep> BeginMethodStep);

  bool empty() const;

  bool popNest(std::shared_ptr<CheffeNest> &NestInfo);

  bool addBreak(std::shared_ptr<CheffeMethodStep> MethodStep);

  CheffeErrorCode fixupNestMethodSteps(
      const std::vector<std::shared_ptr<CheffeMethodStep>> &MethodList);

  void dumpInfo(std::ostream &OS) const;

private:
  std::stack<unsigned> NestStack;
  std::vector<std::shared_ptr<CheffeNest>> NestList;
};

} // end namespace cheffe

#endif // CHEFFE_NEST_INFO
