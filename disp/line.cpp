#include "line.hpp"

#include <cmath>
#include <cassert>

namespace sf {

	LineShape::LineShape(const Vector2f& p0, const Vector2f& p1)
		: direction(p1 - p0)
	{
		this->setPosition(p0);
		this->setThickness(1.0f);
	}

	LineShape::~LineShape() = default;

	void LineShape::setThickness(float width)
	{
		thickness = width;
		this->update();
	}

	float LineShape::getThickness() const
	{
		return thickness;
	}

	float LineShape::getLength() const
	{
		return std::sqrt(direction.x * direction.x + direction.y * direction.y);
	}

	unsigned int LineShape::getPointCount() const
	{
		return 4;
	}

	Vector2f LineShape::getPoint(unsigned int index) const
	{
		Vector2f unitDirection = direction / this->getLength();
		Vector2f unitPerpendicular = {-unitDirection.y, unitDirection.x};

		Vector2f offset = (thickness / 2) * unitPerpendicular;

		switch (index) {
		case 0:
			return offset;
		case 1:
			return direction + offset;
		case 2:
			return direction - offset;
		case 3:
			return -offset;
		default:
			assert(false && "Invalid index");
			return {0, 0}; // arbitrary default
		}
	}

} // namespace sf
