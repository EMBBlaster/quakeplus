#ifdef _UNIX_
#include "swchpal.h"
#include "unix.h"
#else
#include "swchpal.h"
#endif

// <AWE> added for big endian support.
#include "endianfix.h"

// <AWE> "tolower ()" is defined at "ctype.h" under MacOS X.
#if defined (__APPLE__) || defined (MACOSX)
#include <ctype.h>
#endif /* __APPLE__ || MACOSX */

#ifndef _UNIX_
#include <fltenv.h>
#include <fltpnt.h>
#include <dos.h>
#else
// <AWE> "unistd" is required for "getcwd ()".
#include <unistd.h>
// <AWE> "sys/param.h" is required for MAXPATHLEN.
#include <sys/param.h>
#endif

typedef struct visdat_t{
    char File[32];
    unsigned long len;
    unsigned long vislen;
    unsigned char *visdata;
    unsigned long leaflen;
    unsigned char *leafdata;
};

// <AWE> moved from "swchpal.h" to here.
// <AWE> we would get a nameclash because we need to include it at "endianfix.cpp", too.
int *Line,*Line2,*Page,*Strange;
int DWidth= 800, DHeight= 600,ScreenMode,DDepth=8;
// <AWE> end of moved items.

void loadvis(FILE *fp);
void freevis(void);
int OthrFix(unsigned long Offset, unsigned long Lenght);
FILE *InFile, *OutFile, *fVIS, *VISout;

visdat_t *visdat;

pakheader_t NewPak;
pakentry_t NewPakEnt[2048];
int NPcnt,numvis;

#ifdef _UNIX_
// Directory ptr for findfirst emulation
DIR *dptr=0;
struct dirent *de=0;
struct FIND fe;
regex_t rexp;
char lx_Path[256], lx_File[256];
#endif

int mode = 0,cnt,usepak=0;
char FinBSP[256]="*.BSP", PinPAK[256]="PAK*.PAK", VIS[256]="patch.vis", FoutBSP[256] = "", FoutPak[256] = "Pak*.Pak";
char File[256]="Pak*.Pak",CurName[38],Path[256]="",Path2[256],TempFile[256]="~vistmp.tmp";
char FilBak[256];
struct FIND *entry;
char *path;
long vispos, pakpos;

#ifdef _UNIX_

// This code mainly makes use of Posix compliant calls so
// it should compile under most Unix platforms

void fcloseall()
{
    if (InFile)
        fclose(InFile);
    if (OutFile)
        fclose(OutFile);
    if (fVIS)
        fclose(fVIS);
}

char *strlwr(char *string)
{
char *p=string;


  while(*p)
  {
	*p= (char) tolower(*p);
	++p;
	}

  return(string);
}

char *strrev(char *string)
{
char *p=string;
char swap;
int i = strlen(string)-1;

  while(p < (string+i)) {
   swap = *(string+i);
   *(string+i) = *p;
   *p = swap;
   i--;
   p++;
 }
 return(string);
}

int strcmpi(char *a, char *b)
{
  //printf("Comparing %s and %s\n", a, b);
  return(strcasecmp(a,b));
}

struct FIND *findnext( void )
{
static FIND fe;
char fnBuffer[256];
struct stat status;

  de = readdir( dptr );
  if(de == NULL) return( (struct FIND *) NULL );

  // If doesn't match, return next match
  if(regexec(&rexp, de->d_name, 0, 0, 0)) {
   //printf("%s did not match %s\n", de->d_name, lx_File);
   return(findnext());
  }

  strcpy(fe.name, de->d_name);
  strcpy(fnBuffer, lx_Path);
  strcat(fnBuffer, "/");
  strcat(fnBuffer, fe.name);
  stat( fnBuffer, &status );
  // Filemodes mapped to attributes
  fe.attribute = status.st_mode;
  // Only return files!
  if(S_ISREG(status.st_mode)) {
    //printf("Returning %s\n", fe.name);
    return(&fe);
  }
  else
    return(findnext());

}

int filesize(char *filename)
{
struct stat status;

  if(stat(filename, &status)==-1) return(-1);

  return(status.st_size);
}

// Converts file wildcards to regexp string
char *build_regexp(char *string)
{
char *p = string;
char *pos;
static char newstring[512];
char tempstr[512];

  p = newstring;
  strcpy(newstring, string);
  // First pass: comment any regexp special characters
  while((pos = strpbrk(p, ".+[]()|\\^$")) != NULL) {
   strcpy(tempstr, pos);
   *pos = '\\';
   *(pos+1) = 0;
   strcat(p, tempstr);
   p = pos+2;
  }

  // Then build regexp for ? and *
  p = newstring;
  while((pos = strpbrk(p, "?*")) != NULL) {
   strcpy(tempstr, pos);
   *pos = '.';
   *(pos+1) = 0;
   strcat(p, tempstr);
   p = pos+2;
  }

  //printf("\nWildcard String = %s  RegExp String = %s\n", string, newstring);
  return(newstring);
}

struct FIND *findfirst(char *path, int unknown)
{
char *p;
  
  // Free Static data on second pass
  if(dptr) {
	closedir(dptr);
  	regfree(&rexp);
  }

  p = strrchr(path, '/'); 
  if(!p) {
   //printf("No // in filename\n");
   return(NULL);
  }

  *p++=0;
  strcpy(lx_Path, path);
  strcpy(lx_File, p);
  *(p-1) = '/';

  dptr = opendir(lx_Path);
  if(!dptr) {
   //printf("Bad Dptr: %s\n");
   return(NULL);
  }

  // Compile the regular expression matcher
  regcomp( &rexp, build_regexp(lx_File), REG_EXTENDED | REG_ICASE | REG_NOSUB);

  return(findnext());
}

void _dos_setfileattr(char *filename, mode_t attributes)
{
  chmod(filename, attributes);
}

#endif

// <AWE> why not print the usage on error or "-help"?

void usage (const char *theCmdName, const char *theErrorMsg)
{
    if (theErrorMsg != NULL)
    {
        fprintf (stdout, "\n%s: %s\n%s: Try \'vispatch -help\' for more information.\n",
                 theCmdName, theErrorMsg, theCmdName);
        exit (2);
    }

    fprintf (stdout, "Usage: %s [-help] [-data visfile] [-dir workdir] [-extract|-new] pakfile\n\n"
                     "Files:\n"
                     "  -data visfile: The patchdata to use, default: \"patch.vis\".\n"
                     "  -dir workdir:  The directory that holds the pak file, default: current dir.\n"
                     "  pakfile:       The pak file to use, default: \"PAK*.PAK\".\n"
                     "Mode:\n"
                     "  -extract:      Append the visdata of the pakfile to the visfile.\n"
                     "  -new:          Create a new pakfile instead of overwriting \"pakfile\".\n"
                     "  vispatch will overwrite \"pakfile\" by default. Be carefull!\n", theCmdName);

    exit (0);
}

int main(int argc,char **argv){
    printf("Vis Patch v1.2a by Andy Bay (ABay@Teir.Com)\n");
    printf("Big endian support: Axel 'awe' Wefers (awe@fruitz-of-dojo.de)\n");
    int tmp;
#ifdef _UNIX_
    // <AWE> that's better instead of no dir by default:
    char 	myCurrentDir[MAXPATHLEN];
    
    getcwd (myCurrentDir, MAXPATHLEN);
    snprintf (Path, 255, "%s/", myCurrentDir);
#endif /* _UNIX_ */
    if (argc>1)
        for (tmp=1;tmp<argc;tmp++){
			strlwr(argv[tmp]);
            if (argv[tmp][0]=='-' || argv[tmp][0]=='/'){
                if (argv[tmp][0]=='/') argv[tmp][0]='-';

                if (strcmp(argv[tmp],"-help")==0) {
                    usage (argv[0], NULL);
                }
                if (strcmp(argv[tmp],"-data")==0) {
                    argv[tmp][0]=0;
                    strcpy(VIS,argv[++tmp]);
                    argv[tmp][0]=0;
                    printf("The Vis data source is %s.\n",VIS);
                }

                if (strcmp(argv[tmp],"-dir")==0) {
                    argv[tmp][0]=0;
                    strcpy(Path,argv[++tmp]);
                    argv[tmp][0]=0;
#ifdef _UNIX_
		    if(Path[strlen(Path)-1] != '/') strcat(Path, "/");
#endif
                    printf("The pak/bsp directory is %s.\n",Path);
                }

                if (strcmp(argv[tmp],"-extract")==0) {
                    mode=1;
                    argv[tmp][0]=0;
                    printf("Extracting vis data to VisPatch.dat, auto-append.\n");
                }
                if (strcmp(argv[tmp],"-new")==0) {
                    mode = 2;
                    argv[tmp][0]=0;
                    strcpy(Path2,Path);
                    strcat(Path2,FoutPak);
                    entry = findfirst (Path2,0);
                    cnt = 0;
                   while (entry != NULL)
                   {
                      cnt++;
                      entry = findnext ();
                   }
                    sprintf(FoutPak,"%spak%i.pak",Path,cnt);
                    printf("The new pak file is called %s.\n",FoutPak);
                }


            }
            if (tmp<argc) if (strlen(argv[tmp])) strcpy(File,argv[tmp]);

        }
    //printf("mode: %i\n",mode);
    sprintf(TempFile,"%s%s",Path,"~vistmp.tmp");
    //printf("%s",TempFile);
    if (mode==0||mode == 2) {

        strcpy(Path2,Path);
        strcat(Path2,File);
        strcpy(FilBak,File);
        entry = findfirst (Path2, 0);
        while (entry != NULL){
            strcpy(File,entry->name);
            strcpy(Path2,Path);
            strcat(Path2,File);
            if(entry->attribute&_A_ARCH){
                printf("%s",Path2);
                entry->attribute = entry->attribute - _A_ARCH;
                _dos_setfileattr(Path2,entry->attribute);
            }
            entry = findnext ();
        }
        int chk=0;
        fVIS = fopen(VIS,"rb");
        if (!fVIS) {usage (argv[0], "couldn't find the vis source file.");}//printf("couldn't find the vis source file.\n");exit(2);}
        loadvis(fVIS);
        strcpy(Path2,Path);
        strcat(Path2,FilBak);
        OutFile = fopen(TempFile,"w+b");
        entry = findfirst (Path2, 0);
        cnt = 0;
        while (entry != NULL){
            cnt++;
            strcpy(File,entry->name);
            if(entry->attribute&_A_ARCH){
                entry = findnext ();
                continue;
            }
            strcpy(Path2,Path);
            strcat(Path2,File);
            InFile=fopen(Path2,"rb");
            if (!InFile) {usage (argv[0], "couldn't find the level file.");}//printf("couldn't find the level file.\n");exit(2);}
            chk = ChooseLevel(File,0,100000);
            if(mode == 0){
                NPcnt = 0;
                fclose(OutFile);
                fclose(InFile);
                if(chk>0){
                    remove(Path2);
                    rename(TempFile,Path2);
                }
                OutFile = fopen(TempFile,"w+b");
            }
            else if(usepak == 1)
                fclose(InFile);
            else if(chk > 0){
                //printf("%i\n",chk);
                fclose(OutFile);
                fclose(InFile);
                strcpy(Path2,Path);
                strcat(Path2,File);
                strcpy(File,Path2);
                strrev(File);
                File[0] = 'k';
                File[1] = 'a';
                File[2] = 'b';
                strrev(File);
                remove(File);
                strcpy(Path2,Path);
                strcat(Path2,CurName);
                rename(Path2,File);
                rename(TempFile,Path2);
                OutFile = fopen(TempFile,"w+b");
            }
            else{
                fclose(OutFile);
                fclose(InFile);
                OutFile = fopen(TempFile,"w+b");
            }
            entry = findnext ();

        }
        fcloseall();
        //printf("%s\n",FoutPak);
        if(mode == 2 && usepak == 1){
            //printf("hi\n");
            rename(TempFile,FoutPak);
        }
        freevis();


    }
    else if (mode == 1){
        if(filesize(VIS)==-1)
            fVIS = fopen(VIS,"wb");
        else
            fVIS = fopen(VIS,"r+b");

        strcpy(Path2,Path);
        strcat(Path2,File);
        entry = findfirst (Path2, 0);
        while (entry != NULL){
            strcpy(File,entry->name);
            strcpy(Path2,Path);
            strcat(Path2,File);
            InFile=fopen(Path2,"r+b");
            //printf("hi\n");
            if (!InFile) {usage (argv[0], "couldn't find the level file."); }//printf("couldn't find the level file.\n");exit(2);}
            ChooseFile(File,0,0);
            entry = findnext ();
        }
    }
    return 0;
}

int ChooseLevel(char *FileSpec,unsigned long Offset,long length){
    int tmp=0;

    //printf("Looking at file %s %li %i.\n",FileSpec,length,mode);

    if (strstr(strlwr(FileSpec),".pak")) {printf("Looking at file %s.\n",FileSpec);usepak=1;tmp=PakFix(Offset);}
    else if (length > 50000 && strstr(strlwr(FileSpec),".bsp")) {
        printf("Looking at file %s.\n",FileSpec);
        strcpy(CurName, FileSpec);
        tmp=BSPFix(Offset);
    }
    else if (mode == 0 && Offset > 0)  tmp = OthrFix(Offset, length);
    else if (mode == 2 && Offset > 0)  NPcnt--;

    //if (tmp==0)
        //printf("Did not process the file!\n");
    return tmp;
}

int PakFix(unsigned long Offset){
    long ugh;
    int test;
    pakheader_t Pak;
    test = fwrite (&Pak, sizeof(pakheader_t),1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    fseek(InFile,Offset,SEEK_SET);
    fread (&Pak,sizeof(pakheader_t),1,InFile);
    unsigned long numentry=Pak.dirsize/64;
    pakentry_t *PakEnt=(pakentry_t *) malloc(numentry*sizeof(pakentry_t));
    fseek(InFile,Offset+Pak.diroffset,SEEK_SET);
    fread (PakEnt,sizeof(pakentry_t),numentry,InFile);
    for (unsigned int pakwalk=0;pakwalk<numentry;pakwalk++){
        strcpy((char *) NewPakEnt[NPcnt].filename,(char *) PakEnt[pakwalk].filename);
        strcpy(CurName,(char *) PakEnt[pakwalk].filename);
        NewPakEnt[NPcnt].size = PakEnt[pakwalk].size;
        ChooseLevel((char *)PakEnt[pakwalk].filename,Offset+PakEnt[pakwalk].offset,PakEnt[pakwalk].size);
        NPcnt++;
    }
    free(PakEnt);
    //fseek(OutFile,0,SEEK_END);
    fflush(OutFile);
    Pak.diroffset = ftell(OutFile);
    //printf("PAK diroffset = %li, entries = %i\n",Pak.diroffset,NPcnt);
    Pak.dirsize = 64*NPcnt;
    ugh = ftell(OutFile);
    test = fwrite (&NewPakEnt[0],sizeof(pakentry_t),NPcnt,OutFile); // <AWE> changed &NewPakEnt to &NewPakEnt[0]
    if (test < NPcnt) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    fflush(OutFile);
    //chsize(fileno(OutFile),ftell(OutFile));
    fseek(OutFile,0,SEEK_SET);
    test = fwrite (&Pak, sizeof(pakheader_t),1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    fseek(OutFile,ugh,SEEK_SET);
    return numentry;
}

int OthrFix(unsigned long Offset, unsigned long Length){
    int test;
//    int tmperr=
    fseek(InFile,Offset,SEEK_SET);
    NewPakEnt[NPcnt].offset = ftell(OutFile);
    NewPakEnt[NPcnt].size = Length;
    void *cpy;
    cpy = malloc(Length);
    fread (cpy, 1, Length, InFile);
    test = fwrite (cpy, 1, Length, OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    free(cpy);
    return 1;
}

int BSPFix(unsigned long InitOFFS){

    int test;
    fflush(OutFile);
    NewPakEnt[NPcnt].offset = ftell(OutFile);
    if (NewPakEnt[NPcnt].size==0) NewPakEnt[NPcnt].size=filesize(File);

    //printf("Start: %i\n",NewPakEnt[NPcnt].offset);

    unsigned long here;
//    int tmperr=
    fseek(InFile,InitOFFS,SEEK_SET);
    dheader_t bspheader;
    int tmp=fread(&bspheader, sizeof(dheader_t),1,InFile);
    if (tmp==0) return 0;
    printf("Version of bsp file is: %li\n",bspheader.version);
    printf("Vis info is at %li and is %li long.\n",bspheader.visilist.offset,bspheader.visilist.size);
    char *cpy;
    test = fwrite(&bspheader,sizeof(dheader_t),1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }


    char VisName[38];
    strcpy(VisName,CurName);
    strrev(VisName);
    strcat(VisName,"/");
    VisName[strcspn (VisName, "\\/")] = 0;
    strrev(VisName);
    int good=0;
    here = ftell(OutFile);
    bspheader.visilist.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    //printf("%s %s %i\n",VisName,CurName,good);
    for(tmp = 0;tmp<numvis;tmp++){
        ////("%s  ",
        if(!strcmpi(visdat[tmp].File,VisName)){
            good = 1;
            //printf("Name: %s Size: %li %i\n",VisName,visdat[tmp].vislen,tmp);
            fseek(OutFile,here,SEEK_SET);
            bspheader.visilist.size = visdat[tmp].vislen;
            test = fwrite((void *) visdat[tmp].visdata,1,bspheader.visilist.size,OutFile);
            if (test == 0) {
                printf("Not enough disk space!!!  Failing.");
                fcloseall();
                remove(TempFile);
                freevis();
            }

            fflush(OutFile);
            bspheader.leaves.size   = visdat[tmp].leaflen;
            bspheader.leaves.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
            test = fwrite((void *) visdat[tmp].leafdata,1,bspheader.leaves.size,OutFile);
            if (test == 0) {
                printf("Not enough disk space!!!  Failing.");
                fcloseall();
                remove(TempFile);
                freevis();
            }
        }
    }
    if(good == 0){
        if(usepak == 1) {
            fseek(InFile,InitOFFS, SEEK_SET);
            fseek(OutFile,NewPakEnt[NPcnt].offset,SEEK_SET);
            if(mode == 0){
                char *cpy;
                cpy = (char *) malloc(NewPakEnt[NPcnt].size);
                fread((void *) cpy, NewPakEnt[NPcnt].size,1,InFile);
                test = fwrite((void *) cpy,NewPakEnt[NPcnt].size,1, OutFile);
                if (test == 0) {
                    printf("Not enough disk space!!!  Failing.");
                    fcloseall();
                    remove(TempFile);
                    freevis();
                }
                free(cpy);
                return 1;
            }
            else{
                NPcnt--;
                return 0;
            }
        }
        else
            return 0;//Individual file and it doesn't matter.
        //("not good\n");
        /*cpy = malloc(bspheader.visilist.size);
        fseek(InFile,InitOFFS+bspheader.visilist.offset, SEEK_SET);
        fread(cpy, 1,bspheader.visilist.size,InFile);
        fwrite(cpy,bspheader.visilist.size,1,OutFile);
        free(cpy);

        cpy = malloc(bspheader.leaves.size);
        fseek(InFile,InitOFFS+bspheader.leaves.offset, SEEK_SET);
        fread(cpy, 1,bspheader.leaves.size,InFile);
        bspheader.leaves.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
        fwrite(cpy,bspheader.leaves.size,1,OutFile);
        free(cpy);*/
        //("K: %i\n",ftell(OutFile));

    }

    cpy = (char *) malloc(bspheader.entities.size);
    fseek(InFile,InitOFFS+bspheader.entities.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.entities.size,InFile);
    bspheader.entities.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.entities.size,1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    free(cpy);
    //printf("A: %i %i\n",bspheader.entities.offset,ftell(OutFile));

    cpy = (char *) malloc(bspheader.planes.size);
    fseek(InFile,InitOFFS+bspheader.planes.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.planes.size,InFile);
    bspheader.planes.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.planes.size,1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    free(cpy);
    //printf("B: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.miptex.size);
    fseek(InFile,InitOFFS+bspheader.miptex.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.miptex.size,InFile);
    bspheader.miptex.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.miptex.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    //("C: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.vertices.size);
    fseek(InFile,InitOFFS+bspheader.vertices.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.vertices.size,InFile);
    bspheader.vertices.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.vertices.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    cpy = (char *) malloc(bspheader.nodes.size);
    fseek(InFile,InitOFFS+bspheader.nodes.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.nodes.size,InFile);
    bspheader.nodes.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.nodes.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    cpy = (char *) malloc(bspheader.texinfo.size);
    fseek(InFile,InitOFFS+bspheader.texinfo.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.texinfo.size,InFile);
    bspheader.texinfo.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.texinfo.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("G: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.faces.size);
    fseek(InFile,InitOFFS+bspheader.faces.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.faces.size,InFile);
    bspheader.faces.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.faces.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("H: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.lightmaps.size);
    fseek(InFile,InitOFFS+bspheader.lightmaps.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.lightmaps.size,InFile);
    bspheader.lightmaps.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.lightmaps.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("I: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.clipnodes.size);
    fseek(InFile,InitOFFS+bspheader.clipnodes.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.clipnodes.size,InFile);
    bspheader.clipnodes.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.clipnodes.size,1,OutFile);
    free(cpy);
    //("J: %i\n",ftell(OutFile));


    cpy = (char *) malloc(bspheader.lface.size);
    fseek(InFile,InitOFFS+bspheader.lface.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.lface.size,InFile);	
    bspheader.lface.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.lface.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("L: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.edges.size);
    fseek(InFile,InitOFFS+bspheader.edges.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.edges.size,InFile);
    bspheader.edges.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.edges.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("M: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.ledges.size);
    fseek(InFile,InitOFFS+bspheader.ledges.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.ledges.size,InFile);
    bspheader.ledges.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.ledges.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    //("N: %i\n",ftell(OutFile));

    cpy = (char *) malloc(bspheader.models.size);
    fseek(InFile,InitOFFS+bspheader.models.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.models.size,InFile);
    bspheader.models.offset = ftell(OutFile)-NewPakEnt[NPcnt].offset;
    test = fwrite((void *) cpy,bspheader.models.size,1,OutFile);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    here=ftell(OutFile);
    //("O: %i\n",here);
    fflush(OutFile);

    fseek(OutFile,NewPakEnt[NPcnt].offset, SEEK_SET);
    test = fwrite(&bspheader,sizeof(dheader_t),1,OutFile);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    fseek(OutFile,here, SEEK_SET);
    NewPakEnt[NPcnt].size = ftell(OutFile) - NewPakEnt[NPcnt].offset;

    //("End: %i\n",ftell(OutFile));

return 1;
}

int ChooseFile(char *FileSpec,unsigned long Offset,long length){
    int tmp=0;
    if (length == 0 && strstr(strlwr(FileSpec),".pak")) {
        printf("Looking at file %s.\n",FileSpec);
        tmp=PakNew(Offset);
    }
    if (strstr(strlwr(FileSpec),".bsp")) {
        printf("Looking at file %s.\n",FileSpec);
        strcpy(CurName, FileSpec);
        tmp = BSPNew(Offset);
    }
    return tmp;
}

int PakNew(unsigned long Offset){
    pakheader_t Pak;
    fseek(InFile,Offset,SEEK_SET);
    fread(&Pak,sizeof(pakheader_t),1,InFile);
    unsigned long numentry=Pak.dirsize/64;
    pakentry_t *PakEnt= (pakentry_t *) malloc(numentry*sizeof(pakentry_t));
    fseek(InFile,Offset+Pak.diroffset,SEEK_SET);
    fread(PakEnt,sizeof(pakentry_t),numentry,InFile);
    for (unsigned int pakwalk=0;pakwalk<numentry;pakwalk++){
        strcpy((char *) NewPakEnt[NPcnt].filename,(const char *) PakEnt[pakwalk].filename);
        strcpy(CurName,(const char *) PakEnt[pakwalk].filename);
        NewPakEnt[NPcnt].size = PakEnt[pakwalk].size;
        ChooseFile((char *)PakEnt[pakwalk].filename,Offset+PakEnt[pakwalk].offset,PakEnt[pakwalk].size);
        NPcnt++;
    }
    free(PakEnt);
    return numentry;
}

int BSPNew(unsigned long InitOFFS){
    int test;
    unsigned long tes;
//    int tmperr=
    fseek(InFile,InitOFFS,SEEK_SET);
#if !defined (__APPLE__) && !defined (MACOSX)
    unsigned long len;
#endif /* !__APPLE__ && !MACOSX */
    dheader_t bspheader;
    int tmp=fread(&bspheader, sizeof(dheader_t),1,InFile);
    if (tmp==0) return 0;
    printf("Version of bsp file is:  %li\n",bspheader.version);
    printf("Vis info is at %li and is %li long\n",bspheader.visilist.offset,bspheader.visilist.size);
    printf("Leaf info is at %li and is %li long\n",bspheader.leaves.offset,bspheader.leaves.size);
    char *cpy;

    char VisName[38];
    strcpy(VisName,CurName);
    strrev(VisName);
    strcat(VisName,"/");
    VisName[strcspn (VisName, "/\\")] = 0;
    strrev(VisName);
    cpy = (char *) malloc(bspheader.visilist.size);
    fseek(InFile,InitOFFS+bspheader.visilist.offset, SEEK_SET);
    fread(cpy, 1,bspheader.visilist.size,InFile);
#if defined (__APPLE__) || defined (MACOSX)
    if (filesize(VIS) > -1)
#else
    len = filesize(VIS);
    //("%i\n",len);
    if(len > -1)
#endif /* __APPLE__ ||ÊMACOSX */
        fseek(fVIS,0,SEEK_END);
    test = fwrite(&VisName,1,32,fVIS);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    tes = bspheader.visilist.size+bspheader.leaves.size+8;
    Endian32_Swap (tes);						// <AWE> added for big endian support.
    test = fwrite(&tes,sizeof(long),1,fVIS);
    Endian32_Swap (tes);						// <AWE> added for big endian support.
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    Endian32_Swap (bspheader.visilist.size);				// <AWE> added for big endian support.
    test = fwrite(&bspheader.visilist.size,sizeof(long),1,fVIS);
    Endian32_Swap (bspheader.visilist.size);				// <AWE> added for big endian support.
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    test = fwrite((void *) cpy,bspheader.visilist.size,1,fVIS);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }

    cpy = (char *) malloc(bspheader.leaves.size);
    fseek(InFile,InitOFFS+bspheader.leaves.offset, SEEK_SET);
    fread((void *) cpy, 1,bspheader.leaves.size,InFile);
    Endian32_Swap (bspheader.leaves.size);				// <AWE> added for big endian support.
    test = fwrite(&bspheader.leaves.size,sizeof(long),1,fVIS);
    Endian32_Swap (bspheader.leaves.size);				// <AWE> added for big endian support.
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }
    test = fwrite((void *) cpy,bspheader.leaves.size,1,fVIS);
    free(cpy);
    if (test == 0) {
        printf("Not enough disk space!!!  Failing.");
        fcloseall();
        remove(TempFile);
        freevis();
    }



return 1;
}

void loadvis(FILE *fp){
    unsigned int cnt=0,tmp;
    char Name[32];
    unsigned long go;
    fseek(fp,0,SEEK_END);
    unsigned long len = ftell(fp);
    fseek(fp,0,SEEK_SET);
    while((unsigned long) ftell(fp) < len){
        cnt++;
        fread(Name,1,32,fp);
        fread(&go,1,sizeof(long),fp);
        Endian32_Swap (go);						// <AWE> added for big endian support.
        fseek(fp,go,SEEK_CUR);
    }
    visdat = (visdat_t *) malloc(sizeof(visdat_t)*cnt);
    if(visdat == 0) {printf("Ack, not enough memory!");exit(1);}
    fseek(fp,0,SEEK_SET);
    for(tmp=0;tmp<cnt;tmp++){
        fread((void *) visdat[tmp].File,1,32,fp);
        fread((void *) &visdat[tmp].len,1,sizeof(long),fp);
        Endian32_Swap (visdat[tmp].len);				// <AWE> added for big endian support.
        fread((void *) &visdat[tmp].vislen,1,sizeof(long),fp);
        Endian32_Swap (visdat[tmp].vislen);				// <AWE> added for big endian support.
        //printf("%i\n",  visdat[tmp].vislen);
        visdat[tmp].visdata = (unsigned char *) malloc(visdat[tmp].vislen);
        if(visdat[tmp].visdata == 0) {printf("Ack, not enough memory!");exit(2);}
        fread((void *) visdat[tmp].visdata,1,visdat[tmp].vislen,fp);
        fread((void *) &visdat[tmp].leaflen,1,sizeof(long),fp);
        Endian32_Swap (visdat[tmp].leaflen);				// <AWE> added for big endian support.
        visdat[tmp].leafdata = (unsigned char *) malloc(visdat[tmp].leaflen);
        if(visdat[tmp].leafdata == 0) {printf("Ack, not enough memory!");exit(2);}
        fread((void *)visdat[tmp].leafdata,1,visdat[tmp].leaflen,fp);
    }
    numvis = cnt;
}

void freevis(){
    int tmp;
    for(tmp=0;tmp<numvis;tmp++){
        free(visdat[tmp].visdata);
        free(visdat[tmp].leafdata);
    }
    free(visdat);
}
