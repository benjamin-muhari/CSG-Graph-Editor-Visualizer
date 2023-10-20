#define GLM_FORCE_SWIZZLE
#define _USE_MATH_DEFINES
#include "bounding_opt.h"
#include "codegen.h"
#include "Material2.h"
#include "Footmap.h"
#include "glm/glm.hpp"
#include <cmath>
#include <fstream>
#include "Models/models.h"
/*
const std::string hlsl_prefix =
R"(#ifndef SDF_HLSL
#define SDF_HLSL

#include "SDF/primitives.hlsl"

)";
const std::string hlsl_postfix = "\n#endif\n";
*/
const std::string glsl_prefix = "// Start of generated GLSL code\n";
const std::string glsl_postfix = "// End of generated GLSL code\n";

void build_kernel(const std::string& file_name, MatSdfExpr* expr) {
    std::ofstream kernel(file_name);
    kernel << glsl_prefix << sdf(*expr) << "\n" << material2(*expr) << glsl_postfix;
}