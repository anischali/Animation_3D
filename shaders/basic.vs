#version 330
uniform float cr, cg, cb, ca;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform int animation;
layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;
 
out vec3 vsoNormal;
out vec3 vsoPosition;
out vec3 vsoModPos;
out vec4 color;
out vec2 vsoTexCoord;

void main(void) {
  vec4 mp = modelViewMatrix * vec4(vsiPosition, 1.0);
  vsoNormal = (transpose(inverse(modelViewMatrix))  * vec4(vsiNormal, 0.0)).xyz;
  vsoModPos   = mp.xyz;
  vsoPosition = vsiPosition;
  gl_Position = projectionMatrix * mp;
  vsoTexCoord = vec2(vsiTexCoord.x, 1.0 - vsiTexCoord.y); 
 
  
  
   

  if (animation == 0){
    if(vsiPosition.x < 0 && vsiPosition.y < 0)
      color = vec4(0.294, 0.000, 0.510, 1.0);
    else if(vsiPosition.x > 0 && vsiPosition.y < 0)
      color = vec4(0.000, 1.000, 0.000,1.0);
    else if(vsiPosition.x > 0 && vsiPosition.y > 0)
      color = vec4(0.000, 0.808, 0.820, 1.0);
    else
      color = vec4(0.098, 0.098, 0.439,1.0);
  }else if (animation == 1){
    if(vsiPosition.x < 0 && vsiPosition.y < 0)
      color = vec4(1.0, 0.0, 0.0, 1.0);
    else if(vsiPosition.x > 0 && vsiPosition.y < 0)
      color = vec4(1.0, 1.0, 0.0,1.0);
    else if(vsiPosition.x > 0 && vsiPosition.y > 0)
      color = vec4(1.0, 0.894, 0.769, 1.0);
    else
      color = vec4(0.647, 0.165, 0.165,1.0);
  }else if (animation == 2){ 
    color = vec4(cr, cg, cb, ca);
  }else if (animation == 3){ 
    if(vsiPosition.x < 0 && vsiPosition.y < 0)
      color = vec4(cr, 0.0, 0.0, ca);
    else if(vsiPosition.x > 0 && vsiPosition.y < 0)
      color = vec4(cr, cg, 0.0, ca);
    else if(vsiPosition.x > 0 && vsiPosition.y > 0)
      color = vec4(cr, 0.0, cb, ca);
    else
      color = vec4(cr, cg, cb, ca);
  }else if (animation == 4){ 
    if(vsiPosition.x < 0 && vsiPosition.y < 0)
      color = vec4(cr, 0.0, 0.0, ca);
    else if(vsiPosition.x > 0 && vsiPosition.y < 0)
      color = vec4(cr, cg, 0.0, ca);
    else if(vsiPosition.x > 0 && vsiPosition.y > 0)
      color = vec4(cr, 0.0, cb, ca);
    else
      color = vec4(ca, 0.0, 0.0, cr);
  }else{
    color = vec4(1.0, 1.0, 0.878,1.0);
  }

  
}
