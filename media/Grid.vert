#include <OgreUnifiedShader.h>

OGRE_UNIFORMS(
    uniform mat4 worldMatrix;
    uniform mat4 viewProjMatrix;
)

MAIN_PARAMETERS
IN(vec4 vertex, POSITION)
OUT(vec3 worldPosition, TEXCOORD0)
MAIN_DECLARATION
{
    vec4 world = mul(worldMatrix, vertex);

    worldPosition = world.xyz;
    gl_Position   = mul(viewProjMatrix, world);
}
