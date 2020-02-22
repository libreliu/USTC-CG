#include "minidraw.h"

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget();
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();

	setCentralWidget(view_widget_);
}

void MiniDraw::Creat_Action()
{
	Action_About = new QAction(tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

	Action_Rect = new QAction(tr("&Rect"), this);
	connect(Action_Rect, SIGNAL(triggered()), view_widget_, SLOT(setRect()));

	Action_Freehand = new QAction(tr("&Freehand"), this);
	connect(Action_Freehand, SIGNAL(triggered()), view_widget_, SLOT(setFreehand()));

	Action_Ellipse = new QAction(tr("&Ellipse"), this);
	connect(Action_Ellipse, SIGNAL(triggered()), view_widget_, SLOT(setEllipse()));

	Action_Polygon = new QAction(tr("&Polygon"), this);
	connect(Action_Polygon, &QAction::triggered, view_widget_, &ViewWidget::setPolygon);
}

void MiniDraw::Creat_ToolBar()
{
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->addAction(Action_About);
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Freehand);
	pToolBar->addAction(Action_Ellipse);
	pToolBar->addAction(Action_Polygon);
}

void MiniDraw::Creat_Menu()
{
	pMenu = menuBar()->addMenu(tr("&Figure Tool"));
	pMenu->addAction(Action_About);
	pMenu->addAction(Action_Line);
	pMenu->addAction(Action_Rect);
	pMenu->addAction(Action_Freehand);
	pMenu->addAction(Action_Ellipse);
	pMenu->addAction(Action_Polygon);
}

void MiniDraw::AboutBox()
{
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

MiniDraw::~MiniDraw()
{
	delete view_widget_;
}
