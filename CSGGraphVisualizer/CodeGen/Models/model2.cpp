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


MatSdfExpr* base(int mat, float xSize, float ySize, float zSize) {
    return box(xSize / 2, ySize / 2, zSize / 2, Fields{ mat });
}

MatSdfExpr* walls(int mat, float wholeXSize, float wholeYSize, float wholeZSize) {
    float xSize = glm::min(wholeXSize * 0.05f, wholeZSize * 0.05f);
    float zSize = xSize;
    return union_op<Fields>(
        move({ wholeXSize * 0.5f - xSize / 2, wholeYSize * 0.5f, 0.0f },
            box(xSize / 2, wholeYSize / 2, wholeZSize / 2, Fields { mat })),
        move({ -wholeXSize * 0.5f + xSize / 2, wholeYSize * 0.5f, 0.0f },
            box(xSize / 2, wholeYSize / 2, wholeZSize / 2, Fields{ mat })),

        move({ 0.0f, wholeYSize * 0.5f, wholeZSize * 0.5f - zSize / 2 },
            box(wholeXSize / 2, wholeYSize / 2, zSize / 2, Fields{ mat })),
        move({ 0.0f, wholeYSize * 0.5f, -wholeZSize * 0.5f + zSize / 2 },
            box(wholeXSize / 2, wholeYSize / 2, zSize / 2, Fields{ mat }))
    );
}

MatSdfExpr* xRidges(int mat, float wholeXSize, float wholeYSize, float wholeZSize, int count) {
    int ridgeAndGapCount = 2 * count - 1;
    float xRidgeSize = wholeXSize * 0.4f / ridgeAndGapCount;
    float zRidgeSize = wholeZSize * 0.15f;
    
    MatSdfExpr* out = nullptr;
    float startingX = -xRidgeSize * ridgeAndGapCount / 2 + xRidgeSize / 2;
    for (int i = 0; i < 2 * count; i += 2) {
        if (i / 2 == count / 2 || (count % 2 == 0 && i == (count / 2 - 1) * 2)) {
            continue;
        }

        MatSdfExpr* newExpr1 = move({ startingX + i * xRidgeSize, wholeYSize * 0.5f, wholeZSize / 2 - zRidgeSize / 2 },
            box(xRidgeSize / 2, wholeYSize / 2, zRidgeSize / 2, Fields{ mat }));
        MatSdfExpr* newExpr2 = move({ startingX + i * xRidgeSize, wholeYSize * 0.5f, -wholeZSize / 2 + zRidgeSize / 2 },
            box(xRidgeSize / 2, wholeYSize / 2, zRidgeSize / 2, Fields{ mat }));
        
        if (out == nullptr) {
            out = union_op<Fields>(newExpr1, newExpr2);
        } else {
            out = union_op<Fields>(out, newExpr1, newExpr2);
        }
    }

    return out;
}

MatSdfExpr* zRidges(int mat, float wholeXSize, float wholeYSize, float wholeZSize, int count) {
    int ridgeAndGapCount = 2 * count - 1;
    float zRidgeSize = wholeZSize * 0.4f / ridgeAndGapCount;
    float xRidgeSize = wholeXSize * 0.15f;

    MatSdfExpr* out = nullptr;
    float startingZ = -zRidgeSize * ridgeAndGapCount / 2 + zRidgeSize / 2;
    for (int i = 0; i < 2 * count; i += 2) {
        MatSdfExpr* newExpr1 = move({ wholeXSize / 2 - xRidgeSize / 2 , wholeYSize * 0.5f, startingZ + i * zRidgeSize },
            box(xRidgeSize / 2, wholeYSize / 2, zRidgeSize / 2, Fields{ mat }));
        MatSdfExpr* newExpr2 = move({ -wholeXSize / 2  +xRidgeSize / 2 , wholeYSize * 0.5f, startingZ + i * zRidgeSize },
            box(xRidgeSize / 2, wholeYSize / 2, zRidgeSize / 2, Fields{ mat }));

        if (out == nullptr) {
            out = union_op<Fields>(newExpr1, newExpr2);
        }
        else {
            out = union_op<Fields>(out, newExpr1, newExpr2);
        }
    }

    return out;
}

/// <summary>
/// Makes the 2nd type of model.
/// </summary>
MatSdfExpr* model2_expr(Model2Attributes attr) {
    return union_op<Fields>(
        base(1, attr.xSize, attr.ySize * 0.2f, attr.zSize),
        walls(1, attr.xSize, attr.ySize, attr.zSize),
        xRidges(1, attr.xSize, attr.ySize, attr.zSize, attr.xRidgeCount),
        zRidges(1, attr.xSize, attr.ySize, attr.zSize, attr.yRidgeCount)
    );
}
};