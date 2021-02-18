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
ImageBuffer::ImageBuffer( ofParameterGroup* _pg, int _w, int _h, int _nbImage){
    
    //Geometry
    pg = _pg;
    w = _w;
    h = _h;
    nbImage = _nbImage;
    setup();
    
}


//--------------------------------------------------------------
void ImageBuffer::setup(){
    
    // GUI MANAGEMENT
    
    reset.addListener(this, &ImageBuffer::resetBuffer);
    
    pg->setName("Image_buffer");
    pg->add(isShown.set("show", true));
    //pg->add(currentImage.set("Image index", 0, 0, nbImage));
    pg->add(reset.set("Reset", false));
    //pg->add(nbTextureMax.set("History", 60, 1, 400));
    //pg->add(listOfTextureIndex.set("BufferIndex", 5, 0, 59));
    //pg->add(opacityAtDraw.set("Opacity", 255, 100, 255));
    pg->add(darkerInTime.set("History Darker", 0, 0, 35));
    //pg->add(shaderKeepDark.set("Shader :Keep Dark", 0, 0, 1));
    //pg->add(shaderAlphaThresholdBg.set("Shader :Alpha Tresh bg", 0.1, 0, 1));
    //pg->add(shaderAlphaThresholdNi.set("Shader :Alpha Tresh ni", 0.1, 0, 1));
    pg->add(shaderLumThreshold.set("Threshold luminance", 0, -1, 1));
    pg->add(bypass.set("ByPass", false));
    
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
    bgFbo.begin();
    ofClear(0,0,0, 0);
    bgFbo.end();
    
    // SHADER SOLUTION
    #ifdef TARGET_OPENGLES
        shader_add.load("shadersES2/shader");
    #else
        if(ofIsGLProgrammableRenderer()){
            shader_add.load("shadersGL3/shader_add");
        }else{
            shader_add.load("shadersGL2/shader_add");
        }
    #endif
    shader_test.load("mask/img_mask.png");
    
    
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
void ImageBuffer::update(ofFbo* input, int frameNum){

    
    //USING SHADER : GREAT
    bgFbo.begin();
    
    if(!bypass){
        shader_add.begin();
        shader_add.setUniform1f("keep_dark", shaderKeepDark);
        shader_add.setUniform1f("shaderAlphaThresholdBg", shaderAlphaThresholdBg);
        shader_add.setUniform1f("shaderAlphaThresholdNi", shaderAlphaThresholdNi);
        shader_add.setUniform1f("shaderLumThreshold", shaderLumThreshold);
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
    ofSetColor(255-darkerInTime);
    ofDrawRectangle(0,0,w,h);
    ofDisableBlendMode();
    bgFbo.end();
    
    // FINAL FBO draw
    finalFbo.begin();
    ofClear(0, 0, 0,0);
    bgFbo.draw(0,0);
    if(bypass){
        shader_add.begin();
        shader_add.setUniform1f("keep_dark", shaderKeepDark);
        shader_add.setUniform1f("shaderAlphaThresholdBg", shaderAlphaThresholdBg);
        shader_add.setUniform1f("shaderAlphaThresholdNi", shaderAlphaThresholdNi);
        shader_add.setUniform1f("shaderLumThreshold", shaderLumThreshold);
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

}

//--------------------------------------------------------------
void ImageBuffer::resetBuffer(bool &isReset){
    
    if(isReset){
    
        bgFbo.begin();
        //0 means transparent
        //ofClear(0,0,0, 255);
        // let tranparent background of ImageBuffer
        ofClear(0,0,0, 0);
        bgFbo.end();
        
    }
    
}

