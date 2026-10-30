#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

template<int n> struct vec {
    double data[n] = {0};
    double& operator[](const int i)       { assert(i >= 0 && i < n); return data[i]; }
    double  operator[](const int i) const { assert(i >= 0 && i < n); return data[i]; }
};

template<int n> std::ostream& operator << (std::ostream& out, const vec<n>& v) {
    for (int i = 0; i < n; i ++) out << v[i] << " ";
    return out;
}

template<> struct vec<3> {
    double x = 0, y = 0, z = 0;
    double& operator[](const int i)       { assert(i >= 0 && i < 3); return i ? (1 == i ? y : z) : x;} // i = 0, x because i = 0 is false
    double  operator[](const int i) const { assert(i >= 0 && i < 3); return i ? (1 == i ? y : z) : x;}
};

template<> struct vec<4> {
    double x = 0 , y = 0, z = 0, w = 0;
    double& operator[](const int i)       { assert(i >= 0 && i < 4); return i ? (1 == i ? y : (2 == i ? z : w)) : x;}
    double  operator[](const int i) const { assert(i >= 0 && i < 4); return i ? (1 == i ? y : (2 == i ? z : w)) : x;}
};

// Alias
typedef vec<3> vec3;
typedef vec<4> vec4;

template <int n>
vec<n> operator+(const vec<n>& left, const vec<n>& right) {
    vec<n> res;
    for (int i = 0; i < n; i ++) {
        res[i] = left[i] + right[i];
    }
    return res;
}

template<int n>
vec<n> operator-(const vec<n>& left, const vec<n>& right) {
    vec<n> res;
    for (int i = 0; i < n; i ++) {
        res[i] = left[i] - right[i];
    }
    return res;
}

template<int n>
vec<n> operator*(const vec<n>& vector, double factor) {
    vec<n> res;
    for (int i = 0; i < n; i ++) {
        res[i] = vector[i] * factor;
    }
    return res;
}

// Function overload
template<int n>
vec<n> operator*(double factor, const vec<n>& vector) {
    return vector * factor;
}

template<int n>
vec<n> operator/(const vec<n>& vector, double factor) {
    vec<n> res;
    for (int i = 0; i < n; i ++) {
        res[i] = vector[i] / factor;
    }
    return res;
}

// Function overload
template<int n>
vec<n> operator/(double factor, const vec<n>& vector) {
    return vector / factor;
}

template<int n>
double dot(const vec<n>& left, const vec<n>& right) {
    double res = 0;
    for (int i = 0; i < n; i++) {
        res += left[i] * right[i];
    }
    return res;
}

template<int n> double norm(const vec<n>& v) {
    return std::sqrt(dot(v, v));
}

template<int n> vec<n> normalised(const vec<n>& v) {
    double l = norm(v);
    assert(l > 0.0);
    return v / l;
}

vec<3> cross(const vec<3>& left, const vec<3>& right) {
    return vec3 {
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x
    };
}