#pragma once

#include "../expr.h"
#include <string>

struct Fields { int material; };
using MatSdfExpr = Expr<Fields>;
// CHANGED

namespace Models {

	struct Model2Attributes {
		float xSize = 1.0f;
		float ySize = 0.3f;
		float zSize = 0.5f;
		int xRidgeCount = 6;
		int yRidgeCount = 3;
	};

	MatSdfExpr* model1_expr(float cylinderRadius = 0.25f, float baseSize = 0.8f, float baseHeight = 0.2f);
	MatSdfExpr* model2_expr(Model2Attributes attr = Model2Attributes{});
	MatSdfExpr* model3_expr(float r = 1.0f, float height = 0.3f, int holeCount = 4);
	MatSdfExpr* model4_expr(float width = 1.0f, float height = 1.0f);
	MatSdfExpr* model5_expr(float width = 1.0f, float height = 1.0f, float holeRadius = 0.1f);
	MatSdfExpr* model7_expr(float boxWidth = 1.0f, float boxHeight = 1.0f, int N = 3, int M = 3);
	MatSdfExpr* model8_expr(int N = 2, int M = 2, int connectTubeCount = 3, Model2Attributes attr = Model2Attributes{});
	MatSdfExpr* model9_expr(float r = 1.0f, float height = 0.3f, int holeCount = 4, int N = 3, int M = 3);
	MatSdfExpr* toys_exp();
}

void build_kernel(const std::string& file_name, MatSdfExpr* expr);
void build_footmap(const std::string &file_name, MatSdfExpr* expr, int iterations = 5);