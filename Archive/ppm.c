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

/* -------------------- Gestion vecteur -------------------------*/

/* Fonction pour afficher tout le contenu d'un vecteur de GLubyte
 */
void print_byte_vec(GLubyte *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i <= taille; i++) {
    printf("[%u]", vec[i]);
  }
  printf("\n");
}

/* Fonction pour afficher tout le contenu d'un vecteur de char
 */
void print_str_vec(char *vec, int taille) {
  printf("taille : %d\n", taille);
  for (int i = 0; i <= taille; i++) {
    printf("[%u]", (unsigned char)vec[i]);
  }
  printf("\n");
}

/* Fonction pour remplacer tout le contenu d'un vecteur par des 0
 */
void clear_vec(GLubyte *vec, int taille) {
  for (int i = 0; i < taille; i++) {
    vec[i] = 0;
  }
}

/* Fonction pour ajouter une partie d'un vecteur de GLubyte à la fin d'un autre
 */
void vec_append(GLubyte *vec, int idx_start, int idx_end, GLubyte *rst,
                int rst_size) {
  for (int i = idx_start; i < idx_end; i++) {
    rst_size++;
    rst[rst_size] = vec[i];
  }
}

// ------------------ Lecture / Ecriture --------------------

/* Fonction pour écrire tout le contenu d'un vecteur de GLubyte dans un .txt
 */
void write_cpressed(GLubyte *cpressed, int cp_size) {
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
 */
GLubyte *read_cpressed(int cp_size) {
  FILE *fichier = NULL;
  fichier = fopen("RLE_cpressed.txt", "r");
  GLubyte *rst = malloc(cp_size * sizeof(GLubyte));
  if (fichier != NULL) {
    for (int n = 0; n <= cp_size; n++) {
      rst[n] = (GLubyte)fgetc(fichier);
    }
    //printf("\nLecture :\n");
    fclose(fichier);
  } else {
    printf("Error : le fichier n'a pas pu être ouvert");
    exit(1);
  }
  return rst;
}

// --------------------- Compression/Décompression ---------------------

/* Fonction qui trie les 3 couleurs
 */
GLubyte *sortRGB(GLubyte *data, int size) {
  GLubyte *sorted = malloc(size * sizeof(GLubyte));
  int sortedSize = 0;
  for (int rgb = 0; rgb < 3; rgb++) {
    for (int i = rgb; i < size; i += 3) {
      sorted[sortedSize] = data[i];
      sortedSize++;
    }
  }
  /* printf("\nSorted data :\n");
  print_byte_vec(sorted, size - 1);
  printf("\n"); */
  return sorted;
}

/* Fonction qui compresse un tableau trié de couleurs
 */
GLubyte *compressRGBopti(GLubyte *data, int size, int *cp_size) {
  GLubyte *cpressed = malloc((size + size/ 128) * sizeof(GLubyte));
  int mul_pos = 0;
  for (int i = 0; i < size; i++) {
    // Ajout de la première couleur et du premier multiplicateur
    if (i < 1) {
      cpressed[*cp_size] = 1;
      *cp_size += 1;
      cpressed[*cp_size] = data[i];
    } else {
      // cf : fichier Liste_des_test.txt
      if (cpressed[*cp_size] == data[i]) {
        if (cpressed[mul_pos] == 1) { // 1
          cpressed[mul_pos]++;

        } else if (cpressed[mul_pos] > 128) { // 2
          // Le test ne doit pas s'effectuer au dernier elem
          //  Test du cas posant problème
          if (cpressed[*cp_size] == data[i + 1] && i < size - 1) {
            cpressed[mul_pos]++;
            mul_pos = *cp_size;

            cpressed[*cp_size] = 2;
            *cp_size += 1;
            cpressed[*cp_size] = data[i];
          } else {
            cpressed[mul_pos]--;
            *cp_size += 1;
            cpressed[*cp_size] = data[i];
          }
        } else if (cpressed[mul_pos] == 128) { // 3
          cpressed[mul_pos]++;
          mul_pos = *cp_size;

          cpressed[*cp_size] = 2;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] == 127) { // 4
          *cp_size += 2;
          mul_pos = *cp_size - 1;
          cpressed[*cp_size - 1] = 1;
          cpressed[*cp_size] = data[i];
        } else { // 5
          cpressed[mul_pos]++;
        }
      } else {
        if (cpressed[mul_pos] == 1) { // 6
          cpressed[mul_pos] = 254;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] <= 128) { // 7
          *cp_size += 2;
          mul_pos = *cp_size - 1;
          cpressed[*cp_size - 1] = 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] > 128) { // 8
          cpressed[mul_pos]--;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];
        }
      }
    }
  }
  /* printf("Compressed data:\n");
  print_byte_vec(cpressed, *cp_size);
  printf("\n"); */
  return cpressed;
}
GLubyte *compressRGB(GLubyte *data, int size, int *cp_size) {
  GLubyte *cpressed = malloc((size + size) /*  / 128 */ * sizeof(GLubyte));
  int mul_pos = 0;
  for (int i = 0; i < size; i++) {
    // Ajout de la première couleur et du premier multiplicateur
    if (i < 1) {
      cpressed[*cp_size] = 1;
      *cp_size += 1;
      cpressed[*cp_size] = data[i];
    } else {
      // cf : fichier Liste_des_test.txt
      if (cpressed[*cp_size] == data[i]) {
        if (cpressed[mul_pos] == 1) { // 1
          cpressed[mul_pos]++;

        } else if (cpressed[mul_pos] >= 128) { // 2
          cpressed[mul_pos]++;
          mul_pos = *cp_size;

          cpressed[*cp_size] = 2;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] == 127) { // 4
          *cp_size += 2;
          mul_pos = *cp_size - 1;
          cpressed[*cp_size - 1] = 1;
          cpressed[*cp_size] = data[i];
        } else { // 5
          cpressed[mul_pos]++;
        }
      } else {
        if (cpressed[mul_pos] == 1) { // 6
          cpressed[mul_pos] = 254;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] <= 128) { // 7
          *cp_size += 2;
          mul_pos = *cp_size - 1;
          cpressed[*cp_size - 1] = 1;
          cpressed[*cp_size] = data[i];

        } else if (cpressed[mul_pos] > 128) { // 8
          cpressed[mul_pos]--;
          *cp_size += 1;
          cpressed[*cp_size] = data[i];
        }
      }
    }
  }
  /* printf("Compressed data:\n");
  print_byte_vec(cpressed, *cp_size);
  printf("\n"); */
  return cpressed;
}
GLubyte *compressRGBnaif(GLubyte *data, int size, int *cp_size) {
  GLubyte *cpressed = malloc((size*3) /*  / 128 */ * sizeof(GLubyte));
  int mul_pos = 0;
  for (int i = 0; i < size; i++) {
    if (i < 1) {
      cpressed[*cp_size] = 1;
      *cp_size += 1;
      cpressed[*cp_size] = data[i];
    } else {
      if (cpressed[*cp_size] == data[i]) {
        if (cpressed[mul_pos] < 254) {
          cpressed[mul_pos]++;
        } else {
          *cp_size += 2;
          mul_pos = *cp_size - 1;
          cpressed[*cp_size - 1] = 1;
          cpressed[*cp_size] = data[i];
        }
      } else {
        *cp_size += 2;
        mul_pos = *cp_size - 1;
        cpressed[*cp_size - 1] = 1;
        cpressed[*cp_size] = data[i];
      }
    }
  }
  /* printf("Compressed data:\n");
  print_byte_vec(cpressed, *cp_size);
  printf("\n"); */
  return cpressed;
}
/* Fonction qui décompresse un vecteur de GLubyte
 */
GLubyte *decompressRGB(GLubyte *cpressed, int cp_size, int nb_color) {
  GLubyte *decompImg = malloc(nb_color * sizeof(GLubyte));
  int decomp_size = -1;
  for (int n = 0; n <= cp_size; n++) {
    char multipl = (char)cpressed[n];
    if (multipl <= 0) {
      vec_append(cpressed, n + 1, n + 1 + (multipl * -1), decompImg,
                 decomp_size);
      decomp_size += (multipl * -1);
      n += multipl * -1;

    } else {
      for (int x = 0; x < multipl; x++) {
        decomp_size++;
        decompImg[decomp_size] = cpressed[n + 1];
      }
      n++;
    }
  }

  // La taille a augmentée dans la dernière itération de la boucle
  /* decomp_size--;
  printf("Decomp Img :\n");
  print_byte_vec(decompImg, nb_color - 1); */
  return decompImg;
}

/* Fonction qui recréé les données de l'image
à partir d'un tableau trié de couleurs
*/
GLubyte *decomp_toRGB(GLubyte *decompImg, int nb_color) {
  GLubyte *finalImg = malloc(nb_color * sizeof(GLubyte));
  int nb_pixel = nb_color / 3;
  int finalSize = 0;
  for (int x = 0; x < nb_pixel; x++) {
    finalImg[finalSize] = decompImg[x];
    finalImg[finalSize + 1] = decompImg[x + nb_pixel];
    finalImg[finalSize + 2] = decompImg[x + nb_pixel * 2];
    finalSize += 3;
  }

  /* printf("\nFinal Img :\n");
  print_byte_vec(finalImg, finalSize - 1); */
  return finalImg;
}

void RLE(Image *img) {
  int nb_pixels, cp_size = 0;
  nb_pixels = img->sizeX * img->sizeY;
  int nb_color = nb_pixels * 3;
  /* printf("Image data:\n");
  print_byte_vec(img->data, nb_color - 1); */

  GLubyte *sorted;
  sorted = sortRGB(img->data, nb_color);

  int *ref_size = &cp_size;
  GLubyte *cpressed;
  cpressed = compressRGB(sorted, nb_color, ref_size);

  write_cpressed(cpressed, cp_size);
  GLubyte *readImg = read_cpressed(cp_size);

  GLubyte *decompImg;
  decompImg = decompressRGB(readImg, cp_size, nb_color);

  GLubyte *finalImg;
  finalImg = decomp_toRGB(decompImg, nb_color);

  Image *image = malloc(sizeof(Image));
  image->sizeX = img->sizeX;
  image->sizeY = img->sizeY;
  image->data = finalImg;

  imagesave_PPM("finalImage.ppm", image);
}

void RLE_opti(Image *img) {
  int nb_pixels, cp_size = 0;
  nb_pixels = img->sizeX * img->sizeY;
  int nb_color = nb_pixels * 3;
  /* printf("Image data:\n");
  print_byte_vec(img->data, nb_color - 1); */

  GLubyte *sorted;
  sorted = sortRGB(img->data, nb_color);

  int *ref_size = &cp_size;
  GLubyte *cpressed;
  cpressed = compressRGBopti(sorted, nb_color, ref_size);

  write_cpressed(cpressed, cp_size);
  GLubyte *readImg = read_cpressed(cp_size);

  GLubyte *decompImg;
  decompImg = decompressRGB(readImg, cp_size, nb_color);

  GLubyte *finalImg;
  finalImg = decomp_toRGB(decompImg, nb_color);

  Image *image = malloc(sizeof(Image));
  image->sizeX = img->sizeX;
  image->sizeY = img->sizeY;
  image->data = finalImg;

  imagesave_PPM("finalImage.ppm", image);
}

void RLE_naif(Image *img) {
  int nb_pixels, cp_size = 0;
  nb_pixels = img->sizeX * img->sizeY;
  int nb_color = nb_pixels * 3;
  printf("Image data:\n");
  // print_byte_vec(img->data, nb_color - 1);

  GLubyte *sorted;
  sorted = sortRGB(img->data, nb_color);

  int *ref_size = &cp_size;
  GLubyte *cpressed;
  cpressed = compressRGBnaif(sorted, nb_color, ref_size);

  write_cpressed(cpressed, cp_size);
  GLubyte *readImg = read_cpressed(cp_size);

  GLubyte *decompImg;
  decompImg = decompressRGB(readImg, cp_size, nb_color);

  GLubyte *finalImg;
  finalImg = decomp_toRGB(decompImg, nb_color);

  Image *image = malloc(sizeof(Image));
  image->sizeX = img->sizeX;
  image->sizeY = img->sizeY;
  image->data = finalImg;

  imagesave_PPM("finalImage.ppm", image);
}