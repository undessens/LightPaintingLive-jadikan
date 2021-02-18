#version 120

//This is the actual fbo
uniform sampler2DRect tex0;
//This is the new frame
uniform sampler2DRect background;
uniform float keep_dark;
uniform float shaderAlphaThresholdBg;
uniform float shaderAlphaThresholdNi;
uniform float shaderLumThreshold;

varying vec2 texCoordVarying;

//out vec4 outputColor;

vec3 rgb2hsv(vec3 c) {
     float cMax=max(max(c.r,c.g),c.b),
     cMin=min(min(c.r,c.g),c.b),
     delta=cMax-cMin;
     vec3 hsv=vec3(0.,0.,cMax);
     if(cMax>cMin){
         hsv.y=delta/cMax;
         if(c.r==cMax){
            hsv.x=(c.g-c.b)/delta;
         }else if(c.g==cMax){
            hsv.x=2.+(c.b-c.r)/delta;
         }else{
            hsv.x=4.+(c.r-c.g)/delta;
         }
         hsv.x=fract(hsv.x/6.);
     }
     return hsv;
 }

 vec3 rgb2hsl( in vec3 c ){
  float h = 0.0;
	float s = 0.0;
	float l = 0.0;
	float r = c.r;
	float g = c.g;
	float b = c.b;
	float cMin = min( r, min( g, b ) );
	float cMax = max( r, max( g, b ) );

	l = ( cMax + cMin ) / 2.0;
	if ( cMax > cMin ) {
		float cDelta = cMax - cMin;
        
        //s = l < .05 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) ); Original
		s = l < .0 ? cDelta / ( cMax + cMin ) : cDelta / ( 2.0 - ( cMax + cMin ) );
        
		if ( r == cMax ) {
			h = ( g - b ) / cDelta;
		} else if ( g == cMax ) {
			h = 2.0 + ( b - r ) / cDelta;
		} else {
			h = 4.0 + ( r - g ) / cDelta;
		}

		if ( h < 0.0) {
			h += 6.0;
		}
		h = h / 6.0;
	}
	return vec3( h, s, l );
}

float hue2rgb(float f1, float f2, float hue) {
    if (hue < 0.0)
        hue += 1.0;
    else if (hue > 1.0)
        hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
        res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
        res = f2;
    else if ((3.0 * hue) < 2.0)
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
        res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb;
    
    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z); // Luminance
    } else {
        float f2;
        
        if (hsl.z < 0.5)
            f2 = hsl.z * (1.0 + hsl.y);
        else
            f2 = hsl.z + hsl.y - hsl.y * hsl.z;
        
        float f1 = 2.0 * hsl.z - f2;
        
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0/3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0/3.0));
    }
    return rgb;
}

void main()
{
    vec4 ni = texture2DRect(tex0, texCoordVarying); // new image
    vec4 bg = texture2DRect(background, texCoordVarying); // background
    
    //gl_FragColor = vec4(texel0.r + texel1.r, texel0.g + texel1.g, texel0.b + texel1.b , texel0.a + texel1.a);
    
    // Only draw if background is transparent
    /*
    if(texel1.a < 1){
    	gl_FragColor = vec4( texel0.r, texel0.g, texel0.b ,  texel0.a);
    }else{
		gl_FragColor = vec4( texel0.r, texel0.g, texel0.b , 0  );
    }
    */
    vec3 hslColorBackground = rgb2hsl(vec3(bg.r, bg.g, bg.b));
    vec3 hslColorNewFrame = rgb2hsl(vec3(ni.r, ni.g, ni.b));
    
    /*
    //if Luminance is better on the new frame
    float diff = hslColorNewFrame.b - hslColorBackground.b;
    
    float alpha ;
    if(bg.a < shaderAlphaThresholdBg){
    	//if there is something in the backgound
    	alpha = ni.a;
    }else{
    	//IF something in the background
    	if(ni.a > shaderAlphaThresholdNi){
    		//IF there is something in the new image
    		if(diff > shaderLumThreshold){
    			//if new image luminosity is higher thant the background
    			alpha = 1;
    		}else{
    			alpha = keep_dark;
    		}
    	}else{
    		//Something on the back, but not in the new image
    		alpha = 0;
    	}
    }
     */
    
    
    // THIS IS LIGHTEN BLEND MODE
    
    if( ( hslColorNewFrame.b - hslColorBackground.b  ) >=  shaderLumThreshold){
        gl_FragColor = vec4( ni.r, ni.g, ni.b , ni.a);
//        float final_r = 1.0 - (( 1.0 - ni.r)*(1.0-bg.r));
//        float final_g = 1.0 - (( 1.0 - ni.g)*(1.0-bg.g));
//        float final_b = 1.0 - (( 1.0 - ni.b)*(1.0 -bg.b));
//        gl_FragColor = vec4( final_r, final_g, final_b , 1);
    }else{
        gl_FragColor = vec4( bg.r, bg.g, bg.b , 1);

    }
    
    
    
    
    //THIS IS SCREEN MODE
    /*
    float final_r = 1.0 - (( 1.0 - ni.r)*(1.0-bg.r));
    float final_g = 1.0 - (( 1.0 - ni.g)*(1.0-bg.g));
    float final_b = 1.0 - (( 1.0 - ni.b)*(1.0 -bg.b));

    
    gl_FragColor = vec4( final_r, final_g, final_b , 1);
     */

    

    
}

