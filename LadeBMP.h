#include "stdio.h"
#include "stdlib.h"

#pragma once

unsigned char *loadBMP24(const char *filename, int *width, int *height) {
  printf("Reading image %s\n", filename);
  unsigned char header[54];
  unsigned int imageSize;
  // Die BGR Daten
  unsigned char *data;

  FILE *file = fopen(filename, "rb");
  if (!file) {
    printf("Textur %s wurde nicht gefunden!\n", filename);
    getchar();
    return 0;
  }

  // Dateiheader einlesen
  if (fread(header, 1, 54, file) != 54) {
    printf("Nicht das richte (24bit) Bitmapformat!\n");
    fclose(file);
    return 0;
  }
  // Sind die ersten beiden Bytes "BM"?
  if (header[0] != 'B' || header[1] != 'M') {
    printf("Nicht das richte (24bit) Bitmapformat!\n");
    fclose(file);
    return 0;
  }

  // Darf keine Kompression beinhalten
  if (*(int *)&(header[0x1E]) != 0) {
    printf("Nicht das richte Bitmapformat (keine Kompression erlaubt)!\n");
    fclose(file);
    return 0;
  }
  // Farbtiefe muss bei 24 bit sein
  if (*(int *)&(header[0x1C]) != 24) {
    printf("Nicht das richte (24bit) Bitmapformat!\n");
    fclose(file);
    return 0;
  }

  // Bilddaten ermitteln
  imageSize = *(int *)&(header[0x22]);
  *width = *(int *)&(header[0x12]);
  *height = *(int *)&(header[0x16]);

  // Ergänzugen bei fehlerhaftem Header
  if (imageSize == 0) {
    imageSize = (*width) * (*height) *
                3; // 3 : one byte for each Red, Green and Blue component
  }

  data = (unsigned char *)malloc(sizeof(unsigned char) * imageSize);

  // Lesen der Binärdaten
  fread(data, 1, imageSize, file);

  fclose(file);

  // Pixeldaten zurückgeben
  return data;
}
