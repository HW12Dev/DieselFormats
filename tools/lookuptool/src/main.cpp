#include <QApplication>

#include <qtenvironmentvariables.h>

#include "window.h"

#include "lookuptooltrayicon.h"
#include "bundledblookup.h"
#include "idstringlookup.h"

#include "diesel/modern/hashlist.h"

#include <filesystem>

LookupToolTrayIcon* g_trayIcon;
LookupToolTrayIcon* GetTrayIcon() {
  return g_trayIcon;
}

int main(int argc, char* argv[]);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  return main(0, nullptr);
}
int main(int argc, char* argv[]) {
  qputenv("QT_QPA_PLATFORM", "windows:darkmode=2");
  QApplication app(argc, argv);
  app.setStyle("fusion");

  QApplication::setQuitOnLastWindowClosed(false);

  //Reader hashlist("X:\\Projects\\DieselEngineExplorer\\hashlist.txt");
  if(std::filesystem::exists("./hashlist.txt")) {
    Reader hashlist("./hashlist.txt");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
    hashlist.Close();
  } else if (std::filesystem::exists("./hashlist")) {
    Reader hashlist("./hashlist");
    diesel::modern::GetGlobalHashlist()->ReadFileToHashlist(hashlist);
    hashlist.Close();
  }


  IdstringLookup idstring;
  BundleDBLookup bundledbLookup;
  //bundledbLookup.show();
  //idstring.show();

  LookupToolTrayIcon* trayIcon = new LookupToolTrayIcon(&idstring, &bundledbLookup);
  g_trayIcon = trayIcon;
  trayIcon->show();

  //Window window;
  //window.show();
  int exec = app.exec();

  trayIcon->hide();

  delete trayIcon;

  return exec;
}