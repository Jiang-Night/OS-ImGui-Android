#ifndef OS_IMGUI_STRUCT_H
#define OS_IMGUI_STRUCT_H

#include "imgui/imgui.h"

/****************************************************
 * Copyright (C)	: JiangNight
 * @file			: OSImGui_Struct.h
 * @author		: Liv
 * @email		: wan1298178043@gmail.com
 * @version		: 1.0
 * @date			: 2024/10/1	02:54
 ****************************************************/

class Vec2 {
public:
    float x, y;

public:
    Vec2() :
        x(0.f), y(0.f) {}
    Vec2(float x_, float y_) :
        x(x_), y(y_) {}
    Vec2(int x_, int y_) :
        x(x_), y(y_) {}
    Vec2(ImVec2 imVec) :
        x(imVec.x), y(imVec.y) {}

    Vec2 &operator=(ImVec2 &imVec) {
        x = imVec.x;
        y = imVec.y;
        return *this;
    }

    Vec2 operator+(Vec2 Vec2_) { return {x + Vec2_.x, y + Vec2_.y}; }
    Vec2 operator-(Vec2 Vec2_) { return {x - Vec2_.x, y - Vec2_.y}; }
    Vec2 operator*(Vec2 Vec2_) { return {x * Vec2_.x, y * Vec2_.y}; }
    Vec2 operator/(Vec2 Vec2_) { return {x / Vec2_.x, y / Vec2_.y}; }
    Vec2 operator*(float n) { return {x * n, y * n}; }
    Vec2 operator/(float n) { return {x / n, y / n}; }

    bool operator==(Vec2 Vec2_) { return x == Vec2_.x && y == Vec2_.y; }
    bool operator!=(Vec2 Vec2_) { return x != Vec2_.x || y != Vec2_.y; }

    ImVec2 ToImVec2() { return ImVec2(x, y); }
    float Length() { return sqrtf(powf(x, 2) + powf(y, 2)); }

    float DistanceTo(const Vec2 &Pos) {
        return sqrtf(powf(Pos.x - x, 2) + powf(Pos.y - y, 2));
    }
};

class Vec3 {
public:
    float x, y, z;

public:
    Vec3() :
        x(0.f), y(0.f), z(0.f) {}
    Vec3(float x_, float y_, float z_) :
        x(x_), y(y_), z(z_) {}

    float Dot(Vec3 Vec3_) { return x * Vec3_.x + y * Vec3_.y + z * Vec3_.z; }

    Vec3 operator+(Vec3 Vec3_) { return {x + Vec3_.x, y + Vec3_.y, z + Vec3_.z}; }
    Vec3 operator-(Vec3 Vec3_) { return {x - Vec3_.x, y - Vec3_.y, z - Vec3_.z}; }
    Vec3 operator*(Vec3 Vec3_) { return {x * Vec3_.x, y * Vec3_.y, z * Vec3_.z}; }
    Vec3 operator/(Vec3 Vec3_) { return {x / Vec3_.x, y / Vec3_.y, z / Vec3_.z}; }
    Vec3 operator*(float n) { return {x * n, y * n, z * n}; }
    Vec3 operator/(float n) { return {x * n, y * n, z * n}; }

    bool operator==(Vec3 Vec3_) { return x == Vec3_.x && y == Vec3_.y && z == Vec3_.z; }
    bool operator!=(Vec3 Vec3_) { return x != Vec3_.x || y != Vec3_.y || z != Vec3_.z; }

    float Length() { return sqrtf(powf(x, 2) + powf(y, 2) + powf(z, 2)); }

    float DistanceTo(const Vec3 &Pos) {
        return sqrtf(powf(Pos.x - x, 2) + powf(Pos.y - y, 2) + powf(Pos.z - z, 2));
    }
};

class Vec4 {
public:
    float x, y, w, h;

public:
    Vec4() :
        x(0.f), y(0.f), w(0.f), h(0.f) {}

    Vec4(float x_, float y_, float w_, float h_) :
        x(x_), y(y_), w(w_), h(h_) {}

    // 运算符重载
    Vec4 operator+(const Vec4 &Vec4_) const { return {x + Vec4_.x, y + Vec4_.y, w + Vec4_.w, h + Vec4_.h}; }
    Vec4 operator-(const Vec4 &Vec4_) const { return {x - Vec4_.x, y - Vec4_.y, w - Vec4_.w, h - Vec4_.h}; }
    Vec4 operator*(float n) const { return {x * n, y * n, w * n, h * n}; }
    Vec4 operator/(float n) const { return {x / n, y / n, w / n, h / n}; }

    bool operator==(const Vec4 &Vec4_) const { return x == Vec4_.x && y == Vec4_.y && w == Vec4_.w && h == Vec4_.h; }
    bool operator!=(const Vec4 &Vec4_) const { return !(*this == Vec4_); }
    // 计算面积
    float Area() const { return w * h; }
    // 计算与另一个Vec4的距离
    float DistanceTo(const Vec4 &Pos) const {
        return sqrtf(powf(Pos.x - x, 2) + powf(Pos.y - y, 2) + powf(Pos.w - w, 2) + powf(Pos.h - h, 2));
    }
};
// 单例设计
template <typename T>
class Singleton {
public:
    static T &get() {
        static T instance;
        return instance;
    }
};

#endif // OS_IMGUI_STRUCT_H