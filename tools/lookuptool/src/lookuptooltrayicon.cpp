#include "lookuptooltrayicon.h"

#include "idstringlookup.h"

#include <qapplication.h>
#include <qdialog.h>

LookupToolTrayIcon::LookupToolTrayIcon(QDialog* idstringLookup, QDialog* bundleDBLookup) : QSystemTrayIcon(nullptr) {
  this->showIdstringLookupAction = new QAction("Show Idstring Lookup", this);
  this->showBundleDBLookupAction = new QAction("Show Bundle Database Lookup", this);
  this->quitAction = new QAction("Quit", this);

  this->trayIconMenu = new QMenu();

  trayIconMenu->addAction(showIdstringLookupAction);
  trayIconMenu->addAction(showBundleDBLookupAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  this->setContextMenu(trayIconMenu);

  this->setIcon(QIcon("./icon-white.png"));

  this->setToolTip("Idstring Lookup");

  connect(this->quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
  connect(this->showIdstringLookupAction, &QAction::triggered, idstringLookup, &QWidget::showNormal);
  connect(this->showBundleDBLookupAction, &QAction::triggered, bundleDBLookup, &QWidget::showNormal);

  connect(this, &QSystemTrayIcon::activated, (IdstringLookup*)idstringLookup, &IdstringLookup::TrayIconActivated);
  this->show();
}

LookupToolTrayIcon::~LookupToolTrayIcon() {
  delete this->trayIconMenu;
}
