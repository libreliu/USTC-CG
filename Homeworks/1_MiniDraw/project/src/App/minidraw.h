#ifndef MINIDRAW_H
#define MINIDRAW_H

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>
#include <ui_minidraw.h>
#include <viewwidget.h>
#include <QtWidgets/QLabel>


class MiniDraw : public QMainWindow
{
	Q_OBJECT

public:
	MiniDraw(QWidget *parent = 0);
	~MiniDraw();

	void Init();
	void AboutBox();

	// maintain statusbar messages
	void putMessage(const std::string& msg);
	void setShapeStatus(DrawContext::ShapeManager::ShapeType shape);
	void setModeStatus(ViewWidget::mode mode);
	void setCursorStatus(int x, int y);

	ViewWidget::mode modeVal;
	DrawContext::ShapeManager::ShapeType shapeVal;

private:
	void updateSBarIndicator(void);

	Ui::MiniDrawClass ui;
	ViewWidget*	view_widget_;
	QLabel* sbar_perm_indicator;

	int cursor_x;
	int cursor_y;
};

#endif // MINIDRAW_H
