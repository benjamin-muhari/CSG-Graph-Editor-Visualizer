#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>
#include <vector>

template<typename Bounder, typename Fields>
struct BoundOpt {

    float thr;
    Bounder bounder;

    struct Carrier {
        std::string code, reg;
        typename Bounder::Shape shape;
    };

    struct State {
        int next_reg = 0;
        glm::vec3 move;
        glm::mat3 rotate = glm::mat3(1);
    } state;

    Carrier operator()(Box<Fields> expr) {
        auto result = assign_register("box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ");\n");
        result.shape = bounder(expr, state.move, state.rotate);
        return result;
    }
    Carrier operator()(Sphere<Fields> expr) {
        auto result = assign_register("sphere(" + move_rotate() + ", " + std::to_string(expr.r) + ");\n");
        result.shape = bounder(expr, state.move, state.rotate);
        return result;
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        auto result = assign_register("cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? std::to_string(expr.x) : float2(expr.x, expr.y)) + ");\n");
        result.shape = bounder(expr, state.move, state.rotate);
        return result;
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        auto result = assign_register("planeXZ(" + move_rotate() + ");\n");
        result.shape = bounder(expr, state.move, state.rotate);
        return result;
    }
    Carrier operator()(Offset<Fields> expr) {
        auto sub_expr = visit(*this, *expr.a);
        sub_expr.code += sub_expr.reg + " = Offset(" + sub_expr.reg + ", " + std::to_string(expr.r) + ");\n";
        sub_expr.shape = bounder(expr, sub_expr.shape);
        return sub_expr;
    }
    /*Carrier operator()(SetOp<Fields> expr) {
        static const std::array<std::string, 2> op_names{ "Union", "Intersect" };
        std::string op = op_names[(int)expr.type] + "(";
        auto result = visit(*this, *expr.a[0]);
        std::vector<typename Bounder::Shape> shapes{ result.shape };
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            shapes.push_back(sub_expr.shape);
            result.code += sub_expr.code + result.reg + " = " + op + result.reg + ',' + sub_expr.reg + ");\n";
        }
        result.shape = bounder(expr, shapes);
        result.code = check_bound(result);
        return result;
    }
    Carrier operator()(Subtract<Fields> expr) {
        auto result = visit(*this, *expr.a), b = visit(*this, *expr.b);
        result.code += b.code + result.reg + " = Substract(" + result.reg + ',' + b.reg + ");\n";
        result.shape = bounder(expr, result.shape, b.shape);
        result.code = check_bound(result);
        return result;
    }*/
    Carrier operator()(Move<Fields> expr) {
        auto prev_move = state.move;
        state.move += expr.v;
        auto result = visit(*this, *expr.a);
        state.move = prev_move;
        return result;
    }
    Carrier operator()(Rotate<Fields> expr) {
        auto prev_rotate = state.rotate;
        state.rotate *= expr.m;
        auto result = visit(*this, *expr.a);
        state.rotate = prev_rotate;
        return result;
    }

    Carrier assign_register(std::string code) {
        std::string reg = reg_name(state.next_reg++);
        return { reg + " = " + code, reg };
    }
    std::string reg_name(int id) {
        return "r" + std::to_string(id);
    }
    std::string move_rotate() {
        return "mul(p - " + float3(state.move) + ", " + float3x3(state.rotate) + "), mul(v, " + float3x3(state.rotate) + ")";
    }
    std::string check_bound(const Carrier& result) {
        std::string tmp = "bound_" + result.reg;
        return
            "float2 " + tmp + " = " + Bounder::get_dist(result.shape) + ";\n"
            "if(" + tmp + ".x < " + std::to_string(thr) + "){\n" + result.code +
            "}\nelse{\n " + result.reg + " = " + tmp + ";\n}\n";
    }
};

template<typename Fields>
struct BoundingSphere {

    struct Shape {
        glm::vec3 center;
        float r;
    };

    Shape operator()(const Box<Fields>& expr, const glm::vec3& move, const glm::mat3& rotate) { 
        return { move,length(expr.x,expr.y,expr.z) };
    }
    Shape operator()(const Sphere<Fields>& expr, const glm::vec3& move, const glm::mat3& rotate) { 
        return { move,expr.r };
    }
    Shape operator()(const Cylinder<Fields>& expr, const glm::vec3& move, const glm::mat3& rotate) { 
        return { move,expr.y == std::numeric_limits<float>::infinity() ? 999999999.f : length(expr.x,expr.y) };
    }
    Shape operator()(const PlaneXZ<Fields>& expr, const glm::vec3& move, const glm::mat3& rotate) { 
        return { move,999999999.f };
    }
    Shape operator()(const Offset<Fields>& expr, const Shape& a) { return { a.center,a.r + expr.r }; }
    Shape operator()(const Union<Fields>& expr, const std::vector<Shape>& shapes) {
		Shape result = shapes[0];
    	for (int i = 1; i < (int)shapes.size(); ++i) {
				float d = (float)(result.center - shapes[i].center).length();
				if (d + shapes[i].r > result.r) {
					float r = d + shapes[i].r;
					result.center += (shapes[i].center - result.center) * (r - result.r);
					result.r = r; // TODO DODODO this is not working either
				}
			}
    }
    Shape operator()(const Intersect<Fields>& expr, const std::vector<Shape>& shapes) {
		Shape result = shapes[0];
		for (int i = 1; i < (int)shapes.size(); ++i){
			if (result.r > shapes[i].r)
				result = shapes[i];
			/* TODO DODODODODODO
			float a = result.radius;
			float b = shapes[i].radius
			float c = glm::distance(result.center, shapes[i].center);
			if ( c > a+b){
				result.radius = 0;
				break;
			}
			if (b > c + a) continue;
			else (a > c + b) {
				result = shapes[i];
				continue;
			}
			float s = (a + b + c)*0.5f;
			float m2 = s * (s - a)*(s - b)*(s - c) / (c*c);
			float lambda = sqrt(a * a - m2)/c;
			result.center = glm::mix(result.center, shapes[i].center, lambda);
			result.radius = sqrt(m2);
			*/
        }
        return result;
    }
	/*
    Shape operator()(const Subtract<Fields>& expr, const Shape& a, const Shape& b) {
        return a;
    }*/

    static std::string get_dist(const Shape& shape) {
        return "sphere(p-" + float3(shape.center) + ",v," + std::to_string(shape.r) + ")";
    }
};

template<typename Bounder, typename Fields>
std::string sdf_boundopt(float threshold, Expr<Fields> expr) {
    BoundOpt<Bounder, Fields> bound_opt{ threshold, Bounder() };
    auto result = visit(bound_opt, expr);
    std::string alloc = "float2 " + bound_opt.reg_name(0);
    for (int i = 1; i < bound_opt.state.next_reg; ++i)
        alloc += ", " + bound_opt.reg_name(i);
    alloc += ";\n";
    return
        "float2 sdf(float3 p, float3 v)\n{\n" + alloc + result.code + "return " + result.reg + ";\n" +
        "}\nfloat sdf(float3 p) { return sdf(p, float3(0, 0, 0)).x; }\n";
}