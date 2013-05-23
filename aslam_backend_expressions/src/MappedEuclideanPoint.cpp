#include <aslam/backend/MappedEuclideanPoint.hpp>


namespace aslam {
  namespace backend {
    MappedEuclideanPoint::MappedEuclideanPoint(double * p) :
      _p(p), _p_p(_p)
    {

    }
    MappedEuclideanPoint::~MappedEuclideanPoint()
    {

    }

    /// \brief Revert the last state update.
    void MappedEuclideanPoint::revertUpdateImplementation()
    {
      _p = _p_p;
    }
    
    /// \brief Update the design variable.
    void MappedEuclideanPoint::updateImplementation(const double * dp, int size)
    {
        SM_ASSERT_EQ_DBG(std::runtime_error, size, 3, "Incorrect size");
      _p_p = _p;
      
      Eigen::Map< const Eigen::Vector3d > dpv(dp);
      _p += dpv;

    }
    
    /// \brief the size of an update step
    int MappedEuclideanPoint::minimalDimensionsImplementation() const
    {
      return 3;
    }
    
    Eigen::Vector3d MappedEuclideanPoint::toEuclideanImplementation()
    {
      return _p;
    }
    
    void MappedEuclideanPoint::evaluateJacobiansImplementation(JacobianContainer & outJacobians) const
    {
      outJacobians.add(const_cast<MappedEuclideanPoint *>(this), Eigen::Matrix3d::Identity());
    }
    
    void MappedEuclideanPoint::evaluateJacobiansImplementation(JacobianContainer & outJacobians, const Eigen::MatrixXd & applyChainRule) const
    {
      outJacobians.add(const_cast<MappedEuclideanPoint *>(this), applyChainRule);
    }

    void MappedEuclideanPoint::getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const
    {
      designVariables.insert(const_cast<MappedEuclideanPoint *>(this));
    }

    EuclideanExpression MappedEuclideanPoint::toExpression()
    {
      return EuclideanExpression(this);
    }

    void MappedEuclideanPoint::getParametersImplementation(
        Eigen::MatrixXd& value) const {
      value = _p;
    }

    void MappedEuclideanPoint::setParametersImplementation(
        const Eigen::MatrixXd& value) {
      _p_p = _p;
      _p = value;
    }

  } // namespace backend
} // namespace aslam
