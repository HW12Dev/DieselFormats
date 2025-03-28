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

class IdstringLookup : public QDialog {
public:
  IdstringLookup();
  ~IdstringLookup();


  void setVisible(bool visible) override;
  void closeEvent(QCloseEvent* event) override;
  void TrayIconActivated(QSystemTrayIcon::ActivationReason reason);
private:
  void SwapByteOrderCheckStateChanged(Qt::CheckState);
  void HashInputChanged(const QString& str);

  QVBoxLayout* mainLayout;
  QGroupBox* findSourceGB;
  QVBoxLayout* findSourceGBLayout;
  QLineEdit* hashInput;

  QCheckBox* swapByteOrderCB;
  QCheckBox* hexCB;

  QLineEdit* hashlistFound;
};