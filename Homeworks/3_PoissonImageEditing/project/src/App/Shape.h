#pragma once

#include <vector>
#include <Eigen/Dense>
#include <QtGui>
#include <QRect>

/* The Shape class now contains every control point a object is required to display
 * Also a dedicated method for painting the points is given
 * Adding points requires upper logic, and using these points owes to its child
 */

namespace DrawContext {

	class Shape
	{
	public:

		Shape();
		virtual ~Shape();
		virtual void Draw(QPainter& paint) = 0;

		// Draw relative to bounding box
		// virtual void DrawRelative(QPainter& paint);

		// (0,0)----> width(x)
		//   |
		//   |
		//  height(y)
		QRect getBoundingRect();

		// Get a matrix [w * h], with 1 for points inside
		virtual const Eigen::Matrix<int, -1, -1> &getMaskMatrix() = 0;

		/* Draw control points, with lines and "+" mark */
		void DrawCtrlPoints(QPainter& paint);

		/* Attr to maintain and decouple messages */
		void setAttr(const std::string& s, int a);

		/* returns -1 if not found */
		int getAttr(const std::string& s);

		/* Checking if it's the correct control point (within a preset range in px)
		 * return -1 if not, and index (>=0) if found
		 */
		int getCtrlPoint(const QPoint& cursor, int range);

		void addCtrlPoint(const QPoint& p);

		void modifyLastCtrlPoint(const QPoint& p);

		const QPoint& getLastCtrlPoint();

		/* return ctrl_points */
		const std::vector<QPoint>& getCtrlPoints(void);

		/* index is given in getCtrlPoint */
		void modifyCtrlPoint(int index, const QPoint& dest);

	protected:
		std::vector<QPoint> ctrl_points;
		std::map<std::string, int> attr;

		// cached bounding box
		QRect bounding_box;
		Eigen::Matrix<int, -1, -1> mask_mat;

		// cache validity
		bool bounding_box_valid;
		bool mask_mat_valid;

		void drawCross(QPainter& paint, int x, int y);

	};

	//// Segment used for intersection mgmt
	//class Segment {

	//};

};