#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GL4D/gl4du.h>
#include <GL4D/gl4dh.h>
#include <GL4D/gl4dp.h>
#include <SDL_image.h>
#include <GL4D/gl4duw_SDL2.h>
#include <SDL_mixer.h>
#include <fftw3.h>

/* Prototypes des fonctions statiques contenues dans ce fichier C */
static void initFftw(const char * filename);
static void init(void);
static void draw(void);
static void drawTwo(void);
static void initGL(void);
static void initData(void);
static void initAudio(const char * filename);
static void resize(int w, int h);
static void initApi8(void);
static void drawApi8(void);
static void drawApi8Anim(void);
static void drawFin(void);
static void drawMix(void);
static void quit(void);
static void anim1(int state);
static void anim2(int state);
static void anim3(int state);
static void anim4(int state);
static void anim5(int state);
static void anim6(int state);
static void animationsInit(void);

static float color_anim1[] ={1.0, 0.549, 0.0};
static float color_anim2[] ={0.0, 0.749, 1.0};
static float color_anim3[] ={1.0, 1.0, 0.878};
static float color_anim4[] ={1.0, 1.0, 0.878};

static float *color[]={color_anim1, color_anim2, color_anim3, color_anim4};
static int anim = 0;

static const char * _texture_filenames[] = {"./images/Api8.png", "./images/Fin.jpg"};
static const char * _music_name[] = {"MUSIC_NAME.mp3"};
enum texture {
  T_API8 = 0,
  T_FIN,
  T_END
};

static const char *_name_tex[]={"texApi8","texFin"}; 

static GLuint _pId = 0;

static GLuint _tId[2] = {0};

static GLuint _cube[16] = {0}, _quad[16] = {0};



/*!\brief nombre d'échantillons du signal sonore */
#define ECHANTILLONS 1024
/*!\brief amplitude des échantillons du signal sonore */
static Sint16 _hauteurs[ECHANTILLONS];
/*!\brief dimensions de la fenêtre */
static int _wW = 1024, _wH = 800;
/*!\brief id du screen à créer */

static GLuint _quadId = 0;


/*!\brief pointeur vers la musique chargée par SDL_Mixer */
static Mix_Music * _mmusic = NULL;
/*!\brief données entrées/sorties pour la lib fftw */
static fftw_complex * _in4fftw = NULL, * _out4fftw = NULL;
/*!\brief donnée à précalculée utile à la lib fftw */
static fftw_plan _plan4fftw = NULL;


static float signal[16]={0};






static GL4DHanime _animations[] = {
  { 5000, anim1, NULL, NULL },
  { 30000, anim4, NULL, NULL },
  { 30000, anim3, NULL, NULL },
  { 30000, anim2, NULL, NULL },
  { 30000, anim6, NULL, NULL },
  { 5000, anim5, NULL, NULL },
  {    0, NULL, NULL, NULL }
};











int main(int argc, char ** argv) {
  
  if(!gl4duwCreateWindow(argc, argv, "Anis CHALI API8", 10, 10, 
			 _wW, _wH,GL4DW_RESIZABLE | GL4DW_SHOWN))
    return 1;
  initFftw(_music_name[0]);
  init();
  initApi8();
  atexit(quit);
  gl4duwResizeFunc(resize);
  gl4duwDisplayFunc(gl4dhDraw);
  gl4duwMainLoop();
  return 0;
}
static void init(void){
  glClearColor(color[1][0], color[1][1], color[1][2], 1.0f);
  gl4dhInit(_animations, _wW, _wH, animationsInit);
  resize(_wW, _wH);
}

static void initApi8(void){
  int i;
  if (!_tId[0]){
    
    glGenTextures(T_END, _tId);
    for (i = 0; i < T_END; ++i){
      SDL_Surface *t;
      glBindTexture(GL_TEXTURE_2D, _tId[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      if ( (t = IMG_Load(_texture_filenames[i])) != NULL ){
#ifdef __APPLE__
	int mode = t->format->BytesPerPixel == 4 ? GL_BGRA : GL_BGR;
#else
	int mode = t->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB;
#endif       
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, mode, GL_UNSIGNED_BYTE, t->pixels);
	SDL_FreeSurface(t);
      }else {
	fprintf(stderr, "can't open file %s : %s\n", _texture_filenames[i], SDL_GetError());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
      }
      glBindTexture(GL_TEXTURE_2D, 0);
    }
  }
  for (i = 0; i < 16; ++i)
    _quad[i] = gl4dgGenQuadf();
}


static void initFftw(const char * filename) {
  /* préparation des conteneurs de données pour la lib FFTW */
  _in4fftw   = fftw_malloc(ECHANTILLONS *  sizeof *_in4fftw);
  memset(_in4fftw, 0, ECHANTILLONS *  sizeof *_in4fftw);
  assert(_in4fftw);
  _out4fftw  = fftw_malloc(ECHANTILLONS * sizeof *_out4fftw);
  assert(_out4fftw);
  _plan4fftw = fftw_plan_dft_1d(ECHANTILLONS, _in4fftw, _out4fftw, FFTW_FORWARD, FFTW_ESTIMATE);
  assert(_plan4fftw);
  /* préparation GL */
  initGL();
  //resize(_wW, _wH);
  initData();
  initApi8();
  /* chargement de l'audio */
  initAudio(filename);
}

static void resize(int w, int h) {
  _wW  = w;
  _wH = h;
  glViewport(0, 0, _wW, _wH);
  gl4duBindMatrix("projectionMatrix");
  gl4duLoadIdentityf();
  gl4duFrustumf(-0.5, 0.5, -0.5 * _wH / _wW, 0.5 * _wH / _wW, 1.0, 1000.0);
  gl4duBindMatrix("modelViewMatrix");
 
}

static float myRand(float min, float max){

  return (float)(min + ((float) rand() / RAND_MAX * (max - min + 1.0)));
}



static void drawFin(void){

  float r, g, b, a;
  r = myRand(0.0, 1.0);
  g = myRand(0.0, 1.0);
  b = myRand(0.0, 1.0);
  a = myRand(0.0, 1.0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(r,g,b,1.0);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
  glUniform1f(glGetUniformLocation(_pId, "cr"), r);
  glUniform1f(glGetUniformLocation(_pId, "cg"), g);
  glUniform1f(glGetUniformLocation(_pId, "cb"), b);
  glUniform1f(glGetUniformLocation(_pId, "ca"), a);
  gl4duTranslatef(0.0, 0.0, -60.0);
  gl4duScalef(20.0, 20.0, 0.0);
  gl4duSendMatrices();

  glActiveTexture(GL_TEXTURE0+1);
  glBindTexture(GL_TEXTURE_2D, _tId[1]);
  glUniform1i(glGetUniformLocation(_pId, _name_tex[T_API8]), 1);

  gl4dgDraw(_quad[0]);

 
  
}


static void drawApi8(void){

  float r, g, b, a;
  r = myRand(0.0, 1.0);
  g = myRand(0.0, 1.0);
  b = myRand(0.0, 1.0);
  a = myRand(0.0, 1.0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(r,g,b,1.0);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
  glUniform1f(glGetUniformLocation(_pId, "cr"), r);
  glUniform1f(glGetUniformLocation(_pId, "cg"), g);
  glUniform1f(glGetUniformLocation(_pId, "cb"), b);
  glUniform1f(glGetUniformLocation(_pId, "ca"), a);
  gl4duTranslatef(0.0, 0.0, -60.0);
  gl4duScalef(10.0, 10.0, 0.0);
  gl4duSendMatrices();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[0]);
  glUniform1i(glGetUniformLocation(_pId, _name_tex[T_API8]), T_API8);

  gl4dgDraw(_quad[0]);

 
  
}


static void drawApi8Anim(void){
  int i;
  float r, g, b, a;
  r = myRand(0.0, 1.0);
  g = myRand(0.0, 1.0);
  b = myRand(0.0, 1.0);
  a = myRand(0.0, 1.0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
  glUniform1f(glGetUniformLocation(_pId, "cr"), r);
  glUniform1f(glGetUniformLocation(_pId, "cg"), g);
  glUniform1f(glGetUniformLocation(_pId, "cb"), b);
  glUniform1f(glGetUniformLocation(_pId, "ca"), a);
  gl4duTranslatef(0.0, 0.0, -60.0);
  gl4duScalef(10.0, 10.0, 0.0);
  gl4duSendMatrices();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _tId[0]);
  glUniform1i(glGetUniformLocation(_pId, _name_tex[T_API8]), T_API8);

  for (i = 0; i < 16; ++i){
    gl4duPushMatrix(); {
      gl4duScalef(signal[i]+i, signal[i]+i, 0.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    
    gl4dgDraw(_quad[i]);
  
  }
 
  
}



static void draw(void){
  
  int i;
  //  float *signal = getAudio();
  
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  gl4duTranslatef(-23.0, 0.0, -60.0);
  gl4duRotatef(90.0, 0.0,1.0,0.0);
  gl4duTranslatef(-10.0, -10.0, 0.0);
  glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
  
  for (i = 0; i < 16; i++){
    gl4duScalef(1.0, signal[i] ,1.0);
    gl4duSendMatrices();
    gl4dgDraw(_cube[i]);

    gl4duPushMatrix(); {
      gl4duRotatef(-90, 0, 1, 0);
      gl4duTranslatef(+46.0, 0.0, 0.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4dgDraw(_cube[i]);

    
    gl4duTranslatef(3.0, 0.0, 0.0);
    
  }

  gl4duRotatef(+90, 0, 1, 0);
  for (i = 0; i < 14; ++i){
    gl4duPushMatrix(); {
     
      gl4duTranslatef(-3.0, 0.0, 0.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duTranslatef(-3.1, 0.0, 0.0);
    gl4dgDraw(_cube[15]);
   
  }

  
}



static void drawMix(void){
  
  int i;
  float r, g, b, a;
  r = myRand(0.0, 1.0);
  g = myRand(0.0, 1.0);
  b = myRand(0.0, 1.0);
  a = myRand(0.0, 1.0);
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glClearColor(r,g,b,a);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  gl4duTranslatef(-23.0, 0.0, -60.0);
  gl4duRotatef(90.0, 0.0,1.0,0.0);
  gl4duTranslatef(-10.0, -10.0, 0.0);
  glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
  
  glUniform1f(glGetUniformLocation(_pId, "cr"), r);
  glUniform1f(glGetUniformLocation(_pId, "cg"), g);
  glUniform1f(glGetUniformLocation(_pId, "cb"), b);
  glUniform1f(glGetUniformLocation(_pId, "ca"), a);
  for (i = 0; i < 16; i++){
    gl4duScalef(1.0, signal[i] ,1.0);
    gl4duSendMatrices();
    gl4dgDraw(_cube[i]);

    gl4duPushMatrix(); {
      gl4duRotatef(-90, 0, 1, 0);
      gl4duTranslatef(+46.0, 0.0, 0.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4dgDraw(_cube[i]);

    
    gl4duTranslatef(3.0, 0.0, 0.0);
    
  }

  gl4duRotatef(+90, 0, 1, 0);
  for (i = 0; i < 14; ++i){
    gl4duPushMatrix(); {
     
      gl4duTranslatef(-3.0, 0.0, 0.0);
      gl4duSendMatrices();
    } gl4duPopMatrix();
    gl4duTranslatef(-3.1, 0.0, 0.0);
    gl4dgDraw(_cube[15]);
   
  }

  
}


static void drawTwo(void){
  
  float places[]={
    0.0,-5.0,0.0,-5.0,0.0,-5.0,0.0,-5.0,
    -5.0,0.0,0.0,+5.0,0.0,+5.0,0.0,5.0,
    -5.0,0.0,0.0,-5.0,0.0,-5.0,0.0,-5.0,
    -5.0,0.0,0.0,+5.0,0.0,+5.0,0.0,5.0,0.0
  };
  int i, c1 = 0, c2 = 1;
  //float *signal = getAudio();
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_BLEND_SRC_ALPHA);
  gl4duBindMatrix("modelViewMatrix");
  gl4duLoadIdentityf();
  glUseProgram(_pId);
  gl4duTranslatef(+7.0, +10.0, -60.0);
  gl4duScalef(1.0,1.0 ,1.0);
  gl4duSendMatrices();

   glUniform1i(glGetUniformLocation(_pId, "animation"), anim);
   
  for (i = 0; i < 16; i++){
   
    gl4duRotatef(signal[i] * 0.8,1.0,0.0,1.0);
      
   
    gl4duSendMatrices();
    gl4duPushMatrix(); {
      gl4duScalef(signal[i]*2,signal[i]*2,signal[i]*2);
      gl4duSendMatrices();
    } gl4duPopMatrix();
   
    gl4dgDraw(_cube[i]);
    gl4duTranslatef(places[c1+=2], places[c2+=2], 0.0);
  }
  
}



/*!\brief appelée quand l'audio est joué et met dans \a stream les
 * données audio de longueur \a len */
static void mixCallback(void *udata, Uint8 *stream, int len) {
  if(_plan4fftw) {
    int i, j, l = MIN(len >> 1, ECHANTILLONS);
    Sint16 *d = (Sint16 *)stream;
    for(i = 0; i < l; i++)
      _in4fftw[i][0] = d[i] / ((1 << 15) - 1.0);
    fftw_execute(_plan4fftw);
    for(i = 0; i < l >> 2; i++) {
      _hauteurs[4 * i] = (int)(sqrt(_out4fftw[i][0] * _out4fftw[i][0] + _out4fftw[i][1] * _out4fftw[i][1]) * exp(2.0 * i / (double)(l / 4.0)));
      for(j = 1; j < 4; j++)
	_hauteurs[4 * i + j] = MIN(_hauteurs[4 * i], 255);
    }
  }
  int i, j, r = 0;
  float somme = 0.0;
  
  for(i = 0; i < ECHANTILLONS; i += 64, ++r){
    somme = 0.0;
    for (j = 0; j < 64; j++){
      somme += _hauteurs[i+j];
    }
    
    signal[r] = MIN((somme/64.0 / 8.0), 1.50);
    
  }
}

/*!\brief charge le fichier audio avec les bonnes options */
static void initAudio(const char * filename) {
#if defined(__APPLE__)
  int mult = 1;
#else
  int mult = 2;
#endif
  int mixFlags = MIX_INIT_MP3, res;
  res = Mix_Init(mixFlags);
  if( (res & mixFlags) != mixFlags ) {
    fprintf(stderr, "Mix_Init: Erreur lors de l'initialisation de la bibliothèque SDL_Mixer\n");
    fprintf(stderr, "Mix_Init: %s\n", Mix_GetError());
    //exit(3); commenté car ne réagit correctement sur toutes les architectures
  }
  if(Mix_OpenAudio(44100, AUDIO_S16LSB, 1, mult * ECHANTILLONS) < 0)
    exit(4);  
  if(!(_mmusic = Mix_LoadMUS(filename))) {
    fprintf(stderr, "Erreur lors du Mix_LoadMUS: %s\n", Mix_GetError());
    exit(5);
  }
  Mix_SetPostMix(mixCallback, NULL);
  if(!Mix_PlayingMusic())
    Mix_PlayMusic(_mmusic, 1);
}


static void initGL(void) {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glClearColor(color[anim][0],color[anim][1],color[anim][2],1.0);
 
  _pId  = gl4duCreateProgram("<vs>shaders/basic.vs", "<fs>shaders/basic.fs", NULL);
  gl4duGenMatrix(GL_FLOAT, "modelViewMatrix");
  gl4duGenMatrix(GL_FLOAT, "projectionMatrix");

}

static void initData(void) {
  int i;
  for (i = 0; i < 16; ++i)
    _cube[i] = gl4dgGenCubef();
  
}

/*!\brief Cette fonction est appelée au moment de sortir du programme
 *  (atexit), elle libère les données audio, la fenêtre SDL \ref _win
 *  et le contexte OpenGL \ref _oglContext.
 */
static void quit(void) {
  if(_mmusic) {
    if(Mix_PlayingMusic())
      Mix_HaltMusic();
    Mix_FreeMusic(_mmusic);
    _mmusic = NULL;
  }
  Mix_CloseAudio();
  Mix_Quit();
  if(_plan4fftw) {
    fftw_destroy_plan(_plan4fftw);
    _plan4fftw = NULL;
  }
  if(_in4fftw) {
    fftw_free(_in4fftw); 
    _in4fftw = NULL;
  }
  if(_out4fftw) {
    fftw_free(_out4fftw); 
    _out4fftw = NULL;
  }
  gl4duClean(GL4DU_ALL);
}




static void anim1(int state){
  anim = 2;

  switch(state) {
  case GL4DH_INIT:
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    drawApi8();
    return;
  }
  

}


static void anim2(int state){
  anim = 0;
  glClearColor(color[anim][0],color[anim][1],color[anim][2],1.0);
  switch(state) {
  case GL4DH_INIT:
    
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    draw();
    return;
  }
  

}


static void anim3(int state){
  anim = 1;
  glClearColor(color[anim][0],color[anim][1],color[anim][2],1.0);
  switch(state) {
  case GL4DH_INIT:
    
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    drawTwo();
    return;
  }
  

}


static void anim4(int state){
  anim = 3;
  glClearColor(color[anim][0],color[anim][1],color[anim][2],1.0);
  switch(state) {
  case GL4DH_INIT:
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    drawApi8Anim();
    return;
  }
  

}



static void anim5(int state){
  anim = 2;

  switch(state) {
  case GL4DH_INIT:
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    drawFin();
    return;
  }
  

}



static void anim6(int state){
  anim = 4;

  switch(state) {
  case GL4DH_INIT:
    return;
  case GL4DH_FREE:
    return;
  case GL4DH_UPDATE_WITH_AUDIO:
    return;
  default: /* GL4DH_DRAW */
    drawMix();
    return;
  }
  

}



static void animationsInit(void) {
  if(!_quadId)
    _quadId = gl4dgGenQuadf();
}
