#include "window.h"

#include <qcloseevent>
#include "diesel/modern/hashlist.h"

Window::Window() {
  this->showAction = new QAction("Show", this);
  this->quitAction = new QAction("Quit", this);

  this->trayIconMenu = new QMenu(this);

  trayIconMenu->addAction(showAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  this->trayIcon = new QSystemTrayIcon(this);
  this->trayIcon->setContextMenu(trayIconMenu);

  this->trayIconI = QIcon("X:\\Projects\\DieselEngineExplorer\\stringserver\\icon-white.png");
  //this->setWindowIcon(QIcon("X:\\Projects\\DieselEngineExplorer\\stringserver\\icon.png"));
  this->setWindowIcon(trayIconI);
  this->trayIcon->setIcon(trayIconI);


  this->setWindowTitle("Idstring Lookup");
  this->trayIcon->setToolTip("Idstring Lookup");

  this->trayIcon->show();
  

  this->mainLayout = new QVBoxLayout(this);
  this->setLayout(this->mainLayout);


  this->findSourceGB = new QGroupBox("Find Source", this);
  this->findSourceGBLayout = new QVBoxLayout(this->findSourceGB);
  this->findSourceGB->setLayout(this->findSourceGBLayout);

  this->mainLayout->addWidget(this->findSourceGB, 0, Qt::AlignTop);

  this->hashInput = new QLineEdit(this->findSourceGB);
  this->findSourceGBLayout->addWidget(this->hashInput);

  this->hexCB = new QCheckBox("Hex", this->findSourceGB);
  this->hexCB->setCheckState(Qt::CheckState::Checked);
  this->findSourceGBLayout->addWidget(this->hexCB);

  this->swapByteOrderCB = new QCheckBox("Swap Endianness", this->findSourceGB);
  this->findSourceGBLayout->addWidget(this->swapByteOrderCB);
  

  this->hashlistFound = new QLineEdit(this->findSourceGB);
  this->hashlistFound->setReadOnly(true);
  this->findSourceGBLayout->addWidget(this->hashlistFound);

  connect(this->swapByteOrderCB, &QCheckBox::checkStateChanged, this, &Window::SwapByteOrderCheckStateChanged);
  connect(this->hexCB, &QCheckBox::checkStateChanged, this, &Window::SwapByteOrderCheckStateChanged);
  connect(this->hashInput, &QLineEdit::textChanged, this, &Window::HashInputChanged);

  connect(this->showAction, &QAction::triggered, this, &QWidget::showNormal);
  connect(this->quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

  connect(this->trayIcon, &QSystemTrayIcon::activated, this, &Window::TrayIconActivated);


}

Window::~Window() {
  this->trayIcon->hide();

  delete this->showAction;
  delete this->quitAction;
  delete this->trayIconMenu;
  delete this->trayIcon;

  delete this->findSourceGBLayout;
  delete this->findSourceGB;

  delete this->mainLayout;
}

void Window::setVisible(bool visible) {
  QDialog::setVisible(visible);
}

void Window::closeEvent(QCloseEvent* event) {
  if (!event->spontaneous() || !this->isVisible())
    return;
  if (trayIcon->isVisible()) {
    this->hide();
    event->ignore();
  }
}

void Window::SwapByteOrderCheckStateChanged(Qt::CheckState) {
  this->HashInputChanged(this->hashInput->text());
}

void Window::HashInputChanged(const QString& str) {
  if (str == "")
    return;

  QString cpy = QString(str).replace(" ", "");

  std::string hash = cpy.toStdString();

  int base = 10;

  if (this->hexCB->checkState() == Qt::CheckState::Checked) {
    if (!hash.starts_with("0x"))
      hash = "0x" + hash;
    base = 16;
  }

  std::string found;

  try {
    auto hashUll = std::stoull(hash, nullptr, base);

    if (this->swapByteOrderCB->checkState() == Qt::CheckState::Checked)
      hashUll = _byteswap_uint64(hashUll);

    diesel::modern::Idstring idstr(hashUll);

    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(idstr, found);

    this->hashlistFound->setText(QString(found.c_str()));
  } catch (std::exception& e){}
}

void Window::TrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::ActivationReason::Trigger)
    this->showNormal();
}
