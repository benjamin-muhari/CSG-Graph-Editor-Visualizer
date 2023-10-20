#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

template<typename Fields>
struct Material {

    struct Carrier {
        std::string code;
        int reg;
    };

    struct State {
        int next_reg = 0;
        glm::vec3 move;
        glm::mat3 rotate = glm::mat3(1);
    } state;
    Carrier operator()(Box<Fields> expr) {
        return assign_register("box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ").x;\n", expr.material);
    }
    Carrier operator()(Sphere<Fields> expr) {
        return assign_register("sphere(" + move_rotate() + ", " + float1(expr.r) + ").x;\n", expr.material);
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        return assign_register("cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? float1(expr.x) : float2(expr.x, expr.y)) + ").x;\n", expr.material);
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        return assign_register("planeXZ(" + move_rotate() + ").x;\n", expr.material);
    }
    Carrier operator()(Offset<Fields> expr) {
        auto sub_expr = visit(*this, *expr.a);
        sub_expr.code += reg_name(sub_expr.reg) + " -= " + float1(expr.r) + ";\n";
        return sub_expr;
    }
    Carrier operator()(Union<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            std::string r0 = reg_name(result.reg), r1 = reg_name(sub_expr.reg);
            result.code += sub_expr.code + r0 + " = " + "min(" + r0 + ',' + r1 + ");\n" +
                "if (" + r0 + " == " + r1 + ") " + mat_name(result.reg) + " = " + mat_name(sub_expr.reg) + ";\n";
        }
        return result;
    }
	Carrier operator()(Intersect<Fields> expr) {
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            std::string r0 = reg_name(result.reg), r1 = reg_name(sub_expr.reg);
            result.code += sub_expr.code + r0 + " = " + "max(" + r0 + ',' + r1 + ");\n" +
                "if (" + r0 + " == " + r1 + ") " + mat_name(result.reg) + " = " + mat_name(sub_expr.reg) + ";\n";
        }
        return result;
    }
    Carrier operator()(Invert<Fields> expr) {
        auto result = visit(*this, *expr.a);
    	result.code += reg_name(result.reg) + " *= -1.f;\n";
        return result;
    }
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

    Carrier assign_register(std::string code, int material) {
        std::string reg = reg_name(state.next_reg), mat = mat_name(state.next_reg);
        return { "float " + reg + " = " + code + "int " + mat + " = " + std::to_string(material) + ";\n", state.next_reg++ };
    }
    static std::string reg_name(int id) {
        return "r" + std::to_string(id);
    }
    static std::string mat_name(int id) {
        return "m" + std::to_string(id);
    }
    /*std::string move_rotate() {
        return "mul(p - " + float3(state.move) + ", " + float3x3(state.rotate) + "), mul(v, " + float3x3(state.rotate) + ")";
    }*/
	std::string move_rotate() {
        return "("+float3x3(state.rotate) + "*(p - " + float3(state.move)+")";
    }
};

template<typename Fields>
std::string material(Expr<Fields> expr) {
    using namespace std::string_literals;
    auto result = visit(Material<Fields>{}, expr);
    return
        "#define RGB(r,g,b) (vec3(r,g,b)/255.0)\n"
        "const static Material setup = { gAmbient * gDiffuse, gDiffuse, gCookRoughness, gCookIOR };\n"
        "const static Material blue = { RGB(34,41,58) * RGB(52,54,255),RGB(52,54,255),RGB(18,18,18),float3(1.52,1.52,1.52) };\n"
        "const static Material red = { RGB(55,31,31) * RGB(139,25,25),RGB(139,25,25),RGB(18,18,18),float3(1.52,1.52,1.52) };\n"
        "const static Material green = { RGB(27,44,27) * RGB(48,152,36),RGB(48,152,36),RGB(18,18,18),float3(1.52,1.52,1.52) };\n"
        "const static Material yellow = { RGB(13, 13, 7) * RGB(248, 255, 27), RGB(248, 255, 27), RGB(18, 18, 18), float3(1.52, 1.52, 1.52) };\n"
        "const static Material colors[] = { setup, blue, red, green, yellow };\n" + 
        "Material material(vec4 sp)\n{\nvec3 p = sp.xyz;\n"s + result.code + "return colors[" + Material<Fields>::mat_name(result.reg) + "];\n}\n";
}