/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ItemDefinition.h - the definition of a value of an attribute definition.
// .SECTION Description
// ItemDefinition is meant to store definitions of values that can be
// stored inside of an attribute. Derived classes give specific
// types of items.
// .SECTION See Also

#ifndef __smtk_attribute_ItemDefinition_h
#define __smtk_attribute_ItemDefinition_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Item.h" // Needed for Item Types

#include <string>
#include <set>
#include <vector>

namespace smtk
{
  namespace attribute
  {
    class Attribute;
    class Item;
    class GroupItemDefinition;
    class SMTKCORE_EXPORT ItemDefinition
    {
      friend class smtk::attribute::Definition;
      friend class smtk::attribute::GroupItemDefinition;
    public:
      virtual ~ItemDefinition();
      const std::string &name() const
      { return this->m_name;}

      virtual Item::Type type() const = 0;
      // The label is what can be displayed in an application.  Unlike the type
      // which is constant w/r to the definition, an application can change the label
      const std::string &label() const
      { return this->m_label;}

      void setLabel(const std::string &newLabel)
      { this->m_label = newLabel;}

      int version() const
      {return this->m_version;}
      void setVersion(int myVersion)
      {this->m_version = myVersion;}

      bool isOptional() const
      { return this->m_isOptional;}

      void setIsOptional(bool isOptionalValue)
      { this->m_isOptional = isOptionalValue;}

      // This only comes into play if the item is optional
      bool isEnabledByDefault() const
      { return this->m_isEnabledByDefault;}

      void setIsEnabledByDefault(bool isEnabledByDefaultValue)
      { this->m_isEnabledByDefault = isEnabledByDefaultValue;}

      std::size_t numberOfCategories() const
      {return this->m_categories.size();}

      const std::set<std::string> & categories() const
      {return this->m_categories;}

      bool isMemberOf(const std::string &category) const
      { return (this->m_categories.find(category) != this->m_categories.end());}

      bool isMemberOf(const std::vector<std::string> &categories) const;

      virtual void addCategory(const std::string &category);

      virtual void removeCategory(const std::string &category);

      int advanceLevel() const
      {return this->m_advanceLevel;}
      void setAdvanceLevel(int level)
      {this->m_advanceLevel = level;}

      const std::string &detailedDescription() const
      {return this->m_detailedDescription;}
      void setDetailedDescription(const std::string &text)
        {this->m_detailedDescription = text;}

      const std::string &briefDescription() const
      {return this->m_briefDescription;}
      void setBriefDescription(const std::string &text)
        {this->m_briefDescription = text;}

      virtual smtk::AttributeItemPtr buildItem(Attribute *owningAttribute,
                                                int itemPosition) const = 0;
      virtual smtk::AttributeItemPtr buildItem(Item *owningItem,
                                                int position,
                                                int subGroupPosition) const = 0;
    protected:
      // The constructor must have the value for m_name passed
      // in because that should never change.
      ItemDefinition(const std::string &myname);
      virtual void updateCategories();
      int m_version;
      bool m_isOptional;
      bool m_isEnabledByDefault;
      std::string m_label;
      std::set<std::string> m_categories;
      int m_advanceLevel;
      std::string m_detailedDescription;
      std::string m_briefDescription;
    private:
      // constant value that should never be changed
      const std::string m_name;
    };
  }
}

#endif /* __smtk_attribute_ItemDefinition_h */
