
#include "qthread_main.h"


int main(int argc, char* argv[]) {
	//preview(argc, argv);
	//record(argc, argv);
	QApplication app(argc, argv);
	MyThread mt;
	mt.start();
	return app.exec();
}

