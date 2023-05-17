#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "raylib.h"

using namespace std;
const int       LARGEUR_ECRAN = 800;
const int       HAUTEUR_ECRAN = 450;
const int       briqueLargeur = 100;
const int       briqueHauteur = 32; 
const int       briqueSeparateur = 2;
const int       VIE_DEPART_CONST = 1;
int             VIE_DEPART = VIE_DEPART_CONST;
int             vitesseRaquette = 4;
int             vitesseBalleX = 4;
int             vitesseBalleY = -4;
int             balleTaille = 32;
int             raquetteLargeur = 32;
int             raquetteLongueur = 128;
int             raquetteY = 0;
Rectangle       raquette{float(LARGEUR_ECRAN / 2 - raquetteLongueur / 2), float(HAUTEUR_ECRAN - raquetteLargeur), (float)raquetteLongueur, (float)raquetteLongueur};
Rectangle       balle{float(raquette.x + raquetteLongueur / 2), float(raquette.y - balleTaille - 1), (float)balleTaille, (float)balleTaille};
Rectangle       rect{0, 0, (float)briqueLargeur, (float)briqueHauteur};
struct          Brique {Rectangle rect; bool visible;};
Brique          brique{rect};
const int       briqueParLignes = LARGEUR_ECRAN / briqueLargeur;
const int       briqueParColonnes = (HAUTEUR_ECRAN / 2) / briqueHauteur;
vector<Brique>  murBriques;
string          texteResultat = "";
bool            resultatVisible = false;
int             etatDeJeu = 0;
int             nbBriques = briqueParColonnes * briqueParLignes;
Sound           sonBrique;
Sound           sonRaquette;
Sound           sonDie;

void        draw();
void        update();
void        load();
void        unload();
bool        collision(Rectangle A, Rectangle B);
void        rebondSurRaquette(bool cotejoueur);
void        afficherUneBrique(int x, int y);
void        afficherToutesLesBriques(vector<Brique> murBriques);
void        CreerToutesLesBriques();
void        relancerBalle();

int main ()
{
    load();
    while(!WindowShouldClose()){
        update();
        draw();
    }
    unload();
    return 0;
}

void    draw(){
    BeginDrawing();
    ClearBackground(BLACK);
    DrawRectangle(balle.x, balle.y, balle.width, balle.height, WHITE);
    string vie = "Vie : " + to_string(VIE_DEPART);
    DrawText(vie.c_str(), 700, 370, 20, RED);
    DrawRectangleRec(raquette, WHITE);
    afficherToutesLesBriques(murBriques);
    if (resultatVisible)
        DrawText(texteResultat.c_str(), LARGEUR_ECRAN / 2 - 200, HAUTEUR_ECRAN / 2 - 13, 100, RED);
    EndDrawing();
}

void    update()
{
    if (etatDeJeu == 0)
    {
        //Faire bouger la balle
        balle.x += vitesseBalleX;
        balle.y -= vitesseBalleY;
            
        //Faire bouger la raquette
        if (IsKeyDown(KEY_RIGHT))
            raquette.x += vitesseRaquette;
        else if(IsKeyDown(KEY_LEFT))
            raquette.x -= vitesseRaquette;
        
        //Empecher la raquette de sortir
        if (raquette.x < 0)
            raquette.x = 0;
        if (raquette.x > LARGEUR_ECRAN - raquetteLongueur)
            raquette.x = LARGEUR_ECRAN - raquetteLongueur;
        
        //Empecher la balle de sortir
        if (balle.x < 0)
        {
            balle.x = 0;
            vitesseBalleX = -vitesseBalleX;
        }
        if (balle.x > LARGEUR_ECRAN - balleTaille)
        {
            balle.x = LARGEUR_ECRAN - balleTaille;
            vitesseBalleX = -vitesseBalleX;
        }
        if (balle.y < 0)
        {
            balle.y = 0;
            vitesseBalleY = -vitesseBalleY;
        }
        
        // le joueur a perdu
        if (balle.y > HAUTEUR_ECRAN - balleTaille)
        {
            balle.y = HAUTEUR_ECRAN - balleTaille;
            VIE_DEPART -= 1;
            relancerBalle();
            if (VIE_DEPART == 0)
            {
                PlaySound(sonDie);
                texteResultat = "Defaite";
                resultatVisible = true;
                etatDeJeu = 2;
            }
            vitesseBalleY = -vitesseBalleY;
        }
        
        //Collision balle et raquette joueur
        if (collision(balle, raquette))
        {
            vitesseBalleY = -vitesseBalleY;
            balle.y = raquette.y - balleTaille;
            PlaySound(sonRaquette);
        }
        
        // Collision balle et brique
        for(Brique &briqueT : murBriques)
        {
            if(briqueT.visible == true)
                if (collision(briqueT.rect, balle))
                {
                    vitesseBalleY = -vitesseBalleY;
                    balle.y = briqueT.rect.y + briqueHauteur;
                    nbBriques -= 1;
                    PlaySound(sonBrique);
                    if (nbBriques == 0)
                    {
                        texteResultat = "Victoire";
                        resultatVisible = true;
                        etatDeJeu = 1;
                    }
                    briqueT.visible = false;
                }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_R))
        {
            texteResultat = "";
            nbBriques = briqueParColonnes * briqueParLignes;;
            VIE_DEPART = VIE_DEPART_CONST;
            murBriques.clear();
            CreerToutesLesBriques();
            afficherToutesLesBriques(murBriques);
            etatDeJeu = 0;
        }
    }
}

void    load(){
    InitWindow(800, 450, "Interaction");
    SetTargetFPS(60);
    CreerToutesLesBriques();
    
     InitAudioDevice();
    sonBrique = LoadSound("son/brique.wav");
    sonRaquette = LoadSound("son/raquette.wav");
    sonDie = LoadSound("son/die.wav");;
}

void unload(){
    CloseWindow();
}

bool collision(Rectangle A, Rectangle B)
{
    int         xMinA = A.x;
    int         yMinA = A.y;
    int         xMaxA = A.x + A.width;
    int         yMaxA = A.y + A.height;
    int         xMinB = B.x;
    int         yMinB = B.y;
    int         xMaxB = B.x + B.width;
    int         yMaxB = B.y + B.height;
    
    //Test de collision
    if (!(xMinB > xMaxA || yMinB > yMaxA 
        || xMaxB < xMinA || yMaxB < yMinA))
        return true;
    else
        return false;
}

void    rebondSurRaquette(bool cotejoueur)
{
    vitesseBalleY = -vitesseBalleY;
    if (cotejoueur)
        balle.y = raquette.y + balleTaille;
    else
        balle.y = raquette.y - balleTaille;
}

void    afficherUneBrique(int x, int y)
{
    brique.rect.x = x;
    brique.rect.y = y;
    DrawRectangle(brique.rect.x, brique.rect.y, briqueLargeur  - briqueSeparateur, briqueHauteur - briqueSeparateur, WHITE);
}

void   CreerToutesLesBriques()
{
    int x = 0;
    int y = -briqueHauteur;
    
    for(int i = 0; i < briqueParColonnes; i++)
    {
        y += briqueHauteur;
        x = 0;
        for(int j = 0; j < briqueParLignes; j++)
        {
            Brique briqueB;
            Rectangle rectB{(float)x, (float)y, (float)briqueLargeur, (float)briqueHauteur};
            briqueB.visible = true;
            briqueB.rect = rectB;
            murBriques.push_back(briqueB);
            x += briqueLargeur;
        }
    }
}

void   afficherToutesLesBriques(vector<Brique> murBriques)
{   
    for(Brique briqueC : murBriques)
    {
        if (briqueC.visible == true)
            afficherUneBrique(briqueC.rect.x, briqueC.rect.y);
    }
}


void    relancerBalle()
{
    balle.x = raquette.x + raquetteLongueur / 2;
    balle.y = raquette.y - balleTaille - 1;
}