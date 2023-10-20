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
MatSdfExpr* tubeModel3(int mat, Dir1D dir, float r1, float r2, float heightHalf) {
    return subtract<Fields>(
        cylinder(dir, r1, heightHalf, Fields{ mat }), 
        cylinder(dir, r2, 2 * heightHalf, Fields{ mat })
    );
}

MatSdfExpr* baseModel3(int mat, float r, float innerR, float baseHeight) {
    return union_op<Fields>(
        tubeModel3(mat, Dir1D::Y, r, innerR, baseHeight / 2),
        tubeModel3(mat, Dir1D::Y, innerR, 0.9f * innerR, 1.2f * baseHeight / 2)
    );
}

MatSdfExpr* holeCutterModel3(int mat, float outerR, float baseHeight) {
    float innerR = outerR * 0.7f;
    float baseHeightHalf = baseHeight / 2;
    float outerHeightHalf = baseHeightHalf * 0.7f;

    return union_op<Fields>(
        move({ 0.0f, baseHeightHalf, 0.0f }, cylinder(Dir1D::Y, outerR, outerHeightHalf, Fields{ mat })),
        move({ 0.0f, -baseHeightHalf, 0.0f }, cylinder(Dir1D::Y, outerR, outerHeightHalf, Fields{ mat })),
        cylinder(Dir1D::Y, innerR, baseHeight, Fields{ mat })
    );
}

MatSdfExpr* cutBaseModel3(int mat, float r, float height, int holeCount) {
    float cutoutRadius = r * 0.25f;
    float baseInnerR = r * 0.3f;
    float baseHeight = height * 0.7f;
    float tubeHeight = height - baseHeight;

    // if we imagine a ring on the base at this radius, it is where the cutouts are placed around
    float cutoutBaseRadius = baseInnerR + cutoutRadius + r * 0.11f;

    MatSdfExpr* base = baseModel3(mat, r, baseInnerR, baseHeight);
    MatSdfExpr* cutout = holeCutterModel3(mat, cutoutRadius, baseHeight);
    MatSdfExpr* tubes = tubeModel3(mat, Dir1D::Y, cutoutRadius * 0.35f, cutoutRadius * 0.32f, tubeHeight);

    int addedObjectCound = 2 * holeCount;
    for (int i = 0; i < addedObjectCound; i += 2) {
        float rad = 2 * M_PI / addedObjectCound * i;
        float rad2 = 2 * M_PI / addedObjectCound * (i + 1);

        base = subtract<Fields>(base, move({ cutoutBaseRadius * cosf(rad), 0.0f, cutoutBaseRadius * sinf(rad) }, cutout));
        base = union_op<Fields>(base, move({ cutoutBaseRadius * cosf(rad2), (baseHeight + tubeHeight) / 2, cutoutBaseRadius * sinf(rad2) }, tubes));
    }

    return base;
}


MatSdfExpr* model3_expr(float r, float height, int holeCount) {
    return cutBaseModel3(1, r, height, holeCount);
}
};