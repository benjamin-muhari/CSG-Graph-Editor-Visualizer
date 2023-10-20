#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "../bounding_opt.h"
#include "../codegen.h"
#include "glm/glm.hpp"
#include <cmath>
#include <fstream>
#include "models.h"

#include <iostream>

using namespace std;
using namespace glm;
namespace Models {

/// <summary>
/// r1 > r2
/// </summary>
MatSdfExpr* tubeModel4(int mat, Dir1D dir, float r1, float r2, float heightHalf) {
    return subtract<Fields>(
        cylinder(dir, r1, heightHalf, Fields{ mat }),
        cylinder(dir, r2, 2 * heightHalf, Fields{ mat })
    );
}

MatSdfExpr* baseModel4(int mat, float width, float height) {
    float outerCylinderRadius = width * 0.2f;
    float innerCylinderRadius = width * 0.04f;
    float widthHalf = width / 2;

    MatSdfExpr* cutoutCylinderOuter = cylinder(Dir1D::Y, outerCylinderRadius, height, Fields{ mat });
    MatSdfExpr* cutoutCylinderInner = cylinder(Dir1D::Y, innerCylinderRadius, height, Fields{ mat });

    MatSdfExpr* out = box(width / 2, height / 2, width / 2, Fields{ mat });
    out = subtract(out, move({-widthHalf, 0.0f, -widthHalf }, cutoutCylinderOuter));
    out = subtract(out, move({ widthHalf, 0.0f, -widthHalf }, cutoutCylinderOuter));
    out = subtract(out, move({ -widthHalf, 0.0f, widthHalf }, cutoutCylinderOuter));
    out = subtract(out, move({ widthHalf, 0.0f, widthHalf }, cutoutCylinderOuter));

    float offset1 = innerCylinderRadius * 2 + outerCylinderRadius;
    float offset2 = innerCylinderRadius * 2;
    out = subtract(out, move({ -widthHalf + offset1, 0.0f, -widthHalf + offset2 }, cutoutCylinderInner));
    out = subtract(out, move({ widthHalf - offset2, 0.0f, -widthHalf + offset1 }, cutoutCylinderInner));
    out = subtract(out, move({ -widthHalf + offset2 , 0.0f, widthHalf - offset1 }, cutoutCylinderInner));
    out = subtract(out, move({ widthHalf - offset1, 0.0f, widthHalf - offset2 }, cutoutCylinderInner));

    return out;
}

MatSdfExpr* upperModel4(int mat, float width, float height) {
    float halfHeight = height / 2;
    float outerRadius = width * 0.3f;
    float innerRadius = outerRadius * 0.85f;
    float innerInnerRadius = outerRadius * 0.6f;

    return  subtract<Fields>(subtract<Fields>(subtract<Fields>(union_op<Fields>(
        cylinder(Dir1D::Y, outerRadius, halfHeight * 0.6f, Fields{ mat }),
        move({ 0.0f, -height * 0.45, 0.0f }, cylinder(Dir1D::Y, outerRadius, halfHeight * 0.1f, Fields{ mat })),
        move({ 0.0f, height * 0.45f, 0.0f }, cylinder(Dir1D::Y, outerRadius, halfHeight * 0.1f, Fields{ mat })),
        cylinder(Dir1D::Y, innerRadius, halfHeight, Fields{ mat })
        ), move({ 0.0f, height * 0.35f, 0.0f}, cylinder(Dir1D::Y, innerInnerRadius, halfHeight * 0.32f, Fields{ mat }))),
           move({ 0.0f, -height * 0.35f, 0.0f }, cylinder(Dir1D::Y, innerInnerRadius, halfHeight * 0.32f, Fields{ mat }))),
           cylinder(Dir1D::Y, innerInnerRadius * 0.7f, halfHeight, Fields{ mat }));
}

MatSdfExpr* assembleModel4(float width, float height) {
    float baseHeight = height * 0.6f;
    float upperHeight = height - baseHeight;

    return union_op<Fields>(
        baseModel4(1, width, baseHeight),
        move({ 0.0f, height / 2, 0.0f }, upperModel4(1, width, upperHeight))
    );
}

MatSdfExpr* model4_expr(float width, float height) {
    return assembleModel4(width, height);
}
};