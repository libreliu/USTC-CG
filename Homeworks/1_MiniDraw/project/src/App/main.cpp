#include "minidraw.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	
#if defined(_WIN32) || defined(_WIN64)
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

	QApplication a(argc, argv);
	MiniDraw w;
	w.show();
	return a.exec();
}
