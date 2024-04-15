#pragma once

#include "AnimationWindow.h"
#include "widgets/TextInput.h"
#include "myvecs.h"
#include "myrandoms.h"
#include "mytimes.h"
#include <functional>


class KeyRising
{
private:
    TDT4102::AnimationWindow* win;
    KeyboardKey key;
    bool held;
public:
    KeyRising(TDT4102::AnimationWindow& w, KeyboardKey k)
        :win(&w), key(k), held(0) {}

    operator bool()
    {
        bool oldstate = held;
        held = win->is_key_down(key);
        return held && !oldstate;
    }
};

class KeyHeld
{
private:
    TDT4102::AnimationWindow* win;
    KeyboardKey key;
public:
    KeyHeld(TDT4102::AnimationWindow& w, KeyboardKey k)
        :win(&w), key(k) {}

    operator bool()
    {
        return win->is_key_down(key);
    }
};

template<class T = double, class O = KeyRising> struct controlVariable
{
    T value;

    O valup;
    O valdown;

    controlVariable(TDT4102::AnimationWindow& win, KeyboardKey up, KeyboardKey down, T val = 0)
        :value(val), valup(win, up), valdown(win, down)
    {}

    void update(T scale = 1)
    {
        if (valup)
            value += scale;
        if (valdown)
            value -= scale;
    }
    operator T()
    {
        return value;
    }
};

struct panner
{
    controlVariable<double, KeyHeld> vertical;
    controlVariable<double, KeyHeld> horizontal;
    double scale;
    KeyRising scaleUp;
    KeyRising scaleDown;
    double& x;
    double& y;

    panner(TDT4102::AnimationWindow& win, KeyboardKey faster, KeyboardKey slower, KeyboardKey up, KeyboardKey left, KeyboardKey down, KeyboardKey right)
        :vertical(win, up, down), horizontal(win, right, left), scale(1), scaleUp(win, faster), scaleDown(win, slower),
        x(horizontal.value), y(vertical.value){}

    void update(double scaleFactor = 1) {
        if (scaleUp) scale *= 2;
        if (scaleDown) scale *= 0.5;
        //if (scale < 1.) scale = 1.;
        vertical.update(scale * scaleFactor);
        horizontal.update(scale * scaleFactor);
    }
    operator dvec2() const {
        return dvec2(horizontal.value, vertical.value);
    }
};

struct zoomerControlSuite{
    //time stamp of the last update
    uint64_t t0;
    //time between the last update and the one before (microseconds)
    uint64_t lastFrameTime;
    double fps;

    panner pos;
    dvec2 lastpos = 0;
	controlVariable<double, KeyHeld> linzoom;
    double expzoom;
    double lastexpzoom;

    
    zoomerControlSuite(TDT4102::AnimationWindow& win, double z0 = 3)
        :t0(timeMicroseconds()),lastFrameTime(10000), fps(20),
        pos(win, KeyboardKey::P, KeyboardKey::O, KeyboardKey::DOWN, KeyboardKey::LEFT, KeyboardKey::UP, KeyboardKey::RIGHT),
        linzoom(win, KeyboardKey::K, KeyboardKey::L, z0), 
        expzoom(exp(z0))
    {}
    void update(){
        auto thisTime = timeMicroseconds();
        double fpeffc = double(thisTime - t0)/1000000.;
        lastFrameTime = thisTime - t0;
        fps = fps*(1-fpeffc) + 1000000./double(lastFrameTime)*fpeffc;
        t0 = thisTime;

        lastpos = pos;
        lastexpzoom = expzoom;

        linzoom.update(pos.scale/300);
		expzoom = exp(linzoom.value);
		pos.update(expzoom);
    }
    dvec2 coord() const{
        return pos.operator dvec2();
    }
    double ctrlAmp() const{
        return expzoom*pos.scale;
    }
    dvec2 coordChange() const{
        return coord()-lastpos;
    }
    double zoomRatio() const{
        return expzoom/lastexpzoom;
    }
};


struct controlRegister{
    bool mouseHeld=0;
    bool click=0;
    vec2 mousepos = {0,0};
};

struct tablet{
    screen footprint;
    controlRegister controls;
    std::function<void(TDT4102::AnimationWindow& win, const UniformTransform<float>& trans)> draw;

    tablet(screen foot, std::function<void(TDT4102::AnimationWindow& win, const UniformTransform<float>& trans)> drw = 0)
        :footprint(foot), controls(), draw(drw) {}
};

static uint64_t ZEROES[] = {0,0,0,0,0,0,0,0};
struct socket{
    vec2 pos;
    bool sending = 0; //0 for recieving, 1 for sending
    void* source = ZEROES;
    uint connection;
    //if sending, connections are not interpreted. Source is a pointer
    //to a variable with the information being sent.
    //if recieving, source is set equal to the source of the sender
    //and connection to the index of the sender.
    //Note that senders are not altered by connecting to new recievers
    //connections are drawn from reciever to sender
    //you cannot disconnect a reciever from a sender
};


class DrawableEnvironment{
private:
    //the AnimationWindow class does not obay const function qualification rules
    mutable TDT4102::AnimationWindow win;

    ScreenMap wToScreen;

    zoomerControlSuite eye;

public:
    //screen worldscreen;
    //screen screenscreen;
    //UniformTransform<float> toScreen;

    DrawableEnvironment(uvec2 dims = {512, 512}):
        win(50, 50, int(dims.x), int(dims.y)), 
        wToScreen(screen({0,0},{100,100}), screen({0,0},{100,100})), eye(win, 0.){}
    uvec2 getDims() const{
        return {uint(win.width()), uint(win.height())};
    }
    
    TDT4102::AnimationWindow& getwin(){
        return win;
    }

    void updatePanner(){
        eye.update();
        wToScreen.to = {{0,0},getDims()};
        wToScreen.from = centeredQuad<float>(eye.coord(), vec2(getDims())*float(eye.expzoom)*(1.f/float(getDims().x))*200.f);
        wToScreen.update();
    }

    void drawGrid(float scale, vec3 col = {0.5,0.5,0.5}){
        vec2 bml = wToScreen*(vec2(wToScreen.from.lower.x-fmod(wToScreen.from.lower.x, scale),wToScreen.from.lower.y-fmod(wToScreen.from.lower.y, scale)));
        vec2 ssc = vec2(scale, scale)*wToScreen.scale;
        for(float dd = bml.x; dd < wToScreen.to.higher.x; dd+= ssc.x)
            win.draw_line({int(dd), 0}, {int(dd), int(wToScreen.to.higher.y)}, col);

        for(float dd = bml.y; dd < wToScreen.to.higher.y; dd+= ssc.y)
            win.draw_line({0, int(dd)}, {int(wToScreen.to.higher.x), int(dd)}, col);
    }


    void control(){
        updatePanner();
    }

    void render(){
        win.draw_circle(wToScreen*vec2{0,0}, int(10.*wToScreen.scale.x));
        drawGrid(20);
        win.next_frame();
    }

    vec2 getWorldMousePos() const{
        return wToScreen.antitransform(win.get_mouse_coordinates());
    }
    
    double getfps() const{
        return eye.fps;
    }
    uint64_t getFrameTime() const{
        return eye.lastFrameTime;
    }
    uint64_t getCurrFrameTime(){
        return timeMicroseconds()-eye.t0;
    }
    
};