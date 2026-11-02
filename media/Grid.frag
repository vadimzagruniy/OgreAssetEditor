#include <OgreUnifiedShader.h>

// Grid on the XZ plane drawn per pixel: lines stay one pixel wide at any distance
// and fade out with distance and at grazing angles.

OGRE_UNIFORMS(
    uniform vec4 gridLayout;   // x: cell size of the finest level, y: fade distance
    uniform vec4 levelAlpha;   // alpha of the cell, 10x cell and 100x cell levels
    uniform vec3 cameraPosition;
    uniform vec4 lineColor;
    uniform vec4 axisXColor;
    uniform vec4 axisZColor;
)

float lineCoverage(vec2 coord, float cellSize)
{
    vec2 scaled       = coord / cellSize;
    vec2 width        = max(fwidth(scaled), vec2_splat(0.00001));
    vec2 lineDistance = abs(fract(scaled - 0.5) - 0.5) / width;

    return 1.0 - min(min(lineDistance.x, lineDistance.y), 1.0);
}

float axisCoverage(float coord)
{
    float width = max(fwidth(coord), 0.00001);

    return 1.0 - min(abs(coord) / width, 1.0);
}

MAIN_PARAMETERS
IN(vec3 worldPosition, TEXCOORD0)
MAIN_DECLARATION
{
    vec2  coord     = worldPosition.xz;
    float cellSize  = gridLayout.x;

    float alpha = lineCoverage(coord, cellSize) * levelAlpha.x;
    alpha = max(alpha, lineCoverage(coord, cellSize * 10.0) * levelAlpha.y);
    alpha = max(alpha, lineCoverage(coord, cellSize * 100.0) * levelAlpha.z);

    vec4 color = vec4(lineColor.rgb, alpha);
    color = mix(color, axisXColor, axisCoverage(worldPosition.z));
    color = mix(color, axisZColor, axisCoverage(worldPosition.x));

    vec3  toFragment    = worldPosition - cameraPosition;
    float distanceFade  = 1.0 - smoothstep(gridLayout.y * 0.3, gridLayout.y, length(toFragment.xz));
    float horizonFade   = smoothstep(0.0, 0.15, abs(normalize(toFragment).y));

    color.a *= distanceFade * horizonFade;

    gl_FragColor = color;
}
