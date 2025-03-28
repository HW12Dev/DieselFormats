#pragma once

#include "diesel/modern/bundle.h"

#include <qdialog.h>
#include <qlineedit.h>
#include <qgroupbox.h>
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>

#include <functional>

class BundleDBLoadingPanel : public QWidget {
public:
  BundleDBLoadingPanel(QWidget* parent);
  ~BundleDBLoadingPanel();

  std::function<void(diesel::modern::BundleDatabase*)> BundleDBLoaded;

public:
  diesel::modern::BundleDatabase* GetLoadedBundleDatabase();
private:
  diesel::modern::BundleDatabase* loadedBundleDatabase;

private:
  void LoadButtonPressed();
  void BrowsePathButtonPressed();

private:
  QVBoxLayout* mainLayout;

  QWidget* pathEntryWidget;
  QHBoxLayout* pathEntryLayout;
  QLineEdit* pathEntry;
  QPushButton* selectPathButton;

  QWidget* loadDetailsWidget;
  QHBoxLayout* loadDetailsLayout;

  QComboBox* gameVersionComboBox;
  QPushButton* loadButton;
};

class BundleDBLookup : public QDialog {
public:
  BundleDBLookup();
  ~BundleDBLookup();

  void closeEvent(QCloseEvent* event) override;
private:
  void BundleDBLoaded(diesel::modern::BundleDatabase* bdb);

  void UpdateIndexLookup();
  void UpdateFileNameLookup();

  diesel::modern::BundleDatabase* loadedBdb;
private:
  QVBoxLayout* mainLayout;

  QGroupBox* bdbLoadingGroupBox;
  QVBoxLayout* bdbLoadingGroupBoxLayout;
  
  BundleDBLoadingPanel* bdbLoadingPanel;

  QGroupBox* indexLookup;
  QVBoxLayout* indexLookupLayout;

  QLineEdit* indexLookupInput;

  QLineEdit* indexLookupFileOutput;
  QLineEdit* indexLookupMultiFileFileOutput;

  QGroupBox* fileNameLookup;
  QVBoxLayout* fileNameLookupLayout;

  QLineEdit* fileNameLookupInput;
  QLineEdit* fileNameLookupFileOutput;
  QLineEdit* fileNameLookupMultiFileFileOutput;
};