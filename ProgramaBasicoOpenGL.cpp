//-------------------------------------
// Exercício: Space Invaders - CG - 2020/1
// Dupla: Larissa Magistrali e Nikolas Lacerda
//-------------------------------------
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;
#ifdef WIN32
#include <windows.h>
#include <glut.h>
static DWORD last_idle_time;
#else
#include <sys/time.h>
static struct timeval last_idle_time;
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#endif
#ifdef __linux__
#include <glut.h>
#endif
#include "ImageClass.h"
#include "SOIL/SOIL.h"
#include <fstream>
#include <dirent.h>
#include <string>
#include <time.h>
#include <vector>
//------------------------JOGO---------------------------------

//animate
float dt;
double AccumDeltaT=0;
double fps;
double velocidade=1;
double tamanhoRestante;

void animate()
{
    static float AccumTime=0;

#ifdef _WIN32
    DWORD time_now;
    time_now = GetTickCount();
    dt = (float) (time_now - last_idle_time) / 1000.0;
#else
    // Figure out time elapsed since last call to idle function
    struct timeval time_now;
    gettimeofday(&time_now, NULL);
    dt = (float)(time_now.tv_sec  - last_idle_time.tv_sec) +
         1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec);
#endif
    AccumTime +=dt;
    if (AccumTime >=3) // imprime o FPS a cada 3 segundos
    {
        cout << 1.0/dt << " FPS"<< endl;
        AccumTime = 0;
    }
    last_idle_time = time_now;
    glutPostRedisplay();
}

//----Definicao das structs----

//Cor
typedef struct{
    float r, g, b;
} Cor;

//Modelo de Objeto
typedef struct{
    int altura;
    int largura;
    Cor cores[100][100];
} ModeloDeObjeto;

//Instancia de um objeto
typedef struct{
    ModeloDeObjeto modelo;
    float x; //x,y posicao do objeto no universo
    float y;
    float veloc;
} Instancia;

//-----Variaveis-----

//tamanho glortho
int glOrthoX=250, glOrthoY=125;

//vidas
int numeroDeVidas=3;
ImageClass numVidas;

//Plano de fundo
ImageClass Image;

//Modelos
ModeloDeObjeto modeloProjetil, modeloDisparador, modeloNaveAzul, modeloNaveRoxa, modeloNaveVermelha, modeloNaveRosa;

//Instancias
Instancia instanciaDisparador, instanciaNave1,instanciaNave2,instanciaNave3,instanciaNave4,instanciaNave5,instanciaNave6,instanciaNave7,instanciaNave8;
vector <Instancia> Naves;
vector <Instancia> Disparos;
vector <Instancia> DisparosNave;

//objetos
void LeArquivoModelo(ModeloDeObjeto &modelo, char *path){
    printf("LeArquivoModelo() \n");
    std::ifstream infile(path);
    int numeroDeCores, indiceCor = 0;
    infile >> numeroDeCores;
    Cor ListaCores[numeroDeCores] = {};
    while (indiceCor < numeroDeCores){
        Cor c;
        infile >> indiceCor >> c.r >> c.g >> c.b ;
        ListaCores[indiceCor - 1] = c;
    }
    int linhas, colunas,i, j, cor = 0;
    infile >> linhas >> colunas;
    modelo.altura = colunas;
    modelo.largura = linhas;
    Cor Matriz[linhas][colunas] = {};
    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++){
            infile >> cor;
            Matriz[i][j] = ListaCores[cor - 1];
        }
    }
    for (i = 0; i < linhas; i++){
        for (j = 0; j < colunas; j++) {
            modelo.cores[i][j] = Matriz[i][j];
        }
    }
    infile.close();
}

void CriaInstancia(Instancia &i, ModeloDeObjeto &m, float x1, float y1, float v){
    //printf("CriaInstancia() \n");
    i.modelo=m;
    i.x=x1;
    i.y=y1;
    i.veloc=v;
}

void DesenhaIntanciaDeModelo(Instancia &inst){
    //printf("DesenhaInstanciaDeModelo() \n");
    glPushMatrix();
        glLoadIdentity();
        glTranslatef(inst.x, inst.y, 0);
        float dx = -inst.modelo.largura / 4., dy = inst.modelo.altura / 2. - 0.5; //nao entendi pq isso
        int i, j;
        for (i = 0; i < inst.modelo.largura; i++) {
            for (j = 0; j < inst.modelo.altura; j++){
                if(inst.modelo.cores[i][j].r==0 && inst.modelo.cores[i][j].b==0 && inst.modelo.cores[i][j].g==0){}
                else{
                    glBegin(GL_QUADS);
                    glColor3ub(inst.modelo.cores[i][j].r, inst.modelo.cores[i][j].g, inst.modelo.cores[i][j].b);
                    glVertex2f(dx, dy);
                    glVertex2f(dx + 0.5 ,dy);
                    glVertex2f(dx + 0.5, dy + 0.5);
                    glVertex2f(dx, dy + 0.5);
                    glEnd();
                }
                dx += 0.5;
            }
            dy -= 0.5;
            dx = -inst.modelo.largura / 4.;
        }
    glPopMatrix();
}

void CarregaModelos(){
    LeArquivoModelo(modeloDisparador,"./JogoArquivos/Disparador.txt");
    LeArquivoModelo(modeloProjetil,"./JogoArquivos/Projetil.txt");
    LeArquivoModelo(modeloNaveAzul,"./JogoArquivos/NaveAzul.txt");
    LeArquivoModelo(modeloNaveRosa,"./JogoArquivos/NaveRosa.txt");
    LeArquivoModelo(modeloNaveRoxa,"./JogoArquivos/NaveRoxa.txt");
    LeArquivoModelo(modeloNaveVermelha,"./JogoArquivos/NaveVermelha.txt");
}

void CriaInstanciasDeNaves(){
    srand(time(NULL));
    CriaInstancia(instanciaNave1,modeloNaveAzul,rand() % glOrthoX-10,glOrthoY+10,5); //velocidade em segundos
    CriaInstancia(instanciaNave2,modeloNaveAzul,rand() % glOrthoX-10,glOrthoY+10,5);
    CriaInstancia(instanciaNave3,modeloNaveVermelha,rand() % glOrthoX-10,glOrthoY+10,7);
    CriaInstancia(instanciaNave4,modeloNaveVermelha,rand() % glOrthoX-10,glOrthoY+10,8);
    CriaInstancia(instanciaNave5,modeloNaveRoxa,rand() % glOrthoX-10,glOrthoY+10,9);
    CriaInstancia(instanciaNave6,modeloNaveRoxa,rand() % glOrthoX-10,glOrthoY+10,11);
    CriaInstancia(instanciaNave7,modeloNaveRosa,rand() % glOrthoX-10,glOrthoY+10,13);
    CriaInstancia(instanciaNave8,modeloNaveRosa,rand() % glOrthoX-10,glOrthoY+10,15);

    Naves.push_back(instanciaNave1);
    Naves.push_back(instanciaNave2);
    Naves.push_back(instanciaNave3);
    Naves.push_back(instanciaNave4);
    Naves.push_back(instanciaNave5);
    Naves.push_back(instanciaNave6);
    Naves.push_back(instanciaNave7);
    Naves.push_back(instanciaNave8);
}


//disparos
void Dispara(){
    //printf("Dispara() \n");
    Instancia i;
    CriaInstancia(i,modeloProjetil,instanciaDisparador.x-0.5,instanciaDisparador.y+4.5, 0.3);
    Disparos.push_back(i);
}

void naveDispara(Instancia nave){
    //Instancia i;
    //CriaInstancia(i,modeloProjetil, (nave.x + 2) , nave.y + 3, 0.3); //x,y,veloc aleatorios
    //DisparosNave.push_back(i);
}


//colisao
bool VerificaColisao(Instancia &nave, Instancia &inst2){
    float xCnave = (nave.x - 1);
    float xC2 = inst2.x;
    float yCnave = (nave.y + 6 + 0.5);
    float yC2 = inst2.y;

    if (xC2 <= xCnave + nave.modelo.largura &&
        xC2 + inst2.modelo.largura > xCnave &&
        yC2 <= yCnave + nave.modelo.altura / 2 &&
        yC2 + inst2.modelo.altura / 2 > yCnave) {
            return true;
    }
    return false;
}

bool VerificaColisaoNaveDisparador(Instancia &nave, Instancia &disparador){
    float xCnave = (nave.x + 1);
    float xCdisp = disparador.x;
    float yCnave = (nave.y + 6 + 0.5);
    float yCdisp = disparador.y + 2;

    if (xCdisp < xCnave + nave.modelo.largura &&
        xCdisp + disparador.modelo.largura / 2 > xCnave &&
        yCdisp < yCnave + nave.modelo.altura / 2 &&
        yCdisp + disparador.modelo.altura / 2 >= yCnave) {
            return true;
    }
    return false;
}


//chão
void DesenhaChao(){
    glPushMatrix();
        glLoadIdentity();
        glColor3f(0.7,0,0.3);
        glTranslatef(0, 1, 0);
        glBegin(GL_QUADS);
            glVertex2f(glOrthoX,0);
            glVertex2f(0,0);
            glVertex2f(0,2);
            glVertex2f(glOrthoX,2);
        glEnd();
    glPopMatrix();
}


//comandos
void arrow_keys(int a_keys, int x, int y){
    switch (a_keys){
    case GLUT_KEY_RIGHT:
        if(instanciaDisparador.x==glOrthoX-5){}
        else{
            instanciaDisparador.x=instanciaDisparador.x+instanciaDisparador.veloc;
            glutPostRedisplay();
        }
        break;
    case GLUT_KEY_LEFT:
        if(instanciaDisparador.x==5){}
        else{
            instanciaDisparador.x=instanciaDisparador.x-instanciaDisparador.veloc;
            glutPostRedisplay();
        }
        break;
    default:
        break;
    }
}

void keyboard(unsigned char key, int x, int y){
    switch (key){
    case 27:     // Termina o programa qdo
        exit(0); // a tecla ESC for pressionada
        break;
    case ' ':
        Dispara();
        glutPostRedisplay();
    default:
        break;
    }
}


//padrao
void init(void){
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
    int r;
    string nome = "./JogoImagens/cenario.png";
    r = Image.Load(nome.c_str());

    int v;
    string vidas = "./JogoImagens/vida3.png";
    v = numVidas.Load(vidas.c_str());

    if (!r) exit(1); // Erro na carga da imagem
    if (!v) exit(1); // Erro na carga da imagem

    //modelos
    CarregaModelos();

    //instancias
    CriaInstancia(instanciaDisparador, modeloDisparador,glOrthoX/2,3.4,3);
    CriaInstanciasDeNaves();
}

void reshape(int w, int h){
    glMatrixMode(GL_PROJECTION);// Reset the coordinate system before modifying
    glLoadIdentity();
    glViewport(0, 0, w, h);// Define a area a ser ocupada pela �rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);// Define os limites l�gicos da �rea OpenGL dentro da Janela
    glLoadIdentity();
}

void display(void){
    glClear(GL_COLOR_BUFFER_BIT); // Limpa a tela coma cor de fundo
    glMatrixMode(GL_PROJECTION);// Define os limites logicos da area OpenGL dentro da Janela
    glLoadIdentity();
    glOrtho(0, glOrthoX, 0, glOrthoY, -1, 1);// Define os limites logicos da area OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)Image.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)Image.SizeY();
    Image.SetZoomH(zoomH);
    Image.SetZoomV(zoomV);
    Image.SetPos(0, 0);
    Image.Display();

    //tempo naves
    AccumDeltaT += dt; // Tempo acumulado
    fps = 1.0/dt; // FPS

    int i;
    if(Naves.size() != 0 && numeroDeVidas > 0){
        numVidas.SetPos(glOrthoX-70, glOrthoY-20);
        numVidas.Display();

        if(numeroDeVidas == 2){
            int v;
            string vida = "./JogoImagens/vida2.png";
            v = numVidas.Load(vida.c_str());
        }else if(numeroDeVidas == 1){
            int v;
            string vida = "./JogoImagens/vida1.png";
            v = numVidas.Load(vida.c_str());
        }

        srand(time(NULL));

        //atualiza chao
        DesenhaChao();
        //atualiza disparador
        DesenhaIntanciaDeModelo(instanciaDisparador);

        //Atualiza Naves
        for(i=0;i<Naves.size();i++){
            DesenhaIntanciaDeModelo(Naves[i]);
            if(Naves[i].y <= -10){
                Naves[i].y = glOrthoY+10;
                Naves[i].x = rand() % glOrthoX-5;
                glutPostRedisplay();
            }else{
                if(VerificaColisao(Naves[i], instanciaDisparador)){ //Verifica se o tiro pegou em alguma das naves restantes
                    Naves.erase(Naves.begin() + i);
                    numeroDeVidas = numeroDeVidas - 1;
                }else{
                    tamanhoRestante = glutGet(GLUT_WINDOW_HEIGHT) - Naves[i].y;
                    velocidade = tamanhoRestante / (Naves[i].veloc - AccumDeltaT) / fps;
                    Naves[i].y = Naves[i].y - 0.1; //-Naves[i].y * velocidade; //CALCULO TEMPO
                    int chanceDeDesparo = rand() % 15;
                    if(chanceDeDesparo == 1){
                        naveDispara(Naves[i]);
                        glutPostRedisplay();
                    }
                    chanceDeDesparo = 0;
                }
                glutPostRedisplay();
            }
        }

        for(i=0;i<DisparosNave.size();i++){
            DesenhaIntanciaDeModelo(DisparosNave[i]);
            if(DisparosNave[i].y==0){}
            else{
                if(VerificaColisao(instanciaDisparador, DisparosNave[i])){ //Verifica se o tiro pegou em alguma das naves restantes
                    DisparosNave.erase(DisparosNave.begin() + i); // elimina o tiro
                }
                DisparosNave[i].y = DisparosNave[i].y - DisparosNave[i].veloc;
                glutPostRedisplay();
            }
        }

        //atualiza disparos
        for(i=0;i<Disparos.size();i++){
            DesenhaIntanciaDeModelo(Disparos[i]);
            if(Disparos[i].y==glOrthoY+5){}
            else{
                for(int j = 0; j < Naves.size(); j++){
                    if(VerificaColisao(Naves[j],Disparos[i])){ //Verifica se o tiro pegou em alguma das naves restantes
                        Naves.erase(Naves.begin() + j); // elimina a nave
                        Disparos.erase(Disparos.begin() + i); // elimina o tiro
                    }
                }
                Disparos[i].y = Disparos[i].y + Disparos[i].veloc;
                glutPostRedisplay();
            }
        }
    }else{
        int r;
        string imagemFinal;
        if(numeroDeVidas > 0){
            imagemFinal = "./JogoImagens/voce-venceu.png";
            r = Image.Load(imagemFinal.c_str());
            Image.Display();
        }else{
            imagemFinal = "./JogoImagens/fim-de-jogo.png";
            r = Image.Load(imagemFinal.c_str());
            Image.Display();
        }
    }

    glutSwapBuffers();
}

int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 450);
    glutCreateWindow("SpaceInvaders");
    init();
    glutDisplayFunc(display);
     glutIdleFunc(animate);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutMainLoop();
    return 0;
}


