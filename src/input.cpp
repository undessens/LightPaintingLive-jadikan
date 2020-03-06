//
//  input.cpp
//  ofLightPaintApp
//
//  Created by Admin Mac on 18/12/2019.
//

#include "input.hpp"

//--------------------------------------------------------------
Input::Input(ofParameterGroup* _pg,
                         int _w,
                         int _h
                         ){
    
    
    pg = _pg;
    w = _w;
    h = _h;
    setup();
    
}

//--------------------------------------------------------------
void Input::setup(){
    
    useOfVideoPlayer.addListener(this,&Input::setUseVideo);
    videoIndex.addListener(this, &Input::setVideoIndex);
    playerPause.addListener(this, &Input::setVideoPause);
    
    // GUI MANAGEMENT
    pg->setName("Input");
    pg->add(isShown.set("show", true));
    pg->add(useOfVideoPlayer.set("use of video", true));
    pg->add(videoIndex.set("video index", 3, 1, 7));
    pg->add(threshold.set("threshold", 0.1,0, 1.0));
    pg->add(smooth.set("smooth threshold", 0, 0, 5));
    pg->add(playerPause.set("Video Pause", false));
    pg->add(blur.set("blur", 0, 0, 10));
    pg->add(skipStep.set("Skip step", 0, 0, 3));
    
    //BLACK MAGIC
    blackMagic.setup(1920, 1080, 30);
    
    // FBO CLEAR
    fbo.allocate(w, h, GL_RGBA);
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();
    // FBO CLEAR
    fboTresh.allocate(w, h, GL_RGBA);
    fboTresh.begin();
    ofClear(255,255,255, 0);
    fboTresh.end();
    // FBO CLEAR
    fboBlur1.allocate(w, h, GL_RGBA);
    fboBlur1.begin();
    ofClear(255,255,255, 0);
    fboBlur1.end();
    // FBO CLEAR
    fboBlur2.allocate(w, h, GL_RGBA);
    fboBlur2.begin();
    ofClear(255,255,255, 0);
    fboBlur2.end();
    
    // SHADER
#ifdef TARGET_OPENGLES
    //shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        //shader.load("shadersGL3/shader");

    }else{
        shaderTreshHsv.load("shadersGL2/shaderTreshHsv");
        shaderBlurX.load("shadersGL2/shaderBlurX");
        shaderBlurY.load("shadersGL2/shaderBlurY");

    }
#endif
    
    //shader.setUniformTexture("text0", fbo.getTexture(), 0);

    
    
}

//--------------------------------------------------------------
void Input::loadMovie(string name){
    
    player.load(name);
    player.play();
    
}

//--------------------------------------------------------------
void Input::update(){
    
    //Update blackmagic
    isUpdatingRight = blackMagic.update();
    
    
    //Threshold
    fboTresh.begin();
    ofClear(255,255,255, 0);
    shaderTreshHsv.begin();
    shaderTreshHsv.setUniform1f("squared_threshold", threshold);
    shaderTreshHsv.setUniform1f("squared_smooth", smooth);
    if(useOfVideoPlayer){
        player.update();
        ofSetColor(255, 255, 255, 255);
        player.draw(0, 0, w, h);
        
    }else{
        ofSetColor(255, 255, 255, 255);
        blackMagic.drawColor();
        //ofSetColor(0, 255, 0, 255);
        //ofFill();
        //ofDrawRectangle(0, 0, w, h);
    }
    shaderTreshHsv.end();
    fboTresh.end();
    
    // BLUR 1
    fboBlur1.begin();
    ofClear(255,255,255, 0);
    ofEnableAlphaBlending();
    shaderBlurX.begin();
    shaderBlurX.setUniform1f("blurAmnt", blur);
    fboTresh.draw(0, 0);
    shaderBlurX.end();
    ofDisableAlphaBlending();
    fboBlur1.end();
    
    // BLUR 2
    fboBlur2.begin();
    ofClear(255,255,255, 0);
    ofEnableAlphaBlending();
    shaderBlurY.begin();
    shaderBlurY.setUniform1f("blurAmnt", blur);
    fboBlur1.draw(0, 0);
    shaderBlurY.end();
    ofDisableAlphaBlending();
    fboBlur2.end();
    
    // FINAL 2
    fbo.begin();
    ofClear(255,255,255, 0);
    ofEnableAlphaBlending();
    switch (skipStep) {
        case 0:
            fboBlur2.draw(0,0);
            break;
        case 1:
            fboBlur1.draw(0,0);
            break;
        case 2:
            fboTresh.draw(0,0);
            break;
            
        default:
            break;
    }
    ofDisableAlphaBlending();
    fboBlur2.end();
    

    
    
    
}

//--------------------------------------------------------------
void Input::setUseVideo(bool &isUse){
    
    if(isUse){
        
        fbo.begin();
        ofClear(0, 0, 0, 255);
        fbo.end();
        player.stop();
        player.firstFrame();
        player.play();
        
    }else{
        
        player.stop();
        fbo.begin();
        ofClear(0, 255, 0, 127);
        fbo.end();
    }
    
}


//--------------------------------------------------------------
void Input::setVideoIndex(int &newIndex){
    
    ofFile myFile;
    myFile.open("movie/"+ofToString(newIndex)+".mp4");
    if(myFile.exists()){
        ofLog(OF_LOG_NOTICE, "checking video : TRUE");
    }
    else{
        ofLog(OF_LOG_NOTICE, "checking video : FALSE");
        myFile.open("clouds.mov");
    }
    
    player.load(myFile.path());
    player.setLoopState(OF_LOOP_NORMAL);
    player.setVolume(0);
    player.play();
    
    
}

//--------------------------------------------------------------
void Input::setVideoPause(bool &isPause){
    
    player.setPaused(isPause);
    
}


