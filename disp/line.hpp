#pragma once

#include <sfml/graphics/shape.hpp>

namespace sf {

	class LineShape
		: public Shape
	{
	private: // internals

		Vector2f direction;
		float thickness;

	public: // methods

		explicit LineShape(const Vector2f& p0, const Vector2f& p1);
		virtual ~LineShape();

		void setThickness(float width);
		float getThickness() const;

		float getLength() const;

		virtual unsigned int getPointCount() const override;
		virtual Vector2f getPoint(unsigned int index) const override;
	};

} // namespace sf
