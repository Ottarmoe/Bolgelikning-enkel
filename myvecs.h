#pragma once
#include <stdint.h>
#include <iostream>
#include <math.h>
#include "Point.h"
#include "Color.h"


template<class T> struct vec2t {
	T x, y;

    vec2t(T nx = 0, T ny = 0) :x(nx), y(ny) {}
    template<class V> vec2t(vec2t<V> in) :x(T(in.x)), y(T(in.y))
    {}

    vec2t operator+(const vec2t& addend) const {
        return vec2t{ x + addend.x, y + addend.y };
    }
    vec2t operator+=(const vec2t& addend) {
        return vec2t{ x += addend.x, y += addend.y };
    }
    vec2t operator-(const vec2t& subtrahend) const {
        return vec2t{ x - subtrahend.x, y - subtrahend.y };
    }
    vec2t operator-=(const vec2t& subtrahend) {
        return vec2t{ x -= subtrahend.x, y -= subtrahend.y };
    }
    vec2t operator*(T factor) const {
        return vec2t{ x * factor, y * factor };
    }
    vec2t operator*=(T factor) {
        return vec2t{ x *= factor, y *= factor };
    }
    vec2t operator-() const {
        return vec2t{ -x, -y };
    }
    vec2t operator*(const vec2t& factor) const {
        return vec2t{ x * factor.x, y * factor.y };
    }
    vec2t operator*=(const vec2t& factor) {
        return vec2t{ x *= factor.x, y *= factor.y };
    }
    vec2t operator/(T divisor) const {
        return vec2t{x/divisor, y/divisor};
    }
    vec2t operator/(vec2t<T> divisors) const {
        return vec2t{x/divisors.x, y/divisors.y};
    }
    vec2t operator/=(T divisor) {
        return vec2t{x /= divisor, y /= divisor};
    }
    bool operator==(const vec2t<T>& c) const{
        return x==c.x&&y==c.y;
    }

    vec2t(const TDT4102::Point& pt)
        :x(T(pt.x)), y(T(pt.y)){}
    operator TDT4102::Point() const{
        return {int(x), int(y)};
    }
};

template<class T> inline T dot(vec2t<T> a, vec2t<T> b) {
    return a.x * b.x + a.y * b.y;
}
template<class T> inline vec2t<T> clamp(vec2t<T> c, vec2t<T> lw, vec2t<T> hg) {
    return vec2t<T>(std::clamp(c.x, lw.x, hg.x), std::clamp(c.y, lw.y, hg.y));
}
template<class T> inline T lensqr(vec2t<T> v) {
    return dot(v, v);
}
template<class T> inline vec2t<T> round(vec2t<T> v) {
    return vec2t<T>(round(v.x), round(v.y));
}
template<class T> inline vec2t<T> floor(vec2t<T> v) {
    return vec2t<T>(floor(v.x), floor(v.y));
}
template<class T> inline vec2t<T> ceil(vec2t<T> v) {
    return vec2t<T>(ceil(v.x), ceil(v.y));
}
template<class T> inline T len(const vec2t<T>& v) {
    return sqrt(v.x*v.x+v.y*v.y);
}
template<class T> inline vec2t<T> normalize(const vec2t<T>& v){
    return v*(T(1)/len(v));
}

template<typename T>
vec2t<T> orthL(const vec2t<T>& src){
    return {-src.y,src.x};
}
template<typename T>
vec2t<T> orthR(const vec2t<T>& src){
    return {src.y,-src.x};
}


template<class T> struct vec3t {
	T x, y, z;

    vec3t(T nx = 0, T ny = 0, T nz = 0) :x(nx), y(ny), z(nz) {}
    template<class V> vec3t(vec3t<V> in) :x(T(in.x)), y(T(in.y)), z(T(in.y))
    {}

    vec3t operator+(const vec3t& addend) const {
        return vec3t{ x + addend.x, y + addend.y , z + addend.z};
    }
    vec3t operator+=(const vec3t& addend) {
        return vec2t{ x += addend.x, y += addend.y, z += addend.z };
    }
    vec3t operator-(const vec3t& subtrahend) const {
        return vec2t{ x - subtrahend.x, y - subtrahend.y, z - subtrahend.z};
    }
    vec3t operator-=(const vec3t& subtrahend) {
        return vec2t{ x -= subtrahend.x, y -= subtrahend.y, z -= subtrahend.z };
    }
    vec3t operator*(T factor) const {
        return vec3t{ x * factor, y * factor, z * factor };
    }
    vec3t operator*=(T factor) {
        return vec3t{ x *= factor, y *= factor, z *= factor };
    }
    vec3t operator-() const {
        return vec3t{ -x, -y, -z };
    }
    vec3t operator*(const vec3t& factor) const {
        return vec3t{ x * factor.x, y * factor.y, z * factor.z };
    }
    vec3t operator*=(const vec3t& factor) {
        return vec2t{ x *= factor.x, y *= factor.y, z *= factor.z };
    }
    vec3t operator/(T divisor) const {
        return vec3t{x/divisor, y/divisor, z/divisor};
    }
    vec3t operator/=(T divisor) {
        return vec3t{x /= divisor, y /= divisor, z /= divisor};
    }
    bool operator==(const vec2t<T>& c) const{
        return x==c.x&&y==c.y&&z==c.z;
    }
    
    operator TDT4102::Color(){
        return TDT4102::Color((unsigned char)(x*255), (unsigned char)(y*255), (unsigned char)(z*255));
    }
};

template<class T> inline T dot(vec3t<T> a, vec3t<T> b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
template<class T> inline T lensqr(vec3t<T> v) {
    return dot(v, v);
}
template<class T> inline vec3t<T> round(vec3t<T> v) {
    return vec3t<T>(round(v.x), round(v.y), round(v.z));
}
template<class T> inline vec3t<T> floor(vec3t<T> v) {
    return vec3t<T>(floor(v.x), floor(v.y), floor(v.z));
}
template<class T> inline vec3t<T> ceil(vec3t<T> v) {
    return vec3t<T>(ceil(v.x), ceil(v.y), ceil(v.z));
}
template<class T> inline T len(const vec3t<T>& v) {
    return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}
template<class T> inline vec3t<T> normalize(const vec3t<T>& v){
    return v*(T(1.)/len(v));
}
template<class T> inline vec3t<T> cross(vec3t<T> a, vec3t<T> b) {
    return vec3t<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}


typedef uint32_t uint;
typedef uint8_t byte;

typedef vec2t<int> ivec2;
typedef vec2t<uint> uvec2;
typedef vec2t<float> vec2;
typedef vec2t<double> dvec2;

typedef vec3t<int> ivec3;
typedef vec3t<uint> uvec3;
typedef vec3t<float> vec3;
typedef vec3t<double> dvec3;

template<typename T> struct quadt{
    vec2t<T> lower;
    vec2t<T> higher;
    quadt(vec2t<T> lw, vec2t<T> hg)
        :lower(lw), higher(hg)
    {}
    quadt<T> operator+(const vec2t<T>& addend){
        return quadt<T>{lower+addend, higher+addend};
    }
    quadt<T> operator-(const vec2t<T>& subtrahend){
        return quadt<T>{lower-subtrahend, higher-subtrahend};
    }


    bool contains(const vec2t<T>& vec){
        return
            vec.x>=lower.x && vec.x <= higher.x &&
            vec.y>=lower.y && vec.y <= higher.y;
    }
};
template<typename T>
bool isOverlap(const quadt<T>& a, const quadt<T>& b){
    return
        a.lower.x<=b.higher.x &&
        a.higher.x>=b.lower.x &&
        a.lower.y<=b.higher.y &&
        a.higher.y>=b.lower.y;
}


typedef quadt<float> quadf;
typedef quadt<uint> quadu;

typedef quadf screen;

template<typename T> quadt<T> centeredQuad(vec2t<T> center, vec2t<T> dims){
    return quadt<T>(center-dims/2, center+dims/2);
}

//the operator* is the general method of aplying a UniformTransform to an object.
//transforming a transform yields one that equates to applying the transforms sequentially, right to left
//the antitransform function applies the transform in reverse
template<typename T> 
struct UniformTransform{
    vec2t<T> scale;
    vec2t<T> offset;
    UniformTransform(const vec2t<T>& scal, const vec2t<T>& offs)
        :scale(scal), offset(offs){}
    vec2t<T> transform(const vec2t<T>& v) const{
        return (v*scale)+offset;
    }
    vec2t<T> antitransform(const vec2t<T>& v) const{
        return (v-offset)/scale;
    }

    vec2t<T> operator*(const vec2t<T>& v) const{
        return (v*scale)+offset;
    }
    UniformTransform<T> operator*(const UniformTransform<T>& trs) const{
        return UniformTransform<T>(trs.scale*scale, transform(trs.offset));
    }
    quadt<T> operator*(const quadt<T>& qd) const{
        return quadt<T>(*this * (qd.lower), *this * (qd.higher));
    }
};
template<typename T>
UniformTransform<T> screenCast(const quadt<T>& from, const quadt<T>& to){
    //assumes screens from and to have equal aspect
    vec2t<T> scale = vec2t<T>(
        (to.higher.x-to.lower.x)/(from.higher.x-from.lower.x),
        (to.higher.y-to.lower.y)/(from.higher.y-from.lower.y)
        );
    vec2t<T> offset = to.lower-from.lower*scale;
    return UniformTransform(scale, offset);
}
struct ScreenMap : UniformTransform<float>{
    screen from;
    screen to;
    ScreenMap(screen f, screen t)
        :UniformTransform<float>(screenCast(f, t)), from(f), to(t){}
    void update(){
        *static_cast<UniformTransform*>(this) = screenCast(from, to);
    }
};
ScreenMap operator*(const UniformTransform<float>& trf, const ScreenMap& mp){
    ScreenMap ret = mp;
    ret.to = trf*ret.to;
    ret.update();
    return ret;
}



template<class T>
std::ostream& operator<<(std::ostream& stream, const vec3t<T>& vec)
{
	stream << '[' << vec.x << ' ' << vec.y << ' ' << vec.z << ']';
	return stream;
}
template<class T>
std::ostream& operator<<(std::ostream& stream, const vec2t<T>& vec)
{
	stream << '[' << vec.x << ' ' << vec.y << ']';
	return stream;
}

