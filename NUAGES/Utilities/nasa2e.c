/****************************************************************
*   module: nasa2e.c (nasa contours -> .cnt format)
*
*   date  : 30.7.93
*   author: b.beiger
*
*****************************************************************/



#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <fcntl.h>


#define Min(a,b)  (((a) < (b)) ? (a):(b))
#ifndef FALSE
#define TRUE 1
#define FALSE 0
#endif

#define MAX_CONTOUR   100  /*max nb of contours / plane */

#define MAX_SECTION   1000  /*max nb of xsections */
#define MAX_PT_CT     2000 /* max nb of pts per contour */

#define F_OPEN(fp,fname,x,string) \
  fp = fopen(fname, x); \
    if (fp == NULL) { \
      fprintf(stderr,string,fname); \
      fflush(stderr); \
      exit(1); \
    } \


FILE *fopen(),*fp1,*fp2,*fp3,*fp4,*fp5,*fp6;
float	n;
int	successor =	1;
int	global_nb =	1;
int old_format_out =	FALSE;
int	exit_status =	0;
char    *in_file, *out_file;
int     Max_pt_ct = MAX_PT_CT;
int	Max_section =  MAX_SECTION;
int     *nb_ct;
int     nb_sections;
int     obj_nb;
int     all_objects = TRUE;
float   Section_thickness, Angstroms_per_pixel;

/*****************************************************************/
main(argc,argv)
/*****************************************************************/
int argc; char **argv;

{
  int i, k;
  float angle   = 1.0;
  int filecount = 0;
  char *fnam1;
  char *fnam2;

  if (argc < 3) {
    Usage(argv);
    exit(1);
  }
  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "-obj") == 0) {
      if (++i >= argc) Usage();
      obj_nb = atoi(argv[i]);
      all_objects = FALSE;
      continue;
    }
    if (strcmp(argv[i], "-h") == 0) {
      Usage();
    }
  }

  in_file = argv[1];
  out_file = argv[2];
  F_OPEN(fp1, in_file,"r","cannot open %s\n");
  F_OPEN(fp3, out_file, "w", "cannot open %s\n");

  nb_ct = (int *)malloc(Max_section * sizeof(int));
  if (nb_ct == NULL) {
    fprintf(stderr, "malloc failed for %dkB\n", Max_section * sizeof(int)/1024);
    exit(1);
  }
  nb_sections = scan_nb_sections(fp1);
  fclose(fp1);
  F_OPEN(fp1, in_file,"r","cannot open %s\n");
  read_sections();
  fclose(fp1);
  fclose(fp3);
  exit(0);
}




int read_sections()
{
  int i,j,k, start,new_contour,s,p;
  float z;
  char string[132];
  int end = FALSE;
  int section_nb, num_cts, num_pts;
  int dummy;
  float x,y;
  int nb = 0;

  fprintf(fp3, "S %d\n", nb_sections);
  do {
    i = 'x';
    do {
      i = getc(fp1);
    } while ( i != EOF && (char)i != 'S' );
    if ( i == EOF )
      end = TRUE;
    else {
      ungetc((char)i, fp1);
      fscanf(fp1, "%s", string);
      if ( strcmp(string, "Section" ) == 0 ) {
        fscanf(fp1, "%d", &section_nb);
        printf("section %d\n", section_nb);
        z = ((float)section_nb)*Section_thickness/Angstroms_per_pixel;

        fprintf(fp3, "v %d z %f\n", nb_ct[nb], z);

        fscanf(fp1, "%s", string); /* ========= */
        fscanf(fp1, "%s%d", string, &num_cts); /* Num_contours: */
        for (j=0 ; j< num_cts ; j++) {
          fscanf(fp1, "%s%d", string, &dummy); /* Contour 1 */
          fscanf(fp1, "%s", string); /* ---------------- */
          fscanf(fp1, "%s%d", string, &dummy); /* Object_num: 41 */
          if( all_objects || dummy == obj_nb) {
            printf("contour %d\n", j);
            fprintf(fp3, "{\n");
          }
          fscanf(fp1, "%s%d", string, &num_pts);/* Num_pts */
          for (k=0 ; k< num_pts ; k++) {
            fscanf(fp1,"%f%f",&(x),&(y));
            if( all_objects || dummy == obj_nb) 
              fprintf(fp3,"%.2f %.2f\n",x,y);
          }
          if( all_objects || dummy == obj_nb) 
            fprintf(fp3, "}\n");
        }
        nb++;
      }
    }
  } while (!end);
}

Usage(argv)
char **argv;
{
  printf("Usage: %s in_file out_file [-obj <nb>]\n", argv[0]);
}

scan_nb_sections(fp)
FILE *fp;
{
  int i,j,k, nb = 0, end = FALSE;
  char string[132];
  int num_cts, num_pts, sec_nb, dummy;
  float x,y;
  char *ptr;
  
  do {
    i = 'x';
    do {
      i = getc(fp);
    } while ( i != EOF && (char)i != 'S' );
    if ( i == EOF ) {
      fprintf(stderr,"error: no Section_thickness\n");
      exit(1);
    }
    ungetc((char)i, fp);
    fscanf(fp, "%s", string);
      if ( strcmp(string, "Section_thickness:" ) == 0 ) {
        fscanf(fp, "%f", &Section_thickness);
        end = TRUE;
      }
  } while (!end);
  printf("Section_thickness: %f\n", Section_thickness);
  end =FALSE;
  do {
    i = 'x';
    do {
      i = getc(fp);
    } while ( i != EOF && (char)i != 'A' );
    if ( i == EOF ) {
      fprintf(stderr,"error: no Angstroms_per_pixel\n");
      exit(1);
    }
    ungetc((char)i, fp);
    fscanf(fp, "%s", string);
      if ( strcmp(string, "Angstroms_per_pixel:" ) == 0 ) {
        fscanf(fp, "%f", &Angstroms_per_pixel);
        end = TRUE;
      }
  } while (!end);
  printf("Angstroms_per_pixel: %f\n", Angstroms_per_pixel);
  end =FALSE;
  do {
    i = 'x';
    do {
      i = getc(fp);
    } while ( i != EOF && (char)i != 'S' );
    if ( i == EOF )
      end = TRUE;
    else {
      ungetc((char)i, fp);
      fscanf(fp, "%s", string);
      if ( strcmp(string, "Section" ) == 0 ) {
        if (nb >= Max_section) {
          ptr = (char *)nb_ct;
          Max_section = Max_section + 500;
          nb_ct = (int *)realloc(ptr, Max_section*sizeof(int));
          if (nb_ct == NULL ) {
            fprintf(stderr, "realloc failed \n");
            exit(1);
          }
        }
        fscanf(fp, "%d", &sec_nb);
        nb_ct[nb] = 0;
        
        fscanf(fp, "%s", string); /* ===================== */
        fscanf(fp, "%s%d", string,&num_cts); /* Num_contours: */
        for(j = 0 ; j< num_cts ; j++ ) {
          fscanf(fp, "%s%d", string,&dummy); /* Contour 0 */
          fscanf(fp, "%s", string); /* -------------------------------- */
          fscanf(fp, "%s%d", string,&dummy); /* Object_num: 0 */
          fscanf(fp, "%s%d", string,&num_pts); /* Num_points: 7 */
          if ( strcmp(string, "Num_points:" ) != 0 ) {
            fprintf("error in section %d\n", sec_nb);
            exit(1);
          }
          if( all_objects || dummy == obj_nb)
            nb_ct[nb] = nb_ct[nb]+ num_pts;
          for (k = 0; k< num_pts ; k++) {
            fscanf(fp, "%f%f", &x,&y); /* x y */
          }
        }
        nb++;
      }
    }
  } while (!end);
return(nb);
}
