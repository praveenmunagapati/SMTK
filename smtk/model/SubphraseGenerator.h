//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_SubphraseGenerator_h
#define __smtk_model_SubphraseGenerator_h

#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityListPhrase.h"
#include "smtk/model/EntityPhrase.h"
#include "smtk/model/MeshListPhrase.h"
#include "smtk/model/MeshPhrase.h"
#include "smtk/model/PropertyListPhrase.h"

#include "smtk/model/Manager.h" // For PropertyType enum.

namespace smtk
{
namespace model
{

class Instance;

/**\brief Generate subphrases to display for a given descriptive phrase.
  *
  * This abstract class should be subclassed by user interfaces
  * to provide control over what information is presented about a
  * given entity or set of entities.
  *
  * Its subphrases() method takes in a single phrase and
  * returns an ordered array of phrases.
  *
  * Besides this one pure virtual method, some utility routines
  * are provided to fetch phrases for common information.
  * Subclasses may use these in their implementations of subphrases().
  */
class SMTKCORE_EXPORT SubphraseGenerator : smtkEnableSharedPtr(SubphraseGenerator)
{
public:
  smtkTypeMacroBase(SubphraseGenerator);
  virtual ~SubphraseGenerator() {}

  virtual DescriptivePhrases subphrases(DescriptivePhrase::Ptr src);
  virtual int directLimit() const;
  virtual bool setDirectLimit(int val);
  virtual bool shouldOmitProperty(DescriptivePhrase::Ptr parent, smtk::resource::PropertyType ptype,
    const std::string& pname) const;

  virtual void setSkipProperties(bool val);
  virtual bool skipProperties() const;
  virtual void setSkipAttributes(bool val);
  virtual bool skipAttributes() const;

  virtual smtk::model::Model activeModel() const { return m_activeModel; }
  virtual void setActiveModel(const smtk::model::Model activeModel)
  {
    this->m_activeModel = activeModel;
  }

protected:
  SubphraseGenerator();

  void instancesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);
  void attributesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);
  void propertiesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);
  void floatPropertiesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);
  void stringPropertiesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);
  void integerPropertiesOfEntity(
    DescriptivePhrase::Ptr src, const EntityRef& ent, DescriptivePhrases& result);

  void cellOfUse(DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);
  void boundingShellsOfUse(
    DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);
  void toplevelShellsOfUse(
    DescriptivePhrase::Ptr src, const UseEntity& ent, DescriptivePhrases& result);

  void usesOfCell(DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);
  void inclusionsOfCell(
    DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);
  void boundingCellsOfCell(
    DescriptivePhrase::Ptr src, const CellEntity& ent, DescriptivePhrases& result);

  void usesOfShell(DescriptivePhrase::Ptr src, const ShellEntity& ent, DescriptivePhrases& result);

  void membersOfGroup(DescriptivePhrase::Ptr src, const Group& grp, DescriptivePhrases& result);

  void freeSubmodelsOfModel(
    DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);
  void freeGroupsInModel(DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);
  void freeCellsOfModel(DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);
  void freeAuxiliaryGeometriesOfModel(
    DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);

  void childrenOfAuxiliaryGeometry(
    DescriptivePhrase::Ptr src, const AuxiliaryGeometry& aux, DescriptivePhrases& result);

  void prototypeOfInstance(
    DescriptivePhrase::Ptr src, const Instance& ent, DescriptivePhrases& result);

  void modelsOfSession(
    DescriptivePhrase::Ptr src, const SessionRef& sess, DescriptivePhrases& result);

  void entitiesOfEntityList(
    EntityListPhrase::Ptr src, const EntityRefArray& ents, DescriptivePhrases& result);
  void propertiesOfPropertyList(
    PropertyListPhrase::Ptr src, smtk::resource::PropertyType p, DescriptivePhrases& result);

  void meshesOfModel(DescriptivePhrase::Ptr src, const Model& mod, DescriptivePhrases& result);
  void meshsetsOfMesh(MeshPhrase::Ptr meshphr, DescriptivePhrases& result);
  void meshesOfMeshList(MeshListPhrase::Ptr src, DescriptivePhrases& result);
  void meshsetsOfCollectionByDim(
    MeshPhrase::Ptr meshphr, smtk::mesh::DimensionType dim, DescriptivePhrases& result);

  void addEntityProperties(smtk::resource::PropertyType ptype, std::set<std::string>& props,
    DescriptivePhrase::Ptr parent, DescriptivePhrases& result);

  template <typename T>
  void addEntityPhrases(
    const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result);

  template <typename T>
  void addMeshPhrases(
    const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result);

  int m_directlimit;
  bool m_skipAttributes;
  bool m_skipProperties;
  smtk::model::Model m_activeModel;
};

template <typename T>
void SubphraseGenerator::addEntityPhrases(
  const T& ents, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result)
{
  if (limit < 0 || static_cast<int>(ents.size()) < limit)
  {
    for (typename T::const_iterator it = ents.begin(); it != ents.end(); ++it)
    {
      result.push_back(EntityPhrase::create()->setup(*it, parent));
    }
  }
  else
  {
    result.push_back(EntityListPhrase::create()->setup(ents, parent));
  }
}

/**\brief Add child MeshPhrases with an iterable container of meshes.
  *
  * This templated method is provided so that arrays of MeshSets or Collections
  * are both accepted.
  */

template <typename T>
void SubphraseGenerator::addMeshPhrases(
  const T& meshes, DescriptivePhrase::Ptr parent, int limit, DescriptivePhrases& result)
{
  if (limit < 0 || static_cast<int>(meshes.size()) < limit)
  {
    for (typename T::const_iterator it = meshes.begin(); it != meshes.end(); ++it)
    {
      result.push_back(MeshPhrase::create()->setup(*it, parent));
    }
  }
  else
  {
    result.push_back(MeshListPhrase::create()->setup(meshes, parent));
  }
}

} // namespace model
} // namespace smtk

#endif // __smtk_model_SubphraseGenerator_h
