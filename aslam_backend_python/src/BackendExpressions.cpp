#include <numpy_eigen/boost_python_headers.hpp>
#include <aslam/backend/EuclideanExpression.hpp>
#include <aslam/backend/EuclideanExpressionNode.hpp>
#include <aslam/backend/HomogeneousExpression.hpp>
#include <aslam/backend/HomogeneousExpressionNode.hpp>
#include <aslam/backend/TransformationExpression.hpp>
#include <aslam/backend/TransformationExpressionNode.hpp>
#include <aslam/backend/TransformationBasic.hpp>
#include <aslam/backend/RotationQuaternion.hpp>
#include <aslam/backend/EuclideanPoint.hpp>
#include <aslam/backend/HomogeneousPoint.hpp>
#include <aslam/backend/DesignVariableMappedVector.hpp>
#include <aslam/backend/DesignVariableVector.hpp>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <aslam/python/ExportBackendExpressions.hpp>
#include <aslam/backend/ScalarExpression.hpp>
#include <aslam/backend/Scalar.hpp>
#include <aslam/backend/EuclideanDirection.hpp>
#include <aslam/backend/VectorExpression.hpp>
//#include <aslam/python/ExportAPrioriInformationError.hpp>

using namespace boost::python;
using namespace aslam::backend;
using namespace aslam::python;

template<int D>
void exportDesignVariableMappedVector()
{
  std::stringstream str;
  str << "DesignVariableMappedVector" << D;
  class_< DesignVariableMappedVector<D>, boost::shared_ptr< DesignVariableMappedVector<D> >, bases< DesignVariable> >(str.str().c_str(), no_init)
    .def("value", &DesignVariableMappedVector<D>::value)
    .def("toExpression", &DesignVariableMappedVector<D>::toExpression)
    ;
}

template<int D>
void exportDesignVariableVector()
{
  std::stringstream str;
  str << "DesignVariableVector" << D;
  class_< DesignVariableVector<D>, boost::shared_ptr< DesignVariableVector<D> >, bases< DesignVariable> >(str.str().c_str(), no_init)
    .def("value", &DesignVariableVector<D>::value, return_value_policy<copy_const_reference>())
    .def("toExpression", &DesignVariableVector<D>::toExpression)
    ;
}


void exportBackendExpressions()
{

  class_<EuclideanExpression, boost::shared_ptr<EuclideanExpression> >("EuclideanExpression", init<boost::shared_ptr<EuclideanExpressionNode> >() )
    .def("toEuclidean", &EuclideanExpression::toEuclidean)
    .def("toHomogeneousExpression", &EuclideanExpression::toHomogeneousExpression)
    .def("evaluateJacobians", &evaluateJacobians1<EuclideanExpression>)
    .def("evaluateJacobians", &evaluateJacobians2<EuclideanExpression>)
    .def("getDesignVariables", &getDesignVariables<EuclideanExpression>)
    // Decide later if it is useful to export the expression nodes.
    //.def("root", &EuclideanExpression::root)
    ;

  class_<HomogeneousExpression, boost::shared_ptr<HomogeneousExpression> >("HomogeneousExpression", init<boost::shared_ptr<HomogeneousExpressionNode> >() )
    .def("toHomogeneous", &HomogeneousExpression::toHomogeneous)
    //.def("toEuclideanExpression", &HomogeneousExpression::toEuclideanExpression)
    .def("evaluateJacobians", &evaluateJacobians1<HomogeneousExpression>)
    .def("evaluateJacobians", &evaluateJacobians2<HomogeneousExpression>)
    .def("getDesignVariables", &getDesignVariables<HomogeneousExpression>)
    // Decide later if it is useful to export the expression nodes.
    //.def("root", &HomogeneousExpression::root)
    ;
      
  // It is tricky to create expressions. Here we need the types to
  // define the operators so I have to create these here.
  EuclideanPoint euclideanPoint(Eigen::Vector3d::Random());
  EuclideanExpression euclideanExpression = euclideanPoint.toExpression();
  RotationQuaternion rotationQuaternion(Eigen::Vector4d(0,0,0,1));
  RotationExpression rotationExpression = rotationQuaternion.toExpression();
  HomogeneousPoint homogeneousPoint(Eigen::Vector4d::Random());
  HomogeneousExpression homogeneousExpression = homogeneousPoint.toExpression();
  TransformationBasic transformationBasic(rotationExpression, euclideanExpression);

  class_<TransformationExpression,boost::shared_ptr<TransformationExpression> >("TransformationExpression", init<boost::shared_ptr<TransformationExpressionNode> >() )
    .def("toTransformationMatrix", &TransformationExpression::toTransformationMatrix)

    // These guys are not implemented yet.
    //.def("toRotationExpression", &TransformationExpression::toRotationExpression)
    //.def("toHomogeneousExpression", &TransformationExpression::toHomogeneousExpression)
    //.def("toEuclideanExpression", &TransformationExpression::toEuclideanExpression)
    //.def(self * euclideanExpression)
    .def(self * homogeneousExpression)
    .def(self * self)
    .def("inverse", &TransformationExpression::inverse)
    
    .def("getDesignVariables", &getDesignVariables<TransformationExpression>)
    ;


  class_<RotationExpression,boost::shared_ptr<RotationExpression> >("RotationExpression", init<boost::shared_ptr<RotationExpressionNode> >() )
    .def("toRotationMatrix", &RotationExpression::toRotationMatrix)
    .def(self * euclideanExpression)
    .def(self * self)
    .def("inverse", &RotationExpression::inverse)
    .def("getDesignVariables", &getDesignVariables<RotationExpression>)
    ;
  

  // class_<EuclideanPoint, boost::shared_ptr<EuclideanPoint>, bases<DesignVariable> >("EuclideanPointDv", init<const Eigen::Vector3d>())
  //   .def("toExpression", &EuclideanPoint::toExpression)
  //   .def("toEuclidean", &EuclideanPoint::toEuclidean);
  // ;

  class_<RotationQuaternion, boost::shared_ptr<RotationQuaternion>, bases<DesignVariable> >("RotationQuaternionDv", init<const Eigen::Vector4d>())
    .def("toExpression", &RotationQuaternion::toExpression)
    .def("toRotationMatrix", &RotationQuaternion::toRotationMatrix)
    .def("getDesignVariables", &getDesignVariables<RotationQuaternion>)
    ;

  class_<EuclideanPoint, boost::shared_ptr<EuclideanPoint>, bases<DesignVariable> >("EuclideanPointDv", init<const Eigen::Vector3d>())
    .def("toExpression", &EuclideanPoint::toExpression)
    .def("toEuclidean", &EuclideanPoint::toEuclidean)
  .def("getDesignVariables", &getDesignVariables<EuclideanPoint>)
     ;

  class_<HomogeneousPoint, boost::shared_ptr<HomogeneousPoint>, bases<DesignVariable> >("HomogeneousPointDv", init<const Eigen::Vector4d>())
    .def("toExpression", &HomogeneousPoint::toExpression)
    .def("toHomogeneous", &HomogeneousPoint::toHomogeneous)
  .def("getDesignVariables", &getDesignVariables<HomogeneousPoint>)
     ;

  class_<TransformationBasic, boost::shared_ptr<TransformationBasic>, bases<DesignVariable> >("TransformationBasicDv", init<RotationExpression, EuclideanExpression>())
    .def("toExpression", &TransformationBasic::toExpression)
    .def("toTransformationMatrix", &TransformationBasic::toTransformationMatrix)
  .def("getDesignVariables", &getDesignVariables<TransformationBasic>)
     ;



  exportDesignVariableMappedVector<1>();
  exportDesignVariableMappedVector<2>();
  exportDesignVariableMappedVector<3>();
  exportDesignVariableMappedVector<4>();
  exportDesignVariableMappedVector<5>();
  exportDesignVariableMappedVector<6>();
  exportDesignVariableMappedVector<7>();
  exportDesignVariableMappedVector<8>();

  exportDesignVariableVector<1>();
  exportDesignVariableVector<2>();
  exportDesignVariableVector<3>();
  exportDesignVariableVector<4>();
  exportDesignVariableVector<5>();
  exportDesignVariableVector<6>();
  exportDesignVariableVector<7>();
  exportDesignVariableVector<8>();


  class_<ScalarExpression, boost::shared_ptr<ScalarExpression> >("ScalarExpression", init<boost::shared_ptr<ScalarExpressionNode> > () )
      .def("toScalar", &ScalarExpression::toScalar)
      .def("toValue", &ScalarExpression::toScalar)
      .def("evaluateJacobians", &evaluateJacobians1<ScalarExpression>)
      .def("evaluateJacobians", &evaluateJacobians2<ScalarExpression>)
      .def("getDesignVariables", &getDesignVariables<ScalarExpression>)
      .def(self + self)
      .def(self * self)
      .def(self - self)
      .def(self + double())
      .def(self - double())
      .def(self * double())
      //.def(self + float())
      //.def(self - float())
      //.def(self * float())
      ;

  class_<Scalar, boost::shared_ptr<Scalar>, bases<DesignVariable> >("Scalar", init<double>("Scalar(double value)") )
      .def("toExpression", &Scalar::toExpression)
      .def("toScalar", &Scalar::toScalar)
      .def("toValue", &Scalar::toScalar)
      ;
  // \todo reenable this
  // exportAPrioriInformationError<Scalar>("ScalarAPrioriInformationError");


  class_<EuclideanDirection, boost::shared_ptr<EuclideanDirection>, bases<DesignVariable> >("EuclideanDirection", init< const Eigen::Vector3d & >("EuclideanDirection(Vector3d p)"))
      .def("toExpression", &EuclideanDirection::toExpression)
      .def("toEuclidean", &EuclideanDirection::toEuclidean)
      ;

}
