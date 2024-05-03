#version 460

layout(location = 0) in vec2 fs_in_tex;
layout(location = 0) out vec4 fs_out_col;
layout(location = 0) uniform sampler2D texImg;

layout(location = 1) uniform float iTime;
layout(location = 2) uniform vec2 iResolution;
layout(location = 3) uniform vec3 cameraPos;
layout(location = 4) uniform vec3 cameraAt;
layout(location = 5) uniform mat3 cameraView;

const float pi = 3.1415926535897932384626433832795;
    
// Arbitrarily small value
float eps = 0.01;

struct ray_t {
    vec3 start;
    vec3 direction;
};

struct Material {
    vec3  color;        // [0, 1/pi]    reflective color
    float roughness;    // [0, 7]       shininess
    vec3 emission;		// [0, inf]     light emitting surface if nonzero
    float metalness;    // [0.02, 0.05] for non-metals, [0.6, 0.9] for metals
};

// Materials
#define LIGHTSRC(r,g,b)   Material(vec3(0,0,0)   , 1.     , vec3(r,g,b), 0.  )
#define METALLIC(r,g,b,m) Material(vec3(r,g,b)/pi,float(m), vec3(0,0,0), 0.9 )
#define NONMETAL(r,g,b,m) Material(vec3(r,g,b)/pi,float(m), vec3(0,0,0), 0.02)

Material colors[] = Material[](
    METALLIC(.6,.4,.1,0.03),
    NONMETAL(.15,.01,1,.1),
    NONMETAL(.1,1,.5,1),
    NONMETAL(1,1,1,0.3),
    LIGHTSRC(30,30,30),
    LIGHTSRC(10,0,0),
    LIGHTSRC(0,0,5)
);

struct Lightsource {
    vec3 position;
    float radius;
    vec3 emission;
    bool dynamic;
};

Lightsource lights[] = Lightsource[](
    Lightsource(vec3(0),0.4,vec3(30.,30.,30.),true),
    Lightsource(vec3(-5.,10.,10.),0.4,vec3(10.,0.,0.),false),
    Lightsource(vec3(-5.,1.,0.),0.4,vec3(0.,0.,5.),false)
);  

//const Material colors[] = Material[](
//    METALLIC(.6,.4,.1,0.03),
//    METALLIC(.15,.01,1,.1),
//    METALLIC(.1,1,.5,1),
//    NONMETAL(1,1,1,0.3),
//    LIGHTSRC(80,80,80)
//);

struct Value {float d; int mat;};

float max3(vec3 v){	return max(v.x, max(v.y, v.z)); }
float sum(vec3 v){ return v.x + v.y + v.z; }

// SDF Primitives

float sphere(vec3 p, float r){
    return length(p) - r;
}
float box(vec3 p, vec3 b){ //from	https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
	vec3 d = abs(p) - b;
    return length(max(d,0.))+min(max3(d),0.);
}

// Infinite cylinders
float cylinderZ(vec3 p, float r){ return length(p.xy) - r; }
float cylinderY(vec3 p, float r){return cylinderZ(p.xzy,r);}
float cylinderX(vec3 p, float r){return cylinderZ(p.zyx,r);}

// Finite cylinders
float cylinderZ(vec3 p, vec2 h){
    vec2 d = abs(vec2(length(p.xy), abs(p.z))) - h;
    return min(max(d.x, d.y), 0.) + length(max(d,0.));
}
float cylinderY(vec3 p, vec2 h){return cylinderZ(p.xzy,h);}
float cylinderX(vec3 p, vec2 h){return cylinderZ(p.zyx,h);}

// SDF

vec3 getLightPos()
{
//    return vec3(10,10,10);
    float t = 2.*pi + iTime;
//    return vec3(10.,5.1+cos(t*2.)*5.,5);
    return vec3(10.,5.,1.+cos(t*1.)*5.);
    //return vec3(10.,4.1+cos(t*2.)*5.,10.);
}

// Test forward declaration
float sdf(vec3 p);
Material material(vec3 p);

// Surface normal to spheres[idx] on pos
vec3 normal(const in vec3 p)
{
    vec3 plus = vec3(sdf(p+vec3(eps,0,0)),sdf(p+vec3(0,eps,0)),sdf(p+vec3(0,0,eps)));
    vec3 minu = vec3(sdf(p-vec3(eps,0,0)),sdf(p-vec3(0,eps,0)),sdf(p-vec3(0,0,eps)));
    return normalize(plus-minu);
}

float sphereTrace(const in ray_t ray)
{
    // Current step on the ray
	float t = 0.;
    
    // Length of the distance calculated at the last iteration
    float lastd;
    
    // # of iterations
    const int maxIter = 64;
   
    for (int i=0; i<maxIter; i++) {
        lastd = sdf(ray.start + t*ray.direction);
    	t += lastd;
        if (lastd < eps || t > 100.0) break;
    }
   
    return t;
}

vec3 brdf(vec3 n, vec3 l, vec3 v, in Material mat)
{
//    if (length(l.xy) < length(0.5))
//    {
//        return vec3(1,1,1);
//    }
//    if (dot(n, l) <0.)
//    {
//        return vec3(0,0,0);
//    }
    
    vec3 F0 = mat.color*mat.metalness;
    vec3  h = normalize(l + v);
    //CookTorrenceGeometry
    float hn = max(dot(h, n), 0.0), vn = max(dot(v, n), 0.01);
	float ln = max(dot(l, n), 0.01), vh = max(dot(v, h), 0.0);
    float G = min( 2.*hn*min( vn, ln)/vh, 1.0 );
    //GGXDistribution
    float hn2 = hn*hn, m2 = mat.roughness*mat.roughness;
    float tmp = hn2*(m2-1.)+1.;
    float D =  m2/(pi*tmp*tmp);
    //SclickFresnel
    vec3 F = F0 - (1.-F0)*pow(1.-hn,5.);
	vec3 specular  = D*F*G / (4. * vn * ln);
	// Lambertian BRDF
	vec3 diffuse = (1.-mat.metalness) * mat.color * (1. - F)/pi;
//    return F;
	return max(specular + diffuse,0.);
}

vec3 colorByLight(Lightsource light, ray_t ray, float t)
{
    vec3 lightPosition;

    if (light.dynamic)
        lightPosition = getLightPos();
    else
        lightPosition = light.position;

    vec3 p = ray.start + ray.direction*t;
    vec3 n = normal(p);   
    vec3 v = -ray.direction;
    vec3 l = normalize(lightPosition-p);    
    Material mat = material(p);

    float lightDistance = length(lightPosition-p);
    vec3 lightFactor = light.emission / lightDistance;

    vec3 color = lightFactor * brdf(n,l,v,material(p)) * max(dot(l,n),0.);
    if (length(mat.emission) > 0)
        color += max(dot(v,n),0.) * mat.emission;    

    return color;
}

vec3 hitColorBRDF(ray_t ray,float t)
{
//    vec3 lightPos = vec3(10,10,10);
//    Lightsource[3] lightsources = {Lightsource((0,0,0),(10,10,10))};

    vec3 color = vec3(0);

//    Lightsource light1 = Lightsource(getLightPos(),vec3(30.,30.,30.));
//    Lightsource light2 = Lightsource(vec3(-5.,10.,10.),vec3(10.,0.,0.));
//    Lightsource light3 = Lightsource(vec3(-5.,1.,0.),vec3(0.,0.,5.));    
//    color += colorByLight(light1,ray,t);
//    color += colorByLight(light2,ray,t);
//    color += colorByLight(light3,ray,t);

    for (int i = 0; i < lights.length(); i++)
        color += colorByLight(lights[i],ray,t);

    return color;
}



// Tone mapping
vec3 Uncharted2ToneMapping(vec3 color)
{
    const float A = 0.15,B = 0.50,C = 0.10,D = 0.20,E = 0.02,F = 0.30,W = 11.2;
    const float exposure = 2., gamma = 2.2;
    color *= exposure;
    color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
    const float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;

    return pow(color/white, vec3(1. / gamma));
}

mat3 getCamera(vec3 cameraPos, vec3 cameraAt)
{
	vec3 cd = normalize(cameraAt - cameraPos);
	vec3 cr = normalize(cross(vec3(0, 1, 0), cd));
	vec3 cu = normalize(cross(cd, cr));
	
	return mat3(-cr, cu, cd);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{   
    vec2 uv = fragCoord/iResolution.xy*2.-vec2(1.);// Normalized pixel coordinates (from -1 to 1)

    float angleX = 1.; //tan(pi/2.0/2.); // = 1.0
    float angleY = angleX * iResolution.y / iResolution.x;
    float x = (uv.x) * angleX;
    float y = (uv.y) * angleY;

    ray_t ray;
    ray.direction = getCamera(cameraPos, cameraAt) * normalize(vec3(x, y, 1));	//this pixel shoots a ray towards this direction
    //ray.direction = (cameraView * vec4(normalize(vec3(x, y, 1)),1)).xyz;	//this pixel shoots a ray towards this direction
    //ray.direction = cameraView * normalize(vec3(x, y, 1));	//this pixel shoots a ray towards this direction
    ray.start = cameraPos;

    float t = sphereTrace(ray);
    
    vec3 p = ray.start + t*ray.direction;
    vec3 n = normal(p);    
//    vec3 col = vec3(t/45.);
//    vec3 col = vec3(n*t/15.);

    vec3 col = vec3(0,0,0);
    if (t > 100.0)
    {
        fragColor = texture(texImg, fs_in_tex);
		
		//fragColor =  vec4(1.,1.,1.,1.) * vec4(fs_in_tex,0.,1.);
    }
    else
    {
		col = Uncharted2ToneMapping(hitColorBRDF(ray, t));
//        col = Uncharted2ToneMapping(hitColorBRDF(ray, t)*0.55);
//        col = hitColorBRDF(ray, t);
//        col = hitColorBRDF(ray, t).xyz*3;
//        col = vec3(n);
        fragColor = vec4( col, 1.f );
    }
}

void main()
{
	mainImage(fs_out_col, gl_FragCoord.xy);
}
