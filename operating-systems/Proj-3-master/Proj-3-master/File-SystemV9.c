/*
\\\\\\\\\\\\                                                              >
==========================================================================>>>>
////////////                                                              >
STUDENTS    : Zachary Berrhill, Chris Lopes, Jean Patrick Roquebert.
CLASS       : COP 4610 Intro To Operating Systems.
ASSIGNMENT  : Project 3
DATE        : 03/04/2020
*/

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>


/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/
unsigned char cleanTokens(char**);		//delete tokens.
unsigned char grab7saveInput(char**);		//save user input as tokens.
unsigned char clean7printTokens(char**);	//delete tokens.
unsigned char whichCommand(char**);			//determine commands & execute functionality.
unsigned char grabBPBInfo();				//grab image data.
unsigned char printBPBInfo();				//print image data.
unsigned int save4ByteRead(char*);			//grabBPBInfo helper function.
unsigned char uc_infof(char**);		//info functionality.
unsigned char uc_lsf(char**);		//ls functionality.
unsigned char uc_sizef(char**);		//size functionality.
unsigned char uc_cdf(char**);		//cd functionality.
unsigned char uc_creatf(char**);	//creat functionality.
unsigned char uc_mkdirf(char**);	//mkdir functionality.
unsigned char uc_mvf(char**);		//mv functionality.
unsigned char uc_openf(char**);		//open functionality.
unsigned char uc_readf(char**);		//read functionality.
unsigned char uc_writef(char**);	//write functionality.
unsigned char uc_rmf(char**);		//rm functionality.
unsigned char uc_cpf(char**);		//cp functionality.
unsigned short us_RsvdSecCnt();		//determines # of clusters.



char* string2FAT(char*);
void writesize2dir(unsigned int address, unsigned int value);
unsigned char * BigToLil(unsigned int value, unsigned int size);
unsigned int getRootCluster();
unsigned short getBytsperSector();
unsigned int findNextcluster(unsigned int cluster);
unsigned int L2Bint(unsigned char * value, unsigned int size);
unsigned char getSecperClus();
void write2FAT(unsigned int cluster, unsigned int value);
unsigned int findFirstSector(unsigned int);
unsigned char getNumFATs();
unsigned int getFATSz32();
unsigned int findFirstFreeCluster();

void writeClus2Dir(unsigned int address, unsigned int value);

void ui_int2Buf(char*, unsigned int);
char* fullFilePath(char**);


void NextClusterFromSource(int fd, int clusterindex, int select);

unsigned char UpdateOffset(int offset);
int FindOrPrintDirectory(char**, int, int);
void UpdateOrDisplayPath(int select, char**tokens);
void InitFileData();
void addRemovePath(int select, char ** tokens);
//unsigned char * BigToLil(unsigned int value, unsigned int size);

unsigned int hex2int(unsigned char hex[]);
int getNum(char ch);

//UPDATE ZB
/*
void initOpenFileList();//*
unsigned char print6FindNodes(unsigned char, char*);
void pushNode(char*, int*);
unsigned char removeNode(char*);
void copy6Write(char**, unsigned char);
unsigned char copyData(unsigned int*, unsigned int*, int);
void cleanUP();
*/

unsigned int ui_scanFAT(unsigned char);//*
unsigned int ui_comAdd(unsigned int);
unsigned char uc_closef(char**);//*
unsigned char print6FindNodes(unsigned char, char*, int);//*
void pushNode(char*, unsigned char*);//*
									 //void pushNode(char*, int*, unsigned char*);//*
unsigned char removeNode(char*);//*
void initOpenFileList();//*
void cleanUP();//*
void copy6Write(char**, unsigned char);//*
unsigned char copyData(unsigned int*, unsigned int*, int);//*


int IsAFile = 0;
int DirIndex = 0;
int FileIndex = 0;
int checkcurrent = 0;
unsigned int ollyfat = 0;
unsigned int clusty = 0;
int ClusterCounter = 0;
int NameUpdated = 0;


//char* pathName[100] = { NULL };

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

struct
{
	//sorted by offset.
	unsigned short	BPB_BytsPerSec;	//bytes per sector. (2) bytes. offset(11).
	unsigned char	BPB_SecPerClus;	//sectors per cluster. (1) byte. offset(13).
	unsigned short	BPB_RsvdSecCnt;	//reserved region size. (2) bytes. offset(14).
	unsigned char	BPB_NumFATs;	//number of FATs. (1) byte. offset(16).
	unsigned int	BPB_TotSec32;	//total sectors. (4) bytes. offset(32)
	unsigned int	BPB_FATSz32;	//FAT size. (4) bytes. offset(36)
	unsigned int	BPB_RootClus;	//root cluster. (4) bytes. offset(44).
}BPB_Info;

typedef struct Node
{
	char
		*s_open;
	//int
	//*i_open;

	unsigned char
		* uc_openMode;

	struct Node
		*prev,
		*nxt;

} Node;

Node* openFileHead;
struct
{
	unsigned char
		Reserved1[480],
		Reserved2[12],
		LeadSig[4],
		StructSig[4],
		TrailSig[4],
		Nxt_Free[4],
		Free_Count[4];

	unsigned int
		FSI_Free_Count,
		FSI_Nxt_Free,
		FSI_FATStart,
		FSI_FATEnd,
		FSI_LeadSig,
		FSI_StructSig,
		FSI_TrailSig;

} __attribute__((packed)) FSInfo;

struct DIRENTRY
{
	unsigned char DIR_name[11];
	unsigned char DIR_Attributes;


} __attribute__((packed));

struct
{
	unsigned int firstDataSector,	//BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz32).
									//may need to make long int.
		offset,						//firstDataSector + (N-2) * BPB_SecPerClus.
		byteOffset;					//offset * BPB_BytsPerSec.

	char
		pathName[100],
		*lastDirName;



} __attribute__((packed)) FileAccessData;

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

int main(int argc, char* argv[])
{
	char* tokens[6] = { NULL };


	unsigned char ExitStatus = 0x01;

	grabBPBInfo();
	UpdateOffset(2); // set offset to root
					 //printf("-->%x\n", FileAccessData.byteOffset);
	InitFileData(); // set pathName to >>
	ui_scanFAT(0x01);
	initOpenFileList();

	while (ExitStatus)
	{
		grab7saveInput(tokens);

		ExitStatus = whichCommand(tokens);

		cleanTokens(tokens);

		fprintf(stdout, "\n");
	}

	cleanUP();
	return 1;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/
/*
updates or displays path 0 -> update 1 - > display
*/


/*
this is me trying to initialize pathName to the root this is trash can be changed
*/
void InitFileData()
{
	strcpy(FileAccessData.pathName, "./mnt_pnt$");
}



/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
void UpdateOrDisplayPath
-updates the byteoffset or displays the directory name you pass in or displays the path name

-select = 0:
prints the pathname of current directory DOES NOT UPDATE BYTEOFFSET
*note* only displays root path right now
Ex: call UpdateOrDisplayPath(0,tokens) ---> prints path

-select = 1:
prints the directory name you pass in DOES NOT UPDATE BYTEOFFSET

Ex: call UpdateOrDisplayPath(1,tokens) ---> say tokens[1]= RED and you are in root. It will print RED contents but
the byte offset will be = to root.

-select = 3:
UPDATES BYTEOFFSET- first it finds the offset of what you passed in and the calls Updateoffset to update the byteOffset


Ex: call UpdateOrDisplayPath(3,tokens) ---> say tokens[1]= RED and you are in root.
YOUR PATH WILL PERMANENTLY BE CHANGED TO RED until you update it again
ie the byteOffest will now be pointed to RED.
*/
void UpdateOrDisplayPath(int select, char** tokens)
{
	int
		index = 0,
		fd = open("fat32.img", O_RDONLY),
		last_fd,
		pathlen;

	unsigned int
		temp,
		lastOffset;
	char
		tmpChar = '$';;

	if (select == 0)
	{
		fprintf(stdout, "%s ", FileAccessData.pathName);
	}
	else if (select == 1)
	{

		lastOffset = FileAccessData.byteOffset;


		last_fd = fd;
		temp = FindOrPrintDirectory(tokens, fd, 3);
		fd = last_fd;
		UpdateOffset(temp);

		FindOrPrintDirectory(tokens, fd, 1);
		FileAccessData.byteOffset = lastOffset;

	}
	else if (select == 3)
	{

		char buf[100];
		last_fd = fd;
		temp = FindOrPrintDirectory(tokens, fd, 3);
		fd = last_fd;
		UpdateOffset(temp);
		pathlen = strlen(FileAccessData.pathName);
		if (NameUpdated == 0)
		{
			if (strcmp(tokens[1], "..") == 0)
			{
				for (index = pathlen; tmpChar != '/'; index--)
				{
					tmpChar = FileAccessData.pathName[index];
				}
				FileAccessData.pathName[index + 1] = '$';
				FileAccessData.pathName[index + 2] = '\0';
			}
			else
			{
				sprintf(buf, "/%s$", tokens[1]);
				FileAccessData.pathName[pathlen - 1] = '\0';
				strcat(FileAccessData.pathName, buf);
			}
		}
	}
	close(fd);
}


/*
the goal of this was to add or remove a name form the pathName variable in FileAcessData
if select = 1 add to pathName
if select =2 Remove form pathName
is currenty trash can be removed

*/
void addRemovePath(int select, char** tokens)
{
	unsigned char len = 0x00;
	len = strlen(tokens[1]);
	char* temp = (char*)malloc((len + 1) * (sizeof(char)));
	int index = 0;
	printf("%s", tokens[1]);

	printf("%s++", temp);
	if (select == 1)
	{

	}
	else if (select == 0)
	{

	}
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/
/*
functionality for info command
-parse boot sector
-print field name & corresponding valus for each entry
-one entry per line.
-printtBPBInfo.

-print important data about the file system.
-read from BPB
-move to offset of volume and read values individually.
-create a struct representing the BPB & read by passing the struct.
*/
unsigned char uc_infof(char **tokens)
{
	printBPBInfo();
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for ls command.
-show names of files / directories under the dir given in dirname token[1].
-read current dir entries until dirname & its first cluster is found.
-read dir entres and print names.
-if no arguments token[1] == NULL print current directory.
-if dir does not exist, print an error.
*/
unsigned char uc_lsf(char **tokens)
{
	//printf("-1>%x\n", FileAccessData.byteOffset);
	int fd = open("fat32.img", O_RDONLY);
	int last_fd;

	int checkfound;

	if (tokens[1] != NULL)
	{
		last_fd = fd;
		checkfound = FindOrPrintDirectory(tokens, fd, 0);
		fd = last_fd;
		if (checkfound == 0)
		{
			printf("\nFile/Directory not found\n");
			return 'c';
		}
		else
		{
			printf("\n found\n");
			UpdateOrDisplayPath(1, tokens);
		}

	}
	else
	{	//DisplayDirectory(fd);
		FindOrPrintDirectory(tokens, fd, 1);
	}
	close(fd);


}
/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

//print = 0 finds dir/file resturns bool print ==1 prints current dir print =3 returns offeset of seached name 
int FindOrPrintDirectory(char **tokens, int fd, int print)
{
	int holder = fd;

	int MoreDataLeft = 1;
	int DataSectorOffset = 0;

	//int index = 0;

	int result = 0;

	unsigned char buff6[5];
	unsigned char buff7[5];

	int Filefound = 0;

	char buff2[3];   // DirAtt
	char buff3[3] = { '0','F','\0' };
	char buff5[3] = { '0','0','\0' };

	unsigned char buff4[12];
	char * ptr;
	unsigned int lastOffset = FileAccessData.byteOffset;
	unsigned int oldoffset = FileAccessData.offset;
	unsigned int Firstoffset;
	int clusterindex = 0;
	int firsttime = 0;
	int Clustercount;
	while (MoreDataLeft != 0)
	{
		Filefound = 0;
		//------------------------------------------------------------------------------

		if (DataSectorOffset == 512)
		{
			if (checkcurrent == 1)
			{

				break;
			}
			unsigned char fattybuff[5];
			unsigned char fattybuff2[5];
			if (firsttime == 0)
			{

				lseek(fd, FileAccessData.byteOffset + (0 * 32) + 20, SEEK_SET);
				read(fd, buff6, 2);

				lseek(fd, FileAccessData.byteOffset + (0 * 32) + 26, SEEK_SET);
				read(fd, buff7, 2);

				buff7[2] = buff6[0];
				buff7[3] = buff6[1];
				unsigned int N;
				N = save4ByteRead(buff7);
				FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);
				firsttime = 1;
				Firstoffset = FileAccessData.offset;
				Clustercount = N;
			}

			int i = 0;
			int H = 0;
			FileAccessData.offset = Firstoffset;

			do {
				lseek(fd, FileAccessData.offset, SEEK_SET);
				read(fd, fattybuff, 4);
				lseek(fd, FileAccessData.offset, SEEK_SET);

				fattybuff2[0] = fattybuff[0];
				fattybuff2[1] = fattybuff[1];
				fattybuff2[2] = fattybuff[2];
				fattybuff2[3] = fattybuff[3];

				fattybuff2[0] = fattybuff2[2];
				fattybuff2[1] = fattybuff2[3];

				fattybuff2[3] = fattybuff[0];
				fattybuff2[2] = fattybuff[1];

				H = save4ByteRead(fattybuff);

				FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (H * 4);
				/*
				printf("%x:\n", fattybuff2[0]);
				printf("%x:\n", fattybuff2[1]);
				printf("%x:\n", fattybuff2[2]);
				printf("%x:\n", fattybuff2[3]);
				printf("Next cluster %x\n", FileAccessData.offset);
				*/
				if ((fattybuff2[0] == 0xF8 && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0x0F) ||
					(fattybuff2[0] == 0xFF && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0x0F) ||
					(fattybuff2[0] == 0xFF && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0xFF))
				{
					break;
				}
				if (i == clusterindex) { break; }
				i++;
			} while (1);
			UpdateOffset(H);
			clusterindex++;

			DataSectorOffset = 0;

		}//-----------------------------------------------


		lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

		read(fd, buff4, 10);
		//buff4[12] = '\0';

		int index = 0;
		while (index <= strlen(buff4))
		{

			if (buff4[index] == ' ')
			{
				buff4[index] = '\0';
				break;
			}
			index++;
		}



		read(fd, buff2, 2);
		buff2[2] = '\0';

		if (strcmp(buff2, buff3) == 0)
		{
		}
		else
		{

			if (print == 1)
			{
				if ((buff2[1] != 0x0F) && (strcmp(buff4, "") != 0) && (buff4[0] != 0xE5))
				{
					//printf("\n hi3\n");
					if (buff2[1] == 0x10)
					{
						printf("%-10s---> Directory\n", buff4);
						Filefound = 0;

					}
					else
					{
						printf("%-10s---> File\n", buff4);
						Filefound = 1;

					}
				}



			}

			if (print == 0)
			{
				if ((buff2[1] != 0x0F) && (strcmp(buff4, "") != 0))
				{

					if (buff2[1] == 0x10)
					{
						Filefound = 0;
					}
					else
					{
						Filefound = 1;
					}
				}

			}

			/*
			int index = 0;
			while (index <= strlen(buff4))
			{

			if (buff4[index] == ' ')
			{
			buff4[index] = '\0';
			break;
			}
			index++;
			}
			*/

			if (strcmp(buff4, "") == 0)
			{
				MoreDataLeft = 0;

			}



			if (tokens[1] != 0)
			{
				if ((strcmp(tokens[1], buff4) == 0))
				{
					//------------------------------------------------
					if (Filefound == 1)
					{
						IsAFile = 1;
						Filefound = 0;
						FileIndex = DataSectorOffset / 32;
					}
					else
					{
						IsAFile = 0;
						Filefound = 0;
						DirIndex = DataSectorOffset / 32;

					}

					//----------------------------------------------------
					if (print == 3)
					{
						lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 20, SEEK_SET);
						read(fd, buff6, 2);
						lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 26, SEEK_SET);
						read(fd, buff7, 2);
						// 5 8e b4 1
						buff7[2] = buff6[0];
						buff7[3] = buff6[1];

						if ((buff7[0] == 0x00) && (buff7[1] == 0x00) && (buff7[2] == 0x00) && (buff7[3] == 0x00))
						{
							result = 2;
							FileAccessData.offset = oldoffset;
							FileAccessData.byteOffset = lastOffset;
							return result;
						}

						unsigned int  g = save4ByteRead(buff7);

						result = g;
						FileAccessData.offset = oldoffset;
						FileAccessData.byteOffset = lastOffset;
						return result;

					}
					else
					{
						result = 1;
					}

				}
			}

		}


		DataSectorOffset += 32;
	}
	//printf("not found\n");
	FileAccessData.offset = oldoffset;
	FileAccessData.byteOffset = lastOffset;
	return result;
}

//*
void copy6Write(char** tokens, unsigned char opCode)
{
	unsigned int
		address = FileAccessData.byteOffset,
		dirOffset = 0x00000000,
		fileOffset = 0x00000000,
		fileFATAddr,
		dirFATAddr,
		curReadAddr,
		curWriteAddr,
		writeOffset,
		writeSize,
		write2Clus,
		fileSize,
		file3Clus,
		lseekCurAddr,
		ui_idx,
		fileDirEntryLoc = 0x00000000,	//new entries location.
		fileAbsAddr4Wr = 0x00000000,	//files absolute address in DIR.
		availableClus[2],	//0 is first available clus. 1 is second available clus.
		ui_fileClusOffset = 0x00000000;

	int
		fd;

	unsigned short
		sectorOffset = 0x0020,
		bytesLeftInClus = 0x0000,
		charsLeft2Write,
		tmpFileOffset;

	unsigned char
		nameBuff[12] = { 0x20 },
		offsetBuf[4] = { 0x00 },
		shortReadBuf[2] = { 0x00 },
		cpBuf[0x200] = { 0x00 },
		opStatus = 0x00,
		fileCopy[64],
		attribute;

	char
		* openFileQ;

	if (tokens[1] == NULL)
	{
		return;
	}
	else if (tokens[2] == NULL)
	{
		return;
	}
	else if (opCode == 0x01)
	{
		if (tokens[3] == NULL)
		{
			return;
		}
		else if (tokens[4] == NULL)
		{
			return;
		}

		writeOffset = atoi(tokens[2]);
		writeSize = atoi(tokens[3]);
	}

	fd = open("fat32.img", O_RDWR);

	while (sectorOffset < 0x0200)
	{
		lseekCurAddr = lseek(fd, address + sectorOffset, SEEK_SET);
		read(fd, nameBuff, 12);
		attribute = nameBuff[11];
		nameBuff[11] = '\0';

		//last directory entry.
		if (nameBuff[0] == 0x00)
		{
			fileDirEntryLoc = (address + (sectorOffset - 0x20));
			break;	//no more entries. break from while.
		}
		else if (nameBuff[0] != 0xE5)	//non-deleted entry.
		{
			//append null at first space.
			for (ui_idx = 0x00000000; ui_idx < 11; ui_idx++)
			{
				if (nameBuff[ui_idx] == 0x20)
				{
					nameBuff[ui_idx] = 0x00;
					break;		//break from for.
				}
				else {}
			}


			//FILENAME exists.
			if (strcmp(tokens[1], nameBuff) == 0)
			{
				//FILENAME is a directory. error.
				if (attribute == 0x10)
				{
					fprintf(stdout, "\n%s is not a file.", tokens[1]);
					break;	//break from while. error.
				}
				else	//FILENAME is a file.
				{
					opStatus |= 0x01;
					tmpFileOffset = (sectorOffset + 20);
					lseekCurAddr = lseek(fd, address + tmpFileOffset, SEEK_SET);
					read(fd, shortReadBuf, 2);
					offsetBuf[2] = shortReadBuf[0]; offsetBuf[3] = shortReadBuf[1];
					tmpFileOffset += 6;
					lseekCurAddr = lseek(fd, address + tmpFileOffset, SEEK_SET);
					read(fd, shortReadBuf, 2);
					offsetBuf[0] = shortReadBuf[0]; offsetBuf[1] = shortReadBuf[1];
					fileOffset = save4ByteRead(offsetBuf);
					fileFATAddr = ((fileOffset * 0x0004) + 0x4000);
					tmpFileOffset += 2;
					lseekCurAddr = lseek(fd, address + tmpFileOffset, SEEK_SET);
					read(fd, offsetBuf, 4);
					fileSize = save4ByteRead(offsetBuf);
					if (opCode == 0x01)
					{
						fileAbsAddr4Wr = (address + sectorOffset);
						break;
					}	//break from while. ok.
					else
					{
						tmpFileOffset = (sectorOffset - 0x0020);
						lseek(fd, address + tmpFileOffset, SEEK_SET);
						read(fd, fileCopy, 0x40);
					}
				}
			}

			//cp mode.
			if (opCode == 0x00)
			{
				//TO name already exists.
				if (strcmp(tokens[2], nameBuff) == 0)
				{
					//name match and directory.
					if (attribute == 0x10)
					{
						opStatus |= 0x02;
						tmpFileOffset = (sectorOffset + 20);
						lseekCurAddr = lseek(fd, address + tmpFileOffset, SEEK_SET);
						read(fd, shortReadBuf, 2);
						offsetBuf[2] = shortReadBuf[0]; offsetBuf[3] = shortReadBuf[1];
						tmpFileOffset += 6;
						lseekCurAddr = lseek(fd, address + tmpFileOffset, SEEK_SET);
						read(fd, shortReadBuf, 2);
						offsetBuf[0] = shortReadBuf[0]; offsetBuf[1] = shortReadBuf[1];
						dirOffset = save4ByteRead(offsetBuf);
						dirFATAddr = ((dirOffset * 0x0004) + 0x4000);
					}
					else	//To is a file. error. change later for overwriting file on cp.
					{
						fprintf(stdout, "\n%s is not a directory.", tokens[2]);
						break;
					}
				}
			}

		}
		else
		{
			if (fileDirEntryLoc == 0x00000000) { fileDirEntryLoc = (address + (sectorOffset - 0x20)); }
			//skip entry
		}

		sectorOffset += 0x0040;
	}

	switch (opStatus)
	{
	case 0x00:	//FILENAME & DIR not found.
	{
		fprintf(stdout, "\ntokens not found");
		break;
	}
	case 0x01:	//DIR not found.  make a file named tokens[2] or check if write command.
	{
		//copy mode.
		if (opCode == 0x00)
		{
			//allocate new file.
			availableClus[1] = ui_scanFAT(0x00);
			if (fileDirEntryLoc != 0x00000000)
			{
				lseek(fd, fileDirEntryLoc, SEEK_SET);
				write(fd, fileCopy, 0x40);

				//change name to tokens[2].
				tokens[2] = (char*)realloc(tokens[2], 12);
				tokens[2][11] = '\0';	//to avoid overflow errors.
				strcpy(nameBuff, tokens[2]);

				//make any remaining junk chars a ' '.
				for (ui_idx = strlen(tokens[2]); ui_idx < 11; ui_idx++)
				{
					nameBuff[ui_idx] = ' ';
				}

				//write name over new directory entry.
				lseek(fd, fileDirEntryLoc + 0x20, SEEK_SET);
				write(fd, nameBuff, 11);

				//change first cluter low & first cluster high to the new copies FAT index.
				ui_idx = ((availableClus[1] - 0x00004000) / (0x00000004));
				shortReadBuf[1] = ((ui_idx >> 24) & 0x000000FF);
				shortReadBuf[0] = ((ui_idx >> 16) & 0x000000FF);
				lseek(fd, fileDirEntryLoc + 52, SEEK_SET);
				write(fd, shortReadBuf, 2);
				shortReadBuf[1] = ((ui_idx >> 8) & 0x000000FF);
				shortReadBuf[0] = (ui_idx & 0x000000FF);
				lseek(fd, fileDirEntryLoc + 58, SEEK_SET);
				write(fd, shortReadBuf, 2);
				copyData(&fileOffset, availableClus, fd);

			}
			else	//nowhere to place new files dir entry.
			{
				fprintf(stdout, "\ncurrent directory is full. cannot add entry.");
				return;
			}
		}
		else if (opCode == 0x01)	//start writing file @ offset bytes
		{
			if (writeSize > 0x0200)
			{
				fprintf(stdout, "\nstring length longer than 0x0200. cannot append");
				return;
			}

			//check if file is open for writing.
			openFileQ = fullFilePath(tokens);
			if (print6FindNodes(0x00, openFileQ, O_WRONLY) == 0x00)
			{
				fprintf(stdout, "\n%s is not open for writing", tokens[1]);
				free(openFileQ);
				return;
			}
			else {}//file is open

			write2Clus = (unsigned int)(writeOffset / 0x0200);		//cluster number offset lands in.
			file3Clus = (unsigned int)(fileSize / 0x0200);			//number of clusters allocated to file.

																	//file to small to start writing at this offset.
			if (write2Clus > file3Clus)
			{

			}
			else	//okay to start writing.
			{
				//follow FAT to correct start cluster for write.
				for (ui_idx = 0x00000000; ui_idx < write2Clus; ui_idx += 0x00000001)
				{
					//set read offset to correct location in FAT.
					lseekCurAddr = lseek(fd, fileFATAddr, SEEK_SET);
					read(fd, offsetBuf, 4);
					fileOffset = save4ByteRead(offsetBuf);
					fileFATAddr = ((fileOffset * 0x0004) + 0x4000);
				}

				//go to first write cluster & calculate number of bits between
				//offset & end of cluster/
				curWriteAddr = ui_comAdd(fileOffset);
				lseekCurAddr = lseek(fd, (curWriteAddr + (writeOffset -
					(write2Clus * 0x0200))), SEEK_SET);
				bytesLeftInClus = (0x0200 - (writeOffset - (0x0200 * write2Clus)));

				//write "size" bytes. then done writing.
				if (bytesLeftInClus >= writeSize)
				{
					write(fd, tokens[4], writeSize);

					ui_idx = (writeOffset + writeSize);//furtest write location.
													   //file size needs to be updated.
					if (ui_idx > fileSize)
					{
						ui_int2Buf(offsetBuf, ui_idx);
						lseek(fd, fileAbsAddr4Wr + 28, SEEK_SET);
						write(fd, offsetBuf, 4);
					}
					else	//writing over current data.
					{

					}

				}
				else	//write to end of clus. and continue writing in next cluster.
				{
					//first write.
					write(fd, tokens[4], bytesLeftInClus);
					charsLeft2Write = (writeSize - bytesLeftInClus);
					//go to next cluster.
					lseekCurAddr = lseek(fd, fileFATAddr, SEEK_SET);
					//read current clusters offset.
					read(fd, offsetBuf, 4);

					//save data.
					fileOffset = save4ByteRead(offsetBuf);

					//allocate new cluster.
					if (fileOffset > FSInfo.FSI_FATEnd)
					{
						//grab first available cluster.
						availableClus[1] = ui_scanFAT(0x00);


						//allocate new cluster.
						ui_int2Buf(offsetBuf, 0xFFFFFFFF);
						lseek(fd, availableClus[1], SEEK_SET);
						write(fd, offsetBuf, 4);

						//update file offset.
						fileOffset = ((availableClus[1] - 0x00004000) / (0x00000004));

						//update old file offset to point to newly allocated cluster.
						ui_int2Buf(offsetBuf, fileOffset);
						lseek(fd, fileFATAddr, SEEK_SET);
						write(fd, offsetBuf, 4);


						//update file size.
						ui_idx = (((file3Clus + 1) * 0x200) - fileSize);//using as tmp storage
						ui_int2Buf(offsetBuf, (fileSize + ui_idx + charsLeft2Write));
						lseek(fd, fileAbsAddr4Wr + 28, SEEK_SET);
						write(fd, offsetBuf, 4);

					}
					else		//continue writing in next previously allocated cluster.
					{

						//fileFATAddr = ((fileOffset * 0x0004) + 0x4000);
					}

					curWriteAddr = ui_comAdd(fileOffset);
					//go to data location.
					lseekCurAddr = lseek(fd, curWriteAddr, SEEK_SET);
					//second write
					write(fd, tokens[4] + bytesLeftInClus, charsLeft2Write);
				}
			}


		}
		else {}	//added for possible future functionality.

		break;
	}
	case 0x02:	//FILENAME not found.
	{
		fprintf(stdout, "\nFILENAME: %s not found", tokens[1]);
		break;
	}
	case 0x03:	//FILENAME found & DIR found.
	{
		//find directory at dirOffset if there is enough space & add a file. remember
		//to allocate in the FAT.

		lseekCurAddr = (ui_comAdd(dirOffset) + 0x20);

		//go to sub-directories first short filename.
		lseek(fd, lseekCurAddr, SEEK_SET);

		//add an entry to the sub directory.
		for (ui_idx = 0x00000000; ui_idx < 0x0200; ui_idx += 0x00000040)
		{
			lseek(fd, lseekCurAddr + ui_idx, SEEK_SET);
			read(fd, nameBuff, 11);
			//found space available for new entry.
			if ((nameBuff[0] == 0xE5) || (nameBuff[0] == 0x00))
			{
				//copy files dir entry to sub-directory.
				lseek(fd, (lseekCurAddr + (ui_idx - 0x00000020)), SEEK_SET);
				write(fd, fileCopy, 0x40);

				//change FAT index for the copy.
				availableClus[1] = ui_scanFAT(0x00);
				ui_idx = ((availableClus[1] - 0x00004000) / (0x00000004));

				//write first cluster high.
				shortReadBuf[1] = ((ui_idx >> 24) & 0x000000FF);
				shortReadBuf[0] = ((ui_idx >> 16) & 0x000000FF);
				lseek(fd, lseekCurAddr + 20, SEEK_SET);
				write(fd, shortReadBuf, 2);

				//write first cluster low
				shortReadBuf[1] = ((ui_idx >> 8) & 0x000000FF);
				shortReadBuf[0] = (ui_idx & 0x000000FF);
				lseek(fd, lseekCurAddr + 26, SEEK_SET);
				write(fd, shortReadBuf, 2);
				copyData(&fileOffset, availableClus, fd);
				break;
			}
			else	//open entry not found yet.
			{

			}
		}
		break;
	}
	default:
	{
		break;
	}
	}
	close(fd);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/
unsigned char copyData(unsigned int* fileOffset, unsigned int* availableClus, int fd)
{
	unsigned int
		lseekCurAddr,
		curReadAddr,
		curWriteAddr,
		fileFATAddr;

	unsigned char
		cpBuf[0x0200],
		offsetBuf[0x04];


	if (*fileOffset > FSInfo.FSI_FATEnd)
	{
		availableClus[0] = availableClus[1];
		//read data from og cluster.
		curReadAddr = ui_comAdd(*fileOffset);
		lseek(fd, curReadAddr, SEEK_SET);
		read(fd, cpBuf, 0x0200);

		//copy data to new cluster.
		curWriteAddr = ui_comAdd(((availableClus[1] - 0x00004000) / (0x00000004)));
		lseek(fd, curWriteAddr, SEEK_SET);
		write(fd, cpBuf, 0x0200);
	}
	else
	{

		while (*fileOffset < FSInfo.FSI_FATEnd)
		{
			//read file data.
			curReadAddr = ui_comAdd(*fileOffset);
			lseekCurAddr = lseek(fd, curReadAddr, SEEK_SET);
			read(fd, cpBuf, 0x0200);

			//temporarialy allocation of first available clus for scanFAT.
			lseek(fd, availableClus[1], SEEK_SET);
			ui_int2Buf(offsetBuf, 0xFFFFFFFF);
			write(fd, offsetBuf, 0x04);

			//update first / last available.
			availableClus[0] = availableClus[1];
			availableClus[1] = ui_scanFAT(0x00);

			//write next clusters address to newly allocated cluster.
			lseek(fd, availableClus[0], SEEK_SET);
			ui_int2Buf(offsetBuf, ((availableClus[1] - 0x00004000) / (0x00000004)));
			write(fd, offsetBuf, 0x04);

			//copy data to availableClus[0] location.
			curWriteAddr = ui_comAdd(((availableClus[0] - 0x00004000) / (0x00000004)));
			lseek(fd, curWriteAddr, SEEK_SET);
			write(fd, cpBuf, 0x0200);

			//read next OG FAT location.
			fileFATAddr = ((*fileOffset * 0x0004) + 0x4000);
			lseekCurAddr = lseek(fd, fileFATAddr, SEEK_SET);
			read(fd, offsetBuf, 4);
			*fileOffset = save4ByteRead(offsetBuf);
		}
	}

	lseek(fd, availableClus[0], SEEK_SET);
	ui_int2Buf(offsetBuf, 0xFFFFFFFF);
	write(fd, offsetBuf, 0x04);

	return 0x00;
}


/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/


unsigned char UpdateOffset(int  offset)
{
	// set for the first Dir
	FileAccessData.firstDataSector = BPB_Info.BPB_RsvdSecCnt + (BPB_Info.BPB_NumFATs*BPB_Info.BPB_FATSz32);
	FileAccessData.offset = FileAccessData.firstDataSector + (offset - 2)* BPB_Info.BPB_SecPerClus;
	FileAccessData.byteOffset = FileAccessData.offset * BPB_Info.BPB_BytsPerSec;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for size command.
-print size of the file FILENAME in the current working directory (in bytes)
-error if DNE or is not a directory.
*/
unsigned char uc_sizef(char **tokens)
{
	int fd = open("fat32.img", O_RDONLY);

	int MoreDataLeft = 1;
	int DataSectorOffset = 0;


	char buff2[3];   // DirAtt
	char buff4[12];
	unsigned char size[4];
	int f = 0;


	while (MoreDataLeft != 0)
	{
		if (DataSectorOffset == 32000)
		{
			break;
		}
		//-----------------------------------------------

		lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

		read(fd, buff4, 10);
		buff4[12] = '\0';

		read(fd, buff2, 2);
		buff2[2] = '\0';


		string2FAT(tokens[1]);

		if (strcmp(buff4, tokens[1]) == 0)
		{
			//printf("found\n");

			if (buff2[1] != 0x10)
			{
				f = 1;
				//printf("its a file");
				lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 28, SEEK_SET);
				read(fd, size, 4);
				unsigned int x = L2Bint(size, 4);
				printf("\nSIZE IS %d\n", x);
			}
			else
			{
				printf("This is a directory");
			}

			MoreDataLeft = 0;
		}


		DataSectorOffset += 32;
	}
	if (f == 0)
	{
		printf("\nFile not found\n");
	}
}

char * string2FAT(char * input)
{
	char temp[10] = " ";
	int i;
	int length = strlen(input);
	for (i = 0; i<length && i < 10; i++)
	{
		temp[i] = toupper(input[i]);
	}
	for (i = length; i<10; i++)
	{
		temp[i] = ' ';
	}
	temp[10] = '\0';
	strcpy(input, temp);
	return input;
}
/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for cd command.
-change current working directory to DIRNAME
-requirements:
-maintain current working dir state.
-error if DNE or is not a directory.
*/
unsigned char uc_cdf(char **tokens)
{
	int fd = open("fat32.img", O_RDONLY);
	int last_fd;
	int checkfound;


	if (tokens[1] != NULL)
	{
		last_fd = fd;
		checkfound = FindOrPrintDirectory(tokens, fd, 0);
		fd = last_fd;
		if (checkfound == 0)
		{
			printf("\nFile/Directory not found\n");
			return 'c';
		}
		else
		{
			printf("\n found\n");
			UpdateOrDisplayPath(3, tokens);
			printf("%x\n", FileAccessData.byteOffset);
		}

	}
	else
	{
		printf("\n Invalid Command\n");
		//UpdateOffset(4);
		//printf("%x\n",FileAccessData.byteOffset);
	}


	close(fd);


}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for creat command.
-create a file in the current dir
-requirements:
-initial size 0 bytes.
-named FILENAME
-print error if name already exists.
*/
unsigned char uc_creatf(char **tokens)
{
	printf("\nCreat instruction functionality\n");
	int fd = open("fat32.img", O_RDWR);
	unsigned char *  filename = string2FAT(tokens[1]);
	int done = 0;
	int found = FindOrPrintDirectory(tokens, fd, 0);
	printf("\n\n%d\n\n", found);
	unsigned int index = 0, byteindex = 0, byteoffset = 0, newindex = 0, tempnext = 0;
	unsigned int nextcluster = FileAccessData.byteOffset;

	int MoreDataLeft = 1, DataSectorOffset = 0;
	char buff4[12];
	unsigned int size = 0;

	while (MoreDataLeft != 0)
	{
		if (DataSectorOffset == 32000)
		{
			break;
		}

		lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

		read(fd, buff4, 10);

		buff4[10] = '\0';
		buff4[11] = '\0';
		buff4[12] = '\0';

		if (strcmp(buff4, filename) == 0)
		{
			found = 1;
			MoreDataLeft = 0;
			break;
		}
		else
		{
			/*found = 0;*/
		}
		DataSectorOffset += 32;
	}


	if (tokens[1] != NULL)
	{
		if (found == 1)
		{
			printf("\nFile already exists\n");
		}
		else //if it doesnt exist yet
		{
			//printf("%s\n", filename);

			while (done == 0)
			{
				byteindex = nextcluster;//findFirstSector(nextcluster);
										//printf("\nbyteindex %d\n", byteindex);
				byteoffset = 0;
				unsigned char * firstbyte = (char*)malloc(sizeof(char));
				while (done == 0 && byteoffset<((unsigned int)(getBytsperSector()) * (unsigned int)(getSecperClus())))
				{
					lseek(fd, byteindex + byteoffset, SEEK_SET);
					read(fd, firstbyte, 1);
					//	printf("\n firstbyte %u\n",firstbyte[0]);
					if (firstbyte[0] == 0x00 || firstbyte[0] == 0xE5)
					{
						//printf("\n firstbyte %u\n",firstbyte[0]);

						newindex = byteindex + byteoffset;

						//printf("\n newindex %d\n", newindex); //this is the index of the new empty cluster line. It works good until this point

						//write file name
						lseek(fd, newindex, SEEK_SET);
						write(fd, filename, 11);

						//write dir attr
						firstbyte[0] = 0x20;
						lseek(fd, newindex + 11, SEEK_SET);
						write(fd, firstbyte, 1);

						//write cluster value
						writeClus2Dir(newindex, 0x0000);

						//write file size
						writesize2dir(newindex, 0x0000);

						done = 1;
					}//if firstbyte end
					else
					{
						byteoffset += 32;
					}
				}//end nested while

				if (done == 0)
				{
					tempnext = findNextcluster(nextcluster);
					if (tempnext != 0X0FFFFFF8 && tempnext != 0X0FFFFFFF)
					{
						nextcluster = tempnext;
					}//tempnext if end
					else
					{
						unsigned int firstfree = FSInfo.FSI_Nxt_Free;
						write2FAT(nextcluster, firstfree);
						write2FAT(firstfree, 0x0FFFFFF8);
						nextcluster = firstfree;
					}
				}//if done=0 end
			}//end while NOT DONE
		}//end if it doesnt exist

	} //end main if
	else
	{
		printf("No file name given\n");
	}
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for mkdir command.
-create a new dir.
-requirements:
-named DIRNAME
-error if DIRNAME already exists.
*/
unsigned char uc_mkdirf(char **tokens)
{
	printf("\nmkdir instruction functionality\n");

	int fd = open("fat32.img", O_RDWR);
	int found = 0;// FindOrPrintDirectory(tokens, fd, 0);
	int last_fd = 0;
	unsigned int OGbyteoffset = FileAccessData.byteOffset;
	unsigned int OGoffset = FileAccessData.offset;
	int DataSectorOffset = 0;
	unsigned char buff1[2];
	unsigned char buff2[2];
	buff2[0] = 0x10;
	unsigned char buff4[12];
	unsigned char buff6[5];
	unsigned char buff7[5];
	unsigned char buff8[5];
	unsigned char buff9[5];
	char * parent = "..      ";
	char* child = ".       ";
	unsigned char *  filename = string2FAT(tokens[1]);
	int done = 0;
	int needCluster = 0;
	int clusterindex = 0;

	//checkcurrent = 0;
	last_fd = fd;
	found = FindOrPrintDirectory(tokens, fd, 0);
	fd = last_fd;


	if (tokens[1] != NULL)
	{
		if (found == 1)
		{
			printf("\nDirectory already exists\n");
		}
		else //if it doesnt exist yet
		{
			printf("\nMaking Directory \n");

			if (FileAccessData.byteOffset != 0x100400)
			{
				//find offest for ..
				lseek(fd, FileAccessData.byteOffset + 20, SEEK_SET);
				read(fd, buff6, 2);

				lseek(fd, FileAccessData.byteOffset + 26, SEEK_SET);
				read(fd, buff7, 2);
			}
			else
			{
				buff6[0] = 0x00;
				buff6[1] = 0x00;
				buff7[0] = 0x00;
				buff7[1] = 0x00;
			}

			//temp2 has . 
			unsigned char temp2[4];
			unsigned char temp3[4];
			unsigned char temp4[4];

			unsigned int temp = findFirstFreeCluster();
			strcpy(temp2, BigToLil(temp, 4));

			temp3[0] = temp2[0];
			temp3[1] = temp2[1];

			temp4[0] = temp2[2];
			temp4[1] = temp2[3];
			

			UpdateOffset(temp);

			//write .
			lseek(fd, FileAccessData.byteOffset, SEEK_SET);
			write(fd, child, 10);
			lseek(fd, FileAccessData.byteOffset + 11, SEEK_SET);
			write(fd, buff2, 1);
			lseek(fd, FileAccessData.byteOffset + 20, SEEK_SET);
			write(fd, &temp4, 2);
			lseek(fd, FileAccessData.byteOffset  + 26, SEEK_SET);
			write(fd, &temp3, 2);

			//write ..
			lseek(fd, FileAccessData.byteOffset + 32, SEEK_SET);
			write(fd, parent, 10);
			lseek(fd, FileAccessData.byteOffset + 32 + 11, SEEK_SET);
			write(fd, buff2, 1);
			lseek(fd, FileAccessData.byteOffset +32 + 20, SEEK_SET);
			write(fd, &buff6, 2);
			lseek(fd, FileAccessData.byteOffset +32 + 26, SEEK_SET);
			write(fd, &buff7, 2);


			write2FAT(temp, 0xFFFFFFFF);

			printf("%x\n", FileAccessData.byteOffset);

			FileAccessData.byteOffset = OGbyteoffset;

			printf("%x\n", FileAccessData.byteOffset);
			
			while (done == 0)
			{
			//printf("%x\n", FileAccessData.byteOffset);
			lseek(fd, FileAccessData.byteOffset + DataSectorOffset , SEEK_SET);
			read(fd, buff1, 1);

				if (buff1[0] == 0x00 || buff1[0] == 0xE5)
				{
				lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);
				write(fd, filename, 10);
				lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 11, SEEK_SET);
				write(fd, buff2, 1); 
				lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 20, SEEK_SET);
				write(fd, &temp4, 2);
				lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 26, SEEK_SET);
				write(fd, &temp3, 2);

				done = 1;
				}

				if (DataSectorOffset == 512)
				{
					DataSectorOffset = 0;
					needCluster = 1;
				}
				else
				{
					DataSectorOffset += 32;
				}



				if (needCluster == 1)
				{

					FileAccessData.byteOffset = OGbyteoffset;
					FileAccessData.offset = OGoffset;
					clusty = 0;
					NextClusterFromSource(fd, clusterindex++, 0);
					printf("%x\n", FileAccessData.byteOffset);

					if (clusty == 1)
					{
						
						char temp22[4];

						unsigned int temp23 = findFirstFreeCluster();


						strcpy(temp22, BigToLil(temp23, 4));

						//write2FAT(ollyfat, temp23);
						write2FAT(temp23, 0xFFFFFFFF);
						UpdateOffset(temp23);


						clusty = 0;
						DataSectorOffset = 0;
						needCluster = 0;
						
						printf("hi\n");
					}
					needCluster = 0;
					DataSectorOffset = 0;
				}

			}//end while NOT DONE
			
		}//end if it doesnt exist
		
	} //end main if
	else
	{
		printf("No file name given\n");
	}
	FileAccessData.byteOffset = OGbyteoffset;
	FileAccessData.offset = OGoffset;
	close(fd);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for mv command.
-move a file or dir or change the name.
-requrements:
-if TO exists & is a dir then move FROM entry inside to TO entry.
-if TO DNE rename FROM entry to the name in TO entry.
-if TO is a file & FROM is a dir then error.
*/
unsigned char uc_mvf(char **tokens)
{
	fprintf(stdout, "\nmv instruction functionality");



	int fd = open("fat32.img", O_RDWR);

	int checkfound;
	int last_fd;
	int len;
	unsigned char buff1[4];

	unsigned char buff2[3];
	unsigned char buff4[12];
	unsigned char buff6[5];
	unsigned char buff7[5];

	unsigned char buff66[5];
	unsigned char buff77[5];

	tokens[6] = tokens[1];
	tokens[1] = tokens[2];
	tokens[2] = tokens[6];
	tokens[6] = NULL;

	//printf("%s\n", tokens[1]);

	unsigned int O2offset = FileAccessData.offset;
	unsigned int L2offset = FileAccessData.byteOffset;


	if (tokens[2] != NULL)
	{
		//temptokens[1] = tokens[2];
		//printf("%s\n", temptokens[1]);

		IsAFile = 0;
		last_fd = fd;
		checkfound = FindOrPrintDirectory(tokens, fd, 0);
		fd = last_fd;


		if (checkfound == 1)
		{
			if (IsAFile == 1)
			{
				//printf("\n F found\n");

				tokens[6] = tokens[1];
				tokens[1] = tokens[2];
				tokens[2] = tokens[6];
				tokens[6] = NULL;

				IsAFile = 0;
				last_fd = fd;
				checkfound = FindOrPrintDirectory(tokens, fd, 0);   /// temp here
				fd = last_fd;

				if (checkfound == 1)
				{
					if (IsAFile == 1)
					{
						printf("\nERROR: The name is already being used by another file \n");

					}
					else
					{
						printf("\n ERROR Cannot move directory: invalid destination argument\n");
					}

				}


			}
			else
			{
				tokens[6] = tokens[1];
				tokens[1] = tokens[2];
				tokens[2] = tokens[6];
				tokens[6] = NULL;
				//printf("124\n");
				IsAFile = 0;

				checkcurrent = 1;
				last_fd = fd;
				checkfound = FindOrPrintDirectory(tokens, fd, 0);
				fd = last_fd;




				int deadlock = 0;
				int clusterindex = 0;


				unsigned int k2 = FileAccessData.byteOffset;
				if (checkfound == 0)
				{

					do
					{
						FileAccessData.byteOffset = k2;
						NextClusterFromSource(fd, clusterindex, 0);
						last_fd = fd;
						checkfound = FindOrPrintDirectory(tokens, fd, 0);
						fd = last_fd;
						printf("%x-\n", FileAccessData.byteOffset);
						printf("%d-\n", checkfound);
						if (checkfound == 1)
						{
							break;
						}

						if (deadlock > 100)
						{
							break;
						}
						clusterindex++;
						deadlock++;
					} while (checkfound == 0);
				}
				checkcurrent = 0;
				//printf("%x\n", FileAccessData.byteOffset);
				//printf("%d\n", DirIndex);





				if (checkfound == 1)
				{


					unsigned int Index;
					if (IsAFile == 1)
					{
						Index = FileIndex;
					}
					else
					{
						Index = DirIndex;
					}//-----------------------------------------------------------------------------// get name
					unsigned char buff5[12];
					lseek(fd, FileAccessData.byteOffset + (Index * 32), SEEK_SET);
					read(fd, buff4, 10);

					//-------------------------------------------
					int index = 0;
					while (index <= strlen(buff4))
					{

						if (buff4[index] == ' ')
						{
							buff4[index] = '\0';
							break;
						}
						index++;
					}



					//------------------------------------------------------------------------------//file type
					lseek(fd, FileAccessData.byteOffset + (Index * 32) + 11, SEEK_SET);
					read(fd, buff2, 2);
					buff2[2] = '\0';

					//------------------------------------------------------------------------------// file offset
					lseek(fd, FileAccessData.byteOffset + (Index * 32) + 20, SEEK_SET);
					read(fd, buff66, 2);

					lseek(fd, FileAccessData.byteOffset + (Index * 32) + 26, SEEK_SET);
					read(fd, buff77, 2);

					buff77[2] = buff66[0];
					buff77[3] = buff66[1];
					//--------------------------------------------------------------------------------------------
					lseek(fd, FileAccessData.byteOffset + ((Index + 1) * 32), SEEK_SET);
					read(fd, buff1, 1);
					lseek(fd, FileAccessData.byteOffset + (Index * 32), SEEK_SET);



					if (buff1[0] != 0x00)
					{
						unsigned char  temp2;
						temp2 = 0xE5;
						write(fd, &temp2, 1);

					}
					else
					{
						unsigned char  temp2;
						temp2 = 0x00;
						write(fd, &temp2, 1);
					}
					//printf("%s\n", buff4);
					printf("%x\n", buff2[0]);

					//---------------------------------------------------------------------------

					tokens[6] = tokens[1];
					tokens[1] = tokens[2];
					tokens[2] = tokens[6];
					tokens[6] = NULL;

					//--------------------------------------------------
					//UpdateOrDisplayPath(3, tokens);
					FileAccessData.byteOffset = k2;
					//printf("%x=\n", FileAccessData.byteOffset);


					checkcurrent = 0;
					last_fd = fd;
					checkfound = FindOrPrintDirectory(tokens, fd, 3);
					fd = last_fd;
					UpdateOffset(checkfound);

					//printf("%x==\n", FileAccessData.byteOffset);

					unsigned int lastOffset = FileAccessData.byteOffset;
					unsigned int oldoffset = FileAccessData.offset;

					unsigned char fattybuff[5];
					unsigned char fattybuff2[5];
					unsigned char buff10[5];
					unsigned char buff9[5];


					lseek(fd, FileAccessData.byteOffset + (0 * 32) + 20, SEEK_SET);
					read(fd, buff6, 2);

					lseek(fd, FileAccessData.byteOffset + (0 * 32) + 26, SEEK_SET);
					read(fd, buff7, 2);

					buff7[2] = buff6[0];
					buff7[3] = buff6[1];
					unsigned int N;
					N = save4ByteRead(buff7);
					ollyfat = N;
					FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);
					unsigned int oldoffset2 = FileAccessData.offset;

					int end = 0;
					int needCluster = 0;
					int DataSectorOffset = 0;
					unsigned int H;
					int clusterindex = 0;
					int old = 0;
					while (end == 0)
					{
						if (DataSectorOffset == 512)
						{
							//printf("%d)-)", DataSectorOffset);
							needCluster = 1;
							DataSectorOffset = 0;

						}

						if (needCluster == 0) {
							lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);
							read(fd, buff1, 1);
							if (buff1[0] == 0x00)
							{
								//	printf("%x\n", FileAccessData.byteOffset);
								//	printf("hi");
								lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);
								write(fd, &buff4, 10);
								lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 11, SEEK_SET);
								write(fd, &buff2, 1);
								lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 20, SEEK_SET);
								write(fd, &buff66, 2);
								lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 26, SEEK_SET);
								write(fd, &buff77, 2);

								tokens[6] = tokens[1];
								tokens[1] = tokens[2];
								tokens[2] = tokens[6];
								tokens[6] = NULL;

								NameUpdated = 1;
								UpdateOrDisplayPath(3, tokens);
								NameUpdated = 0;


								/*
								printf("%x\n", buff6[0]);
								printf("%x\n", buff6[1]);

								printf("=====================\n");
								printf("%x\n", buff7[0]);
								printf("%x\n", buff7[1]);
								printf("=====================\n");
								*/


								lseek(fd, FileAccessData.byteOffset + 32 + 20, SEEK_SET);
								write(fd, &buff6, 2);
								lseek(fd, FileAccessData.byteOffset + 32 + 26, SEEK_SET);
								write(fd, &buff7, 2);

								end = 1;
								FileAccessData.byteOffset = lastOffset;
								break;
							}
							DataSectorOffset += 32;
							//printf("%d)))", DataSectorOffset);
						}

						//------------------------------------------------ need more room
						if (needCluster == 1)
						{

							FileAccessData.byteOffset=lastOffset;
							FileAccessData.offset = oldoffset;

							clusty = 0;
							NextClusterFromSource(fd, clusterindex++, 0);
							if (clusty == 1)
							{
								//printf("%d)---)", DataSectorOffset);
								char temp2[4];
								FileAccessData.offset = oldoffset2;
								//NextClusterFromSource(fd, clusterindex++);
								unsigned int temp = findFirstFreeCluster();

								//FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (ollyfat* 4);
								//printf("Next clusterssttttt %d\n", temp);
								//printf("Next clusters1s %x\n", FileAccessData.offset);

								//FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (temp * 4);
								//printf("Next clusterssttttt %d\n", temp);
								//printf("Next clusters2s %x\n", FileAccessData.offset);

								strcpy(temp2, BigToLil(temp, 4));
								//printf("Next clusterss %d\n", temp);
								//printf("Next clusterss232 %d\n", ollyfat);
								write2FAT(ollyfat, temp);
								write2FAT(temp, 0xFFFFFFFF);
								UpdateOffset(temp);
								//printf("Next clusters4s %x\n", FileAccessData.byteOffset);
								//return 'c';
								//break;
								clusty = 0;
							}
							needCluster = 0;
							DataSectorOffset = 0;
						}

					}


					FileAccessData.offset = oldoffset;
					FileAccessData.byteOffset = lastOffset;
					IsAFile = 0;
					FileIndex = 0;




				}
				else
				{
					printf("Not found");
				}

			}



			FileAccessData.offset = O2offset;
			FileAccessData.byteOffset = L2offset;
		}
		else
		{
			tokens[6] = tokens[1];
			tokens[1] = tokens[2];
			tokens[2] = tokens[6];
			tokens[6] = NULL;

			IsAFile = 0;
			last_fd = fd;
			checkfound = FindOrPrintDirectory(tokens, fd, 0);
			fd = last_fd;


			if (checkfound == 1)
			{

				strcpy(buff4, tokens[2]);
				buff4[12] = '\0';

				int index = 0;
				while (index <= strlen(buff4))
				{

					if (buff4[index] == ' ')
					{
						buff4[index] = '\0';
						break;
					}
					index++;
				}

				if (IsAFile == 1)
				{
					lseek(fd, FileAccessData.byteOffset + (FileIndex * 32), SEEK_SET);
					write(fd, &buff4, 10);
				}
				else
				{
					lseek(fd, FileAccessData.byteOffset + (DirIndex * 32), SEEK_SET);
					write(fd, &buff4, 10);
				}


			}
			else
			{
				printf("\n F or D Not found\n");
			}

		}
		//temptokens[1] = NULL;
	}
	else
	{
		printf("\n Invalid mv call\n");
		//temptokens[1] = NULL;
	}


	IsAFile = 0;
	//temptokens[1] = NULL;
	close(fd);

}




/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for open command.
-opens a file name FILENAME in the current dir.
-requirements:
-maintain table of open files and add FILENAME.
-open with mode r, rw, w, wr
-error if file is opened or DNE or invalid mode.
*/
unsigned char uc_openf(char** tokens)
{
	int
		fd = open("fat32.img", O_RDONLY),
		last_fd,
		checkfound;

	char
		* openName = fullFilePath(tokens);	//malloced string.

	unsigned char
		alreadyOpenQ = 0x00,
		*openFlags = (unsigned char*)malloc(1 * sizeof(unsigned char));

	if (tokens[2] == NULL)
	{
		fprintf(stdout, "\nno flags for open");
		free(openName);
		free(openFlags);
		return 0x00;
	}
	else
	{
		if ((strcmp(tokens[2], "rw") == 0) || (strcmp(tokens[2], "wr") == 0)) { *openFlags = O_RDWR; }
		else if (strcmp(tokens[2], "w") == 0) { *openFlags = O_WRONLY; }
		else if (strcmp(tokens[2], "r") == 0) { *openFlags = O_RDONLY; }
		else
		{
			fprintf(stdout, "\ninvalid flag (open)");
			free(openName);
			free(openFlags);
			return 0x00;
		}
	}

	IsAFile = 0;
	checkcurrent = 0;
	last_fd = fd;
	checkfound = FindOrPrintDirectory(tokens, fd, 0);   /// temp here
	fd = last_fd;

	if (checkfound == 1)
	{
		//file found.
		if (IsAFile == 1)
		{
			alreadyOpenQ = print6FindNodes(0x00, openName, O_RDWR);

			if (alreadyOpenQ == 0x00)
			{
				pushNode(openName, openFlags);
			}
			else
			{
				fprintf(stdout, "\n%s already open.", openName);
				free(openFlags);
				free(openFlags);
				return 0x00;
			}
		}
		else	//file not found.
		{
			printf("\n%sfile not found.", openName);
		}

	}

	/*
	char
	* openName = fullFilePath(tokens);
	int
	//* fd = (int*)malloc(1 * sizeof(int)),
	fd = open("fat32.img", O_RDONLY),
	last_fd,
	checkfound;
	unsigned char
	alreadyOpenQ,
	*openFlags = (unsigned char*)malloc(1 * sizeof(unsigned char));

	if (tokens[2] == NULL)
	{
	fprintf(stdout, "\nno flags (open)");
	//free(fd); free(openName); return 0x00;
	}
	else
	{
	if ((strcmp(tokens[2], "rw") == 0) || (strcmp(tokens[2], "wr") == 0)) { *openFlags = O_RDWR; }
	else if (strcmp(tokens[2], "w") == 0) { *openFlags = O_WRONLY; }
	else if (strcmp(tokens[2], "r") == 0) { *openFlags = O_RDONLY; }
	else
	{
	fprintf(stdout, "\ninvalid flag (open)");
	//free(fd); free(openName); return 0x00;
	}
	}

	*/
	//find if file exists here.
	/*
	//*fd = open(openName, *openFlags);

	if (errno != 0)
	{
	fprintf(stdout, "\ncould not open %s. errno: %d; fd: %d", tokens[1], errno, *fd);
	free(fd); free(openName); return 0x00;
	}
	else
	{
	alreadyOpenQ = print6FindNodes(0x00, openName, O_RDWR);

	if (alreadyOpenQ == 0x00)
	{
	pushNode(openName, fd, openFlags);
	}
	else
	{
	fprintf(stdout, "\n%s already open", openName);
	free(fd); free(openName); return 0x00;
	}
	}
	*/
	close(fd);
	return 0x00;
}


/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for close command.
-close a file names FILENAME.
-requirements:
-remove file entry from table.
-print error if file is not open or DNE.
*/
unsigned char uc_closef(char **tokens)
{
	unsigned char
		found = 0x00,
		idx = 0x00;
	char
		* cmpString = fullFilePath(tokens);	//malloced string.

	removeNode(cmpString);
	free(cmpString);
	fprintf(stdout, "\nfound (close): %02hhX", found);

	return found;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for read command.
-read data from file in current dir with name FILENAME.
-requirements:
-start reading at offset and stop after size # of bytes.
-if offset + size > filesize then read filesize - offset bytes.
-error if FILENAME DNE or is a dir or not open for reading or offset > filesize.
*/
unsigned char uc_readf(char **tokens)
{
	int fd = open("fat32.img", O_RDONLY);

	int checkfound;
	int last_fd;
	int offsetsize;
	int readsize;
	unsigned char buff1[4];

	unsigned char buff2[3];
	unsigned char buff4[12];
	unsigned char buff6[5];
	unsigned char buff7[5];
	unsigned char fattybuff[5];
	unsigned char fattybuff2[5];

	unsigned char BIGBOYBUFF[33];
	//BIGBOYBUFF[32] = '\0';

	unsigned int OGbyteoffset = FileAccessData.byteOffset;
	unsigned int OGoffset = FileAccessData.offset;

	char*
		openFileQ;


	if (tokens[1] == NULL || tokens[2] == NULL || tokens[3] == NULL)//token has a name,offset and size
	{
		printf("\n Invalid read call \n");
	}
	else
	{
		//***OPEN CHECK ***
		openFileQ = fullFilePath(tokens);
		if (print6FindNodes(0x00, openFileQ, O_RDONLY) == 0x00)
		{
			// print error message
			fprintf(stdout, "\n%s not open for reading.", tokens[1]);
			FileAccessData.byteOffset = OGbyteoffset;
			FileAccessData.offset = OGoffset;
			close(fd);
			return 'c';
		}
		else {}
		free(openFileQ);

		offsetsize = atoi(tokens[2]);
		readsize = atoi(tokens[3]);


		ClusterCounter = 0;
		checkcurrent = 0;
		IsAFile = 0;
		last_fd = fd;
		checkfound = FindOrPrintDirectory(tokens, fd, 0);
		fd = last_fd;

		//printf("%d===\n", ClusterCounter);

		if (checkfound == 1)
		{
			if (IsAFile == 1)
			{
				if (ClusterCounter > 0)
				{

					NextClusterFromSource(fd, ClusterCounter - 1, 0);
					//printf("%x\n", FileAccessData.byteOffset);
				}




				printf("\nFile found \n");
				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32) + 28, SEEK_SET);
				read(fd, buff1, 4);
				int filemax = L2Bint(buff1, 4);


				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32) + 20, SEEK_SET);
				read(fd, buff6, 2);

				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32) + 26, SEEK_SET);
				read(fd, buff7, 2);

				buff7[2] = buff6[0];
				buff7[3] = buff6[1];
				unsigned int N;
				N = save4ByteRead(buff7);

				UpdateOffset(N);
				FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);


				int trueoffset;
				int truesize;
				int clusterindex = 0;


				if (offsetsize > filemax)
				{

					printf("Out of bonds call");
					FileAccessData.byteOffset = OGbyteoffset;
					FileAccessData.offset = OGoffset;
					close(fd);
					return 'c';
				}

				if ((offsetsize + readsize) > filemax)
				{
					readsize = filemax - offsetsize;

				}

				if (readsize > 32)
				{
					while (offsetsize > 512)
					{
						FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);
						NextClusterFromSource(fd, clusterindex++, 1);
						offsetsize -= 512;

						//if(offsetsize%1000==0)
						//	printf("\noffset: %d\n", offsetsize);
					}
					printf("\noffset: %d\n", offsetsize);


					int DataSectorOffset = 0;
					clusty = 0;
					do {

						if (readsize - 32 > 0)
						{
							truesize = 32;
						}
						else
						{
							truesize = readsize;
						}

						unsigned char BIGBOYBUFF2[33];
						memset(BIGBOYBUFF2, 0, sizeof BIGBOYBUFF2);
						lseek(fd, FileAccessData.byteOffset + offsetsize + DataSectorOffset, SEEK_SET);
						read(fd, BIGBOYBUFF2, truesize);

						int index = 0;
						while (index <= strlen(BIGBOYBUFF2))
						{

							if (BIGBOYBUFF2[index] == 0)
							{
								BIGBOYBUFF2[index] = '\0';
								break;
							}
							index++;
						}

						printf("%s", BIGBOYBUFF2);


						DataSectorOffset += 32;
						if (DataSectorOffset == 512)
						{
							//printf("\n need cluster\n");
							FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);
							NextClusterFromSource(fd, clusterindex++, 1);

							if (clusty == 1)
							{
								printf("\n breaking\n");
								break;
							}
							DataSectorOffset = 0;
						}

						offsetsize = 0;
						readsize -= 32;
					} while (readsize>0);

				}
				else
				{
					unsigned char BIGBOYBUFF[33];
					memset(BIGBOYBUFF, 0, sizeof BIGBOYBUFF);
					lseek(fd, FileAccessData.byteOffset + offsetsize, SEEK_SET);
					read(fd, BIGBOYBUFF, readsize);
					int index = 0;
					while (index <= strlen(BIGBOYBUFF))
					{

						if (BIGBOYBUFF[index] == 0)
						{
							BIGBOYBUFF[index] = '\0';
							break;
						}
						index++;
					}

					printf("%s", BIGBOYBUFF);
				}

				printf("\n%d\n", filemax);

			}
			else
			{
				printf("\n Dir entered \n");
			}

		}
		else
		{
			printf("\n File not found \n");
		}

	}
	FileAccessData.byteOffset = OGbyteoffset;
	FileAccessData.offset = OGoffset;
	close(fd);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for write command.
-write to a file in current dir with name FILENAME.
-requirements:
-start writing at offset and stop write after size # of bytes
-if offset + size > filesize extend filesize to hold STRING.
-if STRING > size write size # of chars.
-if STRING < size write remaining characters as NULL.
-error if FILENAME DNE or is dir or is not open for writing.
*/
unsigned char uc_writef(char **tokens)
{
	copy6Write(tokens, 0x01);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
functionality for rm command.
-delete FILENAME from current dir.
-requirements:
-remove entry in the directory
-reclaim file data.
-error if FILENAME DNE or is a dir.
*/
unsigned char uc_rmf(char **tokens)
{
	fprintf(stdout, "\nrm instruction functionality");
	//int fd = open("fat32.img", O_RDONLY);
	int fd = open("fat32.img", O_RDWR);
	int last_fd;

	int checkfound;

	unsigned int temp;
	unsigned int lastOffset;
	unsigned char buff[2];
	unsigned char buff2[2];
	unsigned char buff6[5];
	unsigned char buff7[5];
	unsigned char fattybuff[5];
	unsigned char fattybuff2[5];
	buff[0] = 0xE5;
	buff[1] = '\0';

	//unsigned char * temp2 = (char*)malloc(4 * sizeof(char));

	if (tokens[1] != NULL)
	{
		last_fd = fd;
		checkfound = FindOrPrintDirectory(tokens, fd, 0);
		fd = last_fd;

		if (checkfound == 0)
		{

		}
		else
		{
			lastOffset = FileAccessData.byteOffset;

			//last_fd = fd;
			//temp = FindOrPrintDirectory(tokens, fd, 3);
			//fd = last_fd;
			//UpdateOffset(temp);
			//printf("%x\n", FileAccessData.byteOffset);

			if (IsAFile == 1)
			{

				//printf("\n===>%d\n", FileIndex);
				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32), SEEK_SET);
				//read(fd, buff, 1);
				//printf("%x:\n", buff[0]);
				//printf("%s:\n", buff[0]); 
				unsigned char  temp2;
				temp2 = 0xE5;
				write(fd, &temp2, 1);

				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32), SEEK_SET);
				read(fd, buff2, 1);
				//printf("\n%x:\n", buff2[0]);
				//printf("\n%x:\n", buff2);
				//printf("\n%s:\n", buff2);
				//printf("%x\n", FileAccessData.byteOffset+ 3*32);
				//if (buff2[0] == 0xE5) { printf("\noooo\n"); }

				//=====================================================================================

				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32) + 20, SEEK_SET);
				read(fd, buff6, 2);

				lseek(fd, FileAccessData.byteOffset + (FileIndex * 32) + 26, SEEK_SET);
				read(fd, buff7, 2);

				buff7[2] = buff6[0];
				buff7[3] = buff6[1];
				unsigned int  g = save4ByteRead(buff7);
				//printf("\n%d\n",g);

				unsigned int N;
				N = g;
				printf("\n%d:\n", g);
				unsigned int oldoffset = FileAccessData.offset;
				do
				{
					FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *BPB_Info.BPB_BytsPerSec + (N * 4);

					lseek(fd, FileAccessData.offset, SEEK_SET);
					read(fd, fattybuff, 4);
					lseek(fd, FileAccessData.offset, SEEK_SET);

					fattybuff2[0] = fattybuff[0];
					fattybuff2[1] = fattybuff[1];
					fattybuff2[2] = fattybuff[2];
					fattybuff2[3] = fattybuff[3];

					fattybuff2[0] = fattybuff2[2];
					fattybuff2[1] = fattybuff2[3];

					fattybuff2[3] = fattybuff[0];
					fattybuff2[2] = fattybuff[1];


					N = save4ByteRead(fattybuff);


					unsigned char temp3 = 0x00;
					int i = 0;
					while (i < 4)
					{
						write(fd, &temp3, 1);
						i++;
					}
					//FileAccessData.offset = oldoffset;
					//printf("\n%x:\n", fattybuff[0]);
					//printf("\n%x:\n", fattybuff[1]);
					///printf("\n%d:\n", h);
				} while ((fattybuff2[0] != 0xF8 && fattybuff2[1] != 0xFF && fattybuff2[2] != 0xFF && fattybuff2[3] != 0x0F) ||
					(fattybuff2[0] != 0xFF && fattybuff2[1] != 0xFF && fattybuff2[2] != 0xFF && fattybuff2[3] != 0x0F) ||
					(fattybuff2[0] != 0xFF && fattybuff2[1] != 0xFF && fattybuff2[2] != 0xFF && fattybuff2[3] != 0xFF));

				FileAccessData.offset = oldoffset;
				IsAFile = 0;
				FileIndex = 0;
				printf("\nFile rmoved\n");
			}
			else
			{
				printf("\nFile not found\n");
			}
			FileAccessData.byteOffset = lastOffset;
		}
	}
	close(fd);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/
unsigned char * BigToLil(unsigned int value, unsigned int size)
{
	unsigned char * temp = (char*)malloc(4 * sizeof(char));
	unsigned int temp_val = value, m = 0;
	int i;
	for (i = 0; i<size; i++)
	{
		m = 0x000000FF << (i * 8);
		temp[i] = (temp_val & m) >> (i * 8);
	}
	return temp;
}
/*
functionality for cp command.
-copy file specified by FILENAME.
-requirements:
-if TO is a dir then copy the file directly into folder.
-if TO is not valid, create a copy of file within current dir & name it TO
-error if FILENAME DNE.
*/
unsigned char uc_cpf(char **tokens)
{
	copy6Write(tokens, 0x00);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

unsigned char printBPBInfo()
{
	fprintf(stdout, "\nbytes per sector: %hu."
		"\nsectors per cluster: %hhu."
		"\nreserved section size: %hu."
		"\n# of FATs: %hhu."
		"\ntotal # of sectors: %u."
		"\nFAT size: %u."
		"\nroot cluster: %u.\n",
		BPB_Info.BPB_BytsPerSec,
		BPB_Info.BPB_SecPerClus,
		BPB_Info.BPB_RsvdSecCnt,
		BPB_Info.BPB_NumFATs,
		BPB_Info.BPB_TotSec32,
		BPB_Info.BPB_FATSz32,
		BPB_Info.BPB_RootClus
	);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

unsigned char grabBPBInfo()
{
	char buf[5];

	int fd = open("fat32.img", O_RDONLY);
	unsigned short ui16_tmp = 0x0000; //0x0000, 0b0000 0000 0000 0000.
	unsigned char ui8_tmp = 0x00;

	//grab BytesPerSec.
	lseek(fd, 11, SEEK_SET);
	read(fd, buf, 2);
	ui16_tmp = (buf[1] & 0x00FF); ui16_tmp <<= 0x08; ui16_tmp |= (buf[0] & 0x00FF);
	BPB_Info.BPB_BytsPerSec = ui16_tmp;
	ui16_tmp = 0x0000;

	//grab SecPerClus.
	lseek(fd, 13, SEEK_SET);
	read(fd, buf, 1);
	ui8_tmp = buf[0];
	BPB_Info.BPB_SecPerClus = ui8_tmp;
	ui8_tmp = 0x00;

	//grab RsvdSecCnt.
	lseek(fd, 14, SEEK_SET);
	read(fd, buf, 2);
	ui16_tmp = (buf[1] & 0x00FF); ui16_tmp <<= 0x08; ui16_tmp |= (buf[0] & 0x00FF);
	BPB_Info.BPB_RsvdSecCnt = ui16_tmp;
	ui16_tmp = 0x0000;

	//grab NumFATs.
	lseek(fd, 16, SEEK_SET);
	read(fd, buf, 1);
	ui8_tmp = buf[0];
	BPB_Info.BPB_NumFATs = ui8_tmp;
	ui8_tmp = 0x00;

	//grab TotSec32.
	lseek(fd, 32, SEEK_SET);
	read(fd, buf, 4);
	BPB_Info.BPB_TotSec32 = save4ByteRead(buf);

	//grab FATSz32.
	lseek(fd, 36, SEEK_SET);
	read(fd, buf, 4);
	BPB_Info.BPB_FATSz32 = save4ByteRead(buf);

	//grab RootClus.
	lseek(fd, 44, SEEK_SET);
	read(fd, buf, 4);
	BPB_Info.BPB_RootClus = save4ByteRead(buf);

	close(fd);
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

unsigned int save4ByteRead(char* buf)
{
	unsigned int ui32_tmp[5] = { 0x00000000 };
	ui32_tmp[1] = (buf[3] & 0x000000FF); ui32_tmp[1] <<= 6 * 0x04;
	ui32_tmp[2] = (buf[2] & 0x000000FF); ui32_tmp[2] <<= 4 * 0x04;
	ui32_tmp[3] = (buf[1] & 0x000000FF); ui32_tmp[3] <<= 2 * 0x04;
	ui32_tmp[4] = (buf[0] & 0x000000FF);
	ui32_tmp[0] = (ui32_tmp[1] | ui32_tmp[2] | ui32_tmp[3] | ui32_tmp[4]);
	return ui32_tmp[0];
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

/*
saves up to 5 tokens. last followed by NULL.
*/
unsigned char grab7saveInput(char **tokens)
{
	char *tmpInput = NULL, *tmp = NULL;
	unsigned char len = 0x00, idx = 0x00;

	// pass in temp here just to satifiy function call pass in new name when updatting
	UpdateOrDisplayPath(0, tokens);
	do
	{
		scanf("%ms", &tmpInput);
		len = strlen(tmpInput);
		tmp = (char*)malloc((len + 1) * (sizeof(char)));
		memcpy(tmp, tmpInput, len);
		tmp[len] = '\0';
		if (idx < 5)
		{
			tokens[idx] = tmp;
			idx += 0x01;
		}
	} while ('\n' != getchar());

	tokens[idx] = NULL;

	return 0x00;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

unsigned char cleanTokens(char** tokens)
{
	unsigned char pidx = 0x00;
	while (tokens[pidx] != NULL)
	{
		free(tokens[pidx]);
		tokens[pidx] = NULL;
		pidx += 0x01;
	}
	return 0x00;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
*/

unsigned char whichCommand(char** tokens)
{
	if (strcmp(tokens[0], "exit") == 0)
	{
		return 0x00;
	}
	else if (strcmp(tokens[0], "info") == 0)
	{
		uc_infof(tokens);
	}
	else if (strcmp(tokens[0], "size") == 0)
	{
		uc_sizef(tokens);
	}
	else if (strcmp(tokens[0], "ls") == 0)
	{
		uc_lsf(tokens);
		return 0x02;
	}
	else if (strcmp(tokens[0], "cd") == 0)
	{
		uc_cdf(tokens);
	}
	else if (strcmp(tokens[0], "creat") == 0)
	{
		uc_creatf(tokens);
	}
	else if (strcmp(tokens[0], "mkdir") == 0)
	{
		uc_mkdirf(tokens);
	}
	else if (strcmp(tokens[0], "mv") == 0)
	{
		uc_mvf(tokens);
	}
	else if (strcmp(tokens[0], "open") == 0)
	{
		uc_openf(tokens);
	}
	else if (strcmp(tokens[0], "close") == 0)
	{
		uc_closef(tokens);
	}
	else if (strcmp(tokens[0], "read") == 0)
	{
		uc_readf(tokens);
	}
	else if (strcmp(tokens[0], "write") == 0)
	{
		uc_writef(tokens);
	}
	else if (strcmp(tokens[0], "rm") == 0)
	{
		uc_rmf(tokens);
	}
	else if (strcmp(tokens[0], "cp") == 0)
	{
		uc_cpf(tokens);
	}
	else if (strcmp(tokens[0], "rmdir") == 0)
	{
		fprintf(stdout, "\nrmdir functionality not added.");
	}
	else if (strcmp(tokens[0], "comAdd") == 0)
	{
		fprintf(stdout, "\ncomputed address: %X", ui_comAdd(atoi(tokens[1])));
	}
	else if (strcmp(tokens[0], "scanFAT") == 0)
	{
		ui_scanFAT(0x01);
	}
	else if (strcmp(tokens[0], "POF") == 0)
	{
		print6FindNodes(0x01, (char*)NULL, O_RDWR);
	}
	else
	{
		fprintf(stdout, "\ninvalid instruction");
		return 0x03;
	}
	return 0x01;
}

/*
_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

0x01 initialize
0x00 update next free.
*/

unsigned int ui_scanFAT(unsigned char opCode)
{
	int
		fd = open("fat32.img", O_RDWR),
		lseekOffset = 0x00000000;

	unsigned char
		buf[5],
		fAClus = 0x00;		//first available cluster.
	buf[4] = '\0';

	unsigned int
		FATStart = (BPB_Info.BPB_RsvdSecCnt * BPB_Info.BPB_BytsPerSec),
		FATEnd = (BPB_Info.BPB_FATSz32 * BPB_Info.BPB_BytsPerSec) + FATStart,
		FATVal = 0xFFFFFFFF,
		FSInfoAddress = ui_comAdd(0x00000001),
		idx;

	if (opCode == 0x01)
	{
		FSInfo.FSI_Free_Count = 0x00000000;
		FSInfo.FSI_Nxt_Free = 0x00000000;
		for (idx = FATStart; idx < FATEnd; idx += 0x00000004)
		{
			lseekOffset = lseek(fd, idx, SEEK_SET);
			read(fd, buf, 4);
			FATVal = save4ByteRead(buf);
			if (FATVal == 0x00000000)
			{
				FSInfo.FSI_Free_Count++;
				if (fAClus == 0x00) { FSInfo.FSI_Nxt_Free = lseekOffset; fAClus = 0x01; }
				else {}
			}
		}
		FSInfo.FSI_LeadSig = 0x41615252;
		FSInfo.FSI_StructSig = 0x61417272;
		FSInfo.FSI_TrailSig = 0xAA550000;
		FSInfo.FSI_FATStart = FATStart;
		FSInfo.FSI_FATEnd = FATEnd;
		for (idx = 0x00000000; idx < 488; idx++) { FSInfo.Reserved1[idx] = 0x00; }
		for (idx = 0x00000000; idx < 12; idx++) { FSInfo.Reserved2[idx] = 0x00; }
		FSInfo.LeadSig[0] = 0x52; FSInfo.LeadSig[1] = 0x52;
		FSInfo.LeadSig[2] = 0x61; FSInfo.LeadSig[3] = 0x41;
		FSInfo.StructSig[0] = 0x72; FSInfo.StructSig[1] = 0x72;
		FSInfo.StructSig[2] = 0x41; FSInfo.StructSig[3] = 0x61;
		FSInfo.TrailSig[0] = 0x00; FSInfo.TrailSig[1] = 0x00;
		FSInfo.TrailSig[2] = 0x55; FSInfo.TrailSig[3] = 0xAA;
		lseek(fd, FSInfoAddress, SEEK_SET);
		write(fd, FSInfo.LeadSig, 4);
		lseek(fd, FSInfoAddress + 4, SEEK_SET);
		write(fd, FSInfo.Reserved1, 480);
		lseek(fd, FSInfoAddress + 484, SEEK_SET);
		write(fd, FSInfo.StructSig, 4);
		lseek(fd, FSInfoAddress + 488, SEEK_SET);
		ui_int2Buf(buf, FSInfo.FSI_Free_Count);
		write(fd, buf, 4);
		lseek(fd, FSInfoAddress + 492, SEEK_SET);
		ui_int2Buf(buf, FSInfo.FSI_Nxt_Free);
		write(fd, buf, 4);
		lseek(fd, FSInfoAddress + 496, SEEK_SET);
		write(fd, FSInfo.Reserved2, 12);
		lseek(fd, FSInfoAddress + 508, SEEK_SET);
		write(fd, FSInfo.TrailSig, 4);
	}
	else
	{
		idx = FATStart + 12;
		while ((FATVal != 0x00000000) && (lseekOffset < FATEnd))
		{
			lseekOffset = lseek(fd, idx, SEEK_SET);
			read(fd, buf, 4);
			FATVal = save4ByteRead(buf);
			idx += 0x00000004;
		}
		FSInfo.FSI_Nxt_Free = lseekOffset;
		lseek(fd, FSInfoAddress + 492, SEEK_SET);
		ui_int2Buf(buf, FSInfo.FSI_Nxt_Free);
		write(fd, buf, 4);
		return FSInfo.FSI_Nxt_Free;
	}
	close(fd);
	return 0x00000000;
}



void initOpenFileList()
{
	openFileHead = (Node*)malloc(1 * sizeof(Node));
	char bogusString[] = "%EMPTY-NODE%";
	char *s_save = (char*)malloc((strlen(bogusString) + 1) * sizeof(char));
	strcpy(s_save, bogusString);
	openFileHead->nxt = NULL;
	openFileHead->prev = NULL;

	//openFileHead->i_open = NULL; 

	openFileHead->s_open = s_save;
	openFileHead->uc_openMode = NULL;
}

void cleanUP()
{
	Node
		* curNode = openFileHead,
		*tmpNode = NULL;
	while (curNode != NULL)
	{
		tmpNode = curNode;
		curNode = curNode->nxt;
		if (tmpNode->s_open != NULL)
		{
			free(tmpNode->s_open);
		}

		/*
		if (tmpNode->i_open != NULL)
		{
		free(tmpNode->i_open);
		}
		*/

		if (tmpNode->uc_openMode != NULL)
		{
			free(tmpNode->uc_openMode);
		}
		free(tmpNode);
	}
}


unsigned int ui_comAdd(unsigned int inOffset)
{
	unsigned int
		//inOffset = 0x00000000,
		tmpOffset,
		byteOffset;

	//inOffset = atoi(tokens[1]);
	tmpOffset = FileAccessData.firstDataSector + ((inOffset - 2) * BPB_Info.BPB_SecPerClus);
	byteOffset = tmpOffset * BPB_Info.BPB_BytsPerSec;
	return byteOffset;
}

void ui_int2Buf(char *buf, unsigned int toConvert)
{
	unsigned int
		tmp[4] = { 0x00000000 };

	tmp[0] = (toConvert & 0xFF000000); tmp[0] >>= (6 * 0x04);
	tmp[1] = (toConvert & 0x00FF0000); tmp[1] >>= (4 * 0x04);
	tmp[2] = (toConvert & 0x0000FF00); tmp[2] >>= (2 * 0x04);
	tmp[3] = (toConvert & 0x000000FF);
	buf[0] = (tmp[3] & 0x000000FF);
	buf[1] = (tmp[2] & 0x000000FF);
	buf[2] = (tmp[1] & 0x000000FF);
	buf[3] = (tmp[0] & 0x000000FF);
}

/*
s = select. 1 for print, 0 for find.
*/
unsigned char print6FindNodes(unsigned char s, char* cmpString, int openMode)
{
	Node
		* curNode = openFileHead;

	unsigned char
		found = 0x00;
	if (s == 0x00)
	{
		while (curNode != NULL)
		{
			if (strcmp(cmpString, curNode->s_open) == 0)
			{
				//
				if (openMode == O_WRONLY)
				{
					if ((*curNode->uc_openMode == O_WRONLY) || (*curNode->uc_openMode == O_RDWR))
					{
						return 0x01;
					}
				}
				else if (openMode == O_RDONLY)
				{
					if ((*curNode->uc_openMode == O_RDONLY) || (*curNode->uc_openMode == O_RDWR))
					{
						return 0x01;
					}
				}
				else if (openMode == O_RDWR)
				{
					return 0x01;
				}
				else {}
				break;
			}
			curNode = curNode->nxt;
		}
	}
	else
	{
		while (curNode != NULL)
		{
			fprintf(stdout, "\nNode s_open: %s", curNode->s_open);
			curNode = curNode->nxt;
		}
	}
	return found;
}

unsigned short us_RsvdSecCnt()
{
	return BPB_Info.BPB_RsvdSecCnt;
}

void write2FAT(unsigned int cluster, unsigned int value)
{
	int fd = open("fat32.img", O_RDWR);
	unsigned int clusterValue = 0;
	unsigned char temp[4];

	strcpy(temp, BigToLil(value, 4));

	unsigned FATsectorNum = (unsigned int)(us_RsvdSecCnt()) + ((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

	unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());
	unsigned int byteindex = fatstart + (cluster * 4);

	lseek(fd, byteindex, SEEK_SET);
	write(fd, temp, 4);
}


unsigned int findNextcluster(unsigned int cluster)
{
	int fd = open("fat32.img", O_RDONLY);
	unsigned char temp[4];
	unsigned int FATsectorNum = (unsigned int)(us_RsvdSecCnt()) + ((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

	unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());
	unsigned int byteindex = fatstart + (cluster * 4);

	lseek(fd, byteindex, SEEK_SET);
	read(fd, temp, 4);
	//printf("\n%u\n",temp);
	return L2Bint(temp, 4);
}

unsigned char getSecperClus()
{
	return BPB_Info.BPB_SecPerClus;
}

unsigned int getRootCluster()
{
	return  BPB_Info.BPB_RootClus;
}
unsigned short getBytsperSector()
{
	return BPB_Info.BPB_BytsPerSec;
}


unsigned char getNumFATs()
{
	return BPB_Info.BPB_NumFATs;
}
unsigned int getFATSz32()
{
	return BPB_Info.BPB_FATSz32;
}

unsigned int L2Bint(unsigned char * value, unsigned int size)
{
	unsigned int total = 0;
	int i;
	for (i = 0; i<size; i++)
	{
		total += (unsigned int)(value[i]) << (i * 8);
	}
	return total;
}

void writesize2dir(unsigned int address, unsigned int value)
{
	//printf("\ndoing size now\n");
	int fd = open("fat32.img", O_RDWR);
	unsigned char * temp = (char*)malloc(4 * sizeof(char));
	strcpy(temp, BigToLil(value, 4));
	lseek(fd, address + 28, SEEK_SET);
	write(fd, temp, 4);
}

//write size to dir entry
void writeClus2Dir(unsigned int address, unsigned int value)
{
	int fd = open("fat32.img", O_RDWR);
	unsigned char * temp = (char*)malloc(4 * sizeof(char));
	unsigned char * low = (char*)malloc(2 * sizeof(char));
	unsigned char * high = (char*)malloc(2 * sizeof(char));

	strcpy(temp, BigToLil(value, 4));

	int i;
	for (i = 0; i<2; i++)
	{
		low[i] = temp[i];
		//printf("\nlow %u\n",low[i]);
		high[i] = temp[i + 2];
		//printf("\nhigh %u\n",high[i]);
	}

	lseek(fd, address + 20, SEEK_SET);
	write(fd, high, 2);
	lseek(fd, address + 26, SEEK_SET);
	write(fd, low, 2);
}

char* fullFilePath(char **tokens)
{
	int
		pathlen = strlen(FileAccessData.pathName),
		tokens1len = strlen(tokens[1]);
	char
		*newString;
	newString = (char*)malloc((pathlen + tokens1len + 1) * sizeof(char));
	strcpy(newString, FileAccessData.pathName);
	newString[pathlen - 1] = '/';	//change pathName '$' to '/'
	newString[pathlen] = '\0';		//not needed. here JIC.
	strcat(newString, tokens[1]);
	return newString;
}

//*
//void pushNode(char* s_toAdd, int* i_toAdd, unsigned char* openFlags)
void pushNode(char* s_toAdd, unsigned char* openFlags)
{
	Node
		* curNode = openFileHead;

	while (curNode->nxt != NULL)
	{
		curNode = curNode->nxt;
	}

	curNode->nxt = (Node*)malloc(1 * sizeof(Node));

	curNode->nxt->s_open = s_toAdd;
	//curNode->nxt->i_open = i_toAdd;
	curNode->nxt->uc_openMode = openFlags;


	curNode->nxt->prev = curNode;
	curNode->nxt->nxt = NULL;
}


unsigned char removeNode(char* cmpString)
{
	Node
		* curNode = NULL;
	curNode = openFileHead;

	unsigned char
		found = 0x00,
		opCode = 0x00;

	while (curNode != NULL)
	{
		if (strcmp(cmpString, curNode->s_open) == 0)
		{
			found = 0x01;
			//close(*curNode->i_open);
			break;
		}
		else
		{
			curNode = curNode->nxt;
		}
	}

	if (found == 0x00) { return 0x00; }	//path not found.

										//path found, delete.

	if (curNode->prev == NULL)	//first Node.
	{

	}
	else	//not first Node.
	{
		opCode |= 0x01;
	}
	if (curNode->nxt == NULL)	//last Node.
	{

	}
	else	//not last Node.
	{
		opCode |= 0x02;
	}

	switch (opCode)
	{
	case 0x00:	//no previous Node & no nxt Node.
	{
		break;
	}
	case 0x01:	//previous Node but no nxt Node.
	{
		//free data.
		//set prev->nxt to NULL.
		//delete Node.

		//free(curNode->i_open); 
		free(curNode->s_open);
		free(curNode->uc_openMode);

		curNode->prev->nxt = NULL;
		free(curNode);
		break;
	}
	case 0x02:	//no previous node but nxt Node
	{
		//free data.
		//change openFileHead to ->nxt
		//delete Node.

		//free(curNode->i_open); 
		free(curNode->s_open);
		free(curNode->uc_openMode);

		openFileHead = curNode->nxt;
		free(curNode);
		break;
	}
	case 0x03:	//previous Node & nxt Node.
	{
		//free data.
		//prev->next = ->nxt
		//delete Node.

		//free(curNode->i_open); 
		free(curNode->s_open);
		free(curNode->uc_openMode);

		curNode->prev->nxt = curNode->nxt;
		curNode->nxt->prev = curNode->prev;
		free(curNode);
		break;
	}
	default:
	{
		break;
	}
	}

}

unsigned int findFirstSector(unsigned int cluster)
{
	unsigned int firstDataSector, firstSectorofClusters;

	firstDataSector = (unsigned int)(us_RsvdSecCnt()) + ((unsigned int)(getNumFATs()) * getFATSz32());
	firstSectorofClusters = firstDataSector + ((cluster - 2)*(unsigned int)(getSecperClus()));

	return (firstSectorofClusters *(unsigned int)(getBytsperSector()));
}

unsigned int findFirstFreeCluster()
{
	int fd = open("fat32.img", O_RDONLY);

	unsigned int tempcluster = 0, clustervalue = 0;
	unsigned char temp[4];

	unsigned int FATsectorNum = (unsigned int)(us_RsvdSecCnt()) +
		((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

	unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());

	unsigned int currentbyte = fatstart;

	lseek(fd, fatstart, SEEK_SET);
	read(fd, temp, 4);

	clustervalue = L2Bint(temp, 4);

	while (clustervalue != 0X00000000 && currentbyte < (fatstart + getFATSz32() *
		(unsigned int)(getBytsperSector())))
	{
		tempcluster++;
		currentbyte += 4;
		lseek(fd, currentbyte, SEEK_SET);
		read(fd, temp, 4);
		clustervalue = L2Bint(temp, 4);
	}
	if (clustervalue != 0x00000000)
	{
		printf("\nThere are no free cluster\n");
	}
	return tempcluster;
}

void NextClusterFromSource(int fd, int clusterindex, int select)
{
	//int fd = open("fat32.img", O_RDWR);
	//printf("entering\n");
	unsigned char fattybuff[5];
	unsigned char fattybuff2[5];

	unsigned char buff6[5];
	unsigned char buff7[5];

	if (select == 0)
	{

		lseek(fd, FileAccessData.byteOffset + (0 * 32) + 20, SEEK_SET);
		read(fd, buff6, 2);

		lseek(fd, FileAccessData.byteOffset + (0 * 32) + 26, SEEK_SET);
		read(fd, buff7, 2);

		buff7[2] = buff6[0];
		buff7[3] = buff6[1];

		unsigned int N;
		N = save4ByteRead(buff7);
		FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *
			BPB_Info.BPB_BytsPerSec + (N * 4);
	}


	int i = 0;
	int H = 0;


	do {
		lseek(fd, FileAccessData.offset, SEEK_SET);
		read(fd, fattybuff, 4);
		lseek(fd, FileAccessData.offset, SEEK_SET);

		fattybuff2[0] = fattybuff[0];
		fattybuff2[1] = fattybuff[1];
		fattybuff2[2] = fattybuff[2];
		fattybuff2[3] = fattybuff[3];

		fattybuff2[0] = fattybuff2[2];
		fattybuff2[1] = fattybuff2[3];

		fattybuff2[3] = fattybuff[0];
		fattybuff2[2] = fattybuff[1];

		H = save4ByteRead(fattybuff);
		//ollyfat = H;
		FileAccessData.offset = BPB_Info.BPB_RsvdSecCnt *
			BPB_Info.BPB_BytsPerSec + (H * 4);
		/*
		printf("%x:\n", fattybuff2[0]);
		printf("%x:\n", fattybuff2[1]);
		printf("%x:\n", fattybuff2[2]);
		printf("%x:\n", fattybuff2[3]);

		*/
		//	printf("Next cluster %x\n", FileAccessData.offset);

		if ((fattybuff2[0] == 0xF8 && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0x0F) ||
			(fattybuff2[0] == 0xFF && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0x0F) ||
			(fattybuff2[0] == 0xFF && fattybuff2[1] == 0xFF && fattybuff2[2] == 0xFF && fattybuff2[3] == 0xFF))
		{
			clusty = 1;
			return;
		}
		if (i == clusterindex) { break; }
		i++;
	} while (1);
	//printf("::\n");
	//printf("%x::\n", FileAccessData.byteOffset);
	UpdateOffset(H);
	//printf("returning\n");
	//printf("%x::\n", FileAccessData.offset);

	//close(fd);
}