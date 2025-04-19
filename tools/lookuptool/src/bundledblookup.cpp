#include "bundledblookup.h"
#include "lookuptooltrayicon.h"

#include "diesel/modern/modern_shared.h"
#include "diesel/modern/hashlist.h"

#include <qfile>
#include <qfiledialog>
#include <qcloseevent>

extern LookupToolTrayIcon* GetTrayIcon();

BundleDBLoadingPanel::BundleDBLoadingPanel(QWidget* parent) : QWidget(parent) {
  this->mainLayout = new QVBoxLayout(this);
  this->setLayout(mainLayout);

  this->loadedBundleDatabase = nullptr;

  {
    this->pathEntryWidget = new QWidget(this);

    this->pathEntryLayout = new QHBoxLayout(pathEntryWidget);
    this->pathEntryWidget->setLayout(pathEntryLayout);

    this->pathEntry = new QLineEdit(pathEntryWidget);
    this->selectPathButton = new QPushButton("...", pathEntryWidget);

    this->pathEntryLayout->addWidget(pathEntry, Qt::AlignLeft);
    this->pathEntryLayout->addWidget(selectPathButton);

    this->mainLayout->addWidget(this->pathEntryWidget, 0, Qt::AlignTop);

    this->loadDetailsWidget = new QWidget(this);
    this->loadDetailsLayout = new QHBoxLayout(this->loadDetailsWidget);
    this->loadDetailsWidget->setLayout(this->loadDetailsLayout);

    this->gameVersionComboBox = new QComboBox(loadDetailsWidget);

    this->gameVersionComboBox->addItem("PAYDAY: The Heist V1", QVariant((diesel::EngineVersionBaseType)diesel::EngineVersion::PAYDAY_THE_HEIST_V1));
    this->gameVersionComboBox->addItem("PAYDAY: The Heist", QVariant((diesel::EngineVersionBaseType)diesel::EngineVersion::PAYDAY_THE_HEIST_LATEST));
    this->gameVersionComboBox->addItem("PAYDAY: 2", QVariant((diesel::EngineVersionBaseType)diesel::EngineVersion::PAYDAY_2_LATEST));
    this->gameVersionComboBox->addItem("PAYDAY: 2 Linux", QVariant((diesel::EngineVersionBaseType)diesel::EngineVersion::PAYDAY_2_LINUX_LATEST));
    this->gameVersionComboBox->addItem("RAID: World War II", QVariant((diesel::EngineVersionBaseType)diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST));
    this->gameVersionComboBox->setCurrentIndex(4);

    this->loadButton = new QPushButton("Load", loadDetailsWidget);

    this->loadDetailsLayout->addWidget(this->gameVersionComboBox, 0, Qt::AlignLeft);
    this->loadDetailsLayout->addWidget(this->loadButton);

    connect(this->selectPathButton, &QPushButton::pressed, [this]() { this->BrowsePathButtonPressed(); });
    connect(this->loadButton, &QPushButton::pressed, [this]() { this->LoadButtonPressed(); });

    this->mainLayout->addWidget(this->loadDetailsWidget);
  }
}

BundleDBLoadingPanel::~BundleDBLoadingPanel() {
  if (this->loadedBundleDatabase)
    delete this->loadedBundleDatabase;
}

diesel::modern::BundleDatabase* BundleDBLoadingPanel::GetLoadedBundleDatabase() {
  return this->loadedBundleDatabase;
}

void BundleDBLoadingPanel::LoadButtonPressed() {
  diesel::EngineVersion version = (diesel::EngineVersion)this->gameVersionComboBox->currentData().toInt();

  auto path = this->pathEntry->text();
  if (QFile(path).exists()) {

    Reader bdbReader(path.toStdWString());

    diesel::DieselFormatsLoadingParameters loadParams = diesel::DieselFormatsLoadingParameters();
    loadParams.version = version;
    loadParams.sourcePlatform = (version == diesel::EngineVersion::RAID_WORLD_WAR_II_LATEST ? diesel::FileSourcePlatform::WINDOWS_64 : (version == diesel::EngineVersion::PAYDAY_2_LINUX_LATEST) ? diesel::FileSourcePlatform::LINUX_64 : diesel::FileSourcePlatform::WINDOWS_32);
    this->loadedBundleDatabase = new diesel::modern::BundleDatabase(bdbReader, version);

    bdbReader.Close();

    BundleDBLoaded(loadedBundleDatabase);
  }
}

void BundleDBLoadingPanel::BrowsePathButtonPressed() {
  QFileDialog fileDialog(this, "Select a Bundle Database file", "", "Diesel Bundle Database (*.blb)");

  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setFileMode(QFileDialog::ExistingFile);

  if (fileDialog.exec()) {
    QString selectedFile = fileDialog.selectedFiles()[0];
    this->pathEntry->setText(selectedFile);
  }
}

BundleDBLookup::BundleDBLookup() {
  this->setWindowTitle("Bundle Database Lookup");
  this->setWindowIcon(QIcon("./icon-white.png"));

  this->loadedBdb = nullptr;
  this->BundleDBLoaded(nullptr);

  this->mainLayout = new QVBoxLayout(this);
  this->setLayout(this->mainLayout);

  this->bdbLoadingGroupBox = new QGroupBox("Load Bundle Database", this);
  this->bdbLoadingGroupBoxLayout = new QVBoxLayout(this->bdbLoadingGroupBox);
  this->bdbLoadingGroupBox->setLayout(this->bdbLoadingGroupBoxLayout);

  this->mainLayout->addWidget(this->bdbLoadingGroupBox);

  this->bdbLoadingPanel = new BundleDBLoadingPanel(this->bdbLoadingGroupBox);
  this->bdbLoadingGroupBoxLayout->addWidget(bdbLoadingPanel);

  bdbLoadingPanel->BundleDBLoaded = [this](diesel::modern::BundleDatabase* bdb) {this->BundleDBLoaded(bdb); };

  this->fileNameLookup = new QGroupBox("File Lookup", this);
  this->fileNameLookupLayout = new QVBoxLayout(this->fileNameLookup);
  this->fileNameLookup->setLayout(this->fileNameLookupLayout);
  this->mainLayout->addWidget(this->fileNameLookup);


  this->fileNameLookupInput = new QLineEdit(this->fileNameLookup);
  this->fileNameLookupFileOutput = new QLineEdit(this->fileNameLookup);
  this->fileNameLookupFileOutput->setReadOnly(true);
  this->fileNameLookupMultiFileFileOutput = new QLineEdit(this->fileNameLookup);
  this->fileNameLookupMultiFileFileOutput->setReadOnly(true);

  this->fileNameLookupLayout->addWidget(fileNameLookupInput);
  this->fileNameLookupLayout->addSpacing(5);
  this->fileNameLookupLayout->addWidget(fileNameLookupFileOutput);
  this->fileNameLookupLayout->addWidget(fileNameLookupMultiFileFileOutput);


  this->indexLookup = new QGroupBox("Index Lookup", this);
  this->indexLookupLayout = new QVBoxLayout(this->indexLookup);
  this->indexLookup->setLayout(this->indexLookupLayout);
  this->mainLayout->addWidget(this->indexLookup);

  this->indexLookupInput = new QLineEdit(this->indexLookup);
  this->indexLookupFileOutput = new QLineEdit(this->indexLookup);
  this->indexLookupFileOutput->setReadOnly(true);
  this->indexLookupMultiFileFileOutput = new QLineEdit(this->indexLookup);
  this->indexLookupMultiFileFileOutput->setReadOnly(true);

  this->indexLookupLayout->addWidget(indexLookupInput);
  this->indexLookupLayout->addSpacing(5);
  this->indexLookupLayout->addWidget(indexLookupFileOutput);
  this->indexLookupLayout->addWidget(indexLookupMultiFileFileOutput);

  connect(this->fileNameLookupInput, &QLineEdit::textChanged, [this](const QString& str) {this->UpdateFileNameLookup(); });
  connect(this->indexLookupInput, &QLineEdit::textChanged, [this](const QString& str) {this->UpdateIndexLookup(); });
}

BundleDBLookup::~BundleDBLookup() {
  if (this->loadedBdb)
    delete this->loadedBdb;
  delete this->bdbLoadingPanel;
}

void BundleDBLookup::closeEvent(QCloseEvent* event) {
  if (!event->spontaneous() || !this->isVisible())
    return;
  if (GetTrayIcon()->isVisible()) {
    this->hide();
    event->ignore();
  }
}

void BundleDBLookup::BundleDBLoaded(diesel::modern::BundleDatabase* bdb) {
  if (this->loadedBdb) {
    delete this->loadedBdb;
    this->loadedBdb = nullptr;
  }
  this->loadedBdb = bdb;
  this->UpdateIndexLookup();
  this->UpdateFileNameLookup();
}

void BundleDBLookup::UpdateIndexLookup() {
  if (!this->loadedBdb)
    return;

  auto dbKeyTxt = indexLookupInput->text();

  try {
    auto dbKey = std::stoul(dbKeyTxt.toStdString(), 0, 10);

    auto entry = this->loadedBdb->GetLookupInformationFromDBKey(dbKey);

    std::string type;
    std::string name;
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry._type, type);
    diesel::modern::GetGlobalHashlist()->FindSourceForIdstring(entry._name, name);

    std::string fullPath = name + "." + type;

    std::string multiFilePath = "all" + std::to_string(dbKey % 512) + "/" + std::to_string(dbKey);

    this->indexLookupFileOutput->setText(fullPath.c_str());
    this->indexLookupMultiFileFileOutput->setText(multiFilePath.c_str());

  } catch(std::exception& e) {}
}

void BundleDBLookup::UpdateFileNameLookup() {
  if (!this->loadedBdb)
    return;

  QString fullFileName = fileNameLookupInput->text();

  if (fullFileName.split(".").length() < 2)
    return;

  try {
    QString name = fullFileName.split(".")[0];
    QString type = fullFileName.split(".")[1];

    auto dbKey = this->loadedBdb->GetDBKeyFromTypeAndName(type.toStdString(), name.toStdString());

    std::string multiFilePath = "all" + std::to_string(dbKey % 512) + "/" + std::to_string(dbKey);
    fileNameLookupFileOutput->setText(std::to_string(dbKey).c_str());
    fileNameLookupMultiFileFileOutput->setText(multiFilePath.c_str());
  }
  catch (std::exception& e) {}
}
