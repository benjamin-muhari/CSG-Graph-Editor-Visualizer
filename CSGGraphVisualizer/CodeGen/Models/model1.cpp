#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "../bounding_opt.h"
#include "../codegen.h"
#include "glm/glm.hpp"
#include <cmath>
#include <fstream>
#include "models.h"

using namespace std;
using namespace glm;
namespace Models {


MatSdfExpr* horizontalCylinder1(int mat, float cylinderRadius, float baseHeight) {
    return offset(0.01f, move({ 0.0f, baseHeight + 0.5f, 0.0f }, cylinder(Dir1D::X, cylinderRadius, 0.5f, Fields{ mat })));
}

MatSdfExpr* horizontalCylinder2(int mat, float cylinderRadius, float baseHeight) {
    return offset(0.01f, move({ 0.0f, baseHeight + 0.5f, 0.0f }, cylinder(Dir1D::Z, cylinderRadius, 0.5f, Fields{ mat })));
}

MatSdfExpr* verticalCylinder(int mat, float cylinderRadius, float baseHeight) {
    float heightHalf = (1.0f + baseHeight) / 2;
    return offset(0.01f, move({ 0.0f, heightHalf, 0.0f }, cylinder(Dir1D::Y, cylinderRadius, heightHalf, Fields{ mat })));
}

MatSdfExpr* mySphere(int mat, float cylinderRadius, float baseHeight) {
    return move({ 0.0f, baseHeight + 0.5f, 0.0f }, sphere(cylinderRadius + (0.5f - cylinderRadius) * 0.8f, Fields{ mat }));
}

MatSdfExpr* base(int mat, float baseSize, float baseHeight) {
    return offset(0.01f, box(baseSize / 2, baseHeight / 2, baseSize / 2, Fields{ mat }));
}

/// <summary>
/// Makes the 1st type of model.
/// </summary>
/// <param name="cylinderRadius">No more than 1.0f.</param>
MatSdfExpr* model1_expr(float cylinderRadius, float baseSize, float baseHeight) {
    return union_op<Fields>(
        horizontalCylinder1(1, cylinderRadius, baseHeight),
        horizontalCylinder2(1, cylinderRadius, baseHeight),
        mySphere(1, cylinderRadius, baseHeight),
        verticalCylinder(1, cylinderRadius, baseHeight),
        base(1, baseSize, baseHeight)
    );
}
	
};