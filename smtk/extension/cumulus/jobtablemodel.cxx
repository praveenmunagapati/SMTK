//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "jobtablemodel.h"

#include "job.h"

#include <QtCore/QDebug>

#include <cstddef> // defines size_t for cJSON.h

#include "cJSON.h"

namespace cumulus
{

JobTableModel::JobTableModel(QObject* parentObject)
  : QAbstractTableModel(parentObject)
{
}

int JobTableModel::rowCount(const QModelIndex& modelIndex) const
{
  if (!modelIndex.isValid())
    return m_jobs.size();
  else
    return 0;
}

int JobTableModel::columnCount(const QModelIndex&) const
{
  return COLUMN_COUNT;
}

QVariant JobTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
      case JOB_ID:
        return QVariant("Job Id");
      case MACHINE:
        return QVariant("Machine");
      case JOB_NAME:
        return QVariant("Job Name");
      case JOB_STATUS:
        return QVariant("Status");
      default:
        return QVariant();
    }
  }

  return QVariant();
}

QVariant JobTableModel::data(const QModelIndex& modelIndex, int role) const
{
  if (!modelIndex.isValid() || modelIndex.column() + 1 > COLUMN_COUNT)
    return QVariant();

  Job job = m_jobs[modelIndex.row()];

  if (role == Qt::DisplayRole)
  {
    switch (modelIndex.column())
    {
      case JOB_ID:
        return QVariant(job.id());
      case MACHINE:
        return QVariant(job.machine());
      case JOB_NAME:
        return QVariant(job.name());
      case JOB_STATUS:
        return QVariant(job.status());
      default:
        return QVariant();
    }
  }
  else if (role == Qt::UserRole)
  {
    return QVariant::fromValue(job);
  }
  else
  {
    return QVariant();
  }
}

bool JobTableModel::removeRows(int row, int count, const QModelIndex&)
{
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  endRemoveRows();
  return true;
}

bool JobTableModel::insertRows(int row, int count, const QModelIndex&)
{
  beginInsertRows(QModelIndex(), row, row + count - 1);
  endInsertRows();
  return true;
}

void JobTableModel::jobsUpdated(QList<Job> jobs)
{
  QList<int> rowsToRemove;

  // Load map with input jobs
  QMap<QString, Job> inputMap;
  Job inputJob;
  foreach (inputJob, jobs)
  {
    inputMap.insert(inputJob.id(), inputJob);
  }

  // Traverse current jobs and update those found in inputMap
  for (int row = 0; row < m_jobs.size(); ++row)
  {
    Job modelJob = m_jobs[row];
    QString modelJobId = modelJob.id();

    // If row is not in the input, remember to remove it from the model
    if (!inputMap.contains(modelJobId))
    {
      rowsToRemove.push_back(row);
      continue;
    }

    // Get the input job; also remove it from the input map
    inputJob = inputMap[modelJobId];
    inputMap.remove(modelJobId);

    if (inputJob.status() == modelJob.status())
    {
      continue;
    }

    // Update status
    //qDebug() << "update status " << modelJobId << "to" << inputJob.status();
    modelJob.setStatus(inputJob.status());
    m_jobs[row] = modelJob;
    QModelIndex index = this->index(row, JOB_STATUS);
    emit this->dataChanged(index, index);
  } // for (row)

  // Remove any rows not in the input
  // (Do this in reverse order so that row numbers remain consistent)
  QListIterator<int> rowIter(rowsToRemove);
  rowIter.toBack();
  while (rowIter.hasPrevious())
  {
    int row = rowIter.previous();
    this->beginRemoveRows(QModelIndex(), row, row);
    m_jobs.removeAt(row);
    this->endRemoveRows();
  }

  // If inputMap is empty, then we are done
  if (inputMap.isEmpty())
  {
    return;
  }

  // (else) Insert new input jobs
  int first = m_jobs.size();
  int last = first + inputMap.size() - 1;
  this->beginInsertRows(QModelIndex(), first, last);

  QMap<QString, Job>::const_iterator iter = inputMap.constBegin();
  for (; iter != inputMap.constEnd(); ++iter)
  {
    Job newJob(iter.value());
    //qDebug() << "insert job:" << newJob.id();
    m_jobs.push_back(newJob);
  }

  this->endInsertRows();
}

} // end namespace
