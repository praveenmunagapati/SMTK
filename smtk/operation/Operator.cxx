//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Operator.h"

#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/UUID.h"

#include "cJSON.h"

#include <sstream>

using smtk::attribute::IntItem;
using smtk::attribute::IntItemPtr;
using smtk::attribute::DoubleItem;
using smtk::attribute::StringItem;
using smtk::attribute::FileItem;
using smtk::attribute::DirectoryItem;
using smtk::attribute::GroupItem;
using smtk::attribute::RefItem;
using smtk::attribute::ModelEntityItem;
using smtk::attribute::MeshSelectionItem;
using smtk::attribute::MeshItem;
using smtk::attribute::VoidItem;

namespace smtk
{
namespace operation
{

Operator::Operator()
{
  this->m_debugLevel = 0;
}

Operator::~Operator()
{
}

/**\brief Return whether the operator's inputs are well-defined.
  *
  * This returns true when the Operator considers its inputs to
  * be valid and false otherwise.
  *
  * Subclasses may override this method.
  * By default, it returns true when this->specification()->isValid()
  * returns true.
  */
bool Operator::ableToOperate()
{
  return this->specification()->isValid();
}

/**\brief Perform the operation the subclass implements.
  *
  * This method first tests whether the operation is well-defined by
  * invoking ableToOperate(). If it returns true, then the
  * operateInternal() method (implemented by subclasses) is invoked.
  *
  * You may register callbacks to observe how the operation is
  * proceeding: you can be signaled when the operation is about
  * to be executed and just after it does execute. Neither will
  * be called if the ableToOperate method returns false.
  */
Operator::Result Operator::operate()
{
  // Remember where the log was so we only serialize messages for this operation:
  std::size_t logStart = this->log().numberOfRecords();

  Operator::Result result;
  if (this->ableToOperate())
  {
    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->specification()->findInt("debug level");
    this->m_debugLevel = (debugItem->isEnabled() ? debugItem->value() : 0);
    // Run the operation if possible:
    if (!this->trigger(WILL_OPERATE))
    {
      result = this->operateInternal();
    }
    else
    {
      result = this->createResult(OPERATION_CANCELED);
    }

    this->generateSummary(result);

    // Now grab all log messages and serialize them into the result attribute.
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    { // Serialize relevant log records to JSON.
      cJSON* array = cJSON_CreateArray();
      smtk::io::SaveJSON::forLog(array, this->log(), logStart, logEnd);
      char* logstr = cJSON_Print(array);
      cJSON_Delete(array);
      result->findString("log")->appendValue(logstr);
      free(logstr);
    }
    // Inform observers that the operation completed.
    this->trigger(DID_OPERATE, result);
  }
  else
  {
    // Do not inform observers since this is currently a non-event.
    result = this->createResult(UNABLE_TO_OPERATE);
    // Now grab all log messages and serialize them into the result attribute.
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    { // Serialize relevant log records to JSON.
      cJSON* array = cJSON_CreateArray();
      smtk::io::SaveJSON::forLog(array, this->log(), logStart, logEnd);
      char* logstr = cJSON_Print(array);
      cJSON_Delete(array);
      result->findString("log")->appendValue(logstr);
      free(logstr);
    }
  }
  return result;
}

/// Add an observer of WILL_OPERATE events on this operator.
void Operator::observe(EventType event, Callback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.insert(std::make_pair(functionHandle, callData));
}

/// Add an observer of DID_OPERATE events on this operator.
void Operator::observe(EventType event, CallbackWithResult functionHandle, void* callData)
{
  (void)event;
  this->m_didOperateTriggers.insert(std::make_pair(functionHandle, callData));
}

/// Remove an existing WILL_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(EventType event, Callback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.erase(std::make_pair(functionHandle, callData));
}

/// Remove an existing DID_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(EventType event, CallbackWithResult functionHandle, void* callData)
{
  (void)event;
  this->m_didOperateTriggers.erase(std::make_pair(functionHandle, callData));
}

/**\brief Invoke all WILL_OPERATE observer callbacks.
  *
  * The return value is non-zero if the operation was canceled and zero otherwise.
  * Note that all observers will be called even if one requests the operation be
  * canceled. This is useful since all DID_OPERATE observers are called whether
  * the operation was canceled or not -- and observers of both will expect them
  * to be called in pairs.
  */
int Operator::trigger(EventType event)
{
  int status = 0;
  std::set<Observer>::const_iterator it;
  for (it = this->m_willOperateTriggers.begin(); it != this->m_willOperateTriggers.end(); ++it)
    status |= (*it->first)(event, *this, it->second);
  return status;
}

/// Invoke all DID_OPERATE observer callbacks. The return value is always 0 (this may change in future releases).
int Operator::trigger(EventType event, const Operator::Result& result)
{
  std::set<ObserverWithResult>::const_iterator it;
  for (it = this->m_didOperateTriggers.begin(); it != this->m_didOperateTriggers.end(); ++it)
    (*it->first)(event, *this, result, it->second);
  return 0;
}

/**\brief Return the definition of this operation and its parameters.
  *
  * The OperatorDefinition is a typedef to smtk::attribute::Definition
  * so that applications can automatically-generate a user interface
  * for accepting parameter values.
  *
  * However, be aware that the attribute manager used for this
  * specification is owned by the SMTK's model manager and
  * operators are not required to have a valid manager() at all times.
  * This method will return a null pointer if there is no manager.
  * Otherwise, it will ask the session and model manager for its
  * definition.
  */
Operator::Definition Operator::definition() const
{
  // Manager::Ptr mgr = this->manager();
  // SessionPtr brg = this->session();
  // if (!mgr || !brg)
  //   return attribute::DefinitionPtr();

  // return brg->operatorCollection()->findDefinition(this->name());
  return Operator::Definition();
}

/**\brief Return the specification of this operator (creating one if none exists).
  *
  * The specification of an operator includes values for each of
  * the operator's parameters as necessary to carry out the operation.
  * These values are encoded in an attribute whose definition is
  * provided by the operator (see smtk::model::Operator::definition()).
  * Note that OperatorSpecification is a typedef of
  * smtk::attribute::AttributePtr.
  *
  * The specification is initially a null attribute pointer
  * but is initialized when calling this method or
  * by calling ensureSpecification().
  *
  * If the operator is invoked without a specification, one
  * is created (holding default values).
  */
Operator::Specification Operator::specification() const
{
  this->ensureSpecification();
  return this->m_specification;
}

/**\brief Set the specification of the operator's parameters.
  *
  * The attribute, if non-NULL, should match the definition()
  * of the operator.
  */
bool Operator::setSpecification(attribute::AttributePtr spec)
{
  if (spec == this->m_specification)
  {
    return false;
  }

  if (spec)
  {
    if (!spec->isA(this->definition()))
    {
      return false;
    }
  }

  this->m_specification = spec;
  return true;
}

/**\brief Ensure that a specification exists for this operator.
  *
  * Returns true when a specification was created or already existed
  * and false upon error (e.g., when the session was not set or
  * no definition exists for this operator's name).
  */
bool Operator::ensureSpecification() const
{
  if (this->m_specification)
    return true;

  return false;
}

/**\brief Create an attribute representing this operator's result type.
  *
  * The default \a outcome is UNABLE_TO_OPERATE.
  */
Operator::Result Operator::createResult(Operator::Outcome outcome)
{
  (void)outcome;
  return Operator::Result();
}

/**\brief Set the outcome of the given result.
  *
  * This is a convenience method.
  */
void Operator::setResultOutcome(Operator::Result res, Operator::Outcome outcome)
{
  IntItemPtr outcomeItem = smtk::dynamic_pointer_cast<IntItem>(res->find("outcome"));
  outcomeItem->setValue(outcome);
}

/// A comparator so that Operators may be placed in ordered sets.
bool Operator::operator<(const Operator& other) const
{
  return this->name() < other.name();
}

/**\brief Remove an attribute from the operator's manager.
  *
  * This is a convenience method to remove an operator's result
  * when you are done examining it.
  *
  * When operating in client-server mode, it is possible for
  * result instances on the client and server to have name
  * collisions unless you manage their lifespans by removing
  * them as they are consumed by your application.
  */
void Operator::eraseResult(Operator::Result)
{
}

void Operator::generateSummary(Operator::Result& res)
{
  std::ostringstream msg;
  int outcome = res->findInt("outcome")->value();
  msg << this->specification()->definition()->label() << ": " << outcomeAsString(outcome);
  if (outcome == static_cast<int>(OPERATION_SUCCEEDED))
  {
    smtkInfoMacro(this->log(), msg.str());
  }
  else
  {
    smtkErrorMacro(this->log(), msg.str());
  }
}

/// Return a string summarizing the outcome of an operation.
std::string outcomeAsString(int oc)
{
  switch (oc)
  {
    case Operator::UNABLE_TO_OPERATE:
      return "unable to operate";
    case Operator::OPERATION_CANCELED:
      return "operation canceled";
    case Operator::OPERATION_FAILED:
      return "operation failed";
    case Operator::OPERATION_SUCCEEDED:
      return "operation succeeded";
    case Operator::OUTCOME_UNKNOWN:
      break;
  }
  return "outcome unknown";
}

/// Given a string summarizing the outcome of an operation, return an enumerant.
Operator::Outcome stringToOutcome(const std::string& oc)
{
  if (oc == "unable to operate")
    return Operator::UNABLE_TO_OPERATE;
  if (oc == "operation canceled")
    return Operator::OPERATION_CANCELED;
  if (oc == "operation failed")
    return Operator::OPERATION_FAILED;
  if (oc == "operation succeeded")
    return Operator::OPERATION_SUCCEEDED;

  return Operator::OUTCOME_UNKNOWN;
}

} // model namespace
} // smtk namespace
