#pragma once

#include <qsystemtrayicon.h>
#include <qaction.h>
#include <qmenu.h>
#include <qobject.h>

class LookupToolTrayIcon : public QSystemTrayIcon {
public:
  LookupToolTrayIcon(QDialog* idstringLookup, QDialog* bundleDBLookup);
  ~LookupToolTrayIcon();
private:
  QMenu* trayIconMenu;

  QAction* showIdstringLookupAction;
  QAction* showBundleDBLookupAction;
  QAction* quitAction;
};