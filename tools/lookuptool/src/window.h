#pragma once

#include <qsystemtrayicon.h>
#include <qdialog.h>
#include <qmenu.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qboxlayout.h>
#include <qgroupbox.h>
#include <qmainwindow.h>
#include <qcheckbox.h>


class Window : public QDialog {
  //Q_OBJECT

public:
  Window();
  ~Window();

  void setVisible(bool visible) override;
  void closeEvent(QCloseEvent* event) override;

private:
  QSystemTrayIcon* trayIcon;
  QMenu* trayIconMenu;

  QIcon trayIconI;

  QAction* showAction;
  QAction* quitAction;

  void SwapByteOrderCheckStateChanged(Qt::CheckState);
  void HashInputChanged(const QString& str);
  void TrayIconActivated(QSystemTrayIcon::ActivationReason reason);

  QVBoxLayout* mainLayout;
  QGroupBox* findSourceGB;
  QVBoxLayout* findSourceGBLayout;
  QLineEdit* hashInput;

  QCheckBox* swapByteOrderCB;
  QCheckBox* hexCB;

  QLineEdit* hashlistFound;
};