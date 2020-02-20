#version 150 core

//uniform vec3 ka;            // Ambient reflectivity
//uniform vec3 kd;            // Diffuse reflectivity
//uniform vec3 ks;            // Specular reflectivity
//uniform float shininess;    // Specular shininess factor

uniform vec3 eyePosition;

in vec3 worldPosition;
in vec3 worldNormal;

out vec4 fragColor;

uniform sampler2D tex0;

//#pragma include light.inc.frag

void main()
{
    //vec3 diffuseColor, specularColor;
    //adsModel(worldPosition, worldNormal, eyePosition, shininess, diffuseColor, specularColor);
    //fragColor = vec4( ka + kd * diffuseColor + ks * specularColor, 1.0 );

    float cell_x = floor(worldPosition.x);
    float cell_y = floor(worldPosition.z);
    float pos_x = worldPosition.x - cell_x;
    float pos_y = worldPosition.z - cell_y;

    int num_cells = 20;

    // scale from [0..1] to [-1..1]
    pos_x = pos_x*2 - 1;
    pos_y = pos_y*2 - 1;

    // apply rotation
    float angle = cell_x * 3.14159 * 2 / num_cells;
    float x = pos_x * cos(angle) - pos_y * sin(angle);
    float y = pos_x * sin(angle) + pos_y * cos(angle);

    // apply scaling to the texture coordinates
    float scale = (cell_y + (num_cells/2)) / num_cells;
    x /= scale;
    y /= scale;

    // scale from [-1..1] back to [0..1]
    vec2 UV = vec2((x+1)/2, (y+1)/2);

    float alpha = 0.0;
    if (UV.x >= 0 && UV.y >= 0 && UV.x <= 1 && UV.y <= 1)
    {
      alpha = texture(tex0, UV).a;
    }

    fragColor = vec4(alpha,0.1,0.1,1.0);
}
