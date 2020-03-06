//
//  imageBuffer.hpp
//  ofLightPaintApp
//
//  Created by Admin Mac on 18/12/2019.
//

#ifndef imageBuffer_hpp
#define imageBuffer_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxGui.h"
#include "ofxOscParameterSync.h"
#include "ofxOpenCv.h"

class ImageBuffer
{
    
public:
    
    ImageBuffer();
    ImageBuffer( ofParameterGroup* pg, int w, int h, int nbImage);
    void setup();
    void update(ofFbo* input, int frameNum);
    void draw(int, int, int ,int ); // draw all active images
    void draw(ofRectangle r ); // draw all active images
    void resetBuffer(bool &isReset);
    
    int w;
    int h;
    int nbImage;
    ofFbo fbo; // Will change to an array on fbo, according to each image
    vector<ofTexture> listOfTexture;  // Vector of texture for creating real history of motion
    vector<int> listOfFrame;
    ofParameter<int> nbTextureMax; // nb Max of history
    ofParameter<int> listOfTextureIndex; // nb Max of history
    ofParameterGroup* pg;
    ofParameter<int> currentImage;
    ofParameter<int> darkerInTime;
    ofParameter<int> opacityAtDraw;
    ofParameter<float>shaderKeepDark;
    ofParameter<float>shaderAlphaThresholdBg;
    ofParameter<float>shaderAlphaThresholdNi;
    ofParameter<float>shaderLumThreshold;
    ofParameter<bool> reset;
    ofParameter<bool> isShown;
    
    //SHADER SOLUTION
    ofShader shader_add;
    ofImage shader_test;
    
    //Open CV solution ... not good
    ofxCvGrayscaleImage     grayImage;
    ofxCvColorImage     colorImage;
    ofxCvContourFinder     contourFinder;
    
    
};

#endif /* imageBuffer_hpp */
