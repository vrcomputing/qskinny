uniform lowp float qt_Opacity;
uniform lowp vec4 color;
uniform lowp vec4 rect;
uniform lowp vec4 mouse;

#undef ANIMATE

#define M_PI 3.1415926535897932384626433832795

//    0  1  2  3  4  5 
// R  1  1  0  0  0  1
// G  0  1  1  1  0  0
// B  0  0  0  1  1  1
vec3 getHueColor(vec2 pos)
{
#ifdef ANIMATE
	float theta = mod(3.0 + 3.0 * atan(pos.x, pos.y) / M_PI + iTime, 6.0);
#else
	float theta = 3.0 + 3.0 * atan(pos.x, pos.y) / M_PI;
#endif
		
	vec3 color = vec3(0.0);
	
	return clamp(abs(mod(theta + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 iOffset = vec2(-rect.x, -rect.y);
    vec2 iResolution = rect.zw;
    vec4 iMouse = mouse;

	vec2 uv = vec2(2.0, -2.0) * (fragCoord.xy + iOffset - 0.5 * iResolution.xy) / iResolution.y;
	vec2 mouse = vec2(2.0, -2.0) * (iMouse.xy + iOffset - 0.5 * iResolution.xy) / iResolution.y;
	
	float l = length(uv);
	float m = length(mouse);
	
	fragColor = vec4(0.0);

	if (l >= 0.75 && l <= 1.0)
	{
		l = 1.0 - abs((l - 0.875) * 8.0);
		l = clamp(l * iResolution.y * 0.0625, 0.0, 1.0); // Antialiasing approximation
		
		fragColor = vec4(l * getHueColor(uv), l);
	}
	else if (l < 0.75)
	{
		vec3 pickedHueColor;
		
		if (m < 0.75 || m > 1.0)
		{
#ifdef ANIMATE
			mouse = vec2(sin(iTime), cos(iTime));
#else
			mouse = vec2(0.0, -1.0);
			pickedHueColor = vec3(1.0, 0.0, 0.0);
#endif
		}
#ifndef ANIMATE
		else
		{
#endif
			pickedHueColor = getHueColor(mouse);
#ifndef ANIMATE
		}
#endif
		
		uv = uv / 0.75;
		mouse = normalize(mouse);
		
		float sat = 1.5 - (dot(uv, mouse) + 0.5); // [0.0,1.5]
		
		if (sat < 1.5)
		{
			float h = sat / sqrt(3.0);
			vec2 om = vec2(cross(vec3(mouse, 0.0), vec3(0.0, 0.0, 1.0)));
			float lum = dot(uv, om);
			
			if (abs(lum) <= h)
			{
				l = clamp((h - abs(lum)) * iResolution.y * 0.5, 0.0, 1.0) * clamp((1.5 - sat) / 1.5 * iResolution.y * 0.5, 0.0, 1.0); // Fake antialiasing
				fragColor = vec4(l * mix(pickedHueColor, vec3(0.5 * (lum + h) / h), sat / 1.5), l);
			}
		}
	}
}

void main()
{
    mainImage(gl_FragColor, gl_FragCoord);
    gl_FragColor *= qt_Opacity;
}