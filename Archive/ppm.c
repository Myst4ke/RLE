#include "ima.h"
#include <stdio.h>
#include <stdlib.h>

#define INSPIREPAR "RPFELGUEIRAS"
#define CREATOR "JJ"
#define RGB_COMPONENT_COLOR 255

int ImageLoad_PPM(char *filename, Image *img) {
  char d, buff[16];
  FILE *fp;
  int c, rgb_comp_color, size;
  fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  // read image format
  if (!fgets(buff, sizeof(buff), fp)) {
    perror(filename);
    exit(1);
  }

  // check the image format
  if (buff[0] != 'P' || buff[1] != '6') {
    fprintf(stderr, "Invalid image format (must be 'P6')\n");
    exit(1);
  }

  // check for comments
  c = getc(fp);
  while (c == '#') {
    while (getc(fp) != '\n')
      ;
    c = getc(fp);
  }
  ungetc(c, fp);
  // read image size information
  if (fscanf(fp, "%lu %lu", &img->sizeX, &img->sizeY) != 2) {
    fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
    exit(1);
  }

  // read rgb component
  if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
    fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
    exit(1);
  }
  fscanf(fp, "%c ", &d);
  // check rgb component depth
  if (rgb_comp_color != RGB_COMPONENT_COLOR) {
    fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
    exit(1);
  }
  /* allocation memoire */
  size = img->sizeX * img->sizeY * 3;
  img->data = (GLubyte *)malloc((size_t)size * sizeof(GLubyte));
  assert(img->data);

  // read pixel data from file
  if (fread(img->data, (size_t)1, (size_t)size, fp) == 0) {
    fprintf(stderr, "Error loading image '%s'\n", filename);
  }

  upsidedown(img);

  fclose(fp);
  return 1;
}
void imagesave_PPM(char *filename, Image *img) {
  upsidedown(img);

  FILE *fp;
  // open file for output
  fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Unable to open file '%s'\n", filename);
    exit(1);
  }

  // write the header file
  // image format
  fprintf(fp, "P6\n");

  // comments
  fprintf(fp, "# Created by %s\n", CREATOR);

  // image size
  fprintf(fp, "%lu %lu\n", img->sizeX, img->sizeY);

  // rgb component depth
  fprintf(fp, "%d\n", RGB_COMPONENT_COLOR);

  // pixel data
  fwrite(img->data, (size_t)1, (size_t)(3 * img->sizeX * img->sizeY), fp);
  fclose(fp);

  upsidedown(img);
}

void upsidedown(Image *img) {
  /* remettre l image dans le bon sens */
  int b, c, size, sizex;
  GLubyte tmp, *ptrdeb, *ptrfin, *lastline;
  size = img->sizeX * img->sizeY * 3;
  sizex = img->sizeX;
  lastline = img->data + size - sizex * 3;
  for (b = 0; b < img->sizeY / 2; b++) {
    ptrdeb = img->data + b * sizex * 3;
    ptrfin = lastline - (b * sizex * 3);
    for (c = 0; c < 3 * sizex; c++) {
      tmp = *ptrdeb;
      *ptrdeb = *ptrfin;
      *ptrfin = tmp;
      ptrfin++;
      ptrdeb++;
    }
  }
}

/* ------------------------------------------------------------------------------*/
void RLE(Image* img){}

/* Fonction pour afficher tout le contenu d'un vecteur de GLubyte
   GLubyte* -> printf */
void print_byte_vec(GLubyte *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i <= taille; i++) {
    printf("[%u]", vec[i]);
  }
  printf("\n");
}

/* Fonction pour afficher tout le contenu d'un vecteur de char
   char* -> printf */
void print_str_vec(char *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i <= taille; i++) {
    printf("[%u]", (unsigned char)vec[i]);
  }
  printf("\n");
}


/* Fonction pour remplacer tout le contenu d'un vecteur par des 0
   GLubyte*, int -> void */
void clear_vec(GLubyte *vec, int taille) {
  for (int i = 0; i < taille; i++) {
    vec[i] = 0;
  }
}

/* Fonction pour écrire tout le contenu d'un vecteur de GLubyte dans un .txt
   GLubyte* -> fichier.txt */
void write_cpressed(GLubyte* cpressed, int cp_size){
  // Ecriture de l'image dans le fichier texte
  FILE *fichier = NULL;
  fichier = fopen("RLE_cpressed.txt", "w");
  if (fichier != NULL) {
    for (int n = 0; n <= cp_size; n++) {
      fputc((char)cpressed[n], fichier);
    }
    fclose(fichier);
  } else {
    printf("Error : le fichier n'a pas pu être ouvert");
    exit(1);
  }
}

/* Fonction pour lire tout le contenu d'un fichier.txt
   fichier.txt -> GLubyte* */
GLubyte* read_cpressed(int cp_size) {
  FILE* fichier = NULL;
  fichier = fopen("RLE_cpressed.txt", "r");
  GLubyte *rst = malloc(cp_size * sizeof(GLubyte));
  if (fichier != NULL) {
    for (int n = 0; n <= cp_size; n++) {
      rst[n] = (GLubyte)fgetc(fichier);
    }
    printf("\nLecture :\n");
    print_byte_vec(rst, cp_size);
    fclose(fichier);
  } else {
    printf("Error : le fichier n'a pas pu être ouvert");
    exit(1);
  }
  return rst;
}

/* Fonction pour ajouter une partie d'un vecteur de GLubyte à la fin d'un autre
   GLubyte* -> void */
void vec_append(GLubyte* vec, int idx_start, int idx_end, GLubyte* rst, int rst_size){
  //printf("Appending vec of size %d\n", rst_size);
  for(int i = idx_start; i < idx_end; i++){
    //printf("[%d]", vec[i]);
    rst_size++; 
    rst[rst_size] = vec[i];
  }
  //printf("\n");
}





GLubyte *decompress(GLubyte *cpressed, int cp_size, int nb_color) {
  GLubyte *decompImg = malloc(nb_color * sizeof(GLubyte));
  GLubyte *finalImg = malloc(nb_color * sizeof(GLubyte));
  int decomp_size = -1;
  for (int n = 0; n <= cp_size; n++) {
    char multipl = (char)cpressed[n];
    if (multipl <= 0) {
      vec_append(cpressed, n, n + (multipl * -1), decompImg, decomp_size);
      decomp_size += (multipl * -1);
      n += multipl * -1;

    } else {
      for (int x = 0; x < multipl; x++) {
        decomp_size++;
        decompImg[decomp_size] = cpressed[n+1];
      }
      n++;
    }
  }

  // La taille a augmentée dans la dernière itération de la boucle
  decomp_size--;
  printf("\nDecomp Img :\n");
  print_byte_vec(decompImg, nb_color - 1);

  int nb_pixel = nb_color/3;
  int finalSize = 0;
  //printf("nb pixel = %d\n", nb_pixel);
  for(int x = 0; x < nb_pixel; x++) {
    //printf("x = %d\n", x);
    /* printf("[%d -> %d]\n", finalSize, decompImg[x]);
    printf("[%d -> %d]\n", finalSize + 1, decompImg[x + nb_pixel]);
    printf("[%d -> %d]\n", finalSize + 2, decompImg[x + nb_pixel * 2]); */

    finalImg[finalSize] = decompImg[x];
    finalImg[finalSize + 1] = decompImg[x + nb_pixel];
    finalImg[finalSize + 2] = decompImg[x + nb_pixel * 2];
    finalSize += 3;
  }

  /* for(int j = 0; j < nb_pixel; j+=3) {
    for (int x = 0; x < 3; x++) {
      finalImg[j*x] = decompImg[x];
      finalImg[x + 1] = decompImg[x + nb_pixel];
      finalImg[x + 2] = decompImg[x + nb_pixel * 2];
    }
  } */

  printf("\nFinal Img :\n");
  print_byte_vec(finalImg, finalSize-1);
  return finalImg;
}

void RLE_intel(Image *img) {
  int i, nb_pixels, cp_size = -1, mul_dist = 1;
  nb_pixels = img->sizeX * img->sizeY;
  int nb_color = nb_pixels * 3;
  GLubyte *crt;
  GLubyte *cpressed = malloc((nb_color + nb_color / 128) * sizeof(GLubyte));
  print_byte_vec(img->data, nb_color-1);

  for(int rgb = 0; rgb < 3; rgb++){
    for (crt = img->data, i = rgb; i < nb_color; i += 3) {
      //printf("i = %d\n", i);
      //Ajout de la première couleur et du premier multiplicateur
      if (i < 1) {
        //printf("start\n");
        cp_size++;
        cpressed[cp_size] = 1;
        cp_size++;
        cpressed[cp_size] = crt[i];
      } else {
        //cf : fichier Liste_des_test.txt
        //printf("%d == %d\n", cpressed[cp_size], crt[i]);
        if (cpressed[cp_size] == crt[i]) {
          if (cpressed[cp_size - mul_dist] == 1) { // 1
            //printf("1\n");
            cpressed[cp_size - mul_dist] ++;

          } else if (cpressed[cp_size - mul_dist] >= 128) { // 2
            //printf("2\n");
            cpressed[cp_size - mul_dist]++;
            mul_dist = 1;

            cpressed[cp_size] = 2;
            cp_size++;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - 1] == 127) { // 3
            //printf("3\n");
            cp_size += 2;
            mul_dist = 1;
            cpressed[cp_size - 1] = 1;
            cpressed[cp_size] = crt[i];
          } else {
            //printf("default\n");
            cpressed[cp_size - mul_dist]++;
          }
        } else {
          if (cpressed[cp_size - mul_dist] == 1) { // 4
            //printf("4\n");
            cpressed[cp_size - 1] = 254;
            cp_size++, mul_dist++;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - mul_dist] <= 128) { // 5
            //printf("5\n");
            cp_size += 2;
            mul_dist = 1;
            cpressed[cp_size - 1] = 1;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - mul_dist] > 128) { // 6
            //printf("6\n");
            cpressed[cp_size - mul_dist]--;
            cp_size++, mul_dist++;
            cpressed[cp_size] = crt[i];
          }
        }

      }
      /* print_byte_vec(cpressed, cp_size);
      printf("\n"); */
    }
  }
  printf("\nImage compressée :\n");
  print_byte_vec(cpressed, cp_size);

  

  
  printf("\nDécompression : \n");
  GLubyte* finalImg;
  finalImg = decompress(cpressed, cp_size, nb_color);
  
  Image *image = malloc(sizeof(Image));
  image->sizeX = img->sizeX;
  image->sizeY = img->sizeY;
  image->data = finalImg;
  
  imagesave_PPM("finalImage.ppm", image);
}


void sobel(Image *img) {}

void gris(Image *img) {}
