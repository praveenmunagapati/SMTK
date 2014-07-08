/*=========================================================================

Copyright (c) 1998-2014 Kitware Inc. 28 Corporate Drive,
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
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME ResourceSetWriter.h - Writer for SMTK resource files
// .SECTION Description
// .SECTION See Also

#ifndef __ResourceSetWriter_h
#define __ResourceSetWriter_h

#include "smtk/SMTKCoreExports.h"
#include "smtk/util/Logger.h"
#include "smtk/util/ResourceSet.h"
#include "smtk/util/SystemConfig.h"
#include "smtk/PublicPointerDefs.h"

#include <string>

namespace smtk {
  namespace util {

class SMTKCORE_EXPORT ResourceSetWriter
{
 public:
  bool writeFile(std::string filename, const ResourceSet& resources,
                 smtk::util::Logger& logger, bool writeLinkedFiles = true);
  bool writeString(std::string& content, const ResourceSet& resources,
                   smtk::util::Logger& logger, bool writeLinkedFiles = true);

 protected:
};

  }  // namespace util
}  // namespace smtk

#endif /* __ResourceSetWriter_h */