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
  
#if INPUT_VIDEO == 0
    videoIndex.addListener(this, &Input::setVideoIndex);
    playerPause.addListener(this, &Input::setVideoPause);
#endif
    
    // GUI MANAGEMENT
    pg->setName("Input");
    pg->add(isShown.set("show", true));
#if INPUT_VIDEO == 0
    pg->add(videoIndex.set("video_index", 3, 1, 7));
    pg->add(playerPause.set("video_pause", false));
    
#endif
    pg->add(gain.set("gain lum", 0,-1.0, 1.0));
    pg->add(threshold.set("threshold", 0.1,0, 1.0));
    pg->add(smooth.set("threshold_curve", 0, 0, 1));
    //pg->add(transparency.set("transparency", 0, 0, 1));
    transparency.set("transparency", 0, 0, 1);
    //pg->add(blur.set("blur", 0, 0, 10));
    //pg->add(skipStep.set("skip_step", 2, 0, 3));
    
#if INPUT_VIDEO == 0
    name = "VIDEO PLAYER";
#elif INPUT_VIDEO == 1
    //BLACK MAGIC
    blackMagic.setup(INPUT_WIDTH, INPUT_HEIGHT, INPUT_FRAMERATE);
    name = "BLACK MAGIC";
#elif INPUT_VIDEO == 2
    // VIDEO GRABBER
    videoGrabberIndex.addListener(this, &Input::videoGrabberInit);
    std::vector<ofVideoDevice> listOfDevice = videoGrabber.listDevices();
    int indexBlackMagicDevice = 1;
    int index = 0;
    for(std::vector<ofVideoDevice>::iterator it = listOfDevice.begin(); it<listOfDevice.end(); ++it ){
        ofLog(OF_LOG_NOTICE, "Video Grabber Device :"+it->deviceName+" : "+it->hardwareName);
        if(it->deviceName=="Blackmagic Design") indexBlackMagicDevice = index;
        index++;
    }
    
    pg->add(videoGrabberIndex.set("webcam_index", indexBlackMagicDevice, 0, 10));
    name = "WEB CAM";
#endif
    
    // ADITIONAL PARAMETERS
    isUpdatingRight = false;
    lastTimeNewFrame = 0;


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
    /*
    fboBlur1.allocate(w, h, GL_RGBA);
    fboBlur1.begin();
    ofClear(255,255,255, 0);
    fboBlur1.end();
    // FBO CLEAR
    fboBlur2.allocate(w, h, GL_RGBA);
    fboBlur2.begin();
    ofClear(255,255,255, 0);
    fboBlur2.end();
     */
    
    // SHADER
#ifdef TARGET_OPENGLES
    //shader.load("shadersES2/shader");
#else
    if(ofIsGLProgrammableRenderer()){
        //shader.load("shadersGL3/shader");

    }else{
        shaderTreshHsv.load("shadersGL2/shaderTreshHsv");
        /*
        shaderBlurX.load("shadersGL2/shaderBlurX");
        shaderBlurY.load("shadersGL2/shaderBlurY");
         */

    }
#endif
    
    //shader.setUniformTexture("text0", fbo.getTexture(), 0);

    
    
}


//--------------------------------------------------------------
void Input::update(){
    
    //Update blackmagic
    
#if INPUT_VIDEO == 0
    player.update();
    isUpdatingRight = player.isFrameNew();
    lastTimeAppFrame = ofGetSystemTimeMicros();
    if(isUpdatingRight) lastTimeNewFrame = ofGetSystemTimeMicros();
#elif INPUT_VIDEO == 1
    isUpdatingRight = blackMagic.update();
    lastTimeAppFrame = ofGetSystemTimeMicros();
    if(isUpdatingRight) lastTimeNewFrame = ofGetSystemTimeMicros();
#elif INPUT_VIDEO == 2
    videoGrabber.update();
    isUpdatingRight = videoGrabber.isFrameNew();
    lastTimeAppFrame = ofGetSystemTimeMicros();
    if(isUpdatingRight) lastTimeNewFrame = ofGetSystemTimeMicros();
    
#endif
    
    
    /* Draw only if there is a new frame available
     
     */
    
    if(isUpdatingRight){
        //Threshold
        fboTresh.begin();
        ofClear(255,255,255, 0);
        shaderTreshHsv.begin();
        shaderTreshHsv.setUniform1f("brightness_to_add", gain);
        shaderTreshHsv.setUniform1f("threshold", threshold);
        shaderTreshHsv.setUniform1f("threshold_smooth", smooth);
        shaderTreshHsv.setUniform1f("transparency_smooth", transparency);
        ofSetColor(255, 255, 255, 255);
#if     INPUT_VIDEO == 0
        player.draw(0, 0, w, h);
#elif   INPUT_VIDEO == 1
        blackMagic.drawColor();
#elif   INPUT_VIDEO == 2
        videoGrabber.draw(0, 0, fboTresh.getWidth(), fboTresh.getHeight());
#endif
        shaderTreshHsv.end();
        fboTresh.end();
        
    }
     
     
    
    

    
    
    
    /*
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
     
     */
    
    // FINAL 2
    fbo.begin();
    ofClear(255,255,255, 0);
    ofEnableAlphaBlending();
    fboTresh.draw(0,0);
    /*
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
     */
    
    ofDisableAlphaBlending();
    fbo.end();	
    

    
    
    
}

#if INPUT_VIDEO == 0
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
#endif

#if INPUT_VIDEO == 0
//--------------------------------------------------------------
void Input::setVideoPause(bool &isPause){
    
    player.setPaused(isPause);
    
}
#endif

#if INPUT_VIDEO == 2
//--------------------------------------------------------------
void Input::videoGrabberInit(int &index){
    
videoGrabber.close();
videoGrabber.setDeviceID(index);
videoGrabber.setDesiredFrameRate(INPUT_FRAMERATE);
videoGrabber.initGrabber(INPUT_WIDTH, INPUT_HEIGHT);

}
#endif
