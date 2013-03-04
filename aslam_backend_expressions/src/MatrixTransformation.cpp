
#include <aslam/backend/MatrixTransformation.hpp>
#include <sm/kinematics/rotations.hpp>
#include <aslam/Exceptions.hpp>


namespace aslam {
  namespace backend {

    MatrixTransformation::MatrixTransformation(const Eigen::Matrix3d & A) : _A(A), _A_a(A){
    	_UpdatePattern << 1,1,1,1,1,1,1,1,1;			// ## general case (if no pattern is defined: every entry of the matrix will be updated
    	_UpdateDimension = 9;						    // ## number of unknowns: 9 (3x3 Matrix)

    }

    MatrixTransformation::MatrixTransformation(const Eigen::Matrix3d & A, const Eigen::Matrix3d & UpdatePattern) : _A(A), _A_a(A){
    	_UpdatePattern << UpdatePattern;              // Pattern of the Matrix;  1: design variables; 0: constants
    	_UpdateDimension = 0;
    	for (int i=0; i<9; i++){
    		if (_UpdatePattern(i%3,floor(i/3))==1){
    			_UpdateDimension ++;					// Count, how many design variables are in the matrix
    		}
    	}
    }

    MatrixTransformation::~MatrixTransformation(){}

      
    /// \brief Revert the last state update.
    void MatrixTransformation::revertUpdateImplementation()
    {
      _A = _A_a;
    }
    
    /// \brief Update the design variable.
    void MatrixTransformation::updateImplementation(const double * dp, int size)
    {
      SM_ASSERT_EQ_DBG(aslam::Exception, size, _UpdateDimension , "Incorrect update size");
      _A_a = _A;
      Eigen::Matrix3d dA;
      int j=0;
      for (int i=0; i<9; i++){
    	  _A(i%3,floor(i/3)) += _UpdatePattern(i%3,floor(i/3))*dp[j];
    	  if (_UpdatePattern(i%3,floor(i/3))==1){j++; }
      }
    }
    
    int MatrixTransformation::minimalDimensionsImplementation() const
    { 
      return _UpdateDimension; // number of unknowns in the matrix
    }
    
    // ##
    Eigen::Matrix3d MatrixTransformation::toFullMatrixImplementation()
    {
      return _A;
    }

    ///## will not be used
    void MatrixTransformation::evaluateJacobiansImplementation(JacobianContainer & outJacobians) const
    {
      outJacobians.add( const_cast<MatrixTransformation *>(this), Eigen::Matrix3d::Identity());
    }


    void MatrixTransformation::evaluateJacobiansImplementation(JacobianContainer & outJacobians, const Eigen::MatrixXd & applyChainRule) const
    {

    	//## get the Jacobian out of the the general case for the specific Matrix-Pattern (type)
    	Eigen::MatrixXd finalJacobian(3,_UpdateDimension);
		int j=0;
		for (int i=0; i<9; i++){
			if (_UpdatePattern(i%3,floor(i/3))==1){
				SM_ASSERT_GT_DBG(aslam::Exception, _UpdateDimension, j , "Incorrect update dimension! It doesn't match the pattern");
				finalJacobian.col(j) = applyChainRule.col(i);		// took out only the rows for the values, which are design variables
				j++;
			}
		}
		outJacobians.add( const_cast<MatrixTransformation*>(this), finalJacobian );
    }

    MatrixExpression MatrixTransformation::toExpression()
    {
      return MatrixExpression(this);
    }

    void MatrixTransformation::getDesignVariablesImplementation(DesignVariable::set_t & designVariables) const
    {
      designVariables.insert(const_cast<MatrixTransformation*>(this));
    }

  } // namespace backend
} // namespace aslam

