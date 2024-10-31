#version 460 core
out vec4 FragColor;

in vec3 vertPos;
flat in uint type;
in vec3 normal;

const vec3 colorLoopUp[3] = {
    vec3(0,0,0), // air
    vec3(169,169,169), // stone
    vec3(139,69,19) // dirt
};

void main()
{
    int mode = 1;
    vec3 ambientColor = vec3(52, 25, 0) / 255.0f;
    vec3 diffuseColor = colorLoopUp[type] / 255.0f;
    vec3 specularColor = vec3(255, 255, 255) / 255.0f;

    float Ka = 1;   // Ambient reflection coefficient
    float Kd = 1;   // Diffuse reflection coefficient
    float Ks = 1;   // Specular reflection coefficient
    float shininessVal = 64; // Shininess

    vec3 L = normalize(vec3(100,100,100) - vertPos);
    vec3 N = normalize(normal);

    float lambertian = max(dot(N, L), 0.0);
    float specular = 0.0;
    if (lambertian > 0.0) {
        vec3 R = reflect(-L, N);      // Reflected light vector
        vec3 V = normalize(-vertPos); // Vector to viewer
        // Compute the specular term
        float specAngle = max(dot(R, V), 0.0);
        specular = pow(specAngle, shininessVal);
      }
      
    FragColor = vec4(Ka * ambientColor +
                Kd * lambertian * diffuseColor +
                Ks * specular * specularColor, 1.0);
   // FragColor = vec4(ambientColor, 1);
}