#include "ImguiNodeEditor/imgui_node_editor.h"
#include "application.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>

#include "CodeGen/expr.h"
#include "CodeGen/codegen.h"
#include "CodeGen/Material2.h"
//#include "CodeGen/Objects/toys.cpp"
#include <fstream>

#include <string>
#include <vector>

#define GLM_FORCE_SWIZZLE
#include "CodeGen/Models/models.h"
#include "CodeGen/util.h"

using namespace std;
using namespace glm;

namespace ed = ax::NodeEditor;

enum class PrimitiveType
{
    Sphere,
    Box,
    CylinderY
};

enum class OperationType
{
    Union,
    Intersection,
    Substraction,
    Translation,
    Rotation,
    Offset,
    Scale
};

enum class TransformType
{
    Translation,
    Rotation,
    Scale
};

struct CsgNode {
    ed::NodeId id;
    int material;
};

// Struct to hold basic information about connection between
// pins. Note that connection (aka. link) has its own ID.
// This is useful later with dealing with selections, deletion
// or other operations.
struct LinkInfo
{
    ed::LinkId Id;
    ed::PinId  InputId;
    ed::PinId  OutputId;
};

static ed::EditorContext*   g_Context = nullptr;    // Editor context, required to trace a editor state.
static bool                 g_FirstFrame = true;    // Flag set for first frame only, some action need to be executed once.
static ImVector<LinkInfo>   g_Links;                // List of live links. It is dynamic unless you want to create read-only view over nodes.
static int                  g_NextLinkId = 100;     // Counter to help generate link ids. In real application this will probably based on pointer to user data structure.

const char* Application_GetName()
{
    return "CSG tree editor";
}

void Application_Initialize()
{
    ed::Config config;
    config.SettingsFile = "CSG_tree_editor.json";
    g_Context = ed::CreateEditor(&config);
}

void Application_Finalize()
{
    ed::DestroyEditor(g_Context);
}

void ImGuiEx_BeginColumn()
{
    ImGui::BeginGroup();
}

void ImGuiEx_NextColumn()
{
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
}

void ImGuiEx_EndColumn()
{
    ImGui::EndGroup();
}

void Create_Operation_Node(int& uniqueId, const char* type, ImVec2 pos)
{
    ed::NodeId nodeA_Id = uniqueId++;
    ed::PinId  nodeA_InputPinId = uniqueId++;
    ed::PinId  nodeA_InputPinId_2 = uniqueId++;
    ed::PinId  nodeA_OutputPinId = uniqueId++;

    if (g_FirstFrame)
        ed::SetNodePosition(nodeA_Id, pos);
    ed::BeginNode(nodeA_Id);
    ImGui::Text(type);

    ed::BeginPin(nodeA_InputPinId, ed::PinKind::Input);
    ImGui::Text(">  ");
    ed::EndPin();

    ImGui::SameLine();
    ed::BeginPin(nodeA_OutputPinId, ed::PinKind::Output);
    ImGui::Text("Result");
    ed::EndPin();

    ed::BeginPin(nodeA_InputPinId_2, ed::PinKind::Input);
    ImGui::Text(">  ");
    ed::EndPin();

    ed::EndNode();
}

void Create_Primitive_Node(int& uniqueId, PrimitiveType type, ImVec2 node_pos)
{
    ImGui::PushItemWidth(150);

    ed::NodeId node_Id = uniqueId++;
    ed::PinId  node_OutputPinId = uniqueId++;
    ed::PinId  node_TransformPinId = uniqueId++;

    if (g_FirstFrame)
        ed::SetNodePosition(node_Id, node_pos);    
    ed::BeginNode(node_Id);    
    std::string type_s;
    switch (type) {
        case (PrimitiveType::Box):
            type_s = "Box";
            break;
        case (PrimitiveType::CylinderY):
            type_s = "CylinderY";
            break;
        case (PrimitiveType::Sphere):
            type_s = "Sphere";
            break;
    }
    ImGui::Text(type_s.c_str());

    ImGui::SameLine();
    ed::BeginPin(node_OutputPinId, ed::PinKind::Output);
    ImGui::Text("  >");
    ed::EndPin();

    const char* format = "%.1f";

    float pos[3] = {0.0,0.0,0.0};
    ImGui::InputFloat3("Pos", pos, format);
    
    switch (type) {
        case (PrimitiveType::Box):
            float sides[3];
            ImGui::InputFloat3("Sides", sides, format);
            break;
        case (PrimitiveType::CylinderY):
            break;
        case (PrimitiveType::Sphere):
            float rad = 0.0;
            ImGui::PushItemWidth(50);
            ImGui::InputFloat("Radius", &rad, 0.0F, 0.0F, format);
            ImGui::PopItemWidth();
            break;
    }

    ed::BeginPin(node_TransformPinId, ed::PinKind::Input);
    ImGui::Text("> Transforms");
    ed::EndPin();

    ed::EndNode();

    ImGui::PopItemWidth();
}

void Create_Transform_Node(int& uniqueId, TransformType type, ImVec2 node_pos)
{
    ImGui::PushItemWidth(150);

    ed::NodeId node_Id = uniqueId++;
    ed::PinId  node_OutputPinId = uniqueId++;
    ed::PinId  node_TransformPinId = uniqueId++;

    if (g_FirstFrame)
        ed::SetNodePosition(node_Id, node_pos);
    ed::BeginNode(node_Id);
    std::string type_s;
    switch (type) {
        case (TransformType::Rotation):
            type_s = "Rotation";
            break;
        case (TransformType::Scale):
            type_s = "Scale";
            break;
        case (TransformType::Translation):
            type_s = "Translation";
            break;
    }
    ImGui::Text((type_s + std::to_string(uniqueId)).c_str());

    ImGui::SameLine();
    ed::BeginPin(node_OutputPinId, ed::PinKind::Output);
    ImGui::Text("  >");
    ed::EndPin();

    const char* format = "%.1f";

    switch (type) {
        case (TransformType::Rotation):
            float rot_xyz[3];
            ImGui::InputFloat3("Around XYZ", rot_xyz, format);
            break;
        case (TransformType::Scale):
            float scale;
            ImGui::InputFloat("Scale", &scale, 0.0f, 0.0f, format);
            break;
        case (TransformType::Translation):
            float transl_xyz[3];
            ImGui::InputFloat3("XYZ", transl_xyz, format);
            break;
    }

    ed::EndNode();

    ImGui::PopItemWidth();
}

bool clicked_create = false;


ed::NodeId fake_id = 1;

const float g_offset = 0.25f;
Expr<CsgNode>* toyCube(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(0.9f, 0.9f, 0.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyBrickYthin(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.4f, 2.9f, 0.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyBrickXthin(int mat) { return offset(g_offset, move({ 0.f, 0.5f, 0.f }, box(2.9f, 0.4f, 0.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyBrickYthick(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, box(0.9f, 2.9f, 0.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyBrickXthick(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, CsgNode{ fake_id, mat }))); }
//Expr<CsgNode>* toyBrickXthick(int mat) { return move({ 0.f, 1.f, 0.f }, box(2.9f, 0.9f, 0.9f, MyFields{ mat })); }

Expr<CsgNode>* toyCylinder(int mat) { return offset(g_offset, move({ 0.f, 3.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 2.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyCylinderSmall(int mat) { return offset(g_offset, move({ 0.f, 1.f, 0.f }, cylinder(Dir1D::Y, 0.9f, 0.9f, CsgNode{ fake_id, mat }))); }

Expr<CsgNode>* toyArch(int mat) { return offset(g_offset, subtract<CsgNode>(toyBrickXthick(mat), move({ 0.f, -1.4f, 0.f }, cylinder(Dir1D::Z, 2.6f, CsgNode{ fake_id, mat })))); }

Expr<CsgNode>* toyRoof(int mat) {
    return offset(g_offset,
        intersect<CsgNode>(toyBrickXthick(mat),
            move({ 0.f, -1.5f, 0.f }, cylinder(Dir1D::Z, 3.3f, 0.9f, CsgNode{ fake_id, mat }))));
}

mat3 fitToyY(vec2 a, vec2 b) {
    vec2 v = normalize(b - a);
    return mat3(v.x, 0, -v.y, 0, 1, 0, v.y, 0, v.x);
}

Expr<CsgNode>* toys_exp2() {
    vec3 p1(0, 0, 0), p2(4, 0, 0);
    mat3 M12 = fitToyY(vec2(p1.x, p1.z), vec2(p1.x, p1.z));
    vec3 p3 = 4.0f * normalize(vec3(-1, 0, 2));
    mat3 M13 = fitToyY(vec2(p1.x, p1.z), vec2(p1.x, p1.z));
    mat3 M1 = rotateY(0.5);
    mat3 M2 = rotateY((float)-3.14159265 * 0.5f);
    mat3 M3 = rotateY(0.4f);
    mat3 M4 = rotateZ(0.42f);
    mat3 M5 = rotateY(0.123f);
    mat3 M1t = transpose(M1);

    Expr<CsgNode>* testsphere = sphere(1, CsgNode{ 4 });
    Expr<CsgNode>* testmove = move(vec3(1.20001, 1.00001, 3.20001), testsphere);

    testsphere = toyCube(1);

    return
        union_op<CsgNode>(
            planeXZ(CsgNode{ 3 }),
            move(p1 + vec3(-0.4, 0, 0.4), rotate(M1t, toyCube(1))),
            move(p2 + vec3(-0.1, 0, 1.4), rotate(M2, toyBrickXthick(0))),
            move(p1 + vec3(0, 4, 0), rotate(M5, toyCube(2))),
            move(p3, toyCylinder(0)),
            //move(0.5f * (p1 + p2) + vec3(0, 2, 0), rotate(M12, toyArch(1))),
            move(0.5f * (p1 + p2) + vec3(0, 2, 0), toyArch(1)),
            move(0.5f * (p1 + p3) + vec3(0, 6.5, 0), rotate(M13, toyRoof(2))),
            move(vec3(4, 4, 0), toyCylinderSmall(1)),
            move(vec3(8.5, 0, 1.8), rotate(M5, toyBrickYthick(0))),
            move(vec3(6.5, 6.25, 1), rotate(M3, toyBrickXthin(2))),
            move(vec3(6.625, 1.193, 4.036), rotate(M4, toyBrickXthin(0))),
            move(vec3(9, 8.5, 2.7), sphere(1, CsgNode{ 4 })),
            //move(vec3(1.2, 1, 3.2), sphere(1, CsgNode{ 4 })),
            testmove,
            move(vec3(1, 0, 6), rotate(M13, toyCube(1))));
}
//vec3 lightpos = vec3(5, 1, 6.5;
//vec3 lightpos = vec3(4,4.3,4.6);
//vec3 lightpos = vec3(1,3,6);
//vec3 lightpos = vec3(1.2,1,3.2);
//vec3 lightpos = vec3(9,8.5,2.7);



Expr<CsgNode> SelectRootNode()
{
    return *toys_exp2();
}

void GenerateSdf(Expr<CsgNode> &root)
{
    // TODO: change to .glsl
    std::ofstream kernel("sdf_editortest.cpp");
    kernel << sdf(root) << "\n" << material2(root);
}

Expr<CsgNode> root;

#include <iostream>

void Application_Frame(float ms)
{
    //ImGui::ShowDemoWindow();
   
    //auto& io = ImGui::GetIO();
    ed::SetCurrentEditor(g_Context);

    // Start interaction with editor.
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));

    std::vector<Expr<CsgNode>*> nodes;
    // std:: unordered_map (hashmap)

    int uniqueId = 1;

    float paneWidth = 200;
    //ImGui::BeginChild("Selection", ImVec2(paneWidth, 100));
   // paneWidth = ImGui::GetContentRegionAvailWidth();
    //ImGui::BeginHorizontal("Style Editor", ImVec2(paneWidth, 0));
    if (ImGui::Button("Select root"))
        root = SelectRootNode();
    ImGui::SameLine();
    if (ImGui::Button("Generate sdf"))
        GenerateSdf(root);
    //ImGui::EndHorizontal();
    //ImGui::EndChild();

    //Create_Operation_Node(uniqueId, "Union", ImVec2(310, 10));
    //Create_Operation_Node(uniqueId, "Intersect", ImVec2(310, 110));

    //Create_Transform_Node(uniqueId, TransformType::Translation, ImVec2(10, 360));
    //Create_Transform_Node(uniqueId, TransformType::Rotation, ImVec2(210, 360));
    //Create_Transform_Node(uniqueId, TransformType::Scale, ImVec2(460, 360));

    //Create_Primitive_Node(uniqueId, PrimitiveType::Sphere, ImVec2(10, 10));
    //Create_Primitive_Node(uniqueId, PrimitiveType::Box, ImVec2(10, 110));
    Create_Primitive_Node(uniqueId, PrimitiveType::Box, ImVec2(10, 210));


    // Submit Links
    for (auto& linkInfo : g_Links)
        ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (ed::BeginCreate())
    {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId))
        {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
                // ed::AcceptNewItem() return true when user release mouse button.
                if (ed::AcceptNewItem())
                {                                     
                    // Since we accepted new link, lets add one to our list of links.
                    g_Links.push_back({ ed::LinkId(g_NextLinkId++), inputPinId, outputPinId });

                    // Draw new link.
                    ed::Link(g_Links.back().Id, g_Links.back().InputId, g_Links.back().OutputId);
                }
                // You may choose to reject connection between these nodes
                // by calling ed::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
    }
    ed::EndCreate(); // Wraps up object creation action handling.

    // Handle deletion action
    if (ed::BeginDelete())
    {
        // There may be many links marked for deletion, let's loop over them.
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId))
        {
            // If you agree that link can be deleted, accept deletion.
            if (ed::AcceptDeletedItem())
            {
                // Then remove link from your data.
                for (auto& link : g_Links)
                {
                    if (link.Id == deletedLinkId)
                    {
                        g_Links.erase(&link);
                        break;
                    }
                }
            }
            // You may reject link deletion by calling:
            // ed::RejectDeletedItem();
        }
    }
    ed::EndDelete(); // Wrap up deletion action   


    // Minimized ugly hack, screensize = 4 might be different for other resolution, just set it higher
    bool is_collapsed = (ed::GetScreenSize().y <= 40);
    
    if (!is_collapsed)
    {
        // Handle Rightclick
        ed::Suspend();
        if (ed::ShowBackgroundContextMenu())
            ImGui::OpenPopup("Create New Node");
        //ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        if (ImGui::BeginPopup("Create New Node"))
        {
            if (ImGui::MenuItem("Create"))
                clicked_create = !clicked_create;
            ImGui::EndPopup();
        }
        //ImGui::PopStyleVar();
        ed::Resume();
    }

    if (clicked_create)
        Create_Operation_Node(uniqueId, "Union", ImVec2(310, 110));

    // End of interaction with editor.
    ed::End();
    if (g_FirstFrame)
        ed::NavigateToContent(0.0f);
    ed::SetCurrentEditor(nullptr);
    g_FirstFrame = false;
}

//void Application_Frame2()
//{
//    auto& io = ImGui::GetIO();
//
//    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);
//
//    ImGui::Separator();
//
//    ed::SetCurrentEditor(g_Context);
//
//    // Start interaction with editor.
//    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
//
//    int uniqueId = 1;
//
//    //
//    // 1) Commit known data to editor
//    //
//
//    // Submit Node A
//    ed::NodeId nodeA_Id = uniqueId++;
//    ed::PinId  nodeA_InputPinId = uniqueId++;
//    ed::PinId  nodeA_OutputPinId = uniqueId++;
//
//    if (g_FirstFrame)
//        ed::SetNodePosition(nodeA_Id, ImVec2(10, 10));
//    ed::BeginNode(nodeA_Id);
//    ImGui::Text("Node A");
//    ed::BeginPin(nodeA_InputPinId, ed::PinKind::Input);
//    ImGui::Text("-> In");
//    ed::EndPin();
//    ImGui::SameLine();
//    ed::BeginPin(nodeA_OutputPinId, ed::PinKind::Output);
//    ImGui::Text("Out ->");
//    ed::EndPin();
//    ed::EndNode();
//
//    // Submit Node B
//    ed::NodeId nodeB_Id = uniqueId++;
//    ed::PinId  nodeB_InputPinId1 = uniqueId++;
//    ed::PinId  nodeB_InputPinId2 = uniqueId++;
//    ed::PinId  nodeB_OutputPinId = uniqueId++;
//
//    if (g_FirstFrame)
//        ed::SetNodePosition(nodeB_Id, ImVec2(210, 60));
//    ed::BeginNode(nodeB_Id);
//    ImGui::Text("Node B");
//    ImGuiEx_BeginColumn();
//    ed::BeginPin(nodeB_InputPinId1, ed::PinKind::Input);
//    ImGui::Text("-> In1");
//    ed::EndPin();
//    ed::BeginPin(nodeB_InputPinId2, ed::PinKind::Input);
//    ImGui::Text("-> In2");
//    ed::EndPin();
//    ImGuiEx_NextColumn();
//    ed::BeginPin(nodeB_OutputPinId, ed::PinKind::Output);
//    ImGui::Text("Out ->");
//    ed::EndPin();
//    ImGuiEx_EndColumn();
//    ed::EndNode();
//
//    // Submit Links
//    for (auto& linkInfo : g_Links)
//        ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);
//
//    //
//    // 2) Handle interactions
//    //
//
//    // Handle creation action, returns true if editor want to create new object (node or link)
//    if (ed::BeginCreate())
//    {
//        ed::PinId inputPinId, outputPinId;
//        if (ed::QueryNewLink(&inputPinId, &outputPinId))
//        {
//            // QueryNewLink returns true if editor want to create new link between pins.
//            //
//            // Link can be created only for two valid pins, it is up to you to
//            // validate if connection make sense. Editor is happy to make any.
//            //
//            // Link always goes from input to output. User may choose to drag
//            // link from output pin or input pin. This determine which pin ids
//            // are valid and which are not:
//            //   * input valid, output invalid - user started to drag new ling from input pin
//            //   * input invalid, output valid - user started to drag new ling from output pin
//            //   * input valid, output valid   - user dragged link over other pin, can be validated
//
//            if (inputPinId && outputPinId) // both are valid, let's accept link
//            {
//                // ed::AcceptNewItem() return true when user release mouse button.
//                if (ed::AcceptNewItem())
//                {
//                    // Since we accepted new link, lets add one to our list of links.
//                    g_Links.push_back({ ed::LinkId(g_NextLinkId++), inputPinId, outputPinId });
//
//                    // Draw new link.
//                    ed::Link(g_Links.back().Id, g_Links.back().InputId, g_Links.back().OutputId);
//                }
//
//                // You may choose to reject connection between these nodes
//                // by calling ed::RejectNewItem(). This will allow editor to give
//                // visual feedback by changing link thickness and color.
//            }
//        }
//    }
//    ed::EndCreate(); // Wraps up object creation action handling.
//
//
//    // Handle deletion action
//    if (ed::BeginDelete())
//    {
//        // There may be many links marked for deletion, let's loop over them.
//        ed::LinkId deletedLinkId;
//        while (ed::QueryDeletedLink(&deletedLinkId))
//        {
//            // If you agree that link can be deleted, accept deletion.
//            if (ed::AcceptDeletedItem())
//            {
//                // Then remove link from your data.
//                for (auto& link : g_Links)
//                {
//                    if (link.Id == deletedLinkId)
//                    {
//                        g_Links.erase(&link);
//                        break;
//                    }
//                }
//            }
//
//            // You may reject link deletion by calling:
//            // ed::RejectDeletedItem();
//        }
//    }
//    ed::EndDelete(); // Wrap up deletion action
//
//
//
//    // End of interaction with editor.
//    ed::End();
//
//    if (g_FirstFrame)
//        ed::NavigateToContent(0.0f);
//
//    ed::SetCurrentEditor(nullptr);
//
//    g_FirstFrame = false;
//
//    // ImGui::ShowMetricsWindow();
//}
//
