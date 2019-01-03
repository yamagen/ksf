/*     
 *     ksf.c (Kanji Statistics Filter)
 *     
 *     Hilofumi Yamamoto <yamagen@ucsd.edu>
 *     $Id: ksf.c,v 1.4 2003/07/06 00:55:12 yamagen Exp $
 *     
 *     Hilofumi Yamamoto 03-July-1998 <yamagen@ucsd.edu>
 *     Pipe redirection
 *     Hilofumi Yamamoto 02-July-1998 <yamagen@ucsd.edu>
 *     EUC version. The fraction algorithm is neat.
 *     
 *     Tim Burress       18-Sep.-1991 <burress@twics.co.jp>                 
 *     Examine a specified group of files for JIS kanji codes, recording 
 *     the frequencies of each character.  The JIS processing algorithm  
 *     was originally supplied by <fukumoto@aa.cs.keio.ac.jp>.           
 *     Hacked beyond all belief/recognition by Jason Molenda, Jan 1992 
 *     (molenda@msi.umn.edu).  All comments in this current style were
 *     added by myself; others are the original author's comments
 *     Should now compile on a normal Unix system. (works for me on
 *     an SGI under IRIX 4.0.1 and a Sun under SunOS 4.1.1.)
 *
 */

#include        <stdio.h>

#define	PROG_NAME	"ksf"
#define	USAGE		"% " PROG_NAME " [-ctfevh] file...."
#define	OPTION          "  -c  show only kanji frequency count\n"\
                        "  -t  print only summary table\n"\
                        "  -f  show only kanji fraction data\n"\
                        "  -e  show only table explanation\n"\
                        "  -h  print this help\n"\
                        "  -v  print " PROG_NAME " version"

/*
 *  EUC Character Numerical location
 *
 *  161---FB_EUC-----------------------------------------244
 *  161---FB_KANA-------175|176------FB_KANJI------------244
 *  161---SB_EUC--------------------------------------------254
 *  
 */

#define iseuc(a)	(a >= 161 && a <= 254)
#define iseuckanji(a)	(a >= 176 && a <= 244)

#define FB_SPACE	161
#define SB_SPACE	161
#define FB_EUC_BEGIN	161
#define FB_EUC_END	244

#define FB_PUNC	        161
#define sb_punc(a)	(a >= 162 && a <= 170)

#define fb_symb(a)	(a >= 161 && a <= 162)
#define sb_symb(a)	(a >= 171 && a <= 254)

#define FB_ROMAN_NUM    163
#define sb_num(a)	(a >= 176 && a <= 185)
#define sb_rom(a)	(a >= 193 && a <= 250)

#define FB_HIRAGANA     164
#define FB_KATAKANA     165
#define FB_GREEK	166
#define FB_CYRILLIC     167
#define FB_GRAPHIC      168

#define FB_KANA_BEGIN	161
#define FB_KANA_END	175
#define FB_KANJI_BEGIN	176
#define FB_KANJI_END	244

#define SB_EUC_BEGIN	161
#define SB_EUC_END	254

#define FB_EUC_RANGE	(FB_EUC_END   - FB_EUC_BEGIN   +1) /* 1byte range*/
#define SB_EUC_RANGE	(SB_EUC_END   - SB_EUC_BEGIN   +1) /* 2byte range*/

#define TRUE		1
#define FALSE		0
#define OBOSOLETE 0

/*
 * Flag Control
 */
int TAB = FALSE;
int CAP = FALSE;
int FRA = FALSE;
int CNT = FALSE;

static void usage();
static int opt_show_a, opt_show_b;
static void version(FILE *fp);
int main(int argc, char **argv);

static char
rcsid[] = "$Id: ksf.c,v 1.4 2003/07/06 00:55:12 yamagen Exp $";

typedef struct {
  unsigned int  U,L;
  unsigned long count;
} kanji_record;


typedef struct {
  unsigned long unique_kanji;
  unsigned long kanji_cnt;
  unsigned long hiragana_cnt;
  unsigned long katakana_cnt;
  unsigned long other_cnt;
  unsigned long roman_cnt;
  unsigned long number_cnt;
  unsigned long cyrillic_cnt;
  unsigned long greek_cnt;
  unsigned long punctuation_cnt;
  unsigned long space_cnt;
  unsigned long graphic_cnt;
  unsigned long symbol_cnt;
  unsigned long jis_cnt;
  unsigned long success_files;
} stats_record;

static void usage()
{
  fprintf(stderr, "usage: " USAGE "\n" OPTION "\n");
  exit(1);
}

static void version(FILE *fp)
{
  fprintf(fp, "%s\n",rcsid);
}

int main(int argc, char **argv)
{
  char *format;
  char *op;
  unsigned long kanji_dim[FB_EUC_RANGE][SB_EUC_RANGE];

  void          dump_kanji_stat(), include_file();
  int           index;   /* index into argv */
  stats_record  stats;

  stats.unique_kanji    = 0;
  stats.kanji_cnt       = 0;
  stats.hiragana_cnt    = 0;
  stats.other_cnt       = 0;
  stats.roman_cnt       = 0;
  stats.number_cnt      = 0;
  stats.cyrillic_cnt    = 0;
  stats.greek_cnt       = 0;
  stats.punctuation_cnt = 0;
  stats.space_cnt       = 0;
  stats.graphic_cnt     = 0;
  stats.symbol_cnt      = 0;
  stats.jis_cnt         = 0;
  stats.success_files   = 0;
  index = 1;

  memset(kanji_dim,'\0',sizeof(kanji_dim));

  format = NULL;
  opt_show_a = 'x'; /* default option */
  opt_show_b = 'y'; /* default option */

  for (argv++; *argv && argv[0][0] == '-' && argv[0][1]; argv++) {
    for (op = &argv[0][1]; *op; op++) {
      switch (*op) {
      case 't': TAB = TRUE; break;
      case 'e':	CAP = TRUE; break;
      case 'f': FRA = TRUE; break;
      case 'c':	CNT = TRUE; break;
      case 'v':
				version(stderr);
				exit(0);
				break;
      case 'h':
				usage();
				exit(0);
				break;
      default:
				fprintf(stderr, "%s : invalid option -- %c\n", PROG_NAME, *op);
				usage();
				exit(0);
				break;
      }
    }
  }

/*  printf("file is %s. option is %s\n", *argv, &opt_show); */

  if(!TAB && !FRA && !CNT)
    TAB = CAP = FRA = CNT = TRUE;

  if (*argv == NULL)
    include_file(NULL,  kanji_dim, &stats);
  else
    for (; *argv; argv++)
      include_file(*argv, kanji_dim, &stats);

  dump_kanji_stat(stdout,kanji_dim, &stats); 

  return 0;
}

/*
 *  include_file()
 *
 *  include_file() opens, analyzes, and then closes a given file_name.
 *  It will record the different types of characters and the numbers
 *  encountered in stats, and will keep track of all Kanji encountered
 *  in kanji_array.
 *
 *  Inputs:  file_name, kanji_array, stats
 *  Return:  Nothing
 *
 */

void 
include_file(char *file_name, unsigned long *kanji_array, stats_record *stats)
{
  FILE          *fp;
  int	        done;
  void          classify_kanji();
  unsigned int  ch;
  
  done=FALSE;

  if(file_name == NULL)
    fp = stdin;
  else if ((fp=fopen(file_name, "r")) == NULL)
    fprintf(stderr, "Unable to open '%s'.\n", file_name);

  while (!feof(fp) && !done)
    if ((ch=getc(fp)) != EOF)
      if (iseuc(ch)) classify_kanji(ch, getc(fp), kanji_array, stats);
  fclose(fp);
  if (!done)  stats->success_files++;
}

/*
 *  classify_kanji()
 *
 *  I split this routine out of include_file() because that function was 
 *  just getting too gross and huge.
 *
 *  It will classify a EUC character, record statistics about it, and
 *  update the kanji_array if it is a Kanji. 
 *
 *  Inputs: fb_euc, sb_euc, kanji_array, stats
 *  Returns: nothing
 *  Side-effects:  Potentially modifies kanji_array and/or stats
 *
 *  Note that I just made up the names 'fb_euc' and 'sb_euc'; they may
 *  or may not have any correspondence to which byte is stored first
 *  in the kanji_array.  'fb_euc' is just the first one in the two-char
 *  sequence.  `sb_euc' and `second byte of euc' are just too long.
 *
 */

void classify_kanji(fb_euc, sb_euc, k_array, stats)
     unsigned int   fb_euc,sb_euc;
     unsigned long  *k_array;
     stats_record    *stats;
{
  unsigned long   *ptr;
  stats->jis_cnt++;

  if (fb_euc >= FB_KANJI_BEGIN){
    ptr = k_array+((fb_euc - FB_KANJI_BEGIN)
		   *SB_EUC_RANGE+(sb_euc - SB_EUC_BEGIN));
    if (*ptr == 0)
      stats->unique_kanji++;
    (*ptr)++;
    stats->kanji_cnt++;
  }
  else if(fb_euc == FB_SPACE && sb_euc == SB_SPACE) stats->space_cnt++;
  else if(fb_euc == FB_PUNC  && sb_punc(sb_euc))    stats->punctuation_cnt++;
  else if(fb_symb(fb_euc)    && sb_symb(sb_euc))    stats->symbol_cnt++;
  else if(fb_euc == FB_GREEK)                       stats->greek_cnt++;
  else if(fb_euc == FB_CYRILLIC)                    stats->cyrillic_cnt++;
  else if(fb_euc == FB_GRAPHIC)                     stats->graphic_cnt++;
  else if(fb_euc == FB_HIRAGANA)                    stats->hiragana_cnt++;
  else if(fb_euc == FB_KATAKANA)                    stats->katakana_cnt++;
  else if(fb_euc == FB_ROMAN_NUM){
    if(sb_num(fb_euc))                              stats->number_cnt++;
    if(sb_rom(fb_euc))                              stats->roman_cnt++; 
  }
  else                                              stats->other_cnt++;
}

/*
 *  dump_kanji_stat()
 *
 *  Print copious statistical information to output, based on
 *  the kanji count stats in kanji_array and the general stats
 *  in stats.
 *
 *  Input:  output, kanji_array, stats
 *  Returns:  Nothing
 *  Side-effects:  Sends lots of output to 'output'
 *
 */

void 
dump_kanji_stat(FILE *output, unsigned long *kanji_array,stats_record *stats)
{
  kanji_record	*kanji_vector,*vector_entry,*last_entry;
  double        total_chars,cumulative_percentage,running_total,
                percentage_factor;
  unsigned long kanji_number,single_total,*entry;
  unsigned long	i,j;                     /* index into kanji array */
  int           comparator();
  void          show_fractions();

  /*  
   * This works something like this:
   *    kanji_array is a 2d array of all the possible JIS Kanji codes
   *           its basic element is just a 'unsigned long' which is
   *           the number of Kanji with that code that were scanned
   *    kanji_vector is a 1d array of type 'kanji_record' (which just
   *           contains the JIS kanji code and the number recorded)
   *           whose total size is the number of different (unique)
   *           kanji that were scanned.  E.g. each kanji that was
   *           scanned gets its own kanji_record in kanji_vector
   *
   *    vector_entry is a loop variable that is used to step through
   *           the malloc()'ed kanji_vector space.  (actually a pointer)
   *    entry is a pointer which is used to step through kanji_array
   *    i,j are used to keep the current JIS code accurate (for recording
   *           in kanji_vector
   */

  kanji_vector = (kanji_record *) 
    malloc(sizeof(kanji_record)*stats->unique_kanji);
  vector_entry = kanji_vector;
  entry = kanji_array;

  for(i = FB_KANJI_BEGIN; i <= FB_KANJI_END; i++){
    for(j = SB_EUC_BEGIN; j <= SB_EUC_END; j++){
      if (*entry){
	vector_entry->U = i;
	vector_entry->L = j;
	vector_entry->count = *entry;
	vector_entry++;
      }
      entry++;
    }
  }
  if (stats->unique_kanji > 1)
    qsort(kanji_vector,stats->unique_kanji,sizeof(kanji_record),comparator);
  /* sort them according to total # that occurred */

  /*
   *  Now we print out the sorted list of Kanji with their EUC codes,
   *  how many times they occured, the actual Kanji, and the running
   *  total percentage of Kanji shown so far. 
   *  (although not in that order :-)
   *
   */

  vector_entry = kanji_vector;
  last_entry   = kanji_vector + (stats->unique_kanji - 1);
  kanji_number = 1;
  single_total   = 0;
  running_total = 0.0;
  percentage_factor = 100.0 / stats->kanji_cnt;
  while (vector_entry <= last_entry) {
    running_total += vector_entry->count;
    cumulative_percentage = running_total * percentage_factor;
    
    if(OBOSOLETE)
      fprintf(output,"%d\t%X%X\t%c%c\t%d\t%9.5f\n",
	      kanji_number++,
	      vector_entry->U,	vector_entry->L,
	      vector_entry->U,	vector_entry->L,
	      vector_entry->count,
	      cumulative_percentage);

    if(CNT)
      fprintf(output,"%d\t%c%c\t%d\t%6.4f\n",
	      kanji_number++,
	      vector_entry->U,	vector_entry->L,
	      vector_entry->count,
	      cumulative_percentage);

    if (vector_entry->count == 1)
      single_total++;
    vector_entry++;
  }

  if(CNT && (TAB || FRA) )
    fprintf(output,"\n");
  total_chars = 100.0 / stats->jis_cnt;
  if(TAB){
    fprintf(output,"files        %9d\n", stats->success_files);
    fprintf(output,"kanji(total) %9d\t(%4.1f%%)\n",
	    stats->kanji_cnt, stats->kanji_cnt*total_chars);
    fprintf(output,"kanji(unique)%9d\n", stats->unique_kanji);
    fprintf(output,"kanji(once)  %9d\t(%4.1f%% of unique)\n",
	    single_total,single_total*100.0/stats->unique_kanji);
    fprintf(output,"hiragana     %9d\t(%4.1f%%)\n",
	    stats->hiragana_cnt, stats->hiragana_cnt*total_chars);
    fprintf(output,"katakana     %9d\t(%4.1f%%)\n",
	    stats->katakana_cnt, stats->katakana_cnt*total_chars);
    fprintf(output,"Arabic       %9d\t(%4.1f%%)\n",
	    stats->number_cnt, stats->number_cnt*total_chars);
    fprintf(output,"Roman        %9d\t(%4.1f%%)\n",
	    stats->roman_cnt, stats->roman_cnt*total_chars);
    fprintf(output,"Cyrillic     %9d\t(%4.1f%%)\n",
	    stats->cyrillic_cnt, stats->cyrillic_cnt*total_chars);
    fprintf(output,"Greek        %9d\t(%4.1f%%)\n",
	    stats->greek_cnt, stats->greek_cnt*total_chars);
    fprintf(output,"graphic      %9d\t(%4.1f%%)\n",
	    stats->graphic_cnt, stats->graphic_cnt*total_chars);
    fprintf(output,"punctuation  %9d\t(%4.1f%%)\n",
	    stats->punctuation_cnt, stats->punctuation_cnt*total_chars);
    fprintf(output,"symbols      %9d\t(%4.1f%%)\n",
	    stats->symbol_cnt, stats->symbol_cnt*total_chars);
    fprintf(output,"space        %9d\t(%4.1f%%)\n",
	    stats->space_cnt, stats->space_cnt*total_chars);
    fprintf(output,"other        %9d\t(%4.1f%%)\n",
	    stats->other_cnt, stats->other_cnt*total_chars);
  }

  /*
   *  display percentage-breakdown stats
   */
  if(FRA && TAB)
    fprintf(output,"\n");
  if(FRA)
    show_fractions(output,kanji_vector, stats);
  return;
}

int comparator(A,B)
     kanji_record *A,*B;
{
  return (B->count - A->count);
}

void 
show_fractions( output, kanji_vector, stats )
     FILE         *output;
     kanji_record *kanji_vector;
     stats_record *stats;
{
  kanji_record    *element;
  int		   fraction;
  unsigned long    running_total, kanji_count;
  float            target, one_percent, ten_percent;

  if(CAP){
    fprintf(output,"\n");
    fprintf(output,"Coverage Fractions for Observed Kanji\n");
    fprintf(output,"=====================================\n");
    fprintf(output,"Percentage        Required\n");
    fprintf(output,"of Text           Kanji\n");
    fprintf(output,"-------------------------------------\n");
  }

  ten_percent = (float) stats->kanji_cnt  / 10.0;
  one_percent = (float) stats->kanji_cnt / 100.0;
  target      = ten_percent;
  kanji_count = running_total = 0;
  element     = kanji_vector;

  /*
   * What we do here is set `target' to the current percentage
   * that we're looking for (expressed in total number of occurances).
   * We then go through the kanji_vector (via element) and add up the
   * number of occurances of the kanji as we go along, until we reach
   * target.
   */

  for (fraction = 1; fraction <= 9; fraction++) {
    while (running_total < target){
      running_total += element->count;
      kanji_count++;
      element++;
    }
    fprintf(output,"   %d0              %5d\n",fraction,kanji_count);
    target += ten_percent;
  }

  target -= (ten_percent - one_percent);
    
  for( fraction = 1 ; fraction <= 9 ; fraction++ ){
    while ( running_total < target ){
      running_total += element->count;
      kanji_count++;
      element++;
    }
    fprintf(output,"   9%d              %5d\n",fraction,kanji_count);
    target += one_percent;
  }
  fprintf(output,"  100              %5d\n", stats->unique_kanji);
} /* show_fractions() */

