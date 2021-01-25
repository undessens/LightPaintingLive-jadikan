#include "ofApp.h"

/*
 
 Global architecture
 
 
                 Input ( file or hdmi input )
                    |
                   / \
                  /   \
        videoBuffer     ImageBuffer
                  \   /
                   Mask
                    |
                 ouput_fx
                    |
                image_layer ( can be post of pre-fx )
                    |
                 output_final
 
 
 
 */


/*
 LIST OF OFX ADDONS THOUGHT
 https://github.com/armadillu/ofxFboBlur
 https://github.com/tado/ofxAlphaMaskTexture
 
 gestion des parametres:
 enregistrés les paramètres en sauvegardes
 
 
 Colorimétrie:
 peut etre un shader manuel pour un ajout propre
 
 ajout de débug:
 affichage de la frame en cours du fichier vidéo
 
 Image Buffer, enregitré image sur image, puis effacer la dernière.
 
 
 
 */

/*
 RENCONTRE AVEC SYLVIE :
 
 faire un shader d'impression.
 on efface, l'image courante, mais ce qui a passé un certain seuil, ne bouge pas. en gros, ce qui est cramé reste cramé, le reste d'efface.
 
 Afficher la valeur max de l'image d'entrée.
 Car ça conditionne, la vitesse à laquelle on crame.
 
 
 */

/*
 RENCONTRE AVEC JADIKAN janvier 2021
 Enlever la transparence avant le buffet image, et juste comparer la différence de luminosité
 Entre le fond et la nouvelle image. Si cette différence est faible, alors on ajoute un effet de transparence. ( proportionnelle ).
 
 Rajouter un mirroir
 Rajouter un fullscreen
 
 
*/

/*
 ========================================
 HSV seems to be better than hsl
 because pure blue and pure white are at the same level of value.
 When using the threshold, we don"t want to discriminate any pure color compare to white
 
 
 
 
 //HSV (hue, saturation, value) to RGB.
 //Sources: https://gist.github.com/yiwenl/745bfea7f04c456e0101, https://gist.github.com/sugi-cho/6a01cae436acddd72bdf
 vec3 hsv2rgb(vec3 c){
 vec4 K=vec4(1.,2./3.,1./3.,3.);
 return c.z*mix(K.xxx,saturate(abs(fract(c.x+K.xyz)*6.-K.w)-K.x),c.y);
 }
 
 //RGB to HSV.
 //Source: https://gist.github.com/yiwenl/745bfea7f04c456e0101
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
 //Source: https://gist.github.com/sugi-cho/6a01cae436acddd72bdf
 vec3 rgb2hsv_2(vec3 c){
 vec4 K=vec4(0.,-1./3.,2./3.,-1.),
 p=mix(vec4(c.bg ,K.wz),vec4(c.gb,K.xy ),step(c.b,c.g)),
 q=mix(vec4(p.xyw,c.r ),vec4(c.r ,p.yzx),step(p.x,c.r));
 float d=q.x-min(q.w,q.y),
 e=1e-10;
 return vec3(abs(q.z+(q.w-q.y)/(6.*d+e)),d/(q.x+e),q.x);
 }
 

 */





//--------------------------------------------------------------
void ofApp::setup(){

    input_w = 1920;
    input_h = 1080;
    ofBackground(100);
    ofSetFrameRate(30);
    
    //INSTANCIATE ALL MODULE
    input = new Input(&pg_input, input_w, input_h);
    imageBuffer = new ImageBuffer(&pg_imageBuffer, input_w, input_h, 5);
    mask= new Mask(&pg_mask, input_w, input_h);
    
    //Callback function
    global_zoom.addListener(this, &ofApp::setGlobalZoom);
    
    //GUI
    pg.setName("main");
    pg.add(bg_color.set("BackGround", 35, 0, 255));
    pg.add(zoom_level.set("Zoom Image", 2, 1, 10));
    pg.add(global_zoom.set("App Zoom", 2, 1, 8));
    gui.setup(pg);
    gui.add(pg_input);
    gui.add(pg_imageBuffer);
    gui.add(pg_mask);
    
    

    
    //Temp : TODO change it to fx class
    fboPost.allocate(input_w, input_h, GL_RGBA);
    
    // Utils - transparent image
    transparentBg.load("icon/transparent2.png");
    arrowRight.load("icon/arrow_right.png");
    arrowBottom.load("icon/arrow_bottom.png");
    //Utils font
    fontBigSize=34;
    fontMediumSize = 25;
    fontSmallSize = 12;
    //Utils Margin - resize in global zoom function
    marginH = 10;
    marginW = 10;
    
    
    
    //SYPHON SETTINGS
    syphonOut.setName("Jadikan LightPainting");
    
    //Zoom image setup
    zoomImage.allocate(vignetteZoom.width/2, vignetteZoom.height/2, OF_IMAGE_COLOR_ALPHA);
    zoomPixels.allocate(1920,1080, OF_IMAGE_COLOR_ALPHA );
    zoomPixels.setColor(ofColor::goldenRod);
    zoomRectangle = ofRectangle();
    bool isZoomAllocated = zoomPixels.isAllocated();
    
    //FINISH WITH :
    //Force zoom and init size properly once everything is loaded
    float actualZoom = global_zoom;
    setGlobalZoom(actualZoom);
    

}


//--------------------------------------------------------------
void ofApp::update(){
    
    ofEnableAlphaBlending();

    //update module one by one
    input->update();
    imageBuffer->update(&(input->fbo), input->player.getCurrentFrame() );
    
    mask->update(&imageBuffer->fbo);
    
    fboPost.begin();
    mask->draw(0,0,input_w,input_h);
    fboPost.end();
    
    //Zoom updating
    float zoomRatioX = 0;
    float zoomRatioY = 0;
    
    // TODO : change it to a specific function
    //Set Manually zoom when mouse pressed
    if(ofGetMousePressed()){
    
        if(vignetteInput.inside(ofGetMouseX(), ofGetMouseY())){
            float relativePosX = ( ofGetMouseX() - vignetteInput.x );
            float relativePosY = ( ofGetMouseY() - vignetteInput.y );
            float zoomRatioW = (vignetteInput.width / input->w )*vignetteZoom.width/zoom_level;
            float zoomRatioH = (vignetteInput.height / input->h )*vignetteZoom.height/zoom_level;
            zoomRectangle = ofRectangle(relativePosX + vignetteInput.x, relativePosY+vignetteInput.y , zoomRatioW, zoomRatioH);
            zoomLevelEntry = 1;
            
        }
        
        else if(vignetteBuffer.inside(ofGetMouseX(), ofGetMouseY()) ){
            float relativePosX = ( ofGetMouseX() - vignetteBuffer.x );
            float relativePosY = ( ofGetMouseY() - vignetteBuffer.y );
            float zoomRatioW = (vignetteBuffer.width / input->w )*vignetteZoom.width/zoom_level;
            float zoomRatioH = (vignetteBuffer.height / input->h )*vignetteZoom.height/zoom_level;
            zoomRectangle = ofRectangle(relativePosX + vignetteBuffer.x, relativePosY +vignetteBuffer.y, zoomRatioW, zoomRatioH);
            zoomLevelEntry = 2;
        }
        
        else if(vignetteMask.inside(ofGetMouseX(), ofGetMouseY()) ){
            float relativePosX = ( ofGetMouseX() - vignetteMask.x );
            float relativePosY = ( ofGetMouseY() - vignetteMask.y );
            float zoomRatioW = (vignetteMask.width / input->w )*vignetteZoom.width/zoom_level;
            float zoomRatioH = (vignetteMask.height / input->h )*vignetteZoom.height/zoom_level;
            zoomRectangle = ofRectangle(relativePosX + vignetteMask.x, relativePosY +vignetteMask.y, zoomRatioW, zoomRatioH);
            zoomLevelEntry = 3;
        }
        
        else if(areaGui.inside(ofGetMouseX(), ofGetMouseY())){
        
        }
        
        else {
            
            zoomLevelEntry = 0;
        
        }
        
        if(zoomLevelEntry>0){
            vignetteZoom.x = zoomRectangle.x + zoomRectangle.width;
            vignetteZoom.y = zoomRectangle.y + zoomRectangle.height;
            vignetteZoom.width = 355;
            vignetteZoom.height = 200;
        }
        
    }
    
    updateZoom();
 
 
 
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(bg_color);
    
   
    ofSetColor(255, 255, 255);
    ofFill();
    syphonOut.publishTexture(&(mask->fbo.getTexture()));

    ofSetColor(255,255,255);
    gui.draw();
    
    //Draw app
    drawApp(ofRectangle(areaGui.x + areaGui.width, marginH, ofGetWidth() - areaGui.width, 0));

    //INPUT
    drawInput(areaApp);
    
    //Image Buffer
    drawImageBuffer(areaInput);

    //MASK
    drawMask(areaBuffer);
    /*
    
    //POST FX
    ofSetColor(0, 255, 255);
    ofNoFill();
    ofDrawRectangle(vignetteFx);
    ofSetColor(255,255,255);
    fboPost.draw(vignetteFx);
     */
    
    //Draw Grid
    drawGridArea();
    
    //ZOOM
    if(zoomLevelEntry>0){
    ofSetColor(255, 0, 0);
    ofNoFill();
    ofDrawRectangle(zoomRectangle);
    ofSetColor(0, 0, 255);
    ofNoFill();
    //ofDrawRectangle(vignetteZoom);
    ofSetColor(255,255,255);
    //transparentBg.draw(vignetteZoom);
    //zoomImage.draw(vignetteZoom);
    transparentBg.draw(vignetteZoom);
    zoomImage.draw(vignetteZoom);
    }
    
    //Openframeworks bug as usual
    ofSetColor(255);

}

//--------------------------------------------------------------
void ofApp::updateZoom(){
    
    // ZOOM is set from INPUT
    if(zoomLevelEntry == 1){
        
        input->fbo.readToPixels(zoomPixels);
        float posX = zoomRectangle.x - vignetteInput.x;
        float posY = zoomRectangle.y - vignetteInput.y;
        float ratioX = vignetteInput.width / input_w;
        float ratioY = vignetteInput.height / input_h;
        int cropX = posX / ratioX;
        int cropY = posY / ratioY;
        int cropW = vignetteZoom.width/zoom_level;
        int cropH = vignetteZoom.height/zoom_level;
        zoomPixels.crop(cropX, cropY, cropW, cropH);
        
        
    }
    
    // ZOOM is set from BUFFER_IMAGE
    if(zoomLevelEntry == 2){
        
        imageBuffer->fbo.readToPixels(zoomPixels);
        float posX = zoomRectangle.x - vignetteBuffer.x;
        float posY = zoomRectangle.y - vignetteBuffer.y;
        float ratioX = vignetteBuffer.width / input_w;
        float ratioY = vignetteBuffer.height / input_h;
        int cropX = posX / ratioX;
        int cropY = posY / ratioY;
        int cropW = vignetteZoom.width/zoom_level;
        int cropH = vignetteZoom.height/zoom_level;
        zoomPixels.crop(cropX, cropY, cropW, cropH);
    
    }
    
    // ZOOM is set from BUFFER_IMAGE
    if(zoomLevelEntry == 3){
        
        mask->fbo.readToPixels(zoomPixels);
        float posX = zoomRectangle.x - vignetteMask.x;
        float posY = zoomRectangle.y - vignetteMask.y;
        float ratioX = vignetteMask.width / input_w;
        float ratioY = vignetteMask.height / input_h;
        int cropX = posX / ratioX;
        int cropY = posY / ratioY;
        int cropW = vignetteMask.width/zoom_level;
        int cropH = vignetteMask.height/zoom_level;
        zoomPixels.crop(cropX, cropY, cropW, cropH);
    
    }
    
    // ZOOM is set to NOTHING
    if(zoomLevelEntry == 0){
        zoomPixels.setColor(ofColor::black);
        zoomPixels.crop(0,0, vignetteZoom.width, vignetteZoom.height);
    }
    
    zoomImage.setFromPixels(zoomPixels);
    

     


}

void ofApp::setGlobalZoom(float& zoom){
    
    float ratio = 1.0f/(1.0f*zoom);
    
    fontBigSize=40 * ratio;
    fontMediumSize = 28 * ratio;
    fontSmallSize = 20 * ratio;
    
    fontBig.load("font.ttf", fontBigSize);
    fontMedium.load("font.ttf", fontMediumSize);
    fontSmall.load("font.ttf", fontSmallSize);
    
    marginH = 40 * ratio;
    marginW = 60 * ratio;
    
    int vignetteW = 600 * ratio;
    int vignetteH = 336* ratio;
    areaGui = gui.getShape();
    int vignetteX = areaGui.x + areaGui.width + 10;
    
    vignetteInput = ofRectangle(vignetteX, 0, vignetteW, vignetteH);
    vignetteBuffer = ofRectangle(vignetteX, 220, vignetteW, vignetteH);
    vignetteMask = ofRectangle(vignetteX, 440, vignetteW, vignetteH);
    vignetteFx = ofRectangle(vignetteX, 660, vignetteW, vignetteH);
    
    int areaWidth = (ofGetWidth() - areaGui.width ) ;
    areaApp.width = areaWidth;
    areaInput.width = areaWidth;
    areaBuffer.width = areaWidth;
    areaMask.width = areaWidth;
    
    
}

//--------------------------------------------------------------
ofRectangle ofApp::writeText(string msg, int x, int y, int size){
    
    
    int textColor = 255;
    if(bg_color > 140){
        textColor = 0;
    }
    
    ofSetColor(textColor);
    
    switch (size) {
        case FONT_BIG:
            fontBig.drawString(msg, x, y);
            return fontBig.getStringBoundingBox(msg, x, y);
            break;
        case FONT_MEDIUM:
            fontMedium.drawString(msg, x, y);
            return fontMedium.getStringBoundingBox(msg, x, y);
            break;
        case FONT_SMALL:
            fontSmall.drawString(msg, x, y);
            return fontSmall.getStringBoundingBox(msg, x, y);
            break;
            
        default:
            break;
    }
    
    return ofRectangle(0, 0, 10, 10);
    
}
//--------------------------------------------------------------
void ofApp::drawArraw( int x, int y, int w , bool isOpen){
    
    if(!isOpen){
        arrowRight.draw(x, y, w, w);
    }else{
        arrowBottom.draw(x, y, w, w);
    }
    
}

//--------------------------------------------------------------
void ofApp::drawGridArea(){
    
    ofSetColor(bg_color + 20);
    ofNoFill();
    ofSetLineWidth(1);
    ofDrawRectangle(areaGui);
    ofDrawRectangle(areaApp);
    ofDrawRectangle(areaInput);
    ofDrawRectangle(areaBuffer);
    ofDrawRectangle(areaMask);
    
}

//--------------------------------------------------------------
int ofApp::drawApp(ofRectangle lastElement){
    areaApp.x = lastElement.x;
    areaApp.y = lastElement.y + lastElement.height;
    int totalH = 0;
    totalH += (writeText("app", areaApp.x + 1.5*marginH , areaApp.y + 1.5*marginH, FONT_BIG)).height;
    totalH += 1.5*marginH;
    
    string stringFps = "FPS : "+ofToString(int(  ofGetFrameRate()));
    ofRectangle fps = writeText(stringFps, lastElement.x, areaApp.y + totalH, FONT_SMALL);
    
    string syphonClient  = "syphon : "+ofToString(syphonOut.getName());
    ofRectangle syphon = writeText(syphonClient, fps.x + fps.width + marginW, areaApp.y  + totalH, FONT_SMALL);
    
    totalH += fps.height;
    
    areaApp.height = totalH;

    
    return areaApp.height;
}
//--------------------------------------------------------------
int ofApp::drawInput(ofRectangle lastElement){
    
    areaInput.x = lastElement.x;
    areaInput.y = lastElement.y + lastElement.height ;
    int totalH = 0;
    totalH += ( writeText("Input", areaInput.x + 1.5*marginH, areaInput.y +  1.5*marginH, FONT_BIG)).height;
    totalH += 1.5*marginH;
    drawArraw( areaInput.x,areaInput.y, 1.5*marginH,input->isShown);

    if(input->isShown){
        
        //Input from file or black Magic, small text
        string inputString  = "from : ";
        if(input->useOfVideoPlayer){
            inputString += " video file";
        }else{
            inputString += " black magic";
        }
        ofRectangle inputRect = (writeText(inputString, areaInput.x, areaInput.y + totalH, FONT_SMALL));
        
        
        //Black magic is OK or error, small text
        string blackMagic = " Black Magic :";
        if(input->isUpdatingRight){
            blackMagic += "OK";
        }else{
            blackMagic += "error";
        }
        writeText(blackMagic, areaInput.x + inputRect.width + marginW, areaInput.y + totalH, FONT_SMALL);
        
        totalH += inputRect.height;
        
        vignetteInput.y = areaInput.y + totalH;
        ofSetColor(255,255,255);
        transparentBg.draw(vignetteInput);
        input->fbo.draw(vignetteInput);
        totalH += vignetteInput.height;
    
    }
    areaInput.height = totalH;
    return areaInput.height;
}
//--------------------------------------------------------------
int ofApp::drawImageBuffer(ofRectangle lastElement){
    
    areaBuffer.x = lastElement.x;
    areaBuffer.y =lastElement.y + lastElement.height ;
    int totalH = 0;
    ofSetColor(255, 255, 255);
    totalH += ( writeText("Buffer", areaBuffer.x + 1.5*marginH, areaBuffer.y +  1.5*marginH, FONT_BIG)).height;
    totalH += 1.5*marginH;
    drawArraw( areaBuffer.x,areaBuffer.y, 1.5*marginH,imageBuffer->isShown);
    
    
    
    if(imageBuffer->isShown){
        
        vignetteBuffer.y = areaBuffer.y + totalH;
        writeText("'space'=reset", vignetteBuffer.x + vignetteBuffer.width, vignetteBuffer.y, FONT_SMALL);
        ofSetColor(255,255,255);
        transparentBg.draw(vignetteBuffer);
        imageBuffer->draw(vignetteBuffer);
        totalH += vignetteBuffer.height;
    }
    
    areaBuffer.height = totalH;

    return areaBuffer.height;
}
//--------------------------------------------------------------
int ofApp::drawMask(ofRectangle lastElement){
    
    areaMask.x = lastElement.x;
    areaMask.y =lastElement.y + lastElement.height ;
    int totalH = 0;
    ofSetColor(255, 255, 255);
    totalH += ( writeText("Mask", areaMask.x + 1.5*marginH, areaMask.y + 1.5*marginH, FONT_BIG)).height;
    totalH += 1.5*marginH;
    drawArraw( areaMask.x,areaMask.y, 1.5*marginH,mask->isShown);
    
    if(mask->isShown){
        //Draw Fbo
        vignetteMask.y = areaMask.y + totalH;
        ofSetColor(255,255,255);
        transparentBg.draw(vignetteMask);
        mask->draw(vignetteMask);
        totalH += vignetteMask.height;
        
        //Draw list of background on the right
        mask->drawBgImage(ofRectangle(vignetteMask.x + vignetteMask.width, vignetteMask.y , areaMask.width - vignetteMask.width, vignetteMask.height/2));
        //Draw list of mask on the right
        mask->drawMaskImage(ofRectangle(vignetteMask.x + vignetteMask.width, vignetteMask.y+ vignetteMask.height/2, areaMask.width - vignetteMask.width, vignetteMask.height/2));
        
    }
    
    areaMask.height = totalH;

    return areaMask.height;
}
//--------------------------------------------------------------
int ofApp::drawFx(ofRectangle lastElement){
    
}
//--------------------------------------------------------------
int ofApp::drawFinal(ofRectangle lastElement){
    
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
     
    zoomLevelEntry = 0;
    switch (key) {
        case ' ':
            bool reset = true;
            imageBuffer->resetBuffer(reset);
        break;
            

    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    zoomLevelEntry = 0;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    float z = global_zoom;
    zoomLevelEntry = 0;
    setGlobalZoom(z);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
