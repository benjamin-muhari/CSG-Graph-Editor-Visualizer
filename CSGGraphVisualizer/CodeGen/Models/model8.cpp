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
MatSdfExpr* tubeModel8(int mat, Dir1D dir, float r1, float r2, float heightHalf) {
    return subtract<Fields>(
        cylinder(dir, r1, heightHalf, Fields{ mat }),
        cylinder(dir, r2, 2 * heightHalf, Fields{ mat })
        );
}

MatSdfExpr* doubleModel2(Model2Attributes attr, int tubeCount, float tubeLength) {
    MatSdfExpr* model2 = model2_expr(attr);
    MatSdfExpr* out = union_op<Fields>(
        move({ 0.0f, 0.0f, (attr.zSize + tubeLength) / 2 }, model2),
        move({ 0.0f, 0.0f, -(attr.zSize + tubeLength) / 2 }, model2));

    float tubeWidth = attr.xSize * 0.2f / (2 * tubeCount);
    for (int i = 0; i < 2 * tubeCount; i += 2) {
        out = union_op<Fields>(out, 
            move({ -attr.xSize * 0.4 + i * tubeWidth, attr.ySize * 0.1f, 0.0f },
                box(tubeWidth / 2, attr.ySize * 0.8 / 2, tubeLength / 2, Fields{ 1 })),
            move({ attr.xSize * 0.4 - i * tubeWidth, attr.ySize * 0.1f, 0.0f },
                box(tubeWidth / 2, attr.ySize * 0.8 / 2, tubeLength / 2, Fields{ 1 }))
            );
    }

    return out;
}

MatSdfExpr* model8_expr(int N, int M, int connectTubeCount, Model2Attributes attr) {
    float connectBoxLength = attr.zSize * 0.3f;
    float connectTubeLength = glm::max(attr.xSize, attr.zSize) * 0.25f;
    MatSdfExpr* doubleModel = doubleModel2(attr, connectTubeCount, connectBoxLength);
    MatSdfExpr* out = nullptr;

    float holeRadius = attr.ySize * 0.3f;

    for (int i = 0; i < N; i++) {
        for (int k = 0; k < M; k++) {
            float xPos = i * (attr.xSize + connectTubeLength);
            float zPos = k * (2.0f * attr.zSize + connectBoxLength + connectTubeLength);
            MatSdfExpr* add = move({ xPos , 0.0f, zPos }, doubleModel);

            if (out == nullptr) {
                out = add;
            } else {
                out = union_op<Fields>(out, add);
            }


            if (i != N - 1) {
                MatSdfExpr* tube = tubeModel8(1, Dir1D::X, holeRadius, holeRadius * 0.9f, connectTubeLength / 2);
                out = union_op<Fields>(out,
                    move( { xPos + attr.xSize / 2 + connectTubeLength / 2, holeRadius, zPos + attr.zSize / 2 + connectBoxLength / 2 }, tube),
                    move({ xPos + attr.xSize / 2 + connectTubeLength / 2, holeRadius, zPos - attr.zSize / 2 - connectBoxLength / 2 }, tube));
            }

            if (k != M - 1) {
                MatSdfExpr* tube = tubeModel8(1, Dir1D::Z, holeRadius, holeRadius * 0.9f, connectTubeLength / 2);
                out = union_op<Fields>(out,
                    move({ xPos, holeRadius, zPos + attr.zSize + connectTubeLength / 2 + connectBoxLength / 2 }, tube));
            }
        }
    }

    return out;
}
};