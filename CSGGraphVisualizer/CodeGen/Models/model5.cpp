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

MatSdfExpr* wallsModel5(int mat, float wholeXSize, float wholeYSize) {
    float wallSize = wholeXSize * 0.05f;
    float innerBoxSize = wholeXSize - 2 * wallSize;
    return subtract<Fields>(
        offset(wholeXSize * 0.05f, box(wholeXSize / 2, wholeYSize / 2, wholeXSize / 2, Fields{ mat })),
        box(innerBoxSize / 2, wholeYSize, innerBoxSize / 2, Fields{ mat }));
}

MatSdfExpr* model5_expr(float width, float height, float holeRadius) {
    return subtract<Fields>(subtract<Fields>(union_op<Fields>(
        move({ 0.0f, -height * 0.45f, 0.0f }, box(width / 2, height * 0.1f / 2, width / 2, Fields{ 1 })),
        wallsModel5(1, width, height)
        ), move({0.0f, -height / 4, 0.0f}, cylinder(Dir1D::X, holeRadius, width * 1.2f, Fields{ 1 }))),
           move({ 0.0f, -height / 4, 0.0f }, cylinder(Dir1D::Z, holeRadius, width * 1.2f, Fields{ 1 })));
}
};