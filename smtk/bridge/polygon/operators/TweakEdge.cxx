//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/polygon/operators/TweakEdge.h"

#include "smtk/bridge/polygon/Session.h"
#include "smtk/bridge/polygon/internal/Model.h"
#include "smtk/bridge/polygon/internal/Model.txx"

#include "smtk/io/Logger.h"

#include "smtk/model/Face.h"
#include "smtk/model/Vertex.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/bridge/polygon/TweakEdge_xml.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

smtk::model::OperatorResult TweakEdge::operateInternal()
{
  smtk::bridge::polygon::SessionPtr sess = this->polygonSession();
  smtk::model::Manager::Ptr mgr;
  if (!sess)
  {
    smtkErrorMacro(this->log(), "Invalid session.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  mgr = sess->manager();

  smtk::attribute::ModelEntityItem::Ptr edgeItem = this->specification()->associations();
  smtk::attribute::DoubleItem::Ptr pointsItem = this->findDouble("points");
  smtk::attribute::IntItem::Ptr coordinatesItem = this->findInt("coordinates");
  smtk::attribute::IntItem::Ptr promoteItem = this->findInt("promote");

  smtk::model::Edge src(edgeItem->value(0));
  if (!src.isValid())
  {
    smtkErrorMacro(this->log(), "Input edge was invalid.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  internal::edge::Ptr storage = this->findStorage<internal::edge>(src.entity());
  internal::pmodel* pmod = storage->parentAs<internal::pmodel>();
  if (!storage || !pmod)
  {
    smtkErrorMacro(
      this->log(), "Input edge was not part of its parent model (or not a polygon-session edge).");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  bool ok = true;
  std::set<int> splits(promoteItem->begin(), promoteItem->end());
  int numCoordsPerPt = coordinatesItem->value(0);
  std::size_t npts = pointsItem->numberOfValues() / numCoordsPerPt; // == #pts / #coordsPerPt
  if (npts < 2)
  {
    smtkErrorMacro(this->log(), "Not enough points to form an edge ("
        << pointsItem->numberOfValues() << " coordinates at " << numCoordsPerPt << " per point => "
        << npts << " points)");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  if (!splits.empty())
  {
    std::set<int>::iterator sit;
    std::ostringstream derp;
    bool bad = false;
    derp << "Ignoring invalid split-point indices: ";
    for (sit = splits.begin(); sit != splits.end() && *sit < 0; ++sit)
    {
      derp << " " << *sit;
      std::set<int>::iterator tmp = sit;
      ++sit;
      splits.erase(tmp);
      bad = true;
      --sit;
    }
    std::set<int>::reverse_iterator rsit;
    for (rsit = splits.rbegin(); rsit != splits.rend() && *rsit >= static_cast<int>(npts); ++rsit)
    {
      derp << " " << *rsit;
      std::set<int>::iterator tmp = --rsit.base();
      ++rsit;
      splits.erase(tmp);
      bad = true;
      --rsit;
    }
    if (bad)
    {
      smtkWarningMacro(this->log(), derp.str());
    }
  }

  smtk::model::EntityRefArray modified; // includes edge and perhaps eventually faces.
  smtk::model::Edges edgeCreated;
  smtk::model::Vertices verticesCreated;

  // Done checking input. Perform operation.
  ok &= pmod->tweakEdge(src, numCoordsPerPt, pointsItem->begin(), pointsItem->end(), modified);
  // Split the edge as requested by the user:
  std::vector<internal::vertex::Ptr> promotedVerts;
  std::vector<internal::PointSeq::const_iterator> splitLocs;
  internal::PointSeq& epts(storage->points());
  internal::PointSeq::const_iterator ptit = epts.begin();
  int last = 0;
  for (std::set<int>::iterator promit = splits.begin(); promit != splits.end(); ++promit)
  {
    std::advance(ptit, *promit - last);
    last = *promit;
    if (!!pmod->pointId(*ptit))
    {
      continue; // skip points that are already model vertices (should only happen at start/end)
    }
    smtk::model::Vertex pv = pmod->findOrAddModelVertex(mgr, *ptit);
    verticesCreated.push_back(pv);

    promotedVerts.push_back(this->findStorage<internal::vertex>(pv.entity()));
    splitLocs.push_back(ptit);
    std::cout << "  " << ptit->x() << " " << ptit->y() << "\n";
  }
  smtk::model::EntityRefArray edgesAdded;
  smtk::model::EntityRefArray expunged;
  if (!splitLocs.empty())
  {
    smtkInfoMacro(this->log(), "Splitting tweaked edge at " << splitLocs.size() << " places.");
    if (!pmod->splitModelEdgeAtModelVertices(
          mgr, storage, promotedVerts, splitLocs, edgesAdded, this->m_debugLevel))
    {
      smtkErrorMacro(this->log(), "Could not split edge.");
      ok = false;
    }
    if (!edgesAdded.empty())
    {
      expunged.push_back(src);
    }
    edgeCreated.insert(edgeCreated.end(), edgesAdded.begin(), edgesAdded.end());
  }

  if (this->m_debugLevel > 0)
  {
    for (smtk::model::Edges::iterator eCrit = edgeCreated.begin(); eCrit != edgeCreated.end();
         ++eCrit)
    {
      smtkOpDebug("Created " << eCrit->name() << ".");
    }
    for (auto vCrit = verticesCreated.begin(); vCrit != verticesCreated.end(); ++vCrit)
    {
      smtkOpDebug("Created " << vCrit->name() << ".");
    }

    for (smtk::model::EntityRefArray::iterator moit = modified.begin(); moit != modified.end();
         ++moit)
    {
      smtkOpDebug("Modified " << moit->name() << ".");
    }

    for (smtk::model::EntityRefArray::iterator epit = expunged.begin(); epit != expunged.end();
         ++epit)
    {
      smtkOpDebug("Expunged " << epit->name() << ".");
    }
  }

  smtk::model::OperatorResult opResult;
  if (ok)
  {
    opResult = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
    this->addEntitiesToResult(opResult, expunged, EXPUNGED);
    this->addEntitiesToResult(opResult, edgeCreated, CREATED);
    this->addEntitiesToResult(opResult, verticesCreated, CREATED);
    this->addEntitiesToResult(opResult, modified, MODIFIED);

    // Modified items will have new tessellations, which we must indicate
    // separately for the time being.
    attribute::ModelEntityItemPtr tessItem = opResult->findModelEntity("tess_changed");
    tessItem->appendValues(modified.begin(), modified.end());
  }
  else
  {
    opResult = this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  return opResult;
}

} // namespace polygon
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKPOLYGONSESSION_EXPORT, smtk::bridge::polygon::TweakEdge,
  polygon_tweak_edge, "tweak edge", TweakEdge_xml, smtk::bridge::polygon::Session);
