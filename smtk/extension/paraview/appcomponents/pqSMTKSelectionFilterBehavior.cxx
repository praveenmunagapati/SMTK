//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionFilterBehavior.h"

// SMTK
#include "smtk/resource/SelectionManager.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Model.h"
#include "smtk/model/Vertex.h"
#include "smtk/model/Volume.h"

#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

// Client side
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqPVApplicationCore.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"

// Server side
#include "vtkPVSelectionSource.h"
#include "vtkSMSourceProxy.h"

// VTK
#include "vtkAbstractArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

// Qt
#include <QAction>
#include <QWidget>

// Qt generated UI
#include "ui_pqSMTKSelectionFilterBehavior.h"

using namespace smtk;

#define NUM_ACTIONS 7

static pqSMTKSelectionFilterBehavior* s_selectionFilter = nullptr;

class pqSMTKSelectionFilterBehavior::pqInternal
{
public:
  Ui::pqSMTKSelectionFilterBehavior Actions;
  QAction* ActionArray[NUM_ACTIONS];
  QWidget ActionsOwner;
};

pqSMTKSelectionFilterBehavior::pqSMTKSelectionFilterBehavior(
  QObject* parent, smtk::resource::SelectionManagerPtr mgr)
  : Superclass(parent)
  , m_selectionManager(mgr)
{
  if (!mgr)
  {
    m_selectionManager = smtk::resource::SelectionManager::create();
  }
  m_p = new pqInternal;
  m_p->Actions.setupUi(&m_p->ActionsOwner);

  m_p->ActionArray[0] = m_p->Actions.actionSelnAcceptMeshSets;
  m_p->ActionArray[1] = m_p->Actions.actionSelnAcceptModels;
  m_p->ActionArray[2] = m_p->Actions.actionSelnAcceptModelVolumes;
  m_p->ActionArray[3] = m_p->Actions.actionSelnAcceptModelFaces;
  m_p->ActionArray[4] = m_p->Actions.actionSelnAcceptModelEdges;
  m_p->ActionArray[5] = m_p->Actions.actionSelnAcceptModelVertices;
  m_p->ActionArray[6] = m_p->Actions.actionSelnAcceptModelAuxGeoms;

  if (!s_selectionFilter)
  {
    s_selectionFilter = this;
  }

  for (int ii = 0; ii < NUM_ACTIONS; ++ii)
  {
    this->addAction(m_p->ActionArray[ii]);
  }
  this->setExclusive(false);
  // By default, all the buttons are off. Set some for the initial filter settings:
  m_p->Actions.actionSelnAcceptModelVertices->setChecked(true);
  m_p->Actions.actionSelnAcceptModelEdges->setChecked(true);
  m_p->Actions.actionSelnAcceptModelFaces->setChecked(true);
  m_p->Actions.actionSelnAcceptModelAuxGeoms->setChecked(true);
  // Now force the initial filter to get installed on the selection manager:
  this->onFilterChanged(m_p->Actions.actionSelnAcceptModelVertices);

  QObject::connect(this, SIGNAL(triggered(QAction*)), this, SLOT(onFilterChanged(QAction*)));
}

pqSMTKSelectionFilterBehavior::~pqSMTKSelectionFilterBehavior()
{
  if (s_selectionFilter == this)
  {
    s_selectionFilter = nullptr;
  }
}

pqSMTKSelectionFilterBehavior* pqSMTKSelectionFilterBehavior::instance()
{
  return s_selectionFilter;
}

void pqSMTKSelectionFilterBehavior::setSelectionManager(smtk::resource::SelectionManagerPtr selnMgr)
{
  m_selectionManager = selnMgr;
}

void pqSMTKSelectionFilterBehavior::onFilterChanged(QAction* a)
{
  // Update all action toggle states to be consistent
  bool acceptMesh = false;
  smtk::model::BitFlags modelFlags = 0;
  if ((a == m_p->Actions.actionSelnAcceptMeshSets && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModels && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelVolumes && a->isChecked()))
  { // model-volume and mesh-set selection do not allow other types to be selected at the same time.
    acceptMesh = m_p->Actions.actionSelnAcceptMeshSets->isChecked();
    // Force other model entity buttons off:
    for (int ii = 0; ii < NUM_ACTIONS; ++ii)
    {
      m_p->ActionArray[ii]->setChecked((a != m_p->ActionArray[ii]) ? false : true);
    }
    modelFlags =
      (m_p->Actions.actionSelnAcceptModels->isChecked() ? smtk::model::MODEL_ENTITY : 0) |
      (m_p->Actions.actionSelnAcceptModelVolumes->isChecked() ? smtk::model::VOLUME : 0);
  }
  else if ((a == m_p->Actions.actionSelnAcceptModelVertices && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelEdges && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelFaces && a->isChecked()) ||
    (a == m_p->Actions.actionSelnAcceptModelAuxGeoms && a->isChecked()))
  {
    m_p->Actions.actionSelnAcceptMeshSets->setChecked(false);
    m_p->Actions.actionSelnAcceptModels->setChecked(false);
    m_p->Actions.actionSelnAcceptModelVolumes->setChecked(false);
    acceptMesh = false;
    modelFlags =
      (m_p->Actions.actionSelnAcceptModelVertices->isChecked() ? smtk::model::VERTEX : 0) |
      (m_p->Actions.actionSelnAcceptModelEdges->isChecked() ? smtk::model::EDGE : 0) |
      (m_p->Actions.actionSelnAcceptModelFaces->isChecked() ? smtk::model::FACE : 0) |
      (m_p->Actions.actionSelnAcceptModelAuxGeoms->isChecked() ? smtk::model::AUX_GEOM_ENTITY : 0);
  }
  else
  { // Something was turned off, which will not require us to deactivate any other buttons
    acceptMesh = m_p->Actions.actionSelnAcceptMeshSets->isChecked();
    modelFlags =
      (m_p->Actions.actionSelnAcceptModels->isChecked() ? smtk::model::MODEL_ENTITY : 0) |
      (m_p->Actions.actionSelnAcceptModelVolumes->isChecked() ? smtk::model::VOLUME : 0) |
      (m_p->Actions.actionSelnAcceptModelVertices->isChecked() ? smtk::model::VERTEX : 0) |
      (m_p->Actions.actionSelnAcceptModelEdges->isChecked() ? smtk::model::EDGE : 0) |
      (m_p->Actions.actionSelnAcceptModelFaces->isChecked() ? smtk::model::FACE : 0) |
      (m_p->Actions.actionSelnAcceptModelAuxGeoms->isChecked() ? smtk::model::AUX_GEOM_ENTITY : 0);
  }
  // Rebuild the selection filter
  m_selectionManager->setFilter([acceptMesh, modelFlags](smtk::resource::ComponentPtr comp,
    int value, smtk::resource::SelectionManager::SelectionMap& suggestions) {
    (void)acceptMesh; // meshes are not yet resource components.
    if (modelFlags)
    {
      auto modelEnt = dynamic_pointer_cast<smtk::model::Entity>(comp);
      if (!modelEnt)
      {
        return false;
      }
      auto entBits = modelEnt->entityFlags();
      // Is the entity of an acceptable type?
      if ((modelFlags & smtk::model::MODEL_ENTITY) == smtk::model::MODEL_ENTITY)
      {
        smtk::model::EntityRef eref(modelEnt->modelResource(), modelEnt->id());
        smtk::model::Model model = eref.owningModel();
        smtk::model::EntityPtr suggestion;
        if (model.isValid(&suggestion))
        {
          suggestions.insert(std::make_pair(suggestion, value));
        }
      }
      else if ((modelFlags & smtk::model::VOLUME) == smtk::model::VOLUME)
      {
        smtk::model::CellEntity cell(modelEnt->modelResource(), modelEnt->id());
        smtk::model::Volumes vv;
        if (cell.isValid())
        {
          switch (cell.dimension())
          {
            case 0:
            {
              smtk::model::Edges edges = cell.as<smtk::model::Vertex>().edges();
              for (auto edge : edges)
              {
                smtk::model::Faces faces = edge.faces();
                for (auto face : faces)
                {
                  smtk::model::Volumes tmp = face.volumes();
                  vv.insert(vv.end(), tmp.begin(), tmp.end());
                }
              }
            }
            break;
            case 1:
            {
              smtk::model::Faces faces = cell.as<smtk::model::Edge>().faces();
              for (auto face : faces)
              {
                smtk::model::Volumes tmp = face.volumes();
                vv.insert(vv.end(), tmp.begin(), tmp.end());
              }
            }
            break;
            case 2:
              vv = cell.as<smtk::model::Face>().volumes();
              break;
            default:
              break;
          }
        }
        smtk::model::EntityPtr suggestion;
        for (auto vc : vv)
        {
          if (vc.isValid(&suggestion))
          {
            suggestions.insert(std::make_pair(suggestion, value));
          }
        }
      }
      else if ((entBits & smtk::model::ENTITY_MASK) & (modelFlags & smtk::model::ENTITY_MASK))
      {
        // Ensure the dimension is acceptable, too:
        return ((entBits & smtk::model::AUX_GEOM_ENTITY) ||
                 ((entBits & smtk::model::ANY_DIMENSION) &
                   (modelFlags & smtk::model::ANY_DIMENSION)))
          ? true
          : false;
      }
    }
    return false;
  });
}
