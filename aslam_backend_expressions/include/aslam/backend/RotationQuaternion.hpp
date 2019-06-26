#ifndef ASLAM_BACKEND_DV_QUAT_HPP
#define ASLAM_BACKEND_DV_QUAT_HPP


#include <Eigen/Core>
#include <aslam/backend/DesignVariable.hpp>
#include "RotationExpression.hpp"
#include "RotationExpressionNode.hpp"

namespace aslam {
  namespace backend {

    class RotationQuaternion : public RotationExpressionNode, public DesignVariable
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      RotationQuaternion(const Eigen::Vector4d & q);

      /// Constructs a rotation quaternion expression from a rotation matrix
      RotationQuaternion(const Eigen::Matrix3d& C);

      ~RotationQuaternion() override;

      /// \brief Revert the last state update.
      void revertUpdateImplementation() override;

      /// \brief Update the design variable.
      void updateImplementation(const double * dp, int size) override;

      /// \brief the size of an update step
      int minimalDimensionsImplementation() const override;

      void minimalDifferenceImplementation(const Eigen::MatrixXd& xHat, Eigen::VectorXd& outDifference) const override;

      void minimalDifferenceAndJacobianImplementation(const Eigen::MatrixXd& xHat, Eigen::VectorXd& outDifference, Eigen::MatrixXd& outJacobian) const override;

      RotationExpression toExpression();

      const Eigen::Vector4d & getQuaternion(){ return _q; }

      void set(const Eigen::Vector4d & q) {
        _q = q; _p_q = q;
        _C = sm::kinematics::quat2r(q);
      }
    private:
      Eigen::Matrix3d toRotationMatrixImplementation() const override;
      void evaluateJacobiansImplementation(JacobianContainer & outJacobians) const override;
      void getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const override;

      /// Returns the content of the design variable
      void getParametersImplementation(Eigen::MatrixXd& value) const override;

      /// Sets the content of the design variable
      void setParametersImplementation(const Eigen::MatrixXd& value) override;

      Eigen::Vector4d _q;
      Eigen::Vector4d _p_q;
      Eigen::Matrix3d _C;

    };

  } // namespace backend
} // namespace aslam

#endif /* ASLAM_BACKEND_DV_QUAT_HPP */
