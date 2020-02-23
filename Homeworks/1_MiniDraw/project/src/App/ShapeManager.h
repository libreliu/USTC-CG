#pragma once

#include "Shape.h"
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

	protected:
		static std::map<int, ShapeFactory> shape_factories;
	};

};
