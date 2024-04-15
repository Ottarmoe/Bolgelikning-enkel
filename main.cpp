#include "DrawableEnvironment.h"
#include "myAudioUtilities.h"

int main() {
    DrawableEnvironment env;
    AudioStream austr(44100, 2000);

    while(!env.getwin().should_close()){
        env.control();
        env.render();
        GLOBALFREQ = exp(double(env.getWorldMousePos().y)/100.)*100.;
    }
    
    return 0;
}