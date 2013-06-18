/*
 * QuaternionExpression.hpp
 *
 *  Created on: Apr 24, 201RIndex
 *      Author: hannes
 */

namespace aslam {
namespace backend {
namespace quaternion {

#define _TEMPLATE template <typename TScalar, enum QuaternionMode EMode, typename TNode>
#define _CLASS QuaternionExpression<TScalar, EMode, TNode>

namespace internal {
inline constexpr int getRealIndex(QuaternionMode mode) {
  return realIsFirst(mode) ? 0 : 3;
}
inline constexpr int getIIndex(QuaternionMode mode) {
  return realIsFirst(mode) ? 1 : 0;
}
inline constexpr int getJIndex(QuaternionMode mode) {
  return realIsFirst(mode) ? 2 : 1;
}
inline constexpr int getKIndex(const QuaternionMode mode) {
  return realIsFirst(mode) ? 3 : 2;
}

template<typename TScalar, QuaternionMode EMode>
struct EigenQuaternionCalculator {
  typedef Eigen::Matrix<TScalar, 4, 1> vector_t;
  typedef Eigen::Matrix<TScalar, 3, 1> pure_imag_vector_t;

  enum {
    RIndex = getRealIndex(EMode),
    IIndex = getIIndex(EMode),
    JIndex = getJIndex(EMode),
    KIndex = getKIndex(EMode),
    IPureIndex = 0,
    JPureIndex = 1,
    KPureIndex = 2
  };

  inline static vector_t quatMultTraditional(const vector_t & a, const vector_t & b) {
    vector_t res;

    // aIIndex*bRIndex + aJIndex*bKIndex - aKIndex*bJIndex + aRIndex*bIIndex
    res[IIndex] = a[IIndex] * b[RIndex] + a[JIndex] * b[KIndex] - a[KIndex] * b[JIndex] + a[RIndex] * b[IIndex];
    // aKIndex*bIIndex - aIIndex*bKIndex + aJIndex*bRIndex + aRIndex*bJIndex
    res[JIndex] = a[KIndex] * b[IIndex] - a[IIndex] * b[KIndex] + a[JIndex] * b[RIndex] + a[RIndex] * b[JIndex];
    // aIIndex*bJIndex - aJIndex*bIIndex + aKIndex*bRIndex + aRIndex*bKIndex
    res[KIndex] = a[IIndex] * b[JIndex] - a[JIndex] * b[IIndex] + a[KIndex] * b[RIndex] + a[RIndex] * b[KIndex];
    // aRIndex*bRIndex - aJIndex*bJIndex - aKIndex*bKIndex - aIIndex*bIIndex
    res[RIndex] = a[RIndex] * b[RIndex] - a[JIndex] * b[JIndex] - a[KIndex] * b[KIndex] - a[IIndex] * b[IIndex];
    return res;
  }

  inline static vector_t quatMultTraditional(const pure_imag_vector_t & a, const vector_t & b) {
    vector_t res;
    res[IIndex] = a[IPureIndex] * b[RIndex] + a[JPureIndex] * b[KIndex] - a[KPureIndex] * b[JIndex];
    res[JIndex] = a[KPureIndex] * b[IIndex] - a[IPureIndex] * b[KIndex] + a[JPureIndex] * b[RIndex];
    res[KIndex] = a[IPureIndex] * b[JIndex] - a[JPureIndex] * b[IIndex] + a[KPureIndex] * b[RIndex];
    res[RIndex] = -a[JPureIndex] * b[JIndex] - a[KPureIndex] * b[KIndex] - a[IPureIndex] * b[IIndex];
    return res;
  }

  inline static vector_t quatMultTraditional(const vector_t & a, const pure_imag_vector_t & b) {
    vector_t res;
    res[IIndex] = +a[JIndex] * b[KPureIndex] - a[KIndex] * b[JPureIndex] + a[RIndex] * b[IPureIndex];
    res[JIndex] = a[KIndex] * b[IPureIndex] - a[IIndex] * b[KPureIndex] + +a[RIndex] * b[JPureIndex];
    res[KIndex] = a[IIndex] * b[JPureIndex] - a[JIndex] * b[IPureIndex] + +a[RIndex] * b[KPureIndex];
    res[RIndex] = -a[JIndex] * b[JPureIndex] - a[KIndex] * b[KPureIndex] - a[IIndex] * b[IPureIndex];
    return res;
  }

  template<int ISizeA, int ISizeB>
  inline static vector_t quatMult(const Eigen::Matrix<TScalar, ISizeA, 1> & a, const Eigen::Matrix<TScalar, ISizeB, 1> & b) {
    return isTraditionalMultOrder(EMode) ? quatMultTraditional(a, b) : quatMultTraditional(b, a);
  }

  inline static vector_t conjugate(const vector_t & v) {
    vector_t r(v);
    r.template block<3, 1>(IIndex, 0) *= -1;
    return r;
  }
  inline static vector_t invert(const vector_t & v) {
    vector_t r(conjugate(v));
    r /= r.dot(r);
    return r;
  }

  inline static auto getImagPart(const vector_t & v) -> decltype(v.template block<3, 1>(IIndex, 0)) {
    return v.template block<3, 1>(IIndex, 0);
  }
};
}

_TEMPLATE template <typename TOtherNode>
inline typename _CLASS::self_with_default_node_t _CLASS::operator * (const QuaternionExpression<TScalar, EMode, TOtherNode> & other) const {
  typedef _CLASS::self_with_default_node_t result_t;
  typedef QuaternionExpression<TScalar, EMode, TOtherNode> other_t;

  class ResultNode : public result_t::template BinaryOperationResult<ResultNode, self_t, other_t> {
  public:
    typedef typename result_t::template BinaryOperationResult<ResultNode, self_t, other_t> base_t;

    virtual ~ResultNode() {}

    virtual void evaluateImplementation() const {
      this->_currentValue = internal::EigenQuaternionCalculator<TScalar, EMode>::quatMult(this->getLhsNode().evaluate(), this->getRhsNode().evaluate());
    }

    inline typename base_t::apply_diff_return_t applyLhsDiff(const typename base_t::lhs_t::tangent_vector_t & tangent_vector) const {
      return internal::EigenQuaternionCalculator<TScalar, EMode>::quatMult(tangent_vector, this->getRhsNode().evaluate());  //TODO add matrix version?
  }
  inline typename base_t::apply_diff_return_t applyRhsDiff(const typename base_t::rhs_t::tangent_vector_t & tangent_vector) const {
    return internal::EigenQuaternionCalculator<TScalar, EMode>::quatMult(this->getLhsNode().evaluate(), tangent_vector);
  }
};

return ResultNode::create(*this, other);
}

_TEMPLATE
inline typename _CLASS::self_with_default_node_t _CLASS::inverse() const {
typedef _CLASS::self_with_default_node_t result_t;
typedef internal::EigenQuaternionCalculator<TScalar, EMode> calc;

class ResultNode : public result_t::template UnaryOperationResult<ResultNode, self_t> {
public:
  typedef typename result_t::template UnaryOperationResult<ResultNode, self_t> base_t;

  virtual ~ResultNode() {}

  virtual void evaluateImplementation() const {
    this->_currentValue = calc::invert(this->getOperandNode().evaluate());
  }

  inline typename base_t::apply_diff_return_t applyDiff(const typename base_t::operand_t::tangent_vector_t & tangent_vector) const {
    /*
     * d_q q^{-1} (v) = -(\bar q v \bar q) / (q\bar q)^2
     * while q\bar q = q.dot(q) = \bar q.dot(\bar q)
     */
    auto operandValConj = calc::conjugate(this->getOperandNode().evaluate());
    double valSquared = operandValConj.dot(operandValConj);
    return -calc::quatMult(operandValConj, calc::quatMult(tangent_vector, operandValConj)) / (valSquared * valSquared);
  }
};

return ResultNode::create(*this);
}

_TEMPLATE
inline typename _CLASS::self_with_default_node_t _CLASS::conjugate() const {
typedef _CLASS::self_with_default_node_t result_t;
class ResultNode : public result_t::template UnaryOperationResult<ResultNode, self_t> {
public:
  typedef typename result_t::template UnaryOperationResult<ResultNode, self_t> base_t;

  virtual ~ResultNode() {}

  virtual void evaluateImplementation() const {
    this->_currentValue = internal::EigenQuaternionCalculator<TScalar, EMode>::conjugate(this->getOperandNode().evaluate());
  }

  inline typename base_t::apply_diff_return_t applyDiff(const typename base_t::operand_t::tangent_vector_t & tangent_vector) const {
    return internal::EigenQuaternionCalculator<TScalar, EMode>::conjugate(tangent_vector);
  }
};

return ResultNode::create(*this);
}

#undef _CLASS
#define _CLASS UnitQuaternionExpression<TScalar, EMode, TNode>

    _TEMPLATE
    template <typename TOtherNode>
    inline GenericMatrixExpression<3, 1,TScalar> _CLASS::rotate3Vector(const GenericMatrixExpression<3, 1, TScalar, TOtherNode> & vector) const {
  typedef GenericMatrixExpression<3, 1, TScalar> result_t;
  typedef GenericMatrixExpression<3, 1, TScalar, TOtherNode> other_t;
  typedef internal::EigenQuaternionCalculator<TScalar, EMode> calc;

  class ResultNode : public result_t::template BinaryOperationResult<ResultNode, self_t, other_t> {
  public:
    typedef typename result_t::template BinaryOperationResult<ResultNode, self_t, other_t> base_t;

    virtual ~ResultNode() {}

    virtual void evaluateImplementation() const {
      auto lhsVal = this->getLhsNode().evaluate();
      this->_currentValue = calc::getImagPart(calc::quatMult(calc::quatMult(lhsVal, this->getRhsNode().evaluate()), calc::conjugate(lhsVal)));
    }

    inline typename base_t::apply_diff_return_t applyLhsDiff(const typename base_t::lhs_t::tangent_vector_t & tangent_vector) const {
      auto lhsVal = this->getLhsNode().evaluate();
      auto rhsVal = this->getRhsNode().evaluate();
      return calc::getImagPart(calc::quatMult(calc::quatMult(lhsVal, rhsVal), calc::conjugate(tangent_vector)) + calc::quatMult(calc::quatMult(tangent_vector, rhsVal), calc::conjugate(lhsVal)));
    }
    inline typename base_t::apply_diff_return_t applyRhsDiff(const typename base_t::rhs_t::tangent_vector_t & tangent_vector) const {
      auto lhsVal = this->getLhsNode().evaluate();
      return calc::getImagPart(calc::quatMult(calc::quatMult(lhsVal, tangent_vector), calc::conjugate(lhsVal)));
    }
  };

  return ResultNode::create(*this, vector);
}

#undef _TEMPLATE
#undef _CLASS

}  // namespace quaternion
}  // namespace backend
}  // namespace aslam
