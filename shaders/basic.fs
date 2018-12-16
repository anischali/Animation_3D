/*!\file basic.fs
 *
 * \brief rendu avec lumière directionnelle diffuse et couleur.
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr 
 * \date April 15 2016
 */
#version 330
uniform sampler2D texApi8;
uniform int animation;
uniform mat4 modelViewMatrix;
in  vec3 vsoNormal;
in  vec3 vsoModPos;
in  vec3 vsoPosition;
in  vec2 vsoTexCoord;
in vec4 color;
out vec4 fragColor;

void main(void) {

  const float Iamb = 0.15;
  float Idiffuse = 0, Ispec = 0;


  vec3 N = normalize(vsoNormal);
  vec3 L = normalize(vec3(1, -1, 0)); /*vers le bas vers la gauche*/
  float diffuse = dot(N, -L);
  if (animation == 2){
    fragColor = texture(texApi8,vsoTexCoord) * color;
  }
  else if (animation == 3){
    fragColor = texture(texApi8,vsoTexCoord) * color;
  }else{ 
    fragColor = color;
  }
}
