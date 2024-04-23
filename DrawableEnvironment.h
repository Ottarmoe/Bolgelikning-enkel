#pragma once

#include "AnimationWindow.h"
#include "widgets/TextInput.h"
#include "widgets/Button.h"
#include "myvecs.h"
#include "myrandoms.h"
#include "mytimes.h"
#include <functional>
#include <list>
#include <math.h>


typedef TDT4102::Color Color;

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
        pos(win, KeyboardKey::C, KeyboardKey::X, KeyboardKey::S, KeyboardKey::A, KeyboardKey::W, KeyboardKey::D),
        linzoom(win, KeyboardKey::LEFT_CTRL, KeyboardKey::LEFT_SHIFT, z0), 
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

        linzoom.update(pos.scale/100.);
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

class DrawableEnvironment;

struct Drawable{
    virtual void draw(DrawableEnvironment& src) = 0;
    virtual void update(DrawableEnvironment& src) = 0;
    //used by the DrawableEnvironment to keep track of what drawable is where
    std::list<Drawable*>::iterator footprint;
};



class DrawableEnvironment{
private:
    //the AnimationWindow class does not obay const function qualification rules
    mutable TDT4102::AnimationWindow win;

    zoomerControlSuite eye;

    std::list<Drawable*> drawables;

public:
    ScreenMap wToScreen;
    ivec2 lastMouse = {0,0};
    ivec2 deltaMouse = {0,0};
    bool mouseLeftClick = false;
    bool mouseLeftHeld = false;

    DrawableEnvironment(uvec2 dims = {512, 512}):
        win(50, 50, int(dims.x), int(dims.y)), 
        eye(win, 0.), wToScreen(screen({0,0},{100,100}), screen({0,0},{100,100})){}
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
        vec2 ssc = vec2(scale, scale)*wToScreen.scale;
        //dynamic change of scale
        float scal = 1;
        while(ssc.x < 20){
            scal*=2;
            ssc*=2;
        }
        scale *= scal;
        vec2 bml = wToScreen*(vec2(wToScreen.from.lower.x-fmod(wToScreen.from.lower.x, scale),wToScreen.from.lower.y-fmod(wToScreen.from.lower.y, scale)));
        for(float dd = bml.x; dd < wToScreen.to.higher.x; dd+= ssc.x)
            win.draw_line({int(dd), 0}, {int(dd), int(wToScreen.to.higher.y)}, col);

        for(float dd = bml.y; dd < wToScreen.to.higher.y; dd+= ssc.y)
            win.draw_line({0, int(dd)}, {int(wToScreen.to.higher.x), int(dd)}, col);
    }

    void drawSubScreen(const screen& foot, TDT4102::Color body = TDT4102::Color::light_gray, TDT4102::Color border = TDT4102::Color::black){
        win.draw_rectangle(foot.lower, int(foot.higher.x-foot.lower.x), int(foot.higher.y-foot.lower.y), body, border);
    }
    vec2 getMousePos() const{
        return win.get_mouse_coordinates();
    }

    vec2 getWorldMousePos() const{
        return wToScreen.antitransform(win.get_mouse_coordinates());
    }
    bool mouseRight() const{
        return win.is_right_mouse_button_down();
    }
    bool mouseLeft() const{
        return win.is_left_mouse_button_down();
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

    void bind(Drawable& drawab){
        drawables.emplace_back(&drawab);
        drawab.footprint = drawables.end()--;
    }
    void release(Drawable& drawab){
        drawables.erase(drawab.footprint);
    }

    void control(){
        //mouse stuff
        ivec2 thisMouse = win.get_mouse_coordinates();
        deltaMouse = thisMouse - lastMouse;
        lastMouse = thisMouse;
        bool mouseHeldNow = mouseLeft();
        if(mouseLeftClick) mouseLeftClick = false;
        if(!mouseLeftHeld && mouseHeldNow) mouseLeftClick = true;
        mouseLeftHeld = mouseHeldNow;
        //mouse controls
        if(mouseRight()){
            vec2 change =  vec2(deltaMouse)/wToScreen.scale;
            eye.pos.x -= change.x;
            eye.pos.y -= change.y;
        }
        eye.linzoom.value -= win.getScrollWheelMotion()*0.3;
        //general controls
        updatePanner();

        //update drawables
        for(auto dr : drawables)
            dr->update(*this);
    }

    void render(){
        win.draw_circle(wToScreen*vec2{0,0}, int(10.*wToScreen.scale.x));
        drawGrid(10);
        
        for(auto dr : drawables)
            dr->draw(*this);
        win.next_frame();
    }
    
};

struct DraggableFrame : Drawable{
    screen foot;

    bool draggable = true;
protected:
    bool boxHeld = false;
public:

    Color defaultBodyColor = Color::light_gray;
    Color bodyColor = Color::light_gray;
    Color defaultBorderColor = Color::black;
    Color borderColor = Color::black;

    DraggableFrame(screen startpos = {{0,0},{100,100}})
        :foot(startpos) {}

    virtual void update(DrawableEnvironment& src){
        borderColor = defaultBorderColor;
        bodyColor = defaultBodyColor;
        if(!draggable){
            boxHeld = false;
            return;
        }
        if(boxHeld){
            vec2 itr = vec2(src.deltaMouse)/src.wToScreen.scale;
            foot.lower += itr;
            foot.higher += itr;
            bodyColor = Color::teal;
        }
        if(src.mouseLeftClick && foot.contains(src.getWorldMousePos())){
            boxHeld = true;
        }
        if(!src.mouseLeftHeld) boxHeld = false;
    }

    virtual void draw(DrawableEnvironment& src){
        src.drawSubScreen(src.wToScreen*foot, bodyColor, borderColor);
    }
};

struct ScaleableFrame : DraggableFrame{
protected:
    bool boxScaling = false;
    int draggingVariable; //1 for left, 2 for right, 3 for up, 4 for down
public:
    float scalingLeneancy = 5; //in pixels

    ScaleableFrame(screen startpos = {{0,0},{100,100}})
        :DraggableFrame(startpos) {}

    virtual void update(DrawableEnvironment& src){
        DraggableFrame::update(src); 
        if(!draggable || boxHeld){
            return;
        }
        screen smallFoot = src.wToScreen*foot;
        screen bigFoot = {smallFoot.lower-vec2{scalingLeneancy, scalingLeneancy}, smallFoot.higher+vec2{scalingLeneancy, scalingLeneancy}};
        vec2 mousePos = src.getMousePos();
        if(bigFoot.contains(mousePos) && (!smallFoot.contains(mousePos))){
            borderColor = Color::orange;
            if(src.mouseLeftClick){
                boxScaling = true;
                if(mousePos.x <= smallFoot.lower.x) draggingVariable = 1;//left
                else if(mousePos.x >= smallFoot.higher.x) draggingVariable = 2;//right
                else if(mousePos.y <= smallFoot.lower.y) draggingVariable = 3;//up
                else draggingVariable = 4;//down
            }
        }
        if(boxScaling == true){
            if(src.mouseLeftHeld){
                vec2 itr = vec2(src.deltaMouse)/src.wToScreen.scale;
                if(draggingVariable == 1) {
                    foot.lower.x += itr.x;
                    if(foot.lower.x >= foot.higher.x) foot.lower.x -= itr.x;
                }
                else if(draggingVariable == 2) {
                    foot.higher.x += itr.x;
                    if(foot.lower.x >= foot.higher.x) foot.higher.x -= itr.x;
                }
                else if(draggingVariable == 3){
                    foot.lower.y += itr.y;
                    if(foot.lower.y >= foot.higher.y) foot.lower.y -= itr.y;
                }
                else if(draggingVariable == 4){
                    foot.higher.y += itr.y;
                    if(foot.lower.y >= foot.higher.y) foot.higher.y -= itr.y;
                }
            }
            else boxScaling = false;
        }
    }
};

struct PinableFrame : ScaleableFrame{
    float pinRadius = 2.5f;
    float pinSaturation = 0.7f;

    bool localHeld;

    //gets the mouse position as a vec2 with coordinates 0-1
    //within the bounds of the window
    vec2 localMousePosition(const DrawableEnvironment& src) const{
        return screenCast(foot, {{0,0},{1,1}})*src.getWorldMousePos();
    }

    PinableFrame(screen startpos = {{0,0},{100,100}})
        :ScaleableFrame(startpos)
    {
        draggable = false;
    }

    virtual void update(DrawableEnvironment& src){
        ScaleableFrame::update(src);
        vec2 mousePos = src.getWorldMousePos();
        vec2 pinCenter = foot.lower+vec2{pinRadius, pinRadius};
        if(src.mouseLeftClick && lensqr(pinCenter-mousePos)<=pinRadius*pinRadius*pinSaturation*pinSaturation){
            draggable = !draggable;
            boxHeld = false;
        }
        localHeld = !draggable && src.mouseLeftHeld && foot.contains(src.getWorldMousePos());
    }
    virtual void draw(DrawableEnvironment& src){
        DraggableFrame::draw(src);
        Color col = draggable? Color::black : Color::red;
        vec2 pinCenter = foot.lower+vec2{pinRadius, pinRadius};
        src.getwin().draw_circle(src.wToScreen*pinCenter, int(pinRadius*pinSaturation*src.wToScreen.scale.x), col, Color::black);
    }

};

struct grapher : PinableFrame{
    std::vector<float> graph;
    float miny, maxy;

    void resize(uint siz){
        graph.resize(siz);
    }
    grapher(uint size, float miny, float maxy, screen startpos = {{0,0},{100,100}})
        :PinableFrame(startpos),
        graph(size), miny(miny), maxy(maxy)
    {}

    template<typename T> 
    void load(std::vector<T> gra, float fetch(const T&)){
        float leap = float(gra.size())/float(graph.size());
        for(uint i = 0; i<uint(graph.size()); ++i){
            float fpos = leap*float(i);
            uint hgi = (uint)std::min(int(ceil(fpos)), int(gra.size()));
            uint lwi = (uint)std::max(int(floor(fpos)), 0);
            float lwd = fpos-float(lwi);
            float hg = fetch(gra[hgi]);
            float lw = fetch(gra[lwi]);
            graph[i] = hg*lwd + lw*(1-lwd);
        }
    }

    void draw(DrawableEnvironment& src){
        PinableFrame::draw(src);
        if(graph.size() < 2) return;

        screen graphscr({0, miny}, {float(graph.size()-1), maxy});
        ScreenMap graphTscr = src.wToScreen*ScreenMap(graphscr, foot);

        TDT4102::AnimationWindow& win = src.getwin();
        vec2 lasty = {0,graph[0]};

        for(uint i = 1; i<(uint)graph.size(); ++i){
            vec2 newy = {float(i), graph[i]};
            win.draw_line(graphTscr*lasty, graphTscr*newy, Color::red);
            lasty = newy;
        }
    }

};