// "$Date: 2015-03-17 09:01:20 -0300 (Ter, 17 Mar 2015) $"
// "$Author: fares $"
// "$Revision: 3359 $"

#include "QtSerialTest.h"
#include <QApplication>

#ifdef _WIN32
#	ifndef QT_DLL
#	include <QtPlugin>
	Q_IMPORT_PLUGIN(qsvg)
	// Q_IMPORT_PLUGIN(qico)
	// Q_IMPORT_PLUGIN(qsvgicon)
#	endif
#endif

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	a.setStyle ("Cleanlooks");
	QtSerialTest w(NULL);
	w.show();
	return a.exec();
}
