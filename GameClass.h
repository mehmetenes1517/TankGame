#pragma once 
#include<iostream>
#include<array>
#include<functional>
#include<algorithm>
#include<cstdlib>
#include<cmath>
#include<memory>
#include<Gorgon/Main.h>
#include<Gorgon/Graphics.h>
#include<Gorgon/Graphics/Bitmap.h>
#include<Gorgon/Graphics/Layer.h>
#include<Gorgon/Graphics/FreeType.h>
#include<Gorgon/Graphics/Font.h>
#include<Gorgon/Widgets/Window.h>
#include<Gorgon/Widgets/Registry.h>
#include<Gorgon/Audio/Controllers.h>
#include<Gorgon/Audio.h>
#include<Gorgon/Multimedia/Wave.h>
#include"AudioManager.h"
using namespace Gorgon;
#define TO_RADIANS(X) (X)*M_PI/180.0f
struct Rect2DMesh{
    std::array<Geometry::Pointf,4> points={

            Geometry::Pointf(-1.0f,1.0f),
            Geometry::Pointf(1.0f,1.0f),
            Geometry::Pointf(1.0f,-1.0f),
            Geometry::Pointf(-1.0f,-1.0f),
        };
    std::array<Geometry::Pointf,4> texCoords{
            Geometry::Pointf(0.0f,1.0f),
            Geometry::Pointf(1.0f,1.0f),
            Geometry::Pointf(1.0f,0.0f),
            Geometry::Pointf(0.0f,0.0f)
        };
    struct{
            Geometry::Pointf UpVector{0.0f,-1.0f};
            Geometry::Pointf RightVector{1.0f,0.0f};
    }vectors;
};


struct Rectangle2D{
    uint64_t id;
    std::string tag="";
    Rect2DMesh mesh;

    struct {
        Geometry::Pointf pos={0.0f,0.0f};
        float rotation=0.0f;
        Geometry::Sizef size={100.0f,100.0f};
    }transform;
    uint32_t color=0xffffffff;
    Graphics::Bitmap* texture=NULL;
    float rot_dir=1.0f;
    
    Geometry::Pointf GetUpVector(){
        auto rotate_p=[](Geometry::Pointf p,float teta){return Geometry::Pointf(p.X*cosf((float)TO_RADIANS(teta))-p.Y*sinf((float)TO_RADIANS(teta)),p.X*sinf((float)TO_RADIANS(teta))+p.Y*cosf((float)TO_RADIANS(teta)));};
        return rotate_p(mesh.vectors.UpVector,transform.rotation);
    }
    Geometry::Pointf GetRightVector(){
        auto rotate_p=[](Geometry::Pointf p,float teta){return Geometry::Pointf(p.X*cosf((float)TO_RADIANS(teta))-p.Y*sinf((float)TO_RADIANS(teta)),p.X*sinf((float)TO_RADIANS(teta))+p.Y*cosf((float)TO_RADIANS(teta)));};
        return rotate_p(mesh.vectors.RightVector,transform.rotation);

    }
    Geometry::Rectanglef GetCollider(){
        auto topleft=Geometry::Pointf(transform.pos.X-transform.size.Width,transform.pos.Y-transform.size.Height);
        auto bottomright=topleft+Geometry::Pointf(transform.size.Width*2,transform.size.Height*2);
        return Geometry::Rectanglef(topleft,bottomright);
    }
    void LoadTexture(Graphics::Bitmap* bmp){
        texture=bmp;
    }  
    Rect2DMesh GetTransformedMesh() noexcept {
        Rect2DMesh mesh1=this->mesh;
        auto scale_p=[](Geometry::Pointf p,Geometry::Sizef scale){return Geometry::Pointf(p.X*scale.Width,p.Y*scale.Height);};
        auto rotate_p=[](Geometry::Pointf p,float teta){return Geometry::Pointf(p.X*cosf((float)TO_RADIANS(teta))-p.Y*sinf((float)TO_RADIANS(teta)),p.X*sinf((float)TO_RADIANS(teta))+p.Y*cosf((float)TO_RADIANS(teta)));};

        auto translate_p=[](Geometry::Pointf p,Geometry::Pointf delta_p){return Geometry::Pointf(p.X+delta_p.X,p.Y+delta_p.Y);};
        mesh1.vectors.UpVector=rotate_p(mesh1.vectors.UpVector,transform.rotation);
        mesh1.vectors.RightVector=rotate_p(mesh1.vectors.RightVector,transform.rotation);
        for (auto &p:mesh1.points)
        {
            p=translate_p(rotate_p(scale_p(p,transform.size),transform.rotation),transform.pos);
        }
        return mesh1;
    }
    void Draw(Graphics::Layer& layer,bool debugMode=false){
        if(!texture){throw std::runtime_error("Texture is empty!");}
        auto mesh_1=GetTransformedMesh();
        layer.Draw(*texture,mesh_1.points[0],
                                mesh_1.points[1],
                                mesh_1.points[2],
                                mesh_1.points[3],
                                mesh_1.texCoords[0],                        
                                mesh_1.texCoords[1],                        
                                mesh_1.texCoords[2],                        
                                mesh_1.texCoords[3],
                                color                      
            );
        if(debugMode){
            layer.Draw(GetCollider(),0xff00ff00);
        }

    }

};


struct Bullet{
    Rectangle2D rect;
    bool isDestroyed=false;
    float speed;
    float timer=2.0f;
    int owner;
    Bullet(const Geometry::Pointf& pos,float rotation,float speed,const Geometry::Pointf& size){
        rect.transform.pos=pos;
        rect.transform.size=size;
        rect.transform.rotation=rotation;
        this->speed=speed;
    }
    void Update(Graphics::Layer& mainLayer,float deltaTime,float bulletSpeed){
        rect.Draw(mainLayer);
        rect.transform.pos+=rect.GetUpVector()*deltaTime*bulletSpeed;
        timer-=deltaTime;
        if(timer<0.0f){
            isDestroyed=true;
        }
    }
};


enum class GameState {
    GAME_START,
    GAME_RUNNING,
    GAME_END_P1,
    GAME_END_P2,
    GAME_MAIN_MENU,
    GAME_ENDS
};
enum class Winner{
    PLAYER1,
    PLAYER2,
    NONE
};
constexpr int WIDTH=1600,HEIGHT=900;
struct Game{
    Window window;
    Graphics::Layer mainLayer;
    std::unordered_map<Input::Key,bool> inputMap;
    std::unordered_map<Input::Key,bool> inputMapOnce;
    std::unordered_map<std::string,std::unique_ptr<Graphics::Bitmap>> textureMap;
    std::unordered_map<std::string,std::unique_ptr<Multimedia::Wave>> soundMap;
    int width,height;

    GameState gameState=GameState::GAME_MAIN_MENU;
    Input::Keyboard::Key player1move=Input::Keyboard::Keycodes::W;
    Input::Keyboard::Key player1shoot=Input::Keyboard::Keycodes::Z;
    Input::Keyboard::Key player2move=Input::Keyboard::Keycodes::Up;
    Input::Keyboard::Key player2shoot=Input::Keyboard::Keycodes::T;

    Winner winner=Winner::NONE;



    void LoadTexture(const std::string& tag,const std::string& filename){
        auto ptr=std::make_unique<Graphics::Bitmap>();
        ptr->Import(filename);
        ptr->Prepare();
        textureMap[tag]=std::move(ptr);
    }

    std::vector<Rectangle2D> map_list;

    Rectangle2D player1,player2;
    float bulletspeed=500.0f;
    std::vector<std::unique_ptr<Bullet>> bullets;

    static constexpr float GAME_START_SECONDS=3.0f;
    static constexpr float GAME_END_SECONDS=2.0f;
    Graphics::FreeType font1;
    float startTimer=GAME_START_SECONDS;
    float endTimer=GAME_END_SECONDS;
    float endTimer1=GAME_END_SECONDS;
    std::string endstring="";
    int p1_score=0,p2_score=0;

    struct {
        Graphics::FreeType bigfont;
        std::vector<Rectangle2D> map_list;
        uint32_t selected_item=0;
    }ui_vars;
    AudioManager& audm;
    Game(int w,int h,const std::string& title):window(Gorgon::Window::Fullscreen,title,title),audm(AudioManager::Get()){
        font1.LoadFile("./Assets/Nasa.ttf",36);
        ui_vars.bigfont.LoadFile("./Assets/Nasa.ttf",48);
        if(!font1.IsReady()){throw std::runtime_error("font is not ready!");}
        if(!ui_vars.bigfont.IsReady()){throw std::runtime_error("font is not ready!");}
        audm.LoadSound("bg","Assets/Sounds/gameplay.wav");
        audm.LoadSound("gun","Assets/Sounds/gun.wav");
        audm.LoadSound("explosion","Assets/Sounds/Impact.wav");
        audm.LoadSound("ring","Assets/Sounds/ring.wav");
        audm.LoadSound("ui_beep","Assets/Sounds/ui_beep.wav"); 
        

        bullets.reserve(1024);
        width=window.GetSize().Width;
        height=window.GetSize().Height;
        LoadTexture("brick","Assets/brick.jpg");
        LoadTexture("tank","Assets/tank.png");
        LoadTexture("bullet","Assets/bullet.png");
        InitGame(width,height,title);
    }

    void InitGame(int w,int h,const std::string& title){

        InitWindow(width,height,title);
        InitPlayer(player1,{400.0f,300.0f},0xff00ff00);
        player1.tag="player";
        player1.id=1;
        InitPlayer(player2,{700.0f,300.0f},0xff0000ff);
        player2.tag="player";
        player2.id=2;
        InitMap();
        InitUIMap();
    }
    void InitUIMap(){
        const int map[9][16]={
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
            };
        float cubeWidth=width/16;
        float cubeheight=height/9;
        map_list.reserve(9*16);
        for (size_t i = 0; i < 9; i++)
        {
            for (size_t j = 0; j < 16; j++){
                if(map[i][j]==1){

                    Rectangle2D rect1;
                    rect1.transform.pos.X=cubeWidth/2 + j*cubeWidth;
                    rect1.transform.pos.Y=cubeheight/2 + i*cubeheight;
                    rect1.transform.size.Width=cubeWidth/2;
                    rect1.transform.size.Height=cubeheight/2;
                    rect1.LoadTexture(textureMap["brick"].get());
                    ui_vars.map_list.push_back(rect1);
                }
            }
        }
        
    }
    bool isOpenedFirst=true;
    void UpdateGame(float deltaTime){
        mainLayer.Clear();
        mainLayer.Draw(0xff000000);
        
        switch (gameState)
        {
            case GameState::GAME_START:
                BackwardCounting(deltaTime);
                break;
            case GameState::GAME_RUNNING:
                if(isOpenedFirst){
                    audm.PlaySound("bg",1.0,true);
                    isOpenedFirst=false;
                }
                HandlePlayerInputs(deltaTime,135.0f,200.0f);     
                HandlePlayerCollisions(deltaTime,player1,player2,[&](){
                    gameState=GameState::GAME_END_P1;
                    endstring=std::string("Player 1 Wins !");
                    p1_score++;
                    startTimer=GAME_START_SECONDS;
                    audm.PlaySound("explosion",1.0f);
                    endTimer=GAME_END_SECONDS;
                });   
                HandlePlayerCollisions(deltaTime,player2,player1,[&](){
                    gameState=GameState::GAME_END_P2;
                    endstring=std::string("Player 2 Wins !");
                    p2_score++;
                    audm.PlaySound("explosion",1.0f);
                    startTimer=GAME_START_SECONDS;
                    endTimer=GAME_END_SECONDS;
                });   
                UpdateBullets(deltaTime);
                HandleMap(deltaTime);
                HandleScoreTable(deltaTime);
                break;
            case GameState::GAME_END_P1:
                HandleScoreTable(deltaTime);
                font1.Print(mainLayer,endstring,Geometry::Pointf(width/2-font1.GetSize(endstring).Width/2,height/2-font1.GetSize(endstring).Height/2),(Gorgon::Graphics::RGBAf)player1.color);
                endTimer-=deltaTime;
                if(endTimer<0.0f){
                    gameState=GameState::GAME_START;
                    bullets.clear();
                    map_list.clear();
                    if (p1_score>=5 or p2_score>=5){

                        if(p1_score>=5){
                            winner=Winner::PLAYER1;
                        }
                        else if(p2_score>=5){
                            winner=Winner::PLAYER2;
                        }
                        gameState=GameState::GAME_ENDS;
                    }
                    endTimer=GAME_END_SECONDS;
                }
                break;
            case GameState::GAME_END_P2:   
                HandleScoreTable(deltaTime);
                std::cout<<"\n"<<player2.color;
                font1.Print(mainLayer,endstring,Geometry::Point(width/2-font1.GetSize(endstring).Width/2,height/2-font1.GetSize(endstring).Height/2),(Gorgon::Graphics::RGBAf)player2.color);
                endTimer-=deltaTime;
                if(endTimer<0.0f){
                    gameState=GameState::GAME_START;
                    bullets.clear();
                    map_list.clear();
                    if (p1_score>=5 or p2_score>=5){

                        if(p1_score>=5){
                            winner=Winner::PLAYER1;
                        }
                        else if(p2_score>=5){
                            winner=Winner::PLAYER2;
                        }
                        gameState=GameState::GAME_ENDS;
                    }
                    endTimer=GAME_END_SECONDS;
                }
                break;
            case GameState::GAME_MAIN_MENU:
                HandleUIMainmenu(deltaTime);
                break;
            case GameState::GAME_ENDS:
                endTimer1-=deltaTime;

                HandleGameEndUI(deltaTime);

                if(endTimer1<0.0f){
                    gameState=GameState::GAME_MAIN_MENU;
                    endTimer1=GAME_END_SECONDS;
                }
                break;
        }
        
    }
    void HandleGameEndUI(float deltaTime){
        auto str=std::string("");
        uint32_t color=0xffffffff;
        switch (winner)
        {
        case Winner::PLAYER1:
            str="| Player 1 Won! |";
            color=player1.color;
            break;
        case Winner::PLAYER2:
            str="| Player 2 Won! |";
        color=player2.color;
            break;    
        default:
            str="| Draw |";
            break;
        }
        ui_vars.bigfont.Print(mainLayer,str,Geometry::Pointf(width/2-ui_vars.bigfont.GetSize(str).Width/2,height/2-ui_vars.bigfont.GetSize(str).Height/2),(Graphics::RGBAf)color);
    }
    void ClearInputMap(){
        inputMapOnce.clear();
    }
    float t=1.0f;
    private:
        void InitWindow(int w,int h,const std::string& title){
            window.Add(mainLayer);
            window.KeyEvent.Register([&](Input::Key key,float pressed){
                inputMap[key]=  pressed>=1.0f ;
                inputMapOnce[key]=  pressed>=1.0f ;
                return true;
            });
        }
        void InitPlayer(Rectangle2D& player,const Geometry::Pointf& position,uint32_t color){
            player.transform.pos=position;
            player.transform.size={20.0f,20.0f};
            player.color=color;
            player.LoadTexture(textureMap["tank"].get());
        }
        void HandlePlayerInputs(float deltaTime,float rotation_speed=45.0f,float movement_speed=170.0f){
            player1.Draw(mainLayer),
            player2.Draw(mainLayer);
            namespace Keycodes=Gorgon::Input::Keyboard::Keycodes;
            if(inputMapOnce[player1move]){player1.rot_dir*=-1;}
            if(inputMap[player1move]){
                player1.transform.pos+=player1.GetUpVector()*movement_speed*deltaTime;
            }else{
                player1.transform.rotation+=deltaTime*player1.rot_dir*rotation_speed;
            }
            
            if(inputMapOnce[player2move]){
                player2.rot_dir*=-1;
            }if(inputMap[player2move]){
                player2.transform.pos+=player2.GetUpVector()*movement_speed*deltaTime;
            }else{
                player2.transform.rotation+=deltaTime*player2.rot_dir*rotation_speed;
            }
            Geometry::Pointf bulletSize={5.0f,10.0f};
            if(inputMapOnce[player1shoot]){
                std::unique_ptr<Bullet> ptr(std::move(std::make_unique<Bullet>(player1.transform.pos,player1.transform.rotation,bulletspeed,bulletSize)));
                ptr->rect.LoadTexture(textureMap["bullet"].get());
                ptr->owner=1;
                bullets.push_back(std::move(ptr));
                audm.PlaySound("gun",1.0,false);
            }
            if(inputMapOnce[player2shoot]){
                std::unique_ptr<Bullet> ptr(std::move(std::make_unique<Bullet>(player2.transform.pos,player2.transform.rotation,bulletspeed,bulletSize)));
                ptr->rect.LoadTexture(textureMap["bullet"].get());
                ptr->owner=2;
                bullets.push_back(std::move(ptr));
                audm.PlaySound("gun",1.0,false);
            }
            
        }
        void UpdateBullets(float deltaTime){
            for (auto& b:bullets)
            {
                b->Update(mainLayer,deltaTime,bulletspeed);

            }
            bullets.erase(
                std::remove_if(bullets.begin(), bullets.end(), [](const auto& bullet) {
                    return bullet->isDestroyed;
                }),
                bullets.end()
            );
        }

        bool CheckCollision(const Geometry::Rectanglef& rect1 , const Geometry::Rectanglef& rect2){
            if(rect1.X+rect1.Width > rect2.X&&
                rect2.X+rect2.Width > rect1.X&&
                rect1.Y+rect1.Height > rect2.Y&&
                rect2.Y+rect2.Height > rect1.Y){return true;}
            return false;
        }
        
        void HandlePlayerCollisions(float deltaTime,Rectangle2D& player,Rectangle2D& otherplayer,std::function<void()> func=[](){}){
            for (auto& b : bullets )
            {
                if(otherplayer.id!=b->owner && CheckCollision(otherplayer.GetCollider(),b->rect.GetCollider())){
                    func();
                }
            }
            
        }
        void BackwardCounting(float deltaTime){
            auto str=std::to_string(static_cast<int>(startTimer));
            font1.Print(mainLayer,std::to_string(static_cast<int>(startTimer)),Geometry::Pointf(width/2-font1.GetSize(str).Width/2,height/2-font1.GetSize(str).Height/2),0xff00ff00);

            
            startTimer-=deltaTime;
            t+=deltaTime;
            if(t>=1.0f){
                audm.PlaySound("ring",1.0);
                t=0.0f;
            }
            if(startTimer<0.0f){
                gameState=GameState::GAME_RUNNING;
                InitMap();
                t=1.0f;
                startTimer=GAME_START_SECONDS;
            }
        }

        void HandleScoreTable(float deltaTime){
            auto str=std::string(std::to_string(p1_score)+" | "+std::to_string(p2_score));
            font1.Print(mainLayer,str,Geometry::Pointf(width/2 - font1.GetSize(str).Width/2,3.0f),(Gorgon::Graphics::RGBAf)0xffffffff);
        }
        void HandleMap(float deltaTime){
            for(auto& b:map_list){
                b.Draw(mainLayer);
                HandleMapCollisions(b,deltaTime);
            }
        }
        void HandleMapCollisions(Rectangle2D& map_cell,float deltaTime){
            
                for (auto& b:bullets)
                {
                    if(CheckCollision(map_cell.GetCollider(),b->rect.GetCollider())){
                        b->isDestroyed=true;
                    }                    
                }
            
                auto resolvePlayerCollision = [&](auto& player) {
                    auto pCol = player.GetCollider(); 
                    auto mCol = map_cell.GetCollider();

                    if (CheckCollision(pCol, mCol)) {
                        
                        float pCenterX = pCol.X + (pCol.Width / 2.0f);
                        float pCenterY = pCol.Y + (pCol.Height / 2.0f);
                        
                        float mCenterX = mCol.X + (mCol.Width / 2.0f);
                        float mCenterY = mCol.Y + (mCol.Height / 2.0f);

                        float diffX = pCenterX - mCenterX;
                        float diffY = pCenterY - mCenterY;

                        float minXDist = (pCol.Width + mCol.Width) / 2.0f;
                        float minYDist = (pCol.Height + mCol.Height) / 2.0f;

                        float overlapX = minXDist - std::abs(diffX);
                        float overlapY = minYDist - std::abs(diffY);
                        if (overlapX < overlapY) {
                            float pushX = (diffX > 0 ? overlapX : -overlapX) / 2.0f;
                            player.transform.pos.X += pushX; 
                        } else {
                            float pushY = (diffY > 0 ? overlapY : -overlapY) / 2.0f;
                            player.transform.pos.Y += pushY; 
                        }
                    }
                };

                resolvePlayerCollision(player1);
                resolvePlayerCollision(player2);
                
        }
        void InitMap(){
            const int map[9][16]={
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,0,1,1,0,0,0,0,0,0,0,0,1,1,0,1},
                {1,1,0,0,0,1,0,0,1,0,0,4,0,0,1,1},
                {1,0,3,0,1,1,0,1,1,1,0,0,1,0,0,1},
                {1,0,0,1,1,0,0,0,1,0,0,1,1,1,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
                {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
                {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
            };
            float cubeWidth=width/16;
            float cubeheight=height/9;
            map_list.reserve(9*16);
            for (size_t i = 0; i < 9; i++)
            {
                for (size_t j = 0; j < 16; j++){
                    if(map[i][j]==1){
                        Rectangle2D rect;
                        rect.transform.pos.X=cubeWidth/2+j*cubeWidth;
                        rect.transform.pos.Y=cubeheight/2+i*cubeheight;
                        rect.transform.size={cubeWidth/2,cubeheight/2};
                        rect.transform.rotation=1.0f;
                        rect.color=0xffdddddd;
                        rect.LoadTexture(textureMap["brick"].get());
                        map_list.push_back(rect);
                    }
                    else if(map[i][j]==3){
                        player1.transform.pos={cubeWidth/2+j*cubeWidth,cubeheight/2+i*cubeheight};
                    }
                    else if(map[i][j]==4){
                        player2.transform.pos={cubeWidth/2+j*cubeWidth,cubeheight/2+i*cubeheight};
                    }
                }
            }
            
        }
        void HandleUIMainmenu(float deltaTime){
            auto play=std::string("Play");
            auto loc1=Geometry::Pointf(width/2-ui_vars.bigfont.GetSize(play).Width/2,height/2-ui_vars.bigfont.GetSize(play).Height);
            auto quit=std::string("Quit");
            auto loc2=Geometry::Pointf(width/2-ui_vars.bigfont.GetSize(quit).Width/2,height/2);
            
            ui_vars.bigfont.Print(mainLayer,play,loc1,ui_vars.selected_item ==0 ? (Graphics::RGBAf)0xff0000ff : (Graphics::RGBAf)0xffffffff );
            ui_vars.bigfont.Print(mainLayer,quit,loc2,ui_vars.selected_item ==1 ? (Graphics::RGBAf)0xff0000ff : (Graphics::RGBAf)0xffffffff );
            auto percent= float(width)*0.20;

            player1.transform.pos.X=percent;
            player1.transform.pos.Y=height/2-player1.transform.size.Height/2;
            player2.transform.pos.X=width-percent-player2.transform.size.Width;
            player2.transform.pos.Y=height/2-player2.transform.size.Height/2;
            
            player1.transform.rotation+=220.0f*deltaTime;
            player2.transform.rotation-=220.0f*deltaTime;
            player1.Draw(mainLayer);
            player2.Draw(mainLayer);
            for (auto& cell: ui_vars.map_list)
            {
                cell.Draw(mainLayer);
            }
            if(inputMapOnce[player1move] || inputMapOnce[player2move]){
                ui_vars.selected_item=(ui_vars.selected_item+1)%2;
                audm.PlaySound("ui_beep",1.0);
            }
            if(inputMapOnce[player1shoot] || inputMapOnce[player2shoot]){
                switch (ui_vars.selected_item)
                {
                case 0:
                    gameState=GameState::GAME_START;
                    break;
                default:
                    exit(0);
                    break;
                }
            }


        }
        
};