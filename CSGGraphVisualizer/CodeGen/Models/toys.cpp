#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "models.h"
#include "../util.h"

using namespace std;
using namespace glm;
namespace Models {

const float g_offset = 0.25f;
MatSdfExpr* toyCube(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(0.9f, 0.9f, 0.9f, Fields{ mat }))); }

MatSdfExpr* toyBrickYthin(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.4f, 2.9f, 0.9f, Fields{ mat }))); }

MatSdfExpr* toyBrickXthin(int mat) { return offset(g_offset, move({ 0.f, 0.5f, 0.f }, box(2.9f, 0.4f, 0.9f, Fields{ mat }))); }

MatSdfExpr* toyBrickYthick(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.9f, 2.9f, 0.9f, Fields{ mat }))); }

MatSdfExpr* toyBrickXthick(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, Fields{ mat }))); }
//MatSdfExpr* toyBrickXthick(int mat) { return move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, MyFields{ mat })); }

MatSdfExpr* toyCylinder(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 2.9f, Fields{ mat }))); }

MatSdfExpr* toyCylinderSmall(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 0.9f, Fields{ mat }))); }

MatSdfExpr* toyArch(int mat) { return offset(g_offset, subtract<Fields>(toyBrickXthick(mat), move({ 0.f, -1.4f, 0.f }, cylinder(Dir1D::Z, 2.6f, Fields{ mat })))); }

MatSdfExpr* toyRoof(int mat) {
    return offset(g_offset,
        intersect<Fields>(toyBrickXthick(mat),
        move({ 0.f, -1.5f, 0.f }, cylinder(Dir1D::Z, 3.3f, 0.9f, Fields{ mat }))));
}

mat3 fitToyY(vec2 a, vec2 b) {
    vec2 v = normalize(b - a);
    return mat3(v.x, 0, -v.y, 0, 1, 0, v.y, 0, v.x);
}

MatSdfExpr* toys_exp() {
    vec3 p1(0, 0, 0), p2(4, 0, 0);
    mat3 M12 = fitToyY(p1.xz, p2.xz);
    vec3 p3 = 4.0f * normalize(vec3(-1, 0, 2));
    mat3 M13 = fitToyY(p1.xz, p3.xz);
    mat3 M1 = rotateY(0.5);
    mat3 M2 = rotateY((float)-M_PI * 0.5f);
    mat3 M3 = rotateY(0.4f);
    mat3 M4 = rotateZ(0.42f);
    mat3 M5 = rotateY(0.123f);
    mat3 M1t = transpose(M1);
    return
        union_op<Fields>(
            planeXZ(Fields{ 3 }),
            move(p1 + vec3(-0.4, 0, 0.4), rotate(M1t, toyCube(1))),
            move(p2 + vec3(-0.1, 0, 1.4), rotate(M2, toyBrickXthick(0))),
            move(p1 + vec3(0, 4, 0), rotate(M5, toyCube(2))),
            move(p3, toyCylinder(0)),
            move(0.5f * (p1 + p2) + vec3(0, 2, 0), rotate(M12, toyArch(1))),
            move(0.5f * (p1 + p3) + vec3(0, 6.5, 0), rotate(M13, toyRoof(2))),
            move(vec3(4, 4, 0), toyCylinderSmall(1)),
            move(vec3(8.5, 0, 1.8), rotate(M5, toyBrickYthick(0))),
            move(vec3(6.5, 6.25, 1), rotate(M3, toyBrickXthin(2))),
            move(vec3(6.625, 1.193, 4.036), rotate(M4, toyBrickXthin(0))),
            move(vec3(9,8.5,2.7), sphere(1, Fields{ 4 })),
            move(vec3(1.2,1,3.2), sphere(1, Fields{ 4 })),
            move(vec3(1, 0, 6), rotate(M13, toyCube(1))));
}
//vec3 lightpos = vec3(5, 1, 6.5;
//vec3 lightpos = vec3(4,4.3,4.6);
//vec3 lightpos = vec3(1,3,6);
//vec3 lightpos = vec3(1.2,1,3.2);
//vec3 lightpos = vec3(9,8.5,2.7);
};