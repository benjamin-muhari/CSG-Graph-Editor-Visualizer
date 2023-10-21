//#include "CodeGen/Objects/models.h"
//#include "CodeGen/expr.h"
//
//enum class NodeType
//{
//    Sphere,
//    Box,
//    CylinderY,
//    // etc... TODO
//
//    Union,
//    Intersection,
//    Substraction,
//    // etc... TODO
//
//    Translation,
//    Rotation
//};
//
//struct CsgNode
//{
//    virtual NodeType Type() = 0;
//
//    CsgNode** inputs; // pointers to input nodes
//    CsgNode* output;
//};
//
//struct CsgUnion : CsgNode
//{
//    NodeType Type() { return NodeType::Union; }
//};
//
//struct CsgBox : CsgNode
//{
//    NodeType Type() { return NodeType::Box; }
//
//    void SetSides(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
//
//    glm::vec3 GetSides() { return glm::vec3(x, y, z); }
//
//    private:
//        float x;
//        float y;
//        float z;
//};
//
//struct CsgTranslation : CsgNode
//{
//    NodeType Type() { return NodeType::Translation; }
//
//    void SetVector(float x, float y, float z) { this->x = x; this->y = y; this->z = z; }
//
//    glm::vec3 GetVector() { return glm::vec3(x, y, z); }
//
//    private:
//        float x;
//        float y;
//        float z;
//};
//
//// other primitives, operations, transforms similarly
//
//Expr<Fields>* build_csg_tree(CsgNode* node)
//{
//    // nullptr might cause issues, might have to make sure its never passed, i.e.: node editor works correctly
//    if (node == nullptr)
//        return nullptr;
//
//    // OPERATIONS
//    //if (node->Type() == NodeType::Union)
//    //    // mint a python map, nem tudom cpp-ben hogyan kell egyenlőre lambdákkal, csak el akartam gyorsan küldeni
//    //    return union_op<Fields>(map(node->inputs, build_csg_tree));
//
//    // etc, other operations similarly TODO
//    // ...
//    // ...
//
//    // TRANSFORMS
//    if (node->Type() == NodeType::Translation)
//    {
//        CsgTranslation* transl = dynamic_cast<CsgTranslation*>(node);
//
//        if (transl != nullptr)
//            return move(transl->GetVector(), build_csg_tree(transl->inputs[0]));
//    }
//
//    // etc, other transforms similarly TODO
//    // ...
//    // ...
//
//    // PRIMITIVES
//    if (node->Type() == NodeType::Box)
//    {
//        CsgBox* box_ = dynamic_cast<CsgBox*>(node);
//
//        if (box_ != nullptr)
//        {
//            glm::vec3 sides = box_->GetSides();
//            return box(sides.x, sides.y, sides.z, Fields{});
//        }
//    }
//
//    // etc, other similarly TODO
//    // ...
//    // ...
//}
//
