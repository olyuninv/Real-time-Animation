
#version 330 core
//out vec4 color;
out vec4 gl_FragColor

void main()
{
    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);
    float dist_squared = dot(pos, pos);
 
    gl_FragColor = (dist_squared < 400.0) 
           ? vec4(1.0, 0.0, 0.0, 1.0)
           : vec4(.20, .20, .40, 1.0);
    //color = vec4(1.0f); // Set alle 4 vector values to 1.0f
}