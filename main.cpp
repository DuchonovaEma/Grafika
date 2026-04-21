#include "ImageViewer.h"
#include <QtWidgets/QApplication>
#include "WingedEdge.h"

int main(int argc, char* argv[])
{
	QLocale::setDefault(QLocale::c());

	QCoreApplication::setOrganizationName("MPM");
	QCoreApplication::setApplicationName("ImageViewer");

	QApplication a(argc, argv);

	WingedEdge we;
	/*we.createCube(100.0);
	we.saveToVTK("kocka.vtk");

	printf("Kocka bola ulozena do suboru kocka.vtk\n");*/

	we.createUVSphere(100.0, 10, 10);
	we.saveToVTK("sphere.vtk");
	printf("sfera ulozena");

	ImageViewer w;
	w.show();
	return a.exec();
   
}