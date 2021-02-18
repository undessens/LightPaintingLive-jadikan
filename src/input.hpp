//
//  input.hpp
//  ofLightPaintApp
//
//  Created by Admin Mac on 18/12/2019.
//

#ifndef input_hpp
#define input_hpp

#include <stdio.h>
#include "ofxBlackMagic.h"
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOscParameterSync.h"

//Define input video : 0 = files  | 1 = blackmagic | 2 = videograbber
#define INPUT_VIDEO 0

/*
 
 Conclusion, on peut bien déssiner un shader, à l'intérieur d'un fbo.
 Puis venir donner le pointeur de se fbo.
 
 
*/

class Input
{
    
public:
    
    Input();
    Input( ofParameterGroup* pg, int w, int h);
    void setup();
    void update();
    void setVideoIndex(int &newIndex);
    void setVideoPause(bool &isPause);
    void videoGrabberInit();
    
    //INPUT VIDEO CHOICE
    bool isUpdatingRight;
#if INPUT_VIDEO == 0
    ofVideoPlayer player;
    ofParameter<bool> playerPause;
    ofParameter<int> videoIndex;
#elif INPUT_VIDEO == 1
    ofxBlackMagic blackMagic;
#elif INPUT_VIDEO == 2
    //Camera Grabber ( webcam )
    ofVideoGrabber videoGrabber;

#endif
    
    int w;
    int h;
    ofFbo fbo;
    ofFbo fboTresh;
    ofFbo fboBlur1;
    ofFbo fboBlur2;
    ofParameterGroup* pg;
    ofParameter<int> skipStep;
    ofParameter<double> threshold;
    ofParameter<float> smooth;
    ofParameter<float> transparency;
    ofParameter<float> blur;
    ofParameter<bool> isShown;
    ofShader shaderTreshHsv;
    ofShader shaderBlurX;
    ofShader shaderBlurY;
    string name;
    
    
};
    


/*
 
 ofxBlackMagic cam;
 cam.setup(1920, 1080, 30);
 cam.drawColor();
 
 */


#endif /* input_hpp */
