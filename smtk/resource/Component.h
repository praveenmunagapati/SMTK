//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Component.h - Abstract base class for SMTK Resource Components
// .SECTION Description
//   A SMTK Component is a peice of a Resource such as an Attribute, ModelEntity,
//  or MeshEntity
// .SECTION See Also

#ifndef smtk_resource_Component_h
#define smtk_resource_Component_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include <string>

namespace smtk
{
namespace resource
{

class SMTKCORE_EXPORT Component : smtkEnableSharedPtr(Component)
{
  friend class Resource;

public:
  smtkTypeMacroBase(Component);
  virtual ~Component();
  virtual const ResourcePtr resource() const = 0;

  virtual common::UUID id() const = 0;

protected:
  Component(const common::UUID& myID);
  Component();
  virtual void setId(const common::UUID& myID) = 0;

private:
  common::UUID m_id;
};
}
}

#endif // smtk_resource_Component_h
