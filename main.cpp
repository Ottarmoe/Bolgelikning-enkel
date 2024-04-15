#include "DrawableEnvironment.h"
#include "myAudioUtilities.h"

int main() {
    DrawableEnvironment env;
    AudioStream austr(44100, 2000);

    uint frame = 0;
    while(!env.getwin().should_close()){
        frame++;
        env.control();
        env.render();
        GLOBALFREQ = exp(double(env.getWorldMousePos().y)/100.)*100.;

        uint samplesToQueue = austr.numQueuedIn(env.getFrameTime());
        if(frame%20==0){
            std::cout<<"frameTime "<<env.getFrameTime()<<"\n";
            std::cout<<"generating " <<samplesToQueue<<"\n";
            std::cout<<"from "<<austr.getSampleRate()<<"\n";
        }
        samplesToQueue = std::min(10000u, samplesToQueue);
        for(int i = 0; i<samplesToQueue; ++i){
            austr.queueSample(getNextPhaseSample());
        }
    }
    
    return 0;
}