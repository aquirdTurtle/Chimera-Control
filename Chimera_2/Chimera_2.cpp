#include "Chimera_2.h"
#include "stdafx.h"

Chimera_2::Chimera_2(QWidget *parent)
    : QMainWindow(parent){
	QCoreApplication::addLibraryPath ("./");

    ui.setupUi(this);
}

