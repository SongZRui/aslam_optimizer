/*
 * OptimizerBase.cpp
 *
 *  Created on: 11.04.2016
 *      Author: sculrich
 */

// Schweizer Messer
#include <sm/PropertyTree.hpp>

// self
#include <aslam/Exceptions.hpp>
#include <aslam/backend/OptimizerBase.hpp>

namespace aslam
{
namespace backend
{

OptimizerOptionsBase::OptimizerOptionsBase()
{
  this->check();
}

OptimizerOptionsBase::OptimizerOptionsBase(const sm::PropertyTree& config) {

  convergenceGradientNorm = config.getDouble("convergenceGradientNorm", convergenceGradientNorm);
  convergenceDeltaX = config.getDouble("convergenceDeltaX", convergenceDeltaX);
  convergenceDeltaError = config.getDouble("convergenceDeltaError", convergenceDeltaError);
  maxIterations = config.getInt("maxIterations", maxIterations);
  numThreadsJacobian = config.getInt("numThreadsJacobian", numThreadsJacobian);
  numThreadsError = config.getInt("numThreadsError", numThreadsError);

  this->check();
}

void OptimizerOptionsBase::check() const
{
  SM_ASSERT_GE( Exception, convergenceGradientNorm, 0.0, "");
  SM_ASSERT_GE( Exception, convergenceDeltaX, 0.0, "");
  SM_ASSERT_GE( Exception, convergenceDeltaError, 0.0, "");
  SM_ASSERT_TRUE( Exception, convergenceDeltaX > 0.0 || convergenceGradientNorm > 0.0 || convergenceDeltaError > 0.0, "");
  SM_ASSERT_GE( Exception, maxIterations, -1, "");
}

std::ostream& operator<<(std::ostream& out, const aslam::backend::OptimizerOptionsBase& options)
{
  out << "OptimizerOptions:" << std::endl;
  out << "\tconvergenceGradientNorm: " << options.convergenceGradientNorm << std::endl;
  out << "\tconvergenceDeltaX: " << options.convergenceDeltaX << std::endl;
  out << "\tconvergenceDeltaError: " << options.convergenceDeltaError << std::endl;
  out << "\tmaxIterations: " << options.maxIterations << std::endl;
  out << "\tnumThreadsJacobian: " << options.numThreadsJacobian << std::endl;
  out << "\tnumThreadsError: " << options.numThreadsError << std::endl;
  return out;
}

void OptimizerStatus::reset()
{
  *this = OptimizerStatus();
  this->resetImplementation();
}

bool OptimizerStatus::success() const
{
  return convergence != IN_PROGRESS && !this->failure();
}

bool OptimizerStatus::failure() const
{
  return convergence == FAILURE;
}

std::ostream& operator<<(std::ostream& out, const ConvergenceStatus& convergence)
{
  switch (convergence)
  {
    case ConvergenceStatus::IN_PROGRESS:
      out << "IN_PROGRESS";
      break;
    case ConvergenceStatus::FAILURE:
      out << "FAILURE";
      break;
    case ConvergenceStatus::GRADIENT_NORM:
      out << "GRADIENT_NORM";
      break;
    case ConvergenceStatus::DX:
      out << "DX";
      break;
    case ConvergenceStatus::DOBJECTIVE:
      out << "DOBJECTIVE";
      break;
    case ConvergenceStatus::MAX_ITERATIONS:
      out << "MAX_ITERATIONS";
      break;
  }
  return out;
}

std::ostream& operator<<(std::ostream& out, const OptimizerStatus& ret)
{
  out << "OptimizerStatus: " << std::endl;
  out << "\tconvergence: " << ret.convergence << std::endl;
  out << "\titerations: " << ret.numIterations << std::endl;
  out << "\tgradient norm: " << ret.gradientNorm << std::endl;
  out << "\tobjective: " << ret.error << std::endl;
  out << "\tdobjective: " << ret.deltaError << std::endl;
  out << "\tmax dx: " << ret.maxDeltaX << std::endl;
  out << "\tevals objective: " << ret.numErrorEvaluations << std::endl;
  out << "\tevals derivative: " << ret.numJacobianEvaluations;
  return out;
}

OptimizerBase::OptimizerBase()
{
}

OptimizerBase::~OptimizerBase()
{
}

void OptimizerBase::optimize()
{
  if (!this->isInitialized())
    this->initialize();
  this->optimizeImplementation();
}

void OptimizerBase::initialize()
{
  this->initializeImplementation();
  this->reset();
}

void OptimizerBase::reset()
{
  this->status().reset();
  this->resetImplementation();
}

void OptimizerBase::updateConvergenceStatus()
{
  if (status().gradientNorm < getOptions().convergenceGradientNorm)
  {
    status().convergence = GRADIENT_NORM;
    return;
  }
  if (fabs(status().deltaError) < getOptions().convergenceDeltaError)
  {
    status().convergence = DOBJECTIVE;
    return;
  }
  if (status().maxDeltaX < getOptions().convergenceDeltaX)
  {
    status().convergence = DX;
    return;
  }
}

} /* namespace aslam */
} /* namespace backend */
