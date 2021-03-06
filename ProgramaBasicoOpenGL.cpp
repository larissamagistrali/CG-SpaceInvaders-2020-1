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
#include "Temporizador.h"

//------------------------JOGO---------------------------------

//STRUCTS

//Vetor
class Vetor{
    double x,y;
public:
    Vetor() { x=0; y=120; } //
    void set(double x, double y){
        this->x = x;
        this->y = y;
    }

    void get (double &x, double &y){
        x = this->x;
        y = this->y;
    }

    double getX(){ return this->x; }
    double getY() { return this->y; }

    void add(Vetor v){
        this->x += v.getX();
        this->y += v.getY();
    }

    void multiply(double n){
        this->x *= n;
        this->y *= n;
    }
};

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
    double tempo;
    Vetor A;
    Vetor B;
    Vetor D;
} Instancia;


//VARIAVEIS

//Temporizador
Temporizador T;
double fps;
double AccumDeltaT=0;
double velocidade = 1;
double tamanhoRestante;

//Vetor
Vetor DIRE;

//Tamanho glortho
int glOrthoX=250, glOrthoY=125;

//Vidas
int numeroDeVidas=3;
ImageClass numVidas;

//Plano de fundo
ImageClass Image;

//Modelos
ModeloDeObjeto modeloProjetil, modeloDisparador, modeloNaveAzul, modeloNaveRoxa, modeloNaveVermelha, modeloNaveRosa;

//Instancias
Instancia instanciaDisparador, instanciaNave1,instanciaNave2;
vector <Instancia> Naves;
vector <Instancia> Disparos;
vector <Instancia> DisparosNave;

//OBJETOS

//Arquivos
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

void CriaInstancia(Instancia &i, ModeloDeObjeto &m, float x1, float y1, float v, double t){
    i.modelo=m;
    i.x=x1;
    i.y=y1;
    i.veloc=v;
    i.tempo=t;
}

void DesenhaIntanciaDeModelo(Instancia &inst){
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

void CriaInstanciasDeNaves(int qtd){
    srand(time(NULL));
    for(int i=0;i<qtd;i++){
        Instancia instAzul,instRosa,instVermelha,instRoxa;
        CriaInstancia(instAzul,modeloNaveAzul,10+(rand() % 220),glOrthoY+10,0, 4+(rand() % 15));
        CriaInstancia(instRosa,modeloNaveRosa,10+(rand() % 220),glOrthoY+10,0,4+(rand() % 15));
        CriaInstancia(instVermelha,modeloNaveVermelha,10+(rand() % 220),glOrthoY+10,0,4+(rand() % 15));
        CriaInstancia(instRoxa,modeloNaveRoxa,10+(rand() % 220),glOrthoY+10,0,4+(rand() % 15));
        Naves.push_back(instAzul);
        Naves.push_back(instRosa);
        Naves.push_back(instVermelha);
        Naves.push_back(instRoxa);
    }
}

//Disparo do jogador
void Dispara(){
    Instancia i;
    CriaInstancia(i,modeloProjetil,instanciaDisparador.x - 0.5 ,instanciaDisparador.y + 4 + 0.5, 0.3, 0); //x,y,veloc aleatorios
    Disparos.push_back(i);
}

//Disparo da nave
void NaveDispara(Instancia nave){
    Instancia i;
    CriaInstancia(i,modeloProjetil, (nave.x + 2) , nave.y + 3, 0.3,0); //x,y,veloc aleatorios
    DisparosNave.push_back(i);
}

//Colisao entre nave e disparo do jogador
bool VerificaColisaoNaveDisparo(Instancia &nave, Instancia &disparo){
    float xCentralNave = (nave.x - 1);
    float xCentralDisparo = disparo.x;
    float yCentralNave = (nave.y + 6 + 0.5);
    float yCentralDisparo = disparo.y;

    if (xCentralDisparo <= xCentralNave + nave.modelo.largura &&
        xCentralDisparo + disparo.modelo.largura > xCentralNave &&
        yCentralDisparo <= yCentralNave + nave.modelo.altura / 2 &&
        yCentralDisparo + disparo.modelo.altura / 2 > yCentralNave) {
            return true;
    }
    return false;
}

//Colisao entre nave e jogador
bool VerificaColisaoNaveDisparador(Instancia &nave, Instancia &disparador){
    float xCentralNave = (nave.x + 1);
    float xCentralDisparador = disparador.x;
    float yCentralNave = (nave.y + 6 + 0.5);
    float yCentralDisparador = disparador.y + 2;

    if (xCentralDisparador < xCentralNave + nave.modelo.largura &&
        xCentralDisparador + disparador.modelo.largura / 2 > xCentralNave &&
        yCentralDisparador < yCentralNave + nave.modelo.altura / 2 &&
        yCentralDisparador + disparador.modelo.altura / 2 >= yCentralNave) {
            return true;
    }
    return false;
}

//Colisao entre disparo da nave e jogador
bool VerificaColisaoDisparoNaveDisparador(Instancia &disparador, Instancia &disparo){
    float xCentralDisparador = disparador.x - 3;
    float xCentralDisparo = disparo.x;
    float yCentralDisparador = disparador.y + 2;
    float yCentralDisparo = disparo.y;

    if (xCentralDisparo < xCentralDisparador + disparador.modelo.largura / 2 &&
        xCentralDisparo + disparo.modelo.largura > xCentralDisparador &&
        yCentralDisparo < yCentralDisparador + disparador.modelo.altura / 2 &&
        yCentralDisparo + disparo.modelo.altura >= yCentralDisparador) {
            return true;
    }
    return false;
}

//Gera o chao
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

//Comandos do teclado
void arrow_keys(int a_keys, int x, int y){
    switch (a_keys){
    case GLUT_KEY_RIGHT:
        if(instanciaDisparador.x==glOrthoX-5){} //Limite do final da tela
        else{
            instanciaDisparador.x=instanciaDisparador.x+instanciaDisparador.veloc;
            glutPostRedisplay();
        }
        break;
    case GLUT_KEY_LEFT:
        if(instanciaDisparador.x==5){} //Limite do inicio da tela
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
    case 27:     //Termina o programa quando a tecla ESC for pressionada
        exit(0);
        break;
    case ' ':
        Dispara();
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

//Padrao
void init(void){
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //Cor de fundo
    int r;
    string nome = "./JogoImagens/cenario.png";
    r = Image.Load(nome.c_str());

    int v;
    string vidas = "./JogoImagens/life3.png";
    v = numVidas.Load(vidas.c_str());

    if (!r) exit(1); //Erro na carga da imagem
    if (!v) exit(1); //Erro na carga da imagem

    //Carrega os modelos
    CarregaModelos();

    //Cria as instancias
    CriaInstancia(instanciaDisparador, modeloDisparador,glOrthoX/2,3.5,3,0);
    int qtdInstancias=2; //Cria x instancias de cada modelo de nave (4 modelos * 2 = 8 naves)
    CriaInstanciasDeNaves(qtdInstancias);
}

void reshape(int w, int h){
    glMatrixMode(GL_PROJECTION);//Reset the coordinate system before modifying
    glLoadIdentity();
    glViewport(0, 0, w, h);//Define a area a ser ocupada pela �rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);//Define os limites l�gicos da �rea OpenGL dentro da Janela
    glLoadIdentity();
}

//Variaveis para o disparo da nave
int chanceDeDesparo = 0;
double umSegundo = 0;

void display(void){
    srand(time(0));
    //srand(time(NULL));

    //Tempo
    double dt = T.getDeltaT();

    umSegundo += dt; //Verifica um segundo
    AccumDeltaT += dt; //Tempo acumulado
    fps = 1.0/dt; //FPS

    //Imprime FPS a cada 3 segundos
    if (AccumDeltaT >=3){
        cout << 1.0/dt << " FPS"<< endl;
        AccumDeltaT = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT); //Limpa a tela coma cor de fundo
    glMatrixMode(GL_PROJECTION);//Define os limites logicos da area OpenGL dentro da Janela
    glLoadIdentity();
    glOrtho(0, glOrthoX, 0, glOrthoY, -1, 1);//Define os limites logicos da area OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float zoomH = (glutGet(GLUT_WINDOW_WIDTH))/(double)Image.SizeX();
    float zoomV = (glutGet(GLUT_WINDOW_HEIGHT))/(double)Image.SizeY();
    Image.SetZoomH(zoomH);
    Image.SetZoomV(zoomV);
    Image.SetPos(0, 0);
    Image.Display();

    int i;

    //Se ainda tem nave e o jogador tem vida o jogo continua
    if(Naves.size() != 0 && numeroDeVidas > 0){
        numVidas.SetPos(glOrthoX-40, glOrthoY-15);
        numVidas.Display(); //Mostra vidas

        if(numeroDeVidas == 2){ //Troca vida
            int v;
            string vida = "./JogoImagens/life2.png";
            v = numVidas.Load(vida.c_str());
        }else if(numeroDeVidas == 1){ //Troca vida
            int v;
            string vida = "./JogoImagens/life1.png";
            v = numVidas.Load(vida.c_str());
        }

        //Atualiza o chao
        DesenhaChao();

        //Atualiza o jogador
        DesenhaIntanciaDeModelo(instanciaDisparador);

        //Atualiza naves
        for(i=0;i<Naves.size();i++){
            DesenhaIntanciaDeModelo(Naves[i]);
            if(Naves[i].y <= -10){ //Se a nave passou do chão, volta pro topo
                Naves[i].y = glOrthoY + 10;
                Naves[i].x = rand() % glOrthoX-5;
                Vetor RESET;
                Naves[i].A = RESET;
                Naves[i].B = RESET;
            }else{
                if(VerificaColisaoNaveDisparador(Naves[i], instanciaDisparador)){ //Se a nave colidiu com o jogador, ele perde vida
                    Naves.erase(Naves.begin() + i);
                    numeroDeVidas = numeroDeVidas - 1;
                }else{
                    tamanhoRestante = glOrthoY - Naves[i].B.getY(); //Calcula quanto falta pra chegar no final da janela
                    velocidade = (tamanhoRestante / (Naves[i].tempo - AccumDeltaT)) / fps; //Calcula quanto de velocidade tem que ter para chegar ao fim

                    //Deslocamento
                    DIRE.set(0,-1);
                    DIRE.multiply(velocidade);
                    Naves[i].A.add(DIRE);
                    Naves[i].B.set(Naves[i].A.getX(), Naves[i].A.getY());
                    Naves[i].y = Naves[i].B.getY();

                    if(umSegundo >=1){ // A cada segundo a nave tem 25% de chance de realizar um disparo

                        chanceDeDesparo = rand() % 4;

                        if(chanceDeDesparo == 1){
                            NaveDispara(Naves[i]);
                        }

                    chanceDeDesparo = 0;

                    }
                }
            }
        }

        if(umSegundo >= 1){ //Reseta o segundo para auxiliar o disparo da nave
            umSegundo = 0;
        }

        //Atualiza os disparos das naves
        for(i=0;i<DisparosNave.size();i++){
            DesenhaIntanciaDeModelo(DisparosNave[i]);
            if(DisparosNave[i].y<=0){ //Se o disparo ultrapassa o chão, ele some
                DisparosNave.erase(DisparosNave.begin() + i);
            }else{
                if(VerificaColisaoDisparoNaveDisparador(instanciaDisparador, DisparosNave[i])){ //Se o disparo colidiu com o jogador, ele perde vida
                    DisparosNave.erase(DisparosNave.begin() + i); //Elimina o tiro
                    numeroDeVidas = numeroDeVidas - 1; //Perde uma vida
                }
            DisparosNave[i].y = DisparosNave[i].y - DisparosNave[i].veloc; //Movimenta o disparo
            }
        }

        //Atualiza os disparos do Jogador
        for(i=0;i<Disparos.size();i++){
            DesenhaIntanciaDeModelo(Disparos[i]);
            if(Disparos[i].y>=glOrthoY+3){} //Se o disparo ultrapassa o teto, ele some
            else{
                for(int j = 0; j < Naves.size(); j++){
                    if(VerificaColisaoNaveDisparo(Naves[j], Disparos[i])){ //Se o disparo colidiucom a nave, ela "morre"
                        Naves.erase(Naves.begin() + j); //Elimina a nave
                        Disparos.erase(Disparos.begin() + i); //Elimina o tiro
                    }
                }
                Disparos[i].y = Disparos[i].y + Disparos[i].veloc; //Movimenta o disparo
            }
        }
    }else{ //Se o jogador perdeu todas as vidas, ou todas as naves foram eliminadas
        int r;
        string imagemFinal;
        if(numeroDeVidas > 0){ //Venceu
            imagemFinal = "./JogoImagens/voce-venceu.png";
            r = Image.Load(imagemFinal.c_str());
            Image.Display();
        }else{ //Perdeu
            imagemFinal = "./JogoImagens/fim-de-jogo.png";
            r = Image.Load(imagemFinal.c_str());
            Image.Display();
        }
    }
    glutSwapBuffers();
}

//Main
int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 450);
    glutCreateWindow("SpaceInvaders");
    glutFullScreen();
    init();
    glutIdleFunc(display);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(arrow_keys);
    glutMainLoop();
    return 0;
}
