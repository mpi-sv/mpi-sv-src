/* File: mandelbrot.c
   Modified by: Jharrod LaFon
   Date: Spring 2011
   Purpose:  Compute and display the Mandelbrot set
   */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define        X_RESN    4   /* x resolution */
//#define		Y_RESN	4   /* y resolution */
#define     M_MAX   1048576       /* Max iterations for Mandelbrot */
// BITMAP header
#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1, reserved2;
    uint32_t offset;
} HEADER;
#pragma pack(pop)


// BITMAP information header
#pragma pack(push, 1)
typedef struct {
    uint32_t size;
    int32_t width, height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t isize;
    int32_t xres, yres;
    uint32_t colors;
    uint32_t impcolors;
} IHEADER;
#pragma pack(pop)

// Pixel 
#pragma pack(push, 1)
typedef struct Pixel {
    uint8_t b, g, r;
} Pixel;
#pragma pack(pop)

// Struct to hold info on a bitmap
#pragma pack(push, 1)
typedef struct {
    HEADER header;
    IHEADER iheader;
    Pixel **array;
} BMP;
#pragma pack(pop)

int get_bmp_header(FILE *fp, BMP *img) {
    if (sizeof((*img).header) != fread(&(*img).header, 1, sizeof((*img).header), fp)) {
        perror("Unable to read file");
        exit(1);
    }
    if (sizeof((*img).iheader) != fread(&(*img).iheader, 1, sizeof((*img).iheader), fp)) {
        perror("Unable to read file");
        exit(1);
    }
    if ((*img).header.type != 0x4D42) {
        printf("Error: Not a valid bitmap file.\n");
        exit(1);
    }
    return 0;
}


int print_bmp_attr(BMP *bmp) {
    int linebits = (*bmp).iheader.width * (*bmp).iheader.bits;
    int linebytes = ((linebits + 31) / 32) * 4;
    printf("File: \n\t\
        Linebytes: %d\n\t\
        Type: %X\n\t\
        Size: %d\n\t\
        Offset: %d\n\t\
        Size: %d\n\t\
        Res1: %d\n\t\
        Res2: %d\n\t\
        Width: %d\n\t\
        Height: %d\n\t\
        Image Size: %u\n\t\
        Xres: %u\n\t\
        Yres: %u\n\t\
        Planes: %u\n\t\
        Bits/Pixel: %u\n\t\
        Compression: %u\n\t\
        Number of colors: %u\n\t\
        Important colors: %u\n", linebytes, (*bmp).header.type, (*bmp).header.size, (*bmp).header.offset,
           (*bmp).iheader.size, (*bmp).header.reserved1, (*bmp).header.reserved2, (*bmp).iheader.width,
           (*bmp).iheader.height, (*bmp).iheader.isize,
           (*bmp).iheader.xres, (*bmp).iheader.yres, (*bmp).iheader.planes, (*bmp).iheader.bits,
           (*bmp).iheader.compression, (*bmp).iheader.colors, (*bmp).iheader.impcolors);
    return 0;
}


typedef struct complextype {
    float real, imag;
} Compl;

typedef enum {
    DATA_TAG, TERM_TAG, RESULT_TAG
} Tags;

Pixel cal_pixel(Compl c, int size, int rank) {
    Pixel result;
    uint64_t count;
    uint64_t colors = 0xFFFFFF / M_MAX;
    int rcolors = 0xFF / size;
    Compl z;
    float temp, lengthsq;
    z.real = 0;
    z.imag = 0;
    count = 0;
    do {
        temp = z.real * z.real - z.imag * z.imag + c.real;
        z.imag = 2 * z.real * z.imag + c.imag;
        z.real = temp;
        lengthsq = z.real * z.real + z.imag * z.imag;
        count++;
    } while ((lengthsq < 4.0) && (count < M_MAX));
    result.b = rank * count * colors & 0xFF;
    result.g = rank * count * colors;
    //result.r = z.real 
    //result.r = rank*rcolors & 0xFF0000;
    result.r = rank * colors % count;
    return result;
}

void worker(int linebytes) {
    Pixel *colors = malloc(linebytes);
    MPI_Status status;
    int row, x, rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    Compl c;
    Pixel xp;
    while (status.MPI_TAG != TERM_TAG) {
        c.imag = ((float) row - 5500) / 3500.0;
        for (x = 0; x < linebytes / 3; x++) {
            c.real = ((float) x - 8000) / 3500.0;
            colors[x] = cal_pixel(c, rank, size);
        }
        MPI_Send(&colors[0], linebytes, MPI_BYTE, 0, row, MPI_COMM_WORLD);
        //   fprintf(stderr,"Worker: %d Row: %d\n",rank,row);
        MPI_Recv(&row, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    }
}

int main(int argc, char **argv) {
    int rank, size;
    if (MPI_Init(&argc, &argv) != MPI_SUCCESS) {
        perror("Unable to initialize MPI\n");
        exit(1);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;
    BMP bmp;
    //block
    bmp.header.type = 0x4D42;
    bmp.iheader.xres = bmp.iheader.yres = 3780;
    bmp.header.reserved1 = bmp.header.reserved2 = 0;
    bmp.iheader.size = 40;
    bmp.iheader.compression = 0;
    bmp.iheader.width = X_RESN;
    bmp.iheader.bits = 24;
    bmp.iheader.colors = 0;
    bmp.iheader.impcolors = 0;


    //block
    FILE *fp = 0;
    if ((fp = fopen("/home/MPISE_Install/CLOUD9_WJ/src/MPISE/benchmarks/bitmap/dot.bmp", "r+")) == NULL) {
        perror("Cannot create file");
        exit(1);
    }
    int linebits = bmp.iheader.width * bmp.iheader.bits;
    int linebytes = ((linebits + 31) / 32) * 4;
    if (rank != 0)
        worker(linebytes);
    else {
        //block
        int Y_RESN;
        klee_make_symbolic(&Y_RESN, sizeof(Y_RESN), "Y_RESN");
        klee_assume(Y_RESN >= 2);
        klee_assume(Y_RESN <= 10);
        bmp.iheader.height = Y_RESN;


        bmp.iheader.planes = 1;
        bmp.iheader.isize = linebytes * bmp.iheader.height;
        bmp.header.offset = 54;
        bmp.header.size = bmp.header.offset + bmp.iheader.isize;

        //block
        //print_bmp_attr(&bmp);
        fseek(fp, 0, SEEK_SET); // to make sure that the point is pointing the beginning of the file
        fwrite(&bmp.header, 1, sizeof(bmp.header), fp);// write from bmp.header to the file fp.
        fwrite(&bmp.iheader, 1, sizeof(bmp.iheader), fp);
        Pixel **newimg = malloc(
                sizeof(Pixel *) * 10); // if I replace  bmp.iheader.height with 10, it can terminate successfully.
        int k, count, row, column, byte;
        Pixel *current;
        for (row = bmp.iheader.height - 1; row >= 0; row--) {
            printf("enter the for loop!!!");
            current = (Pixel *) malloc(linebytes);
            column = 0;
            for (byte = 0; byte < linebytes; byte += 3) {
                fread(&current[column], 1, sizeof(Pixel),
                      fp); //&current[column++] is the address to store,read from fp, size of element is 1 byte, number of element sizeof(Pixel)
            }
            newimg[row] = current;
            fseek(fp, bmp.header.offset + (bmp.iheader.height - 1 - row) * linebytes,
                  SEEK_SET);//move the point to offset of the second parameter
        }
        fclose(fp);
        /* Mandlebrot variables */

        Pixel *colors;
        row = 0;
        double start, end;
        MPI_Status status;
        start = MPI_Wtime();
        do {
            printf("enter the while loop");
            end = MPI_Wtime();
            for (int i = 1; i < size; i++) {
                MPI_Send(&row, 1, MPI_INT, i, DATA_TAG, MPI_COMM_WORLD);
                row++;
                if (row == bmp.iheader.height) {
                    printf("enter the notify loop");
                    for (int j = 1; j < size; j++) { MPI_Send(&j, 1, MPI_INT, j, TERM_TAG, MPI_COMM_WORLD); }
                    break;
                }
            }
        } while (row < bmp.iheader.height);
        for (int i = 0; i < bmp.iheader.height; i++) {
            colors = (Pixel *) malloc(linebytes * sizeof(Pixel));
            MPI_Recv(&colors[0], linebytes, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            newimg[status.MPI_TAG] = colors;
        }
        fprintf(stderr, "Received %d rows\n", row);
        if ((fp = fopen("/home/MPISE_Install/CLOUD9_WJ/src/MPISE/benchmarks/bitmap/dot.bmp", "r+")) == NULL) {
            perror("Cannot create file");
            exit(1);
        }
        if (fseek(fp, bmp.header.offset, SEEK_SET) != 0)
            perror("seek");
        for (row = bmp.iheader.height - 1; row >= 0; row--) {
            if (fwrite(&newimg[row][0], 1, linebytes, fp) == 0)
                perror("fwrite");
        }
        fclose(fp);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}
