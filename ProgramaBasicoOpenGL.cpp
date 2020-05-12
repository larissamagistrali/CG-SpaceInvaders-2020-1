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
float dt=1;
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

//Definicao das structs

//Ponto
typedef struct{
    int x;
    int y;
} Ponto;

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
    float xCentral;
    float yCentral;
} Instancia;

//Variaveis
int glOrthoX=450, glOrthoY=225;

int numeroDeVidas=3;
ImageClass numVidas;

ImageClass Image;

ModeloDeObjeto modeloProjetil, modeloDisparador, modeloNaveAzul, modeloNaveRoxa, modeloNaveVermelha, modeloNaveRosa;

Instancia instanciaDisparador, instanciaNave1,instanciaNave2,instanciaNave3,instanciaNave4,instanciaNave5,instanciaNave6,instanciaNave7,instanciaNave8;
vector <Instancia> Naves;
vector <Instancia> Disparos;

//ponto central
void calculaPontoCentral(Instancia inst){
    float xC = inst.x - (inst.modelo.largura / 2);
    float yC = inst.y + (inst.modelo.altura / 2);
    inst.xCentral = xC;
    inst.yCentral = yC;
    printf("XC %f YC %f\n", xC, yC);
}

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
    i.xCentral = x1 - (m.largura / 2);
    i.yCentral = y1 - (m.altura / 2);
}

void DesenhaIntanciaDeModelo(Instancia &inst){
    //printf("DesenhaInstanciaDeModelo() \n");
    glPushMatrix();
        glLoadIdentity();
        glTranslatef(inst.x, inst.y, 0);
        float dx =inst.modelo.largura, dy = inst.modelo.altura;
        int i, j;
        for (i = 0; i < inst.modelo.largura; i++) {
            for (j = 0; j < inst.modelo.altura; j++){
                if(inst.modelo.cores[i][j].r==0 && inst.modelo.cores[i][j].g==0 && inst.modelo.cores[i][j].b==0){}
                else{
                    glBegin(GL_QUADS);
                    glColor3ub(inst.modelo.cores[i][j].r, inst.modelo.cores[i][j].g, inst.modelo.cores[i][j].b);
                    glVertex2f(dx, dy);
                    glVertex2f(dx + 1 ,dy);
                    glVertex2f(dx + 1, dy + 1);
                    glVertex2f(dx, dy + 1);
                    glEnd();
                }
                dx=dx+1;
            }
            dy=dy-1;
            dx=inst.modelo.largura;
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
    printf("Dispara() \n");
    Instancia i;
    CriaInstancia(i,modeloProjetil,instanciaDisparador.x,instanciaDisparador.y, 0.3);
    Disparos.push_back(i);
}


//colisao
bool VerificaColisao(Instancia &inst1, Instancia &inst2){
    float xC1 = inst1.x - (inst1.modelo.largura / 2);
    float xC2 = inst2.x - (inst2.modelo.largura / 2);
    float yC1 = inst1.y + (inst1.modelo.altura / 2);
    float yC2 = inst2.y + (inst2.modelo.altura / 2);

    if (xC2 <= xC1 + inst1.modelo.largura &&
        xC2 + inst2.modelo.largura > xC1 &&
        yC2 <= yC1 + inst1.modelo.altura &&
        yC2 + inst2.modelo.altura >= yC1) {
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
        if(instanciaDisparador.x==glOrthoX-10){}
        else{
            instanciaDisparador.x=instanciaDisparador.x+instanciaDisparador.veloc; //arrumar pra segundos
            glutPostRedisplay();
        }
        break;
    case GLUT_KEY_LEFT:
        if(instanciaDisparador.x==20){}
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
    CriaInstancia(instanciaDisparador, modeloDisparador,glOrthoX/2,3,2.5);
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
        int i;
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
                    Naves[i].y = Naves[i].y - 1; //(glOrthoY/Naves[i].veloc)*(1/dt); //CALCULO DE TEMPO
                    if(i ==1){
                        calculaPontoCentral(Naves[i]);
                        //("NAVE %d [ larg %d, alt %d, x %f y %f xC %f yC %f \n", i, Naves[i].modelo.largura, Naves[i].modelo.altura, Naves[i].x, Naves[i].y, Naves[i].xCentral, Naves[i].yCentral);
                    }
                }
                glutPostRedisplay();
            }
        }
        //atualiza disparos
        for(i=0;i<Disparos.size();i++){
            DesenhaIntanciaDeModelo(Disparos[i]);
            if(Disparos[i].y==glOrthoY+5){}
            else{
                for(int j = 0; j < Naves.size(); j++){
                    if(VerificaColisao(Disparos[i], Naves[j])){ //Verifica se o tiro pegou em alguma das naves restantes
                        //printf("COLIDIU NAVE (%d, %d) TIRO (%d, %d)", Naves[j].modelo.altura, Naves[j].modelo.largura, Disparos[i].modelo.altura, Disparos[i].modelo.largura);
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
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutMainLoop();
    return 0;
}


