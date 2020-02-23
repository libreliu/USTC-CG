#pragma once

#include "Shape.h"
#include <memory>

namespace DrawContext {

	class ShapeManager {
	public:

		typedef Shape* (*ShapeFactory)(int param);

		/* static member used to register type
		 * if type has already occupied, return -1
		 * else >=0 number given
		 */
		static int registerShape(ShapeFactory fact, int type);

		/* return the factory for one shape */
		static ShapeFactory getFactory(int type);

		/* Hard-coded map */
		typedef enum ShapeType {
			Rect = 0,
			Polygon = 1,
			Ellipse = 2,
			Line = 3,
			Point = 4,
			Freehand = 5,
			Circle = 6,
		} ShapeType;

	protected:
		static std::map<int, ShapeFactory> shape_factories;
	};

};
