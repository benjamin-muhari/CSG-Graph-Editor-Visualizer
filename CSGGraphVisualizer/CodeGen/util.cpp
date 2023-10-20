#include "util.h"
#include <sstream>

using namespace std;
using namespace glm;

template<int N>
string number_list(float *floats){	static_assert(N>=1);
    stringstream ss;        ss << floats[0];
	for(int i=1; i<N; ++i)	ss << ',' << floats[i];
	return ss.str();
}

string float1(float x) {
	string ret = number_list<1>(&x);
	if(ret.find('.') == std::string::npos) ret += '.';
	return ret;
}

string float2(float x, float y) {
	return float2(vec2(x,y));
}

string float2(vec2 v) {
	return "vec2(" + number_list<2>(&v.x) + ')';
}

string float3(float x, float y, float z) {
    return float3(vec3(x,y,z));
}

string float3(vec3 v) {
	return "vec3(" + number_list<3>(&v.x) + ')';
}

string float3x3(mat3 m) {
	return "mat3(" + number_list<9>(&m[0][0]) +')';
}

mat3 rotateX(float angle)
{
    float s = sin(angle), c = cos(angle);
    return mat3(1, 0, 0, 0, c, s, 0, -s, c);
}
mat3 rotateY(float angle)
{
    float s = sin(angle), c = cos(angle);
    return mat3(c, 0, -s, 0, 1, 0, s, 0, c);
}
mat3 rotateZ(float angle)
{
    float s = sin(angle), c = cos(angle);
    return mat3(c, s, 0, -s, c, 0, 0, 0, 1);
}

float length(float x, float y) {
    return (float)vec2(x, y).length();
}
float length(float x, float y, float z) {
    return (float)vec3(x, y, z).length();
}