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

MatSdfExpr* model9_expr(float r, float height, int holeCount, int N, int M) {
    float tubeLength = r * 0.5f;

    MatSdfExpr* out = nullptr;
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < M; k++) {
            MatSdfExpr* model = model3_expr(r, height, holeCount);
            model = move({ i * (2 * r + tubeLength), 0.0f, k * (2 * r + tubeLength) }, model);

            if (out == nullptr) {
                out = model;
            }
            else {
                out = union_op<Fields>(model, out);
            }

            if (i != N - 1) {
                MatSdfExpr* tube = subtract<Fields>(box(tubeLength / 2 * 1.1f, height * 0.7f / 2, tubeLength / 2, Fields{ 1 }),
                    cylinder(Dir1D::Y, tubeLength * 0.3, tubeLength, Fields{ 1 }));
                out = union_op<Fields>(out,
                    move({ i * (2 * r + tubeLength) + 2 * r / 2 + tubeLength / 2, 0.0f, k * (2 * r + tubeLength) }, tube));
            }

            if (k != M - 1) {
                MatSdfExpr* tube = box(tubeLength / 2, height * 0.7f / 2, tubeLength / 2 * 1.1f, Fields{ 1 });
                out = union_op<Fields>(out,
                    move({ i * (2 * r + tubeLength), 0.0f, k * (2 * r + tubeLength) + 2 * r / 2 + tubeLength / 2 }, tube));
            }
        }
    }

    return out;
}
};