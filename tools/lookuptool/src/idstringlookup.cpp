#include "idstringlookup.h"
#include "lookuptooltrayicon.h"

#include <qcloseevent>
#include "diesel/modern/hashlist.h"

extern LookupToolTrayIcon* GetTrayIcon();

IdstringLookup::IdstringLookup() {
  this->setWindowTitle("Idstring Lookup");

  this->setWindowIcon(QIcon("./icon-white.png"));

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

  connect(this->swapByteOrderCB, &QCheckBox::checkStateChanged, this, &IdstringLookup::SwapByteOrderCheckStateChanged);
  connect(this->hexCB, &QCheckBox::checkStateChanged, this, &IdstringLookup::SwapByteOrderCheckStateChanged);
  connect(this->hashInput, &QLineEdit::textChanged, this, &IdstringLookup::HashInputChanged);

}

IdstringLookup::~IdstringLookup() {
  delete this->findSourceGBLayout;
  delete this->findSourceGB;

  delete this->mainLayout;
}

void IdstringLookup::setVisible(bool visible) {
  QDialog::setVisible(visible);
}

void IdstringLookup::closeEvent(QCloseEvent* event) {
  if (!event->spontaneous() || !this->isVisible())
    return;
  if (GetTrayIcon()->isVisible()) {
    this->hide();
    event->ignore();
  }
}

void IdstringLookup::SwapByteOrderCheckStateChanged(Qt::CheckState) {
  this->HashInputChanged(this->hashInput->text());
}

void IdstringLookup::HashInputChanged(const QString& str) {
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
  }
  catch (std::exception& e) {}
}

void IdstringLookup::TrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::ActivationReason::Trigger)
    this->showNormal();
}
