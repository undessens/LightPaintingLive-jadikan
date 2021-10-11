//
//  imageBuffer.cpp
//  ofLightPaintApp
//
//  Created by Admin Mac on 18/12/2019.
//


/*
 Contrairement à videoBuffer, imageBuffer, enregistre l'entiereté des frames dans un slot mémoire, qui est alors possible d'enregistrer en jpg en tant que image, d'afficher ou non.
Il est donc moins lourd en terme de mémoire, et permet typiquement d'enregistrer un petit dessin, une petite forme, dans une case mémoire temporaire ou un fichier jpeg.
 
 
*/

#include "imageBuffer.hpp"

//--------------------------------------------------------------
ImageBuffer::ImageBuffer(){}


//--------------------------------------------------------------
ImageBuffer::ImageBuffer( ofParameterGroup* _pg, ofxOscSender* _sender, int _w, int _h, int _nbImage){
    
    //Geometry
    pg = _pg;
    oscSender = _sender;
    w = _w;
    h = _h;
    nbImage = _nbImage;
    setup();
    
}


//--------------------------------------------------------------
void ImageBuffer::setup(){
    
    // GUI MANAGEMENT
    
    //Deactivate because need to export in ofApp before resetting
    //reset.addListener(this, &ImageBuffer::resetBuffer);
    
    record.addListener(this, &ImageBuffer::setRecordPause);
    
    pg->setName("Image_buffer");
    pg->add(isShown.set("show", true));
    pg->add(activeInput.set("input", true));
    pg->add(record.set("record", false));
    pg->add(autoOffInput.set("auto_off_input", true));
    pg->add(recordStrobeSpeed.set("rec_strobe_speed", 0, 0, 1));
    pg->add(add_substract.set("add_substract", true));
    pg->add(reset.set("reset", false));
    //pg->add(shaderLumThreshold.set("threshold_luminance", 0, -1, 1));
    shaderLumThreshold.set("threshold_luminance", 0, -1, 1); // Don't add this to gui;
    pg->add(darkerInTime.set("history_darker", 0, 0, 1));
    
    /*
     ofParameter<float>shaderAlphaThresholdBg;
     ofParameter<float>shaderAlphaThresholdNi;
     ofParameter<float>shaderLumThreshold;
     */
    
    // FBO CLEAR
    finalFbo.allocate(w, h, GL_RGBA32F_ARB);
    finalFbo.begin();
    ofClear(0,0,0, 0);
    finalFbo.end();
    
    bgFbo.allocate(w, h, GL_RGBA32F_ARB);
    resetBuffer();
    
    // SHADER SOLUTION
    #ifdef TARGET_OPENGLES
        shader_add.load("shadersES2/shader_add	");
    #else
        if(ofIsGLProgrammableRenderer()){
            shader_add.load("shadersGL3/shader_add");
        }else{
            shader_add.load("shadersGL2/shader_add");
        }
    #endif
    shader_test.load("mask/img_mask.png");
    
    //Record Strobe. Allows to strobe the record, and add to buffer One image over X
    recordStrobe = true;
    recordStrobeIndex = 0;
    
    
    /* LONG STORY ABOUT HISTORY AND TEXTURE
     At least, copy texture to anoter "save texture" is not easy, = is not sufficient, just give the link.
     You have to draw the texture on an other, using fbo for exemple.
     One FBO per frame is pretty insane, 1GB  of VRAM for 5 sec ... why not
     But something wrong is the time of drawing fbo ... and seems complex to change it.
     
     Another solution, is using Pixels, according to the fact we are only using few pixels when light painting
     But at least, we need to draw huge amount of tiny images ...
     using a vector<ofImage> with ofImage allocated to 50*50 pixels;
     
     */
    /*
    listOfTexture.clear();
    listOfFrame.clear();
    for(int i = 0 ; i < nbTextureMax; i++)
    {
        ofTexture tex;
        tex.allocate(w, h, GL_RGBA);
        listOfTexture.push_back(tex);
        listOfFrame.push_back(0);
    }
     */
    
    //ALLOCATE OPENCV IMAGE
    grayImage.allocate(w, h);
    colorImage.allocate(w,h);
    
}

//--------------------------------------------------------------
void ImageBuffer::update(ofFbo* input){

    //Strobe recorder
   // If recordStrobeSpeed = 0 = 100% , all images are added to buffer
    // If recordStrobeSpeed = 1, one image over 101 is taken
    // 0.01 is added in order to have X % 1 as a minimum value
    int recordStrobeFinalValue = (recordStrobeSpeed + 0.01 )*100;
    recordStrobe = (( (ofGetFrameNum() - recordStrobeIndex ) % recordStrobeFinalValue ) == 0 );
    
    //USING SHADER : GREAT
    bgFbo.begin();
    
    if(record && activeInput && recordStrobe){
        shader_add.begin();
        shader_add.setUniform1f("shaderLumThreshold", shaderLumThreshold);
        shader_add.setUniform1i("add_substract", add_substract? 1 : 0);
        shader_add.setUniformTexture("background",bgFbo.getTexture(), 1);
        //FBO drawing in itself.
        input->draw(0, 0);
        shader_add.end();
    }

    bgFbo.end();
    
    // ADD BLACK RECTANGLE FOR DARKER IN TIME
    bgFbo.begin();
    //Should be replaced to something related to time;
    ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
    ofFill();
    ofSetColor(255-(darkerInTime*35));
    ofDrawRectangle(0,0,w,h);
    ofDisableBlendMode();
    bgFbo.end();
    
    // FINAL FBO draw
    finalFbo.begin();
    ofClear(0, 0, 0,0);
    bgFbo.draw(0,0);
    if(!record && activeInput){
        shader_add.begin();
        shader_add.setUniform1f("shaderLumThreshold", shaderLumThreshold);
        shader_add.setUniform1i("add_substract", add_substract? 1 : 0);
        shader_add.setUniformTexture("background",bgFbo.getTexture(), 1);
        //FBO drawing in itself.
        input->draw(0, 0);
        shader_add.end();
    }
    finalFbo.end();
    
}

//--------------------------------------------------------------
void ImageBuffer::draw(int x,int y,int width, int height){
    
    ofSetColor(255);
    finalFbo.draw(x, y, width, height);
    ofSetColor(ofColor::green);
    contourFinder.draw(x, y, width, height);
    
}

//--------------------------------------------------------------
void ImageBuffer::draw(ofRectangle rect){
    
    finalFbo.draw(rect);
    if(recordStrobe && record){
        ofNoFill();
        ofSetColor(255);
        ofDrawRectangle(rect);
    }


}

//--------------------------------------------------------------
void ImageBuffer::resetBuffer(){
    
    
        bgFbo.begin();
        //0 means transparent
        //ofClear(0,0,0, 255);
        // let tranparent background of ImageBuffer
        ofClear(0,0,0, 255);
        bgFbo.end();
        
    
}

//--------------------------------------------------------------
void ImageBuffer::setRecordPause(bool &isRecord){
    
    //Auto deactivation of active input : auto off input
    // Permet un jeu beaucoup plus live de voir constamment l'input, mais
    // perd le coté photographique
    
    if(isRecord || autoOffInput){
    activeInput = isRecord;
    }
    
    
    
    ofxOscMessage msg ;
    msg.setAddress("/main/Image_buffer/record");
    msg.addBoolArg(isRecord);
    oscSender->sendMessage(msg);
    msg.clear();
    if(isRecord){
        recordStrobeIndex = ofGetFrameNum();
    }
}

