#include "Structs.h"

#include <cmath>

using namespace BSPStructs;

Vector2D Vector2D::operator+(const Vector2D& b) const
{
	return Vector2D{ x + b.x, y + b.y };
}
Vector2D Vector2D::operator-() const
{
	return Vector2D{ -x, -y };
}
Vector2D Vector2D::operator-(const Vector2D& b) const
{
	return Vector2D{ x - b.x, y - b.y };
}
Vector2D Vector2D::operator*(const Vector2D& b) const
{
	return Vector2D{ x * b.x, y * b.y };
}
Vector2D Vector2D::operator/(const Vector2D& b) const
{
	return Vector2D{ x / b.x, y / b.y };
}

Vector2D& Vector2D::operator+=(const Vector2D& b)
{
	x += b.x;
	y += b.y;
	return *this;
}
Vector2D& Vector2D::operator-=(const Vector2D& b)
{
	x -= b.x;
	y -= b.y;
	return *this;
}
Vector2D& Vector2D::operator*=(const Vector2D& b)
{
	x *= b.x;
	y *= b.y;
	return *this;
}
Vector2D& Vector2D::operator/=(const Vector2D& b)
{
	x /= b.x;
	y /= b.y;
	return *this;
}

Vector2D Vector2D::operator+(const float& b) const
{
	return Vector2D{ x + b, y + b };
}
Vector2D Vector2D::operator-(const float& b) const
{
	return Vector2D{ x - b, y - b };
}
Vector2D Vector2D::operator*(const float& b) const
{
	return Vector2D{ x * b, y * b };
}
Vector2D Vector2D::operator/(const float& b) const
{
	return Vector2D{ x / b, y / b };
}

Vector2D& Vector2D::operator+=(const float& b)
{
	x += b;
	y += b;
	return *this;
}
Vector2D& Vector2D::operator-=(const float& b)
{
	x -= b;
	y -= b;
	return *this;
}
Vector2D& Vector2D::operator*=(const float& b)
{
	x *= b;
	y *= b;
	return *this;
}
Vector2D& Vector2D::operator/=(const float& b)
{
	x /= b;
	y /= b;
	return *this;
}

void Vector2D::Normalise()
{
	float len = sqrtf(Dot(*this));
	x /= len;
	y /= len;
}
