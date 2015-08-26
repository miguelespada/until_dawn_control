#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxJSON.h"
#include "ofxOsc.h"

#define HOST_1 "192.168.1.31"
#define HOST_2 "192.168.1.31"
#define HOST_3 "192.168.1.30"

#define PORT_0 12350
#define PORT_1 12341
#define PORT_2 12342
#define PORT_3 12343


#define LOCAL true

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    void reset();
    void idle();
    void next();
    void index();
    void calculandoIndex();
    void sendAll(ofxOscMessage m);
    
    ofxJSONElement user;
    ofTrueTypeFont font;
    ofxFloatSlider indice;
    
    ofxFloatSlider thermal_target_x;
    ofxFloatSlider thermal_target_y;
    ofxToggle bFake;
    bool bSave;
    
    
    ofxFloatSlider heart_threshold;
    
    float thermal_target_delta_y;
    float thermal_target_delta_x;
    
    ofxPanel gui;
    
    float stress;
    bool bStress;
    bool bHStress;
    
    ofxOscReceiver receiver;
    ofxOscSender sender_1;
    ofxOscSender sender_2;
    ofxOscSender sender_3;
    
    ofxOscSender local_sender_1;
    ofxOscSender local_sender_2;
    ofxOscSender local_sender_3;
    
    
    vector<int> beats;
    vector<int> fake_beats;
    int fake_beats_index = 0;
    
    int beatAvg;
    int beatN;
    float lastBeat;
    int avg = 60;
    
    void updateBeat(ofxOscMessage m);
    void updateStress();
    void updatePositions();
    void saveData();
    void parseOsc();
    void saveFrames();
    string computeIndice();

    int initTime;
    int runningTime;
    
    int temperature;
    int conductance;
    int galvanicVoltage;
    
    float avgFlow;
    float avgThermal;
    
};
