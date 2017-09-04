#include <aslam/backend/ScalarExpression.hpp>

#include <sm/boost/null_deleter.hpp>

#include <aslam/backend/ExpressionNodeVisitor.hpp>
#include <aslam/backend/ScalarExpressionNode.hpp>

namespace aslam {
namespace backend {
ScalarExpression::ScalarExpression() {
}

ScalarExpression::ScalarExpression(double value)
    : _root(new ScalarExpressionNodeConstant(value)) {
}

class ScalarExpressionNodeNamedConstant : public ScalarExpressionNodeConstant
{
 public:
  ScalarExpressionNodeNamedConstant(const char *name, double s) : ScalarExpressionNodeConstant(s), _name(name){}
  void accept(ExpressionNodeVisitor& visitor) override {
    visitor.visit(_name.c_str(), this);
  }
 protected:
  std::string _name;
};

ScalarExpression::ScalarExpression(const char * name, double value) :
    _root(new ScalarExpressionNodeNamedConstant(name, value))
{
}

ScalarExpression::ScalarExpression(ScalarExpressionNode * designVariable)
    : _root(designVariable, sm::null_deleter()) {
}

ScalarExpression::ScalarExpression(boost::shared_ptr<ScalarExpressionNode> root)
    : _root(root) {
}

ScalarExpression::~ScalarExpression() {

}

double ScalarExpression::toScalar() const {
  return _root->toScalar();
}

void ScalarExpression::evaluateJacobians(JacobianContainer & outJacobians) const {
  _root->evaluateJacobians(outJacobians);
}

void ScalarExpression::evaluateJacobians(JacobianContainer & outJacobians, const Eigen::MatrixXd & applyChainRule) const {
  _root->evaluateJacobians( outJacobians.apply(applyChainRule) );
}

void ScalarExpression::getDesignVariables(DesignVariable::set_t & designVariables) const {
  _root->getDesignVariables(designVariables);
}

ScalarExpression ScalarExpression::operator+(const ScalarExpression & s) const {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAdd(_root, s._root));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator-(const ScalarExpression & s) const {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAdd(_root, s._root, -1.0));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator-(double s) const {
  boost::shared_ptr<ScalarExpressionNode> constant(new ScalarExpressionNodeConstant(-s));
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAdd(_root, constant));
  return ScalarExpression(newRoot);

}

ScalarExpression ScalarExpression::operator-() const {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeNegated(_root));
  return ScalarExpression(newRoot);

}


ScalarExpression ScalarExpression::operator/(const ScalarExpression & s) const {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeDivide(_root, s._root));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator/(double s) const {
  boost::shared_ptr<ScalarExpressionNode> constant(new ScalarExpressionNodeConstant(s));
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeDivide(_root, constant));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator+(double s) const {
  boost::shared_ptr<ScalarExpressionNode> constant(new ScalarExpressionNodeConstant(s));
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAdd(_root, constant));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator*(double s) const {
  boost::shared_ptr<ScalarExpressionNode> constant(new ScalarExpressionNodeConstant(s));
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeMultiply(_root, constant));
  return ScalarExpression(newRoot);
}

ScalarExpression ScalarExpression::operator*(const ScalarExpression & s) const {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeMultiply(_root, s._root));
  return ScalarExpression(newRoot);
}


std::ostream& operator<<(std::ostream& os, const ScalarExpression& e) {
  os << e.toScalar();
  return os;
}

class UnaryScalarExpressionNode : public ScalarExpressionNode
{
 protected:
  UnaryScalarExpressionNode(boost::shared_ptr<ScalarExpressionNode> arg) : _arg(arg) {}
  void getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const override {
    _arg->getDesignVariables(designVariables);
  }
  boost::shared_ptr<ScalarExpressionNode> _arg;
};
template <typename Eval, typename EvalDerivative>
ScalarExpression createUnaryScalarExpressionOp(Eval eval, EvalDerivative evalD, const ScalarExpression& e){
  class Result : public UnaryScalarExpressionNode {
   public:
    Result(Eval eval, EvalDerivative evalD, const ScalarExpression & e) : UnaryScalarExpressionNode(e.root()), eval(eval), evalD(evalD) {}
   private:
    double evaluateImplementation() const override {
      return eval(_arg->evaluate());
    }
    void evaluateJacobiansImplementation(JacobianContainer & outJacobians) const override {
      _arg->evaluateJacobians(outJacobians.apply(evalD(_arg->evaluate())));
    }
   private:
    Eval eval;
    EvalDerivative evalD;
  };
  return ScalarExpression(boost::shared_ptr<ScalarExpressionNode>(new Result(eval, evalD, e)));
}

ScalarExpression createUnaryScalarExpressionOp(double(* eval)(double), double(* evalD)(double), const ScalarExpression& e){
  return createUnaryScalarExpressionOp<decltype(eval), decltype(evalD)>(eval, evalD, e);
}

ScalarExpression sqrt(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeSqrt(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression log(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeLog(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression exp(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeExp(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression atan(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAtan(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression tanh(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeTanh(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression atan2(const ScalarExpression& e0, const ScalarExpression& e1) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAtan2(e0.root(), e1.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression acos(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAcos(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression sin(const ScalarExpression& e) {
  return createUnaryScalarExpressionOp(&std::sin, &std::cos, e);
}

ScalarExpression cos(const ScalarExpression& e) {
  return createUnaryScalarExpressionOp(&std::cos, [](double v){return -std::sin(v);}, e);
}

ScalarExpression acosSquared(const ScalarExpression& e) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeAcosSquared(e.root()));
  return ScalarExpression(newRoot);
}

ScalarExpression inverseSigmoid(const ScalarExpression& e, const double height, const double scale, const double shift) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodeInverseSigmoid(e.root(), height, scale, shift));
  return ScalarExpression(newRoot);
}

ScalarExpression powerExpression(const ScalarExpression& e, const int k) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionNodePower(e.root(), k));
  return ScalarExpression(newRoot);
}

ScalarExpression piecewiseExpression(const ScalarExpression& e1, const ScalarExpression& e2, std::function<bool()> useFirst) {
  boost::shared_ptr<ScalarExpressionNode> newRoot(new ScalarExpressionPiecewiseExpression(e1.root(), e2.root(), useFirst));
  return ScalarExpression(newRoot);
}

void ScalarExpression::accept(ExpressionNodeVisitor& visitor) const {
  visitor.beAcceptedBy(_root);
}


}  // namespace backend
}  // namespace aslam
