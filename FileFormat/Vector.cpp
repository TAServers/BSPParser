#include "Structs.h"

#include <cmath>

using namespace BSPStructs;

Vector Vector::operator+(const Vector& b) const
{
	return Vector{ x + b.x, y + b.y, z + b.z };
}
Vector Vector::operator-() const
{
	return Vector{ -x, -y, -z };
}
Vector Vector::operator-(const Vector& b) const
{
	return Vector{ x - b.x, y - b.y, z - b.z };
}
Vector Vector::operator*(const Vector& b) const
{
	return Vector{ x * b.x, y * b.y, z * b.z };
}
Vector Vector::operator/(const Vector& b) const
{
	return Vector{ x / b.x, y / b.y, z / b.z };
}

Vector& Vector::operator+=(const Vector& b)
{
	x += b.x;
	y += b.y;
	z += b.z;
	return *this;
}
Vector& Vector::operator-=(const Vector& b)
{
	x -= b.x;
	y -= b.y;
	z -= b.z;
	return *this;
}
Vector& Vector::operator*=(const Vector& b)
{
	x *= b.x;
	y *= b.y;
	z *= b.z;
	return *this;
}
Vector& Vector::operator/=(const Vector& b)
{
	x /= b.x;
	y /= b.y;
	z /= b.z;
	return *this;
}

Vector Vector::operator+(const float& b) const
{
	return Vector{ x + b, y + b, z + b };
}
Vector Vector::operator-(const float& b) const
{
	return Vector{ x - b, y - b, z - b };
}
Vector Vector::operator*(const float& b) const
{
	return Vector{ x * b, y * b, z * b };
}
Vector Vector::operator/(const float& b) const
{
	return Vector{ x / b, y / b, z / b };
}

Vector& Vector::operator+=(const float& b)
{
	x += b;
	y += b;
	z += b;
	return *this;
}
Vector& Vector::operator-=(const float& b)
{
	x -= b;
	y -= b;
	z -= b;
	return *this;
}
Vector& Vector::operator*=(const float& b)
{
	x *= b;
	y *= b;
	z *= b;
	return *this;
}
Vector& Vector::operator/=(const float& b)
{
	x /= b;
	y /= b;
	z /= b;
	return *this;
}

Vector Vector::Cross(const Vector& b) const
{
	return Vector{
		y * b.z - z * b.y,
		z * b.x - x * b.z,
		x * b.y - y * b.x
	};
}

void Vector::Normalise()
{
	float len = sqrtf(Dot(*this));
	x /= len;
	y /= len;
	z /= len;
}
