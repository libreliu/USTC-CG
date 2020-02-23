#include "ShapeManager.h"


using namespace DrawContext;

int ShapeManager::registerShape(ShapeFactory fact, int type)
{
	auto it = shape_factories.find(type);
	if (shape_factories.end() == it) {
		// TODO
	}
	return 0;
}

ShapeFactory DrawContext::ShapeManager::getFactory(int type)
{
	return ShapeFactory();
}
