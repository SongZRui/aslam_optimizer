#include <aslam/backend/TransformationExpressionNode.hpp>
#include <sm/kinematics/transformations.hpp>
#include <aslam/backend/EuclideanExpression.hpp>
#include <aslam/backend/EuclideanExpressionNode.hpp>
#include <aslam/backend/ExpressionNodeVisitor.hpp>
#include <aslam/backend/RotationExpression.hpp>
#include <aslam/backend/RotationExpressionNode.hpp>
#include <Eigen/Dense>

namespace aslam {
  namespace backend {
    
    ////////////////////////////////////////////
    // TransformationExpressionNode: The Super Class
    ////////////////////////////////////////////
    TransformationExpressionNode::TransformationExpressionNode(){}

    TransformationExpressionNode::~TransformationExpressionNode(){}

    void TransformationExpressionNode::evaluateJacobians(JacobianContainer & outJacobians) const
    {
      evaluateJacobiansImplementation(outJacobians);
    }      
      
    void TransformationExpressionNode::getDesignVariables(DesignVariable::set_t & designVariables) const
    {
      getDesignVariablesImplementation(designVariables);
    }


    /////////////////////////////////////////////////
    // TransformationExpressionNodeMultiply: A container for C1 * C2
    /////////////////////////////////////////////////

    TransformationExpressionNodeMultiply::TransformationExpressionNodeMultiply(boost::shared_ptr<TransformationExpressionNode> lhs, boost::shared_ptr<TransformationExpressionNode> rhs):
      _lhs(lhs), _rhs(rhs)
    {
    }

    TransformationExpressionNodeMultiply::~TransformationExpressionNodeMultiply(){}

    void TransformationExpressionNodeMultiply::getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const
    {
      _lhs->getDesignVariables(designVariables);
      _rhs->getDesignVariables(designVariables);
    }

    void TransformationExpressionNode::accept(ExpressionNodeVisitor& visitor) {
      visitor.visit("T", this);
    }

    Eigen::Matrix4d TransformationExpressionNodeMultiply::toTransformationMatrixImplementation()
    {
      _T_lhs = _lhs->toTransformationMatrix();
      _T_rhs = _rhs->toTransformationMatrix();
      return  _T_lhs * _T_rhs;
    }

    void TransformationExpressionNodeMultiply::evaluateJacobiansImplementation(JacobianContainer & outJacobians) const
    {	
      _rhs->evaluateJacobians(outJacobians,sm::kinematics::boxTimes(_T_lhs));
      _lhs->evaluateJacobians(outJacobians);
    }

    ////////////////////////////////////////////////////
    /// TransformationExpressionNodeInverse: A container for T^-1
    ////////////////////////////////////////////////////
    
    TransformationExpressionNodeInverse::TransformationExpressionNodeInverse(boost::shared_ptr<TransformationExpressionNode> dvTransformation) : _dvTransformation(dvTransformation)
    {
    }
    
    TransformationExpressionNodeInverse::~TransformationExpressionNodeInverse(){}

    Eigen::Matrix4d TransformationExpressionNodeInverse::toTransformationMatrixImplementation()
    {
      _T = _dvTransformation->toTransformationMatrix().inverse();
      return  _T;
    }

    void TransformationExpressionNodeInverse::evaluateJacobiansImplementation(JacobianContainer & outJacobians) const
    {
      _dvTransformation->evaluateJacobians(outJacobians, -sm::kinematics::boxTimes(_T));
    }

    void TransformationExpressionNodeInverse::getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const
    {
      _dvTransformation->getDesignVariables(designVariables);
    }

      TransformationExpressionNodeConstant::TransformationExpressionNodeConstant(const Eigen::Matrix4d & T) : _T(T) {}
      TransformationExpressionNodeConstant::~TransformationExpressionNodeConstant(){}

    
      Eigen::Matrix4d TransformationExpressionNodeConstant::toTransformationMatrixImplementation(){ return _T; }
  void TransformationExpressionNodeConstant::evaluateJacobiansImplementation(JacobianContainer & /* outJacobians */) const{}
  void TransformationExpressionNodeConstant::getDesignVariablesImplementation(DesignVariable::set_t & /* designVariables */) const{}

  RotationExpression aslam::backend::TransformationExpressionNode::toRotationExpression(const boost::shared_ptr<TransformationExpressionNode>& thisShared) const {
    assert(thisShared.get() == this);
    return boost::shared_ptr< RotationExpressionNode >( new RotationExpressionNodeTransformation( thisShared ) );
  }

  EuclideanExpression aslam::backend::TransformationExpressionNode::toEuclideanExpression(const boost::shared_ptr<TransformationExpressionNode>& thisShared) const {
    assert(thisShared.get() == this);
    return boost::shared_ptr< EuclideanExpressionNode >( new EuclideanExpressionNodeTranslation( thisShared ) );
  }

  void TransformationExpressionNodeMultiply::accept(ExpressionNodeVisitor& visitor) {
    visitor.visit("*", this, _lhs, _rhs);
  }

  void TransformationExpressionNodeInverse::accept(ExpressionNodeVisitor& visitor) {
    visitor.visit("^-1", this, _dvTransformation);
  }

  void TransformationExpressionNodeConstant::accept(ExpressionNodeVisitor& visitor) {
    visitor.visit("#", this);
  }

  } // namespace backend
}  // namespace aslam
