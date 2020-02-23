#include "minidraw.h"

MiniDraw::MiniDraw(QWidget *parent) : QMainWindow(parent) {
  ui.setupUi(this);
  /* if we assign a widget as this widget's parent, deconstr will be
   * called once parent deconstructs.
   */
  view_widget_ = new ViewWidget(this);
  Init();

  setCentralWidget(view_widget_);
}

// QAction* actionView;
// QAction* actionDrag_Shape;
// QAction* actionDrag_Point;
// QAction* actionAdd_Shape;
// QAction* actionLine;
// QAction* actionRect;
// QAction* actionPoly;
// QAction* actionEllipse;
// QAction* actionFreehand;
// QAction* actionHelp;
// QAction* actionAbout;
// QAction* actionRaw_Mode;

void MiniDraw::putMessage(const QString msg)
{
    statusBar()->showMessage(msg);
}

// ViewWidget is held responsible for the logic here
void MiniDraw::setShapeStatus(DrawContext::ShapeManager::ShapeType shape) {
	shapeVal = shape;
	updateSBarIndicator();
}
void MiniDraw::setModeStatus(ViewWidget::mode mode) {
	modeVal = mode;
	updateSBarIndicator();
}

void MiniDraw::Init() {
  connect(ui.actionAbout, &QAction::triggered, this, &MiniDraw::AboutBox);

  connect(ui.actionView, &QAction::triggered, this,
          [=]() { this->view_widget_->setMode(ViewWidget::mode::VIEW); });

  connect(ui.actionDrag_Shape, &QAction::triggered, this,
          [=]() { this->view_widget_->setMode(ViewWidget::mode::DRAG_WHOLE); });

  connect(ui.actionDrag_Point, &QAction::triggered, this,
          [=]() { this->view_widget_->setMode(ViewWidget::mode::DRAG_POINT); });

  connect(ui.actionAdd_Shape, &QAction::triggered, this,
          [=]() { this->view_widget_->setMode(ViewWidget::mode::ADD_POINT); });

  connect(ui.actionRaw_Mode, &QAction::triggered, this,
          [=]() { this->view_widget_->setMode(ViewWidget::mode::RAW); });

  connect(ui.actionLine, &QAction::triggered, this, [=]() {
    this->view_widget_->setShape(DrawContext::ShapeManager::ShapeType::Line);
  });

  connect(ui.actionRect, &QAction::triggered, this, [=]() {
    this->view_widget_->setShape(DrawContext::ShapeManager::ShapeType::Rect);
  });

  connect(ui.actionPoly, &QAction::triggered, this, [=]() {
    this->view_widget_->setShape(DrawContext::ShapeManager::ShapeType::Polygon);
  });

  connect(ui.actionEllipse, &QAction::triggered, this, [=]() {
    this->view_widget_->setShape(
        DrawContext::ShapeManager::ShapeType::Ellipse);
  });

  connect(ui.actionFreehand, &QAction::triggered, this, [=]() {
  	this->view_widget_->setShape(DrawContext::ShapeManager::ShapeType::Freehand);
  });

  // add permanent widget
  sbar_perm_indicator = new QLabel(this);
  statusBar()->addPermanentWidget(sbar_perm_indicator);

  // set default status
  this->view_widget_->setMode(ViewWidget::mode::VIEW);
  this->view_widget_->setShape(DrawContext::ShapeManager::ShapeType::Line);
  
  statusBar()->showMessage(tr("Ready"));
}

void MiniDraw::updateSBarIndicator(void) {
  QString modeStr;
  QString shapeStr;
  QString cursorStr;
  cursorStr = QString("(%1, %2)").arg(cursor_x).arg(cursor_y);

  switch (modeVal) {
  case ViewWidget::mode::VIEW:
    modeStr = tr("VIEW");
    break;
  case ViewWidget::mode::ADD_POINT:
    modeStr = tr("ADDPNT");
    break;
  case ViewWidget::mode::DRAG_POINT:
    modeStr = tr("DRAGPNT");
    break;
  case ViewWidget::mode::DRAG_WHOLE:
    modeStr = tr("DRAGWHL");
    break;
  case ViewWidget::mode::RAW:
    modeStr = tr("RAW");
    break;
  default:
    modeStr = tr("ERR");
  }

  switch (shapeVal) {
  case DrawContext::ShapeManager::ShapeType::Circle:
    shapeStr = tr("CIRC");
    break;
  case DrawContext::ShapeManager::ShapeType::Ellipse:
    shapeStr = tr("ELPS");
    break;
  case DrawContext::ShapeManager::ShapeType::Freehand:
    shapeStr = tr("FREEHND");
    break;
  case DrawContext::ShapeManager::ShapeType::Line:
    shapeStr = tr("LINE");
    break;
  case DrawContext::ShapeManager::ShapeType::Point:
    shapeStr = tr("PNT");
    break;
  case DrawContext::ShapeManager::ShapeType::Polygon:
    shapeStr = tr("POLY");
    break;
  case DrawContext::ShapeManager::ShapeType::Rect:
    shapeStr = tr("RECT");
    break;
  default:
    shapeStr = tr("ERR");
    break;
  }

  sbar_perm_indicator->setText(cursorStr + " | " + modeStr + " | " + shapeStr);
}

void MiniDraw::AboutBox() {
  QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

MiniDraw::~MiniDraw() {
  // no need to do this - handled by Qt automatically
  // delete view_widget_;
}

void MiniDraw::setCursorStatus(int x, int y) {
	cursor_x = x;
	cursor_y = y;
	updateSBarIndicator();
}