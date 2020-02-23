#include "minidraw.h"

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	/* if we assign a widget as this widget's parent, deconstr will be 
	 * called once parent deconstructs.
	 */
	//view_widget_ = new ViewWidget(this);
	//Init();

	//setCentralWidget(view_widget_);
}

//QAction* actionView;
//QAction* actionDrag_Shape;
//QAction* actionDrag_Point;
//QAction* actionAdd_Shape;
//QAction* actionLine;
//QAction* actionRect;
//QAction* actionPoly;
//QAction* actionCircle;
//QAction* actionFreehand;
//QAction* actionHelp;
//QAction* actionAbout;
//QAction* actionRaw_Mode;

void MiniDraw::Init()
{
	connect(ui.actionAbout, &QAction::triggered, this, &MiniDraw::AboutBox);
	connect(ui.actionView, &QAction::triggered, this, [=]() {
		this->view_widget_->setMode(ViewWidget::mode::VIEW); });

	//pToolBar = addToolBar(tr("&Main"));
	//pMenu = menuBar()->addMenu(tr("&Figure Tool"));
}

void MiniDraw::AboutBox()
{
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

MiniDraw::~MiniDraw()
{
	//delete view_widget_;
}
