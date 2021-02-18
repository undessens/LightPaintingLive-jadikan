#pragma once

#include "ofMain.h"
#include "ofxOscParameterSync.h"
#include "ofxGui.h"
#include "input.hpp"
#include "imageBuffer.hpp"
#include "mask.hpp"
#include "ofxSyphon.h"
//#include "ofxPostProcessing.h"

#define FONT_BIG 3
#define FONT_MEDIUM 2
#define FONT_SMALL 1


class ofApp : public ofBaseApp{

    public:
    
    void setup();
    void update();
    void draw();
    void updateZoom();
    void setGlobalZoom(float& zoom);
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    // Optionnal drawing function
    ofRectangle writeText(string msg, int x, int y, int size);
    void drawArraw( int x, int y, int w , bool isOpen);
    void drawGridArea();
    void exportImage();
    // List of draw function , that return the height of their last draw
    int drawApp(ofRectangle lastElement);
    int drawInput(ofRectangle lastElement);
    int drawImageBuffer(ofRectangle lastElement);
    int drawMask(ofRectangle lastElement);
    int drawFx(ofRectangle lastElement);
    int drawFinal(ofRectangle lastElement);

    //GUI
    ofxPanel gui;
    ofxOscParameterSync sync;
    ofParameterGroup pg;
    ofParameterGroup pg_input;
    ofParameterGroup pg_imageBuffer;
    ofParameterGroup pg_videoBuffer;
    ofParameterGroup pg_mask;
    ofParameterGroup pg_fx;
    ofParameterGroup pg_output;
    ofParameter<int> bg_color;
    ofParameter<int> zoom_level;
    ofParameter<float> global_zoom;
    ofParameter<bool> auto_export;
    
    //Syphon output
    ofxSyphonServer syphonOut;
    ofTexture textureToPublish;
    ofImage imgTest;
    
    //IMAGE parameter
    int input_w;
    int input_h;
    int output_w;
    int output_h;
    
    //Instance
    Input*  input;
    ImageBuffer* imageBuffer;
    Mask* mask;
    ofFbo fboPost;
    
    //Draw the entire UI using Grid
    ofRectangle areaGui;
    ofRectangle areaApp;
    ofRectangle areaInput;
    ofRectangle vignetteInput;
    ofRectangle areaBuffer;
    ofRectangle vignetteBuffer;
    ofRectangle areaMask;
    ofRectangle vignetteMask;
    ofRectangle areaFx;
    ofRectangle vignetteFx;
    ofRectangle vignetteZoom;
    int marginH;
    int marginW;
    ofImage arrowBottom;
    ofImage arrowRight;
    
    //Zoom Image: TODO, dedicated class is needed
    ofRectangle zoomRectangle;
    ofPixels zoomPixels;
    ofImage zoomImage;
    ofImage transparentBg;
    int zoomLevelEntry;
    
    //Font
    ofTrueTypeFont fontBig;
    ofTrueTypeFont fontMedium;
    ofTrueTypeFont fontSmall;
    int fontBigSize;
    int fontMediumSize;
    int fontSmallSize;
    
    //Auto export jpg image
    ofImage exportOfImage;
    ofPixels exportPixels;
    bool exportFlag;
    
    
    
    
		
};
