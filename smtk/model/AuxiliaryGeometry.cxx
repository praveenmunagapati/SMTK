//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/AuxiliaryGeometry.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/Manager.h"

namespace smtk
{
namespace model
{

bool AuxiliaryGeometry::reparent(const Model& m)
{
  if (!m.isValid() || this->owningModel() != m)
  {
    return false;
  }
  EntityRef immediateParent = this->embeddedIn();
  if (immediateParent == m)
  {
    return false;
  }

  immediateParent.unembedEntity(*this);
  m.as<Model>().addAuxiliaryGeometry(*this);
  return true;
}

bool AuxiliaryGeometry::reparent(const AuxiliaryGeometry& a)
{
  if (!a.isValid() || this->owningModel() != a.owningModel())
  {
    return false;
  }
  EntityRef immediateParent = this->embeddedIn();
  if (immediateParent == a)
  {
    return false;
  }

  immediateParent.unembedEntity(*this);
  a.as<AuxiliaryGeometry>().embedEntity(*this);
  return true;
}

/**\brief Indicate whether the entity has a URL to external geometry.
  *
  */
bool AuxiliaryGeometry::hasURL() const
{
  if (!this->hasStringProperties())
  {
    return false;
  }
  const StringData& sprops(this->stringProperties());
  return (sprops.find("url") != sprops.end());
}

/**\brief Return the URL to external geometry if any exists, and an empty string otherwise.
  *
  * Note that only the first string (in an array of strings that may be held) is returned.
  */
std::string AuxiliaryGeometry::url() const
{
  if (!this->hasStringProperties())
  {
    return std::string();
  }
  const StringData& sprops(this->stringProperties());
  StringData::const_iterator url = sprops.find("url");
  if (url == sprops.end() || url->second.empty())
  {
    return std::string();
  }
  return url->second[0];
}

/**\brief Set the URL of external geometry referenced by this auxiliary geometry.
  *
  */
void AuxiliaryGeometry::setURL(const std::string& url)
{
  this->setStringProperty("url", url);
}

bool AuxiliaryGeometry::isModified() const
{
  smtk::model::Manager::Ptr mgr = this->manager();
  if (!mgr || !this->hasIntegerProperty("clean"))
  {
    return false;
  }

  const IntegerList& prop(this->integerProperty("clean"));
  return (!prop.empty() && (prop[0] != 1));
}

void AuxiliaryGeometry::setIsModified(bool isModified)
{
  smtk::model::Manager::Ptr mgr = this->manager();
  if (!mgr)
  {
    return;
  }

  this->setIntegerProperty("clean", isModified ? 0 : 1);
}

AuxiliaryGeometries AuxiliaryGeometry::auxiliaryGeometries() const
{
  return this->embeddedEntities<AuxiliaryGeometries>();
}

} // namespace model
} // namespace smtk
