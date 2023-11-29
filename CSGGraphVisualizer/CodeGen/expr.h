#pragma once

#include "glm/glm.hpp"
#include <limits>
#include <string>
#include <variant>
#include <vector>

template<typename Fields>
struct Box : Fields {
    float x, y, z;
};
template<typename Fields>
struct Sphere : Fields {
    float r;
};
enum class Dir1D : char {
    X = 'X', Y = 'Y', Z = 'Z'
};
template<typename Fields>
struct Cylinder : Fields {
    Dir1D dir;
    float x, y;
};
template<typename Fields>
struct PlaneXZ : Fields {};
template<typename Fields>
struct Offset;
template<typename Fields>
struct Union;
template<typename Fields>
struct Intersect;
template<typename Fields>
struct Invert;
template<typename Fields>
struct Move;
template<typename Fields>
struct Rotate;

template<typename Fields>
using Expr = std::variant<Box<Fields>, Sphere<Fields>, Cylinder<Fields>, PlaneXZ<Fields>, Offset<Fields>, Union<Fields>, Intersect<Fields>, Invert<Fields>, Move<Fields>, Rotate<Fields>>;

template<typename Fields>
struct Offset : Fields {
    float r;
    Expr<Fields>* a;
};
template<typename Fields>
struct Union : Fields {
    std::vector<Expr<Fields>*> a;
};

template<typename Fields>
struct Intersect : Fields {
    std::vector<Expr<Fields>*> a;
};

template<typename Fields>
struct Invert : Fields {
    Expr<Fields>* a;
};
template<typename Fields>
struct Move : Fields {
    glm::vec3 v;
    Expr<Fields>* a;
};
template<typename Fields>
struct Rotate : Fields {
    glm::mat3 m;
    Expr<Fields>* a;
    // Node Editor UI fields
    enum rotation_type {X,Y,Z,M3x3};
    rotation_type _rotation_type;
    int angle_degree;
};

template<typename Fields>
Expr<Fields>* box(float x, float y, float z, const Fields& f = Fields()) { return new Expr<Fields>{ Box<Fields>{f, x, y, z} }; }
template<typename Fields>
Expr<Fields>* sphere(float r, const Fields& f = Fields()) { return new Expr<Fields>{ Sphere<Fields>{f, r} }; }
template<typename Fields>
Expr<Fields>* cylinder(Dir1D dir, float x, float y = std::numeric_limits<float>::infinity(), const Fields& f = Fields()) {
    return new Expr<Fields>{ Cylinder<Fields>{f, dir, x, y} };
}
template<typename Fields>
Expr<Fields>* cylinder(Dir1D dir, float x, const Fields& f = Fields()) { return cylinder(dir, x, std::numeric_limits<float>::infinity(), f); }
template<typename Fields>
Expr<Fields>* planeXZ(const Fields& f = Fields()) { return new Expr<Fields>{ PlaneXZ<Fields>{f} }; }
template<typename Fields>
Expr<Fields>* offset(float r, Expr<Fields>* a, const Fields& f = Fields()) { return new Expr<Fields>{ Offset<Fields>{f,r,a} }; }

template<typename Fields, typename... Exp>
Expr<Fields>* intersect(const Fields& f, Exp... a) { return new Expr<Fields>{ Intersect<Fields>{f,{a...}} }; }
template<typename Fields, typename... Exp> Expr<Fields>* intersect(Exp... a) { return intersect(Fields(), a...); }

template<typename Fields, typename... Exp>
Expr<Fields>* union_op(const Fields& f, Exp... a) { return new Expr<Fields>{ Union<Fields>{f,{a...}} }; }
template<typename Fields, typename... Exp> Expr<Fields>* union_op(Exp... a) { return union_op(Fields(), a...); }

template<typename Fields>
Expr<Fields>* invert(Expr<Fields>* a, const Fields& f = Fields()) { return new Expr<Fields>{ Invert<Fields>{f,a} }; }

template<typename Fields>
Expr<Fields>* subtract(Expr<Fields>* a, Expr<Fields>* b, const Fields& f = Fields()) { return intersect(f,a,invert(b,f)); }

template<typename Fields>
Expr<Fields>* move(glm::vec3 v, Expr<Fields>* a, const Fields& f = Fields()) { return new Expr<Fields>{ Move<Fields>{f,v,a} }; }
template<typename Fields>
Expr<Fields>* rotate(glm::mat3 m, Expr<Fields>* a, const Fields& f = Fields()) { return new Expr<Fields>{ Rotate<Fields>{f,m,a} }; }
