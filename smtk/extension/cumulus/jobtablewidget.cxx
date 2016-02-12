
#include "ui_jobtablewidget.h"

#include "jobtablewidget.h"
#include "jobtablemodel.h"

#include <QtGui/QMessageBox>

namespace cumulus
{

JobTableWidget::JobTableWidget(QWidget *parentObject):
  QWidget(parentObject),
  ui(new Ui::JobTableWidget)
{
  ui->setupUi(this);

  ui->table->setSortingEnabled(true);
}

JobTableWidget::~JobTableWidget()
{
  delete ui;
}

void JobTableWidget::setModel(QAbstractItemModel *model)
{
  ui->table->setModel(model);

  ui->table->horizontalHeader()
      ->setResizeMode(QHeaderView::Stretch);

}

} // end namespace
