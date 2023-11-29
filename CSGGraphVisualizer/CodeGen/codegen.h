#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

template<typename Fields>
struct CodeGen {
	
    using Carrier = CodeCarrier;

    struct State {
    	std::string functions;
        int next_reg        = 0;
        int next_fun        = 0;
        glm::vec3 move      = glm::vec3(0);
        glm::mat3 rotate    = glm::mat3(1);
    	float offset = 0;
    } state;

    Carrier operator()(Box<Fields> expr) {
        return Var("float","box(" + move_rotate() + ", " + float3(expr.x, expr.y, expr.z) + ")" + offset());
    }
    Carrier operator()(Sphere<Fields> expr) {
        return Var("float","sphere(" + move_rotate() + ", " + float1(expr.r) + ")" + offset());
    }
    Carrier operator()(Cylinder<Fields> expr) {
        using namespace std::string_literals;
        return Var("float","cylinder"s + (char)expr.dir + "(" + move_rotate() + ", " +
            (expr.y == std::numeric_limits<float>::infinity() ? float1(expr.x) : float2(expr.x, expr.y)) + ")" + offset());
    }
    Carrier operator()(PlaneXZ<Fields> expr) {
        return Var("float","(" + move_rotate() + ").y" + offset());
    }
    Carrier operator()(Offset<Fields> expr) {
        if (!expr.a)
            return Empty();
		state.offset += expr.r;
        return visit(*this, *expr.a);
    }
    Carrier operator()(Invert<Fields> expr) {
        if (!expr.a)
            return Empty();
        auto result = visit(*this, *expr.a);
        if (result.code != "")
    	    result += result.reg + " *= -1.; //invert";
		state.offset *=-1;
        return result;
    }
    Carrier operator()(Intersect<Fields> expr) {
        if (expr.a.size() == 0)
            return Empty();
		float off = state.offset; state.offset = 0;
        // This can still be empty, handle inside for loop
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            // Ignore empty/meaningless members of intersect
            if (sub_expr.code != "")
            {
                // If the intersect was so far empty
                if (result.code == "")
                {
                    // Then its register was not set, set it
                    result.reg = sub_expr.reg;
                    // Also don't need max() operation on first element
                    result.code += sub_expr.code;
                }
                else
                {
                    result.code += sub_expr.code;
                    result += result.reg + " = " + "max(" + result.reg + ',' + sub_expr.reg + "); //intersect " + std::to_string(i);
                }
            }
        }
		if(off != 0) result += result.reg + " -= " + float1(off) + ';';
        return result;
    }
	Carrier operator()(Union<Fields> expr) {
        if (expr.a.size() == 0)
            return Empty();
		float off = state.offset; state.offset = 0;
        // This can still be empty, handle inside for loop
        auto result = visit(*this, *expr.a[0]);
        for (int i = 1; i < (int)expr.a.size(); ++i) {
            auto sub_expr = visit(*this, *expr.a[i]);
            // Ignore empty/meaningless members of union
            if (sub_expr.code != "")
            {
                // If the union was so far empty
                if (result.code == "")
                {
                    // Then its register was not set, set it
                    result.reg = sub_expr.reg;
                    // Also don't need min() operation on first element
                    result.code += sub_expr.code;
                }
                else
                {
                    result.code += sub_expr.code;
                    result += result.reg + " = " + "min(" + result.reg + ',' + sub_expr.reg + "); //union " + std::to_string(i);
                }
            }
        }
		if(off != 0) result += result.reg + " -= " + float1(off) + ';';
        return result;
    }
    Carrier operator()(Move<Fields> expr) {
        if (!expr.a)
            return Empty();
        auto prev_move = state.move;
        state.move += expr.v;
        auto result = visit(*this, *expr.a);
        state.move = prev_move;
        return result;
    }
    Carrier operator()(Rotate<Fields> expr) {
        if (!expr.a)
            return Empty();
        auto prev_rotate = state.rotate;
        state.rotate *= expr.m;
        auto result = visit(*this, *expr.a);
        state.rotate = prev_rotate;
        return result;
    }
    std::string move_rotate() {
        glm::vec3 m = state.rotate * state.move;
    	return (state.rotate == glm::mat3(1.f)? "" : float3x3(state.rotate) + "*")
    		+   'p'
    		+  (m == glm::vec3(0)             ? "" : " - " + float3(m));
    }
	std::string offset() {
		const float off = state.offset;	state.offset = 0;
		return off == 0.f ? "" : "-" + float1(off);
	}

    // Defines a new function based on the code within code carrier and returns the name of the function.
	std::string Fun(const Carrier &carr, const std::string &arguments = "vec3 p")
    {
    	//state.next_reg = 0;
    	std::string name = "f" + std::to_string(state.next_fun++);
    	state.functions += + "\n" + carr.reg_type + " " + name + "(" + arguments + "){\n"
    						+ carr.code
    						+"\treturn " + carr.reg
    					+ ";\n}\n";
	    return name;
    }

	// Initializes a new variable of type type_ with value_ and adds the line into existing code carrier carr_.
	std::string Var(const std::string& type_, const std::string &value_, Carrier &carr_)
    {
        std::string name = "r" + std::to_string(state.next_reg);
    	++state.next_reg;
    	carr_.code += "\t" + type_ + " " + name + " = " + value_ + ";\n"  ;
    	return name;
    }

	// Initializes a new variable of type type_ with value_, and returns the corresponding code carrier.
	Carrier Var(const std::string& type_, const std::string &value_)
    {
    	Carrier carr = {"","",type_};
    	carr.reg = Var(type_, value_, carr);
    	return carr;
    }

    Carrier Empty()
    {
        Carrier carr;

        // Adds redundant line: rx = min(rx,rx) after warning, not ideal
        //carr.code += "\t//Warning: Empty node\n";
        //carr.reg = "r" + std::to_string(state.next_reg-1);
        //return carr;
        // 
        // Comments entire line, bugs if the empty node is first
        //carr.code += "\t//Warning: Empty node:";
        //carr.reg = "empty node";
        //carr.reg_type = "empty node";
        //return carr;

        // Moved this outside of the visitor into sdf()
        //if (state.next_reg == 0)
        //    return Carrier{ "\tfloat r0 = 1.0 / 0.0;\n", "r0", "float"};
        // 
        // Better handle the empty register values when they need to be handled, dont add confusing code
        //return Carrier{"", "r" + std::to_string(state.next_reg), ""};

        return carr;
    }
};

template<typename Fields>
std::string sdf(Expr<Fields> expr) {
    auto result = visit(CodeGen<Fields>{}, expr);
    if (result.code == "")
        result = CodeCarrier{ "\tfloat r0 = 1.0 / 0.0;\n", "r0", "float" };

    return "float sdf(vec3 p) {\n" + result.code + "\treturn " + result.reg + ";\n}\n";
}