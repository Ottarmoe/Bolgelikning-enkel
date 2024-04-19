#include "DrawableEnvironment.h"
#include "myAudioUtilities.h"
#include "generators.h"


#include "std_lib_facilities.h"
#include "AnimationWindow.h"
#include "widgets/TextInput.h"


void optionalCallback(const std::string& str){
    std::cout<<"new value: "<<str<<"\n";
}


float fetchY(const StringSegment<float>& seg){
    return seg.y;
}

int main() {
    DrawableEnvironment env;
    AudioStream austr(44100, 4000);
    String<float> stringsim;
    //stringsim.substeps = 2;
    grapher gra;
    gra.miny = -1;
    gra.maxy = 1;
    gra.resize(50);
    env.bind(gra);

    Pick lastpick = {0,0,0};
    while(!env.getwin().should_close()){
        Pick thispick;
        thispick.active = gra.localHeld;
        if(thispick.active){
            thispick.pos = gra.localMousePosition(env);
            thispick.pos.y = thispick.pos.y*(gra.maxy-gra.miny)+gra.miny;
            thispick.radius = 0.01f;
        }

        uint numtoQueue = std::min(austr.numQueuedIn(env.getFrameTime()), 10000u);
        
        uint substeps = 1;

        vec2 pickstep = (thispick.pos-lastpick.pos)*(1.f/float(numtoQueue*substeps));
        for(uint i = 0; i<numtoQueue; ++i){
            for(int s = 0; s<substeps-1; ++s){
                lastpick.pos += pickstep;
                stringsim.stepStroked(lastpick);
            }
            lastpick.pos += pickstep;
            austr.queueSample(stringsim.stepStroked(lastpick));
        }
        lastpick = thispick;
        gra.load(stringsim.string, fetchY);
        env.control();
        env.render();
    }
    return 0;

    /*AudioStream austr(44100, 4000);
    sinusoidalGenerator sinsim;
    String<float> stringsim;
    stringsim.substeps = 2;

    uint frame = 0;
    for(int i = 0; i<200 && (!env.getwin().should_close()); ++i){
        env.control();
        env.render();
        sinsim.frequency = exp(double(env.getWorldMousePos().y)/100.)*100.;
        uint samplesToQueue = austr.numQueuedIn(env.getFrameTime());
        samplesToQueue = std::min(10000u, samplesToQueue);
        for(uint i = 0; i<samplesToQueue; ++i){
            austr.queueSample(sinsim.step()*0.9);
        }
    }

    while(!env.getwin().should_close()){
        frame++;
        env.control();
        env.render();
        sinsim.frequency = exp(double(env.getWorldMousePos().y)/100.)*100.;
        uint samplesToQueue = austr.numQueuedIn(env.getFrameTime());
        samplesToQueue = std::min(10000u, samplesToQueue);
        if(frame%20==0){
            std::cout<<"frameTime "<<env.getFrameTime()<<"\n";
            std::cout<<"generating " <<samplesToQueue<<"\n";
            std::cout<<"from "<<austr.getSampleRate()<<"\n";
        }
        for(uint i = 0; i<samplesToQueue; ++i){
            austr.queueSample(sinsim.step()*0.9);
        }
    }
    
    return 0;*/
}