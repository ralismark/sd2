#pragma once

#include <sfml/graphics/shape.hpp>

/**
 * \file
 * \brief SFML Line with thickness
 *
 * This file provides \ref sf::LineShape, which allows the drawing of lines
 * that have thickness. It is created to be similar to the standard SFML
 * shapes, and has a similar interface.
 */

namespace sf {

	/**
	 * \class LineShape
	 * \brief SFML Line with thickness
	 *
	 * This class allows a line with a specified thickness to be drawn.
	 * This improves upon sf::Line, which does not allow thickness. Use the
	 * setThickness() method to set the thickness, and setFillColor() to
	 * set the colour of the line.
	 */

	class LineShape
		: public Shape
	{
	private: // internals

		Vector2f direction;
		float thickness;

	public: // methods

		/// Construct from start point to end point
		explicit LineShape(const Vector2f& p0, const Vector2f& p1);
		virtual ~LineShape();

		/// Set the width
		void setThickness(float width);

		/// Get the width
		float getThickness() const;

		/// Get the length of the line
		float getLength() const;

		virtual unsigned int getPointCount() const override;
		virtual Vector2f getPoint(unsigned int index) const override;
	};

} // namespace sf
