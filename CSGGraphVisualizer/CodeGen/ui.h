#pragma once

#include "expr.h"
#include "util.h"
#include "glm/glm.hpp"
#include <array>
#include <string>

#include <ImGui/imgui.h>

template<typename Fields>
struct UI {

    struct Carrier {
    };

    struct State {
    } state;

    Carrier operator()(Box<Fields> &expr) {
		if(ImGui::TreeNode("Box"))
		{
			//ImGui::SameLine(); ImGui::InputInt("MatID", &expr.material);
			ImGui::DragFloat3("Size", &expr.x);
			ImGui::TreePop();
		}
		return Carrier();
    }
    Carrier operator()(Sphere<Fields> &expr) {
		if(ImGui::TreeNode("Sphere"))
		{
			//ImGui::SameLine();  ImGui::InputInt("MatID", &expr.material);
			ImGui::DragFloat("Radius", &expr.r);
			ImGui::TreePop();
		}
		return Carrier();
    }
    Carrier operator()(Cylinder<Fields> &expr) {
		if(ImGui::TreeNode("Cylinder"))
		{
			//ImGui::SameLine();  ImGui::InputInt("MatID", &expr.material);
			//TODO Dir1D !!
			bool b = expr.y == std::numeric_limits<float>::infinity();
			if(b)	ImGui::DragFloat("Radius", &expr.x);
			else	ImGui::DragFloat2("Radius & Height", &expr.x);	
			ImGui::SameLine(); ImGui::Checkbox("Inf", &b);
			ImGui::TreePop();
		}
		return Carrier();
    }
    Carrier operator()(PlaneXZ<Fields> &expr) {
		if (ImGui::TreeNode("PlaneXZ"))
		{
			//ImGui::SameLine();  ImGui::InputInt("MatID", &expr.material);

			ImGui::TreePop();
		}
		return Carrier();
    }
    Carrier operator()(Offset<Fields> &expr) {
			//todo choose children without Offset
		if (ImGui::TreeNode("Offset"))
		{
			ImGui::DragFloat("Offset", &expr.r);
	        visit(*this, *expr.a);
			ImGui::TreePop();
		}
    	return Carrier();
    }
    const std::array<std::string, 2> op_names{ "Union", "Intersect" };
    Carrier operator()(Union<Fields> &expr) {
		if (ImGui::TreeNode("Union"))
		{
			ImGui::PushID("Union");
			for (int i = 0; i < (int)expr.a.size(); ++i) {
				ImGui::PushID(i);
				visit(*this, *expr.a[i]);
				ImGui::PopID();
			}
			ImGui::PopID();
			ImGui::TreePop();
		}
		return Carrier();
    }
	    Carrier operator()(Intersect<Fields> &expr) {
		if (ImGui::TreeNode("Intersect"))
		{
			ImGui::PushID("Intersect");
			for (int i = 0; i < (int)expr.a.size(); ++i) {
				ImGui::PushID(i);
				visit(*this, *expr.a[i]);
				ImGui::PopID();
			}
			ImGui::PopID();
			ImGui::TreePop();
		}
		return Carrier();
    }
    Carrier operator()(Invert<Fields> &expr) {
        if (ImGui::TreeNode("Invert")) {
            ImGui::PushID(0);
            visit(*this, *expr.a);
            ImGui::PopID();
            ImGui::TreePop();
        }
        return Carrier();
    }
    Carrier operator()(Move<Fields> &expr) {
		if (ImGui::TreeNode("Move")) {
			ImGui::DragFloat3("Translation", &expr.v.x);
			visit(*this, *expr.a);
			ImGui::TreePop();
		}
    	return Carrier();
    }
    Carrier operator()(Rotate<Fields> &expr) {
		if (ImGui::TreeNode("Rotate")) {
			ImGui::TextUnformatted("Rotate...");
			visit(*this, *expr.a);
			ImGui::TreePop();
		}
    	return Carrier();
    }
};

template<typename Fields>
void RenderCSG_UI(Expr<Fields> &expr) {
	std::visit(UI<Fields>{}, expr);
}