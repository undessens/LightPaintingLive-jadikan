//
//  mask.cpp
//  ofLightPaintApp
//
//  Created by Admin Mac on 18/12/2019.
//

#include "mask.hpp"

/*
 
 Le mask est un fichier image, ajouté au résultat en multiply. Avec une option d'aperçu.
 
 */

//--------------------------------------------------------------
Mask::Mask(){
    
    
    
    
}



//--------------------------------------------------------------
Mask::Mask( ofParameterGroup* _pg, int _w, int _h){
    
    pg = _pg;
    w = _w;
    h = _h;
    setup();
    
}

//--------------------------------------------------------------
void Mask::setup(){
    
    maskIndex = 0;
    bgIndex = 0;
    
    maskIndex.addListener(this, &Mask::setMaskIndex);
    bgIndex.addListener(this, &Mask::setBgIndex);
    
    ofDirectory dir;
    dir.open("mask");
    dir.allowExt("jpg");
    dir.listDir("mask");
    int nbFile;
    if(dir.exists()){
        vector<ofFile> listOfFile  = dir.getFiles();
        for(vector<ofFile>::iterator it = listOfFile.begin(); it<listOfFile.end(); it++ ){
            ofImage img;
            img.load(it->path());
            listOfMask.push_back(img);
        }
    }
    dir.open("background");
    dir.listDir("background");
    if(dir.exists()){
        vector<ofFile> listOfFile  = dir.getFiles();
        for(vector<ofFile>::iterator it = listOfFile.begin(); it<listOfFile.end(); it++ ){
            ofImage img;
            img.load(it->path());
            listOfBackground.push_back(img);
        }
        
    }

    
    pg->setName("Mask");
    pg->add(isShown.set("show", false));
    pg->add(bgIndex.set("background_index", 0 ,0, listOfBackground.size()));
    pg->add(maskIndex.set("mask_index",0, 0, listOfMask.size()));
    
    // FBO CLEAR
    fbo.allocate(w, h, GL_RGBA);
    fbo.begin();
    ofClear(0,0,0, 0);
    fbo.end();
    
    
    
}

//--------------------------------------------------------------
void Mask::update(ofFbo *input){
    
    clearFbo();
    
    fbo.begin();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    if(bgIndex>0){
        listOfBackground[bgIndex-1].draw(0, 0);
        ofSetColor(255, 255, 255,255);
    }
    
    input->draw(0,0);
    ofDisableBlendMode();
    
    if(maskIndex>0){
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        ofSetColor(255, 255, 255,255);
        listOfMask[maskIndex-1].draw(0, 0);
        ofDisableBlendMode();
    }
    
    fbo.end();
    
}

//--------------------------------------------------------------
void Mask::draw(int x,int  y, int w, int h){
    

    fbo.draw(x, y, w, h);
    
    ofDisableBlendMode();
}

//--------------------------------------------------------------
void Mask::draw(ofRectangle rec){
    
    if(maskIndex>0){
        
        ofSetColor(255, 255, 255,255);
        fbo.draw(rec);
        
        ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
        listOfMask[maskIndex-1].draw(rec);
        ofDisableBlendMode();
        
    }else{
        
    fbo.draw(rec);
    }
    
    
    
}

//--------------------------------------------------------------
void Mask::setMaskIndex(int &index){
    int i=0;
    i++;
}

//--------------------------------------------------------------
void Mask::setBgIndex(int &index){
    int i=0;
    i++;
}

//--------------------------------------------------------------
void Mask::drawMaskImage(ofRectangle r){
    
    int nbMask = listOfMask.size();
    int marginX = 2;
    int finalWidth = (r.width / nbMask) - nbMask*marginX;
    int finalHeight = finalWidth * 0.58;
    
    for(int i=0; i<listOfMask.size(); i++){
        
        int finalX = r.x + marginX + (finalWidth+marginX)* i;
        int finalY = r.y + (r.height - finalHeight)/2;
        ofRectangle finalRect = ofRectangle(finalX, finalY, finalWidth, finalHeight);
        ofSetColor(255);
        listOfMask[i].draw(finalRect);
        if(i == (maskIndex-1)){
            ofNoFill();
            ofSetColor(ofColor::yellow);
            ofDrawRectangle(finalRect);
        }
        
        
    }
    
}

//--------------------------------------------------------------
void Mask::drawBgImage(ofRectangle r){
    
    int nbBg = listOfBackground.size();
    int marginX = 2;
    int finalWidth = (r.width / nbBg) - nbBg*marginX;
    int finalHeight = finalWidth * 0.58;
    
    for(int i=0; i<nbBg; i++){
        int finalX = r.x + marginX + (finalWidth+marginX)* i;
        int finalY = r.y + (r.height - finalHeight)/2;
        ofRectangle finalRect = ofRectangle(finalX, finalY, finalWidth, finalHeight);
        ofSetColor(255);
        listOfBackground[i].draw(finalRect);
        if(i == (bgIndex-1)){
            ofNoFill();
            ofSetColor(ofColor::yellow);
            ofDrawRectangle(finalRect);
        }
        
        
    }
    
}

//--------------------------------------------------------------
void Mask::clearFbo(){
    
        fbo.begin();
        ofClear(0,0,0, 0);
        fbo.end();
        
}



