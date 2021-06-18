//comando compilação: gcc main.c -o main -lpthread -lm

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#define COLOR_SIZE 3

unsigned char *fileColors;

// Nome do arquivo da imagem
const char* filename = "output.ppm";

// Resolução da imagem
const int xres = 1000;

// Número máximo de iterações
const uint16_t maxCount = 200;

// Tamanho da janela
const double xmin = -1.0;  
const double xmax = 1.0;
const double ymin = -1.0;
const double ymax = 1.0;

// Número de threads
const int numThreads = 50;

// Estrutura da thread
typedef struct{
    int xres;
    double xmin;
    double ymin;
    double ymax;
    uint16_t maxCount;
    int counter;
    int threadStart;
    int threadEnd;
    double dx;
    double dy;
}thread_arg, *ptr_thread_arg;

void *calc_mandelbrot_fractal(void *arg){
    
    ptr_thread_arg targ = (ptr_thread_arg)arg;
    
    int xres = targ->xres;
    double xmin = targ->xmin;
    double ymin = targ->ymin;
    double ymax = targ->ymax;
    uint16_t maxCount = targ->maxCount;
    int counter = targ->counter;
    int threadStart = targ->threadStart;
    int threadEnd = targ->threadEnd;
    double dx = targ->dx;
    double dy = targ->dy;
        
    double x, y; /* Cordenadas do ponto atual no plano. */
    double u, v; /* Cordenadas do ponto iterado. */
    int i,j; /* contadores do pixel */
    int k; /* contador de iteração */
    
    for (j = threadStart; j < threadEnd; j++) { // iterando no eixo Y o número de pixels correspondente ao quadrante da thread
        
      y = ymax - j * dy; // calcula o altura imaginária do pixel
      for(i = 0; i < xres; i++) { // iterando no eixo X o número de pixels correspondente ao quadrante da thread
        double u = 0.0;
        double v = 0.0;
        double u2 = pow(u, 2);
        double v2 = pow(v, 2);
        x = xmin + i * dx; // calcula a largura imaginário do pixel
        /* itera o ponto */
        for (k = 1; k < maxCount && (u2 + v2 < 4.0); k++) { // itera k até o número máximo de iterações definida ou até alcançar o limite da janela
          v = 2 * u * v + y;
          u = u2 - v2 + x;
          u2 = pow(u, 2);
          v2 = pow(v, 2);
        };
        /* Salva cor do pixel*/
        if (k >= maxCount) {
          /* Cor interior */
          fileColors[counter++] = 0;
          fileColors[counter++] = 255;
          fileColors[counter++] = 0;
        }
        else {
          /* Cor exterior */
          fileColors[counter++] = k & 255;
          fileColors[counter++] = 0;
          fileColors[counter++] = 0;
        }
      }
    }
    return NULL;
  }
  

int main(int argc, char* argv[])
{
  /* Calcula altura da imagem */
  int yres = (xres*(ymax-ymin))/(xmax-xmin);
  int arraySize = yres * xres * COLOR_SIZE;
    
  /* Aloca array de cores PPM para salvar no arquivo */
  fileColors = (unsigned char *)malloc(arraySize * sizeof(unsigned char));

    /* Calcula o tamanho do pixel */
    double dx=(xmax-xmin)/xres;
    double dy=(ymax-ymin)/yres;
        
    //Inicializa threads
    pthread_t threads[numThreads-1];
    thread_arg threadArgs[numThreads];

    int i, j;
        
    //Inicializa estrutura de cada thread
    for (i = 0; i < numThreads; i++){
        threadArgs[i].xres = xres;
        threadArgs[i].xmin = xmin;
        threadArgs[i].ymin = ymin;
        threadArgs[i].ymax = ymax;
        threadArgs[i].maxCount = maxCount;
        threadArgs[i].dx = dx;
        threadArgs[i].dy = dy;
        threadArgs[i].threadStart = (yres/numThreads) * i;
        threadArgs[i].threadEnd = (yres/numThreads) * (i+1);
        threadArgs[i].counter = threadArgs[i].threadStart * xres * COLOR_SIZE;
    }
            
    threadArgs[numThreads-1].threadEnd = yres;
    
    //Computa os escravos
    for (i = 0; i < numThreads-1; i++){
        pthread_create(&(threads[i]), NULL, calc_mandelbrot_fractal, &(threadArgs[i+1]));
    }
        
    // Computa a master
    calc_mandelbrot_fractal(&(threadArgs[0]));
    
    for (i = 0; i < numThreads-1; i++){
      pthread_join(threads[i], NULL);
    }
            
    //Escreve o resultado no arquivo
    // Escreve o header ASCII no arquivo*/
    FILE * fp = fopen(filename,"wb");
    fprintf(fp,"P6\n%d\n%d\n%d\n", xres, yres, 255);

    unsigned char color[COLOR_SIZE];
    
    for (i = 0; i < arraySize; ){
      for (j = 0; j < COLOR_SIZE; j++){
        color[j] = fileColors[i++];
      }
      fwrite(color, COLOR_SIZE, 1, fp);
    }
            
  fclose(fp);
  free(fileColors);
  return 0;
}

