#include "Models/models.h"

using namespace std;

void Test1()
{
    //build_kernel("test1sdf.glsl", Models::model2_expr(Models::Model2Attributes()));
    build_kernel("test1sdf.glsl", Models::model5_expr());
}
//
//int main()
//{    
//    Test1();
//    //build_kernel("codegensdf.glsl", Models::toys_exp());
//}
