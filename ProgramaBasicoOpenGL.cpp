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

//------------------------JOGO---------------------------------
//structs
typedef struct{
    float r, g, b;
} Cor;

typedef struct{
    int altura;
    int largura;
    Cor cores[100][100];
} ModeloDeObjeto;

typedef struct{
    ModeloDeObjeto modelo;
    float x; //x,y posicao do objeto no universo
    float y;
    float veloc;
} Instancia;


//variaveis
int numeroDeVidas=3;
ImageClass Image;
ModeloDeObjeto modeloProjetil, modeloDisparador, modeloNaveAzul, modeloNaveRoxa, modeloNaveVermelha, modeloNaveRosa;
Instancia instanciaDisparador, instanciaNave1,instanciaNave2,instanciaNave3,instanciaNave4,instanciaNave5,instanciaNave6,instanciaNave7,instanciaNave8;
Instancia Naves[8];
Instancia Disparos[100];
int contDisparos=0;


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
        float dx = -inst.modelo.largura / 4., dy = inst.modelo.altura / 2. - 0.4; //nao entendi pq isso
        int i, j;
        for (i = 0; i < inst.modelo.largura; i++) {
            for (j = 0; j < inst.modelo.altura; j++){
                glBegin(GL_QUADS);
                glColor3ub(inst.modelo.cores[i][j].r, inst.modelo.cores[i][j].g, inst.modelo.cores[i][j].b);
                glVertex2f(dx, dy);
                glVertex2f(dx + 0.4 ,dy);
                glVertex2f(dx + 0.4, dy + 0.4);
                glVertex2f(dx, dy + 0.4);
                glEnd();
                dx += 0.4;
            }
            dy -= 0.4;
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

void CriaInstanciasDeNaves(){ //precisa fazer envelopes para cada nave
    srand(time(NULL));
    CriaInstancia(instanciaNave1,modeloNaveAzul,rand() % 145,80,0.01);
    CriaInstancia(instanciaNave2,modeloNaveAzul,rand() % 145,80,0.015);
    CriaInstancia(instanciaNave3,modeloNaveVermelha,rand() % 145,80,0.02);
    CriaInstancia(instanciaNave4,modeloNaveVermelha,rand() % 145,80,0.025);
    CriaInstancia(instanciaNave5,modeloNaveRoxa,rand() % 145,80,0.03);
    CriaInstancia(instanciaNave6,modeloNaveRoxa,rand() % 145,80,0.035);
    CriaInstancia(instanciaNave7,modeloNaveRosa,rand() % 145,80,0.04);
    CriaInstancia(instanciaNave8,modeloNaveRosa,rand() % 145,80,0.045);
    Naves[0]=instanciaNave1;
    Naves[1]=instanciaNave2;
    Naves[2]=instanciaNave3;
    Naves[3]=instanciaNave4;
    Naves[4]=instanciaNave5;
    Naves[5]=instanciaNave6;
    Naves[6]=instanciaNave7;
    Naves[7]=instanciaNave8;
}


//disparos
void Dispara(){
    printf("Dispara() \n");
    Instancia i;
    CriaInstancia(i,modeloProjetil,instanciaDisparador.x,instanciaDisparador.y+7,0.1); //x,y,veloc aleatorios
    Disparos[contDisparos]=i;
    contDisparos++;
}


//colisao


//mostra vidas


//chão
void DesenhaChao(){
    glPushMatrix();
        glLoadIdentity();
        glColor3f(0.7,0,0.3);
        glTranslatef(0, 1, 0);
        glBegin(GL_QUADS);
            glVertex2f(150,0);
            glVertex2f(0,0);
            glVertex2f(0,2);
            glVertex2f(150,2);
        glEnd();
    glPopMatrix();
}


//comandos
void arrow_keys(int a_keys, int x, int y){
    switch (a_keys){
    case GLUT_KEY_RIGHT:
        if(instanciaDisparador.x==148){}
        else{
            instanciaDisparador.x=instanciaDisparador.x+instanciaDisparador.veloc;
            glutPostRedisplay();
        }
        break;
    case GLUT_KEY_LEFT:
        if(instanciaDisparador.x==4){}
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
    if (!r) exit(1); // Erro na carga da imagem

    //modelos
    CarregaModelos();

    //instancias
    CriaInstancia(instanciaDisparador, modeloDisparador,75,1.8,2.5);
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
    glOrtho(0, 150, 0, 75, -1, 1);// Define os limites logicos da area OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)Image.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)Image.SizeY();
    Image.SetZoomH(zoomH);
    Image.SetZoomV(zoomV);
    Image.SetPos(0, 0);
    Image.Display();
    srand(time(NULL));


    //atualiza chao
    DesenhaChao();
    //atualiza disparador
    DesenhaIntanciaDeModelo(instanciaDisparador);
    //atualiza naves
    int i;
    for(i=0;i<8;i++){
        DesenhaIntanciaDeModelo(Naves[i]);
        if(Naves[i].y<=-10){
            Naves[i].y=80;
            Naves[i].x=rand() % 145;
            glutPostRedisplay();
        }else{
            Naves[i].y=Naves[i].y-Naves[i].veloc;
            glutPostRedisplay();
        }

    }
    //atualiza disparos
    for(i=0;i<contDisparos;i++){
        DesenhaIntanciaDeModelo(Disparos[i]);
        if(Disparos[i].y==80){}
        else{
            Disparos[i].y=Disparos[i].y+Disparos[i].veloc;
            glutPostRedisplay();
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


