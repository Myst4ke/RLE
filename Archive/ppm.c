#include "ima.h"

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

/* ------------------------------------------------------------------------------
 */

struct Pixel {
  int mult;
  GLubyte RGB[3];
};
typedef struct Pixel pixel;
/* //Affichage de tout les pixels
  for (crt = img->data, i = 0; i < nb_pixels; i++) {
    if(i % 3 == 0){
      printf("\n");
    }
    printf("%d ", crt[i]);
  } */
/* GLubyte *pixel[3];
    pixel[0] = 0; */
// GLubyte *rst = malloc(nb_pixels * sizeof(GLubyte));
void print_byte_vec(GLubyte *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i <= taille; i++) {
    printf("[%u]", vec[i]);
  }
  printf("\n");
}
void print_int_vec(int *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i < taille; i++) {
    printf("[%d]", vec[i]);
  }
  printf("\n");
}

void clear_vec(GLubyte *vec, int taille) {
  for (int i = 0; i < taille; i++) {
    vec[i] = 0;
  }
}

void RLE(Image *img) {
  int i, nb_pixels, cp_size = 0, idx_size = 1, iterator = 0;
  nb_pixels = img->sizeX * img->sizeY;
  GLubyte *crt;
  GLubyte *cpressed = malloc(nb_pixels * 3 * sizeof(GLubyte));
  int *index = malloc(nb_pixels * sizeof(int));
  index[0] = 1;
  for (crt = img->data, i = 0; i < nb_pixels * 3; i++) {
    // si le premier pixel est ajouté
    if (i > 2) {
      if (cpressed[cp_size - 3] == crt[i] &&
          cpressed[cp_size - 2] == crt[i + 1] &&
          cpressed[cp_size - 1] == crt[i + 2]) {

        printf("(%d, %d, %d) = (%d, %d, %d)\n", cpressed[cp_size - 3],
               cpressed[cp_size - 2], cpressed[cp_size - 1], crt[i], crt[i + 1],
               crt[i + 2]);
        print_int_vec(index, idx_size);
        print_byte_vec(cpressed, cp_size);
        index[iterator] += 1;
        i += 2;
      } else {

        printf("(%d, %d, %d) != (%d, %d, %d)\n", cpressed[cp_size - 3],
               cpressed[cp_size - 2], cpressed[cp_size - 1], crt[i], crt[i + 1],
               crt[i + 2]);
        print_int_vec(index, idx_size);
        print_byte_vec(cpressed, cp_size);

        iterator++;
        idx_size++;
        index[iterator] = 1;
        cpressed[cp_size] = crt[i];
        cpressed[cp_size + 1] = crt[i + 1];
        cpressed[cp_size + 2] = crt[i + 2];
        cp_size += 3;
        i += 2;
      }
      printf("\n");
    } else {
      // on ajoute le premiers pixel
      cpressed[i] = crt[i];
      printf("Cpressed n°%d = %u\n", cp_size, cpressed[i]);
      cp_size++;
    }
  }
  printf("Yeah finished ! Final result :\n");
  print_int_vec(index, idx_size);
  print_byte_vec(cpressed, cp_size);

  FILE *fichier = NULL;
  fichier = fopen("test.txt", "w+");
  if (fichier != NULL) {
    // On lit et on écrit dans le fichier
    // ...
    fclose(fichier); // On ferme le fichier qui a été ouvert
  }
}

// Regrouper les couleurs entre elles
// Check du multiple quand c'est négatif
// Check limite a 127
void RLE_intel(Image *img) {
  int i, nb_pixels, cp_size = -1, mul_dist = 1;
  nb_pixels = img->sizeX * img->sizeY;
  int nb_color = nb_pixels * 3;
  GLubyte * crt;
  printf("Malloc de %d\n", (nb_color + nb_color / 128));
  GLubyte *cpressed = malloc((nb_color + nb_color / 128) * sizeof(GLubyte));
  for(int rgb = 0; rgb < 3; rgb++){
    for (crt = img->data, i = rgb; i < nb_color; i += 3) {
      if (i < 3) {
        cp_size++;
        cpressed[cp_size] = 0;
        cp_size++;
        cpressed[cp_size] = crt[i];
      } else {
        if (cpressed[cp_size] == crt[i]) {
          if (cpressed[cp_size - mul_dist] == 0) { // 1
            cpressed[cp_size - mul_dist] += 2;

          } else if (cpressed[cp_size - mul_dist] >= 128) { // 2
            cpressed[cp_size - mul_dist]++;
            mul_dist = 1;

            cpressed[cp_size] = 2;
            cp_size++;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - 1] == 127) { // 3
            cp_size += 2;
            mul_dist = 1;
            cpressed[cp_size - 1] = 0;
            cpressed[cp_size] = crt[i];
          } else {
            cpressed[cp_size - mul_dist]++;
          }
        } else {
          if (cpressed[cp_size - mul_dist] == 0) { // 4
            cpressed[cp_size - 1] = 254;
            cp_size++, mul_dist++;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - mul_dist] <= 128) { // 5
            cp_size += 2;
            mul_dist = 1;
            cpressed[cp_size - 1] = 0;
            cpressed[cp_size] = crt[i];

          } else if (cpressed[cp_size - mul_dist] > 128) { // 6
            cpressed[cp_size - mul_dist]--;
            cp_size++, mul_dist++;
            cpressed[cp_size] = crt[i];
          }
        }
      }
    }
  }
  printf("Finished\n");
  print_byte_vec(cpressed, cp_size);
}

void sobel(Image *img) {}

void gris(Image *img) {}
