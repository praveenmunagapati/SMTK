//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_SharedFromThis_h
#define __smtk_SharedFromThis_h
/*! \file SharedFromThis.h
 *  \brief Macros for dealing with shared-pointer classes.
 */

#include "smtk/SharedPtr.h"

#include <string>

/**\brief Add typedefs to a class for identifcation.
  *
  * This macro takes a single parameter naming the
  * class in which the macro is placed. It defines
  * the following types inside the class scope:
  * + `SelfType` (the type of the class itself),
  * + `Ptr` (a shared pointer to an instance of the class), and
  * + `ConstPtr` (a shared pointer to a constant instance of the class).
  * + `WeakPtr` (a weak pointer to an instance of the class), and
  * + `WeakConstPtr` (a weak pointer to a constant instance of the class).
  *
  * Use it like so:<pre>
  * class X
  * {
  * public:
  *   smtkTypeMacro(X);
  *   ...
  * };
  * </pre>
  *
  * These types are useful when dealing with shared pointers
  * in a class hierarchy.
  */
#define smtkTypedefs(...)                                                                          \
  typedef __VA_ARGS__ SelfType;                                                                    \
  typedef smtk::shared_ptr<__VA_ARGS__> Ptr;                                                       \
  typedef smtk::shared_ptr<const __VA_ARGS__> ConstPtr;                                            \
  typedef smtk::weak_ptr<__VA_ARGS__> WeakPtr;                                                     \
  typedef smtk::weak_ptr<const __VA_ARGS__> WeakConstPtr
#define smtkTypeMacro(...)                                                                         \
  smtkTypedefs(__VA_ARGS__);                                                                       \
  std::string classname() const override { return #__VA_ARGS__; }
#define smtkTypeMacroBase(...)                                                                     \
  smtkTypedefs(__VA_ARGS__);                                                                       \
  virtual std::string classname() const { return #__VA_ARGS__; }

/**\brief Add a typedef to the superclass of this class.
  *
  * This adds typedefs named Superclass and SuperclassPtr
  * (i.e., a shared pointer to the superclass) to the
  * class.
  *
  * Unlike VTK's type macro, it is separate from smtkTypeMacro
  * in order to support classes with multiple template
  * parameters, since preprocessor macros do not properly
  * handle commas inside template parameter lists.
  */
#define smtkSuperclassMacro(...)                                                                   \
  typedef __VA_ARGS__ Superclass;                                                                  \
  typedef smtk::shared_ptr<__VA_ARGS__> SuperclassPtr

/**\brief Add static create() methods to a class.
  *
  * This macro takes a single parameter naming either
  * the class of interest (if no ancestor classes use
  * enable_shared_from_this() or smtk::EnableSharedPtr())
  * or the first -- and only -- ancestor class that inherits
  * enable_shared_from_this() or smtk::EnableSharedPtr().
  *
  * This macro also requires the use of smtkTypeMacro()
  * as it needs SelfType defined.
  *
  * Two static class functions are declared: both return
  * a shared pointer to a newly created class instance
  * but one takes no arguments and the other takes one.
  * The 1-argument version also sets the passed shared
  * pointer to refer to the newly created instance.
  * This is useful in declarative APIs for referring
  * back to an instance created as part of a statement
  * whose value is transformed before the returned value
  * can be assigned to a variable.
  */
#define smtkCreateMacro(...)                                                                       \
  static smtk::shared_ptr<SelfType> create()                                                       \
  {                                                                                                \
    smtk::shared_ptr<__VA_ARGS__> shared(new SelfType);                                            \
    return smtk::static_pointer_cast<SelfType>(shared);                                            \
  }                                                                                                \
  /* variant for declarative programming: */                                                       \
  static smtk::shared_ptr<SelfType> create(smtk::shared_ptr<SelfType>& ref)                        \
  {                                                                                                \
    ref = SelfType::create();                                                                      \
    return ref;                                                                                    \
  }

/**\brief An abbreviation for enabling shared pointers.
  *
  * Use like so:<pre>
  * class X : smtkEnableSharedPtr(X)
  * {
  * public:
  *   smtkTypeMacro(X);
  *   smtkCreateMacro(X);
  * };
  * </pre>
  * Note that this may be complicated on some systems by the
  * C preprocessor's inability to handle macros whose arguments
  * include a multiple-parameter template. (The comma separating
  * the template parameters is taken as an additional macro
  * argument.)
  * However, in general shared pointers will be enabled on
  * non-templated base classes from which templated classes may
  * be derived.
  *
  * It is recommended that you make constructors protected or private
  * to avoid heap allocation of objects that may return shared pointers.
  *
  * For non-abstract classes, it is recommended that you
  * call smtkCreateMacro() as a safe way to expose public construction.
  */
#define smtkEnableSharedPtr(...)                                                                   \
public                                                                                             \
  smtk::enable_shared_from_this<__VA_ARGS__>

/**\brief A macro to help with derived classes whose bases enable shared_from_this().
  *
  * Use like so:<pre>
  * class X : smtkEnableSharedPtr(X)
  * {
  * public:
  *   smtkTypeMacro(X);
  * };
  *
  * class Y : public X
  * {
  * public:
  *   smtkTypeMacro(Y);
  *   smtkSharedFromThisMacro(X);
  *   ...
  *   Y::Ptr method()
  *     {
  *     return shared_from_this();
  *     }
  * };
  * </pre>
  *
  * Note that the macro argument is the <b>base class</b>
  * on which shared pointers are enabled (or another
  * inherited class in between which also defines a
  * shared_from_this() method).
  *
  * This macro implements a shared_from_this() method
  * in the derived class that returns a shared pointer
  * of the proper type.
  */
#define smtkSharedFromThisMacro(...)                                                               \
  typedef __VA_ARGS__ SharedPtrBaseType;                                                           \
  smtk::shared_ptr<SelfType> shared_from_this()                                                    \
  {                                                                                                \
    return smtk::static_pointer_cast<SelfType>(SharedPtrBaseType::shared_from_this());             \
  }                                                                                                \
  smtk::shared_ptr<const SelfType> shared_from_this() const                                        \
  {                                                                                                \
    return smtk::static_pointer_cast<const SelfType>(SharedPtrBaseType::shared_from_this());       \
  }

/// A convenience macro for declaring shared_from_this and create methods.
#define smtkSharedPtrCreateMacro(...)                                                              \
  smtkSharedFromThisMacro(__VA_ARGS__);                                                            \
  smtkCreateMacro(__VA_ARGS__)

/**\brief A convenience macro to use in the body of create methods that take arguments.
  *
  * This macro acts like a function that takes a pointer
  * to an instance of your class and returns a shared pointer
  * to the instance, even when shared_from_this is inherited
  * from a base class. For example: <pre>
  * class Y : smtkEnabledSharedPtr(X)
  * {
  * public:
  *   smtkTypeMacro(Y);
  *   static Ptr create(int a, double b);
  * protected:
  *   Y(int a, double b);
  * };
  *
  * Y::Ptr create(int a, double b)
  * {
  *   return smtkSharedPtrHelper(
  *     new Y(a, b));
  * }
  * </pre>
  *
  * It is important to use this method in classes derived
  * from those that use smtkEnabledSharedPtr rather than
  * naively constructing a shared pointer of the proper type,
  * since that will likely result in an exception being thrown;
  * the path must be from Y* to shared_ptr<X> to shared_ptr<Y>,
  * not straight from Y* to shared_ptr<Y>, or the pointer will
  * be inserted into the wrong pool of shared pointers leading
  * to premature deletion or multiple-deletion.
  */
#define smtkSharedPtrHelper(...)                                                                   \
  smtk::static_pointer_cast<SelfType>(SharedPtrBaseType::Ptr(__VA_ARGS__))

#endif // __smtk_SharedFromThis_h
