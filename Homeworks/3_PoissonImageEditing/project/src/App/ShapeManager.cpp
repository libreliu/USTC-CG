#include "ShapeManager.h"

#include "Ellipse.h"
#include "Freehand.h"
#include "Line.h"
#include "Rect.h"
#include "Polygon.h"

using namespace DrawContext;

// static member initialization
std::map<int, ShapeManager::ShapeFactory> ShapeManager::shape_factories {
	// {ShapeManager::ShapeType::Ellipse, [](int param) {
	// 	return dynamic_cast<DrawContext::Shape *>(new DrawContext::Ellipse());
	// }},
	// {ShapeManager::ShapeType::Freehand, [](int param) {
	// 	return dynamic_cast<DrawContext::Shape *>(new DrawContext::Freehand());
	// }},
	{ShapeManager::ShapeType::Rect, [](int param) {
		return dynamic_cast<DrawContext::Shape *>(new DrawContext::Rect());
	}},
	// {ShapeManager::ShapeType::Line, [](int param) {
	// 	return dynamic_cast<DrawContext::Shape *>(new DrawContext::Line());
	// }},
	// {ShapeManager::ShapeType::Polygon, [](int param) {
	// 	return dynamic_cast<DrawContext::Shape *>(new DrawContext::Polygon());
	// }}
};

int ShapeManager::registerShape(ShapeFactory fact, int type)
{
	auto it = shape_factories.find(type);
	if (shape_factories.end() == it) {
		// Register
		ShapeManager::shape_factories[type] = fact;
		return 0;
	} else {
		return -1;
	}
}

ShapeManager::ShapeFactory DrawContext::ShapeManager::getFactory(int type)
{
	auto it = shape_factories.find(type);
	if (shape_factories.end() != it) {
		return it->second;
	} else {
		return nullptr;
	}
}
