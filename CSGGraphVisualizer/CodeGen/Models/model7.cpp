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
MatSdfExpr* tubeModel7(int mat, Dir1D dir, float r1, float r2, float heightHalf) {
    return subtract<Fields>(
        cylinder(dir, r1, heightHalf, Fields{ mat }),
        cylinder(dir, r2, 2 * heightHalf, Fields{ mat })
    );
}

MatSdfExpr* model7_expr(float boxWidth, float boxHeight, int N, int M) {
    float holeRadius = boxWidth * 0.1f;
    float tubeLength = boxWidth * 0.7f;

    MatSdfExpr* out = nullptr;
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < M; k++) {
            MatSdfExpr* box = model5_expr(boxWidth, boxHeight, holeRadius);
            box = move({ i * (boxWidth + tubeLength), 0.0f, k * (boxWidth + tubeLength) }, box);

            if (out == nullptr) {
                out = box;
            } else {
                out = union_op<Fields>(box, out);
            }

            if (i != N - 1) {
                MatSdfExpr* tube = tubeModel7(1, Dir1D::X, holeRadius, holeRadius * 0.9f, tubeLength / 2);
                out = union_op<Fields>(out, 
                    move({ i * (boxWidth + tubeLength) + boxWidth / 2 + tubeLength / 2, -boxHeight / 4, k * (boxWidth + tubeLength) }, tube));
            }

            if (k != M - 1) {
                MatSdfExpr* tube = tubeModel7(1, Dir1D::Z, holeRadius, holeRadius * 0.9f, tubeLength / 2);
                out = union_op<Fields>(out,
                    move({ i * (boxWidth + tubeLength), -boxHeight / 4, k * (boxWidth + tubeLength) + boxWidth / 2 + tubeLength / 2 }, tube));
            }
        }
    }

    return out;
}
};