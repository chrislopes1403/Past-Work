char* string2FAT (char*);
void writesize2dir(unsigned int address, unsigned int value);
unsigned char * BigToLil (unsigned int value, unsigned int size);
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
	fprintf(stdout, "\nread instruction functionality");
	int fd = open("fat32.img",O_RDWR);

	unsigned char* name=tokens[1];
	unsigned int offset = atoi(tokens[2]);
	unsigned int size = atoi(tokens[3]);
	
	//printf("\n%s\n%d\n%d\n",name,offset,size);
	int MoreDataLeft = 1;
	int DataSectorOffset = 0;
	char attbuff2[3];   // DirAtt
	char namebuff4[12];
	unsigned char filesize[4];

	if (tokens[1]!=NULL && tokens[2]!=NULL && tokens[3]!=NULL)//token has a name,offset and size
        {
		while (MoreDataLeft!=0)
		{
			if (DataSectorOffset == 32000)
			{
				break;
			}

			lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

			read(fd,namebuff4, 10);
			namebuff4[12] = '\0';

			read(fd, attbuff2, 2);
			attbuff2[2] = '\0';
			

			string2FAT(tokens[1]);

			if (strcmp(namebuff4, tokens[1]) == 0)
			{//FILE FOUND IN DIRECTORY
				printf("\nfile found in directory\n");
	
				lseek(fd,28,SEEK_CUR);
				read(fd,filesize,4);

				if (attbuff2[1]==0x20)//correct file type
				{
					printf("file is correct type");

					//THIGS TO FIX:
					//check if file is open properly, check if file is opened in correct mode properly (r or rw)
					//
					//

					int alreadyopen = 1;//print6FindNodes(0,fullFilePath(tokens));
					if (alreadyopen!=0x00)//file is open
					{
						printf("\nFile is OPENNN\n");
						//Node* curnode = findFile(name);
						/* 
						 * CHECK IF MODE IS WRITE ONLY
						 */
						unsigned int numbytes;
						if (alreadyopen!=0x00)//mode is read)
						{
							lseek(fd, FileAccessData.byteOffset + DataSectorOffset + 28, SEEK_SET);
							read(fd, filesize, 4);
							unsigned int x = L2Bint(filesize, 4);
							printf("\nsize of file is %d\n",x);
							if (x==0)
							{
								printf("\nFile is an empty file'n");
							}
							else if (offset<x)
							{
								//check if byte size is too large
								if((offset + size)>x)
								{
									numbytes=x-offset;
								}
								else
								{
									numbytes=size;
								}
								
								unsigned char* finalstring=(char*)malloc((numbytes+1)*sizeof(char));
								unsigned int byteindex =0,bytesremaining=numbytes,offsetremaining=offset;
								unsigned int searchsize=0,currentbyte=0;
								unsigned short done=0;
								unsigned int nextcluster=FileAccessData.byteOffset;
								while(nextcluster!=0x0FFFFFF8 && nextcluster!=0x0FFFFFFF && done==0)
								{
									//check if offset is in this cluster
									searchsize=((unsigned int)(getBytsperSector())*(unsigned int)(getBytsperSector()));
									if(offsetremaining<searchsize)
									{
										byteindex=findFirstSector(nextcluster);

										if (bytesremaining<(searchsize-offsetremaining))
										{
										searchsize=bytesremaining;
										bytesremaining=0;
										}
										else
										{
										searchsize-=offsetremaining;
										bytesremaining-=searchsize;
										}
										//allocate temp string
										unsigned char* tempstring=(char*)malloc((searchsize+1)*sizeof(char));
										lseek(fd,byteindex+offsetremaining,SEEK_SET);
										//set remaining offset to 0
										offsetremaining=0;
										read(fd,tempstring,searchsize);
										//move current chars to final string
										for(int j=0;j<searchsize;j++)
										{
										finalstring[currentbyte]=tempstring[j];
										currentbyte++;
										}
										if(bytesremaining==0)
										{
										finalstring[numbytes]='\0';
										printf("\"");
									
				
										printf("\n%x\n",finalstring);
										//fwrite(finalstring,sizeof(char),numbytes+1,stdout);
										printf("\"\n");
										done=1;
										}
									}
									else
									{
										offsetremaining-=searchsize; //offset too large for current cluster
									}
									nextcluster=findNextcluster(nextcluster);
								}

							}
							else
							{
								printf("\nInvalid offset, larget than file\n"); //bad offset
							}
						}
						else
						{
							printf("\nFile is opened in write only mode\n");
						}
					}
					else
					{
						printf("\nFile %s is not open\n",namebuff4);
					}
				}//finish if file type correct
				else
				{
					printf("\nNot a valid readable file\n");
				}

				MoreDataLeft=0;
			}//finish IF FOUND
			else
			{
				printf("FILE not found in directory\n");
			}


			DataSectorOffset += 32;
		}//finish while loop
	}
	else//no file name was provided in tokens
	{
		printf("Invalid argument count for read\n");
	}

}//end read



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
	unsigned int index = 0, byteindex = 0, byteoffset = 0, newindex = 0, tempnext = 0;
	unsigned int nextcluster = FileAccessData.byteOffset;

	int MoreDataLeft = 1, DataSectorOffset = 0;
	char buff4[12];
	unsigned int size = 0;

	unsigned int Firstoffset;
	int clusterindex = 0;
	int firsttime = 0;
	int Clustercount;
	unsigned char buff6[5];
	unsigned char buff7[5];

	while (MoreDataLeft != 0)
	{
		if (DataSectorOffset == 32000)
		{
			break;
		}

		lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

		read(fd, buff4, 10);
		buff4[12] = '\0';

		string2FAT(tokens[1]);

		if (strcmp(buff4, tokens[1]) == 0)
		{
			printf("found\n");
			found = 1;
			MoreDataLeft = 0;
		}
		else
		{
			found = 0;
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
			printf("%s\n", filename);

			while (done == 0)
			{
				byteindex = nextcluster;//findFirstSector(nextcluster);
				printf("\nbyteindex %d\n", byteindex);
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

						printf("\n newindex %d\n", newindex); //this is the index of the new empty cluster line. It works good until this point

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
	unsigned char *  filename = string2FAT(tokens[1]);
	int done = 0;
	int found = FindOrPrintDirectory(tokens, fd, 0);
	unsigned int index = 0, byteindex = 0, byteoffset = 0, newindex = 0, tempnext = 0;
	unsigned int nextcluster = FileAccessData.byteOffset;

	int MoreDataLeft = 1, DataSectorOffset = 0;
	char buff4[12];

	unsigned int Firstoffset;
	int clusterindex = 0;
	int firsttime = 0;
	int Clustercount;
	unsigned char buff6[5];
	unsigned char buff7[5];


	while (MoreDataLeft != 0)
	{
		if (DataSectorOffset == 32000)
		{
			break;
		}

		lseek(fd, FileAccessData.byteOffset + DataSectorOffset, SEEK_SET);

		read(fd, buff4, 10);
		buff4[12] = '\0';

		string2FAT(tokens[1]);

		if (strcmp(buff4, tokens[1]) == 0)
		{
			printf("found\n");
			found = 1;
			MoreDataLeft = 0;
		}
		else
		{
			found = 0;
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
			//	printf("%s\n",filename);

			while (done == 0)
			{
				byteindex = nextcluster;
				printf("\nbyteindex %d\n", byteindex);
				byteoffset = 64;
				unsigned char * firstbyte = (char*)malloc(sizeof(char));
				unsigned int firstfree = 0;

				while (done == 0 && byteoffset<((unsigned int)(getBytsperSector()) * (unsigned int)(getSecperClus())))
				{
					lseek(fd, byteindex + byteoffset, SEEK_SET);
					read(fd, firstbyte, 1);

					if (firstbyte[0] == 0x00 || firstbyte[0] == 0xE5)
					{
						newindex = byteindex + byteoffset;

						printf("\n index for dirname %d\n", newindex);

						//write file name
						lseek(fd, newindex, SEEK_SET);
						write(fd, filename, 11);

						//write dir attr
						firstbyte[0] = 0x10;
						lseek(fd, newindex + 11, SEEK_SET);
						write(fd, firstbyte, 1);

						//find new cluster
						firstfree = findFirstFreeCluster();


						//write cluster value
						writeClus2Dir(newindex, firstfree);

						//write file size
						writesize2dir(newindex, 0x0000);

						//create parent and child entries
						byteindex = findFirstSector(firstfree);
						byteoffset = 32;
						char * parent = "..      ";
						char* child = ".       ";

						printf("\nindex for child %d\n", byteindex);
						lseek(fd, byteindex, SEEK_SET);
						write(fd, child, 11);

						//write dir attr
						lseek(fd, byteindex + 11, SEEK_SET);
						write(fd, firstbyte, 1);

						//write child cluster
						writeClus2Dir(byteindex, firstfree);
						writesize2dir(byteindex, 0x0000);

						//parent
						printf("\nindex for parent %d\n", byteindex + byteoffset);
						lseek(fd, byteindex + byteoffset, SEEK_SET);
						write(fd, parent, 11);

						//write dir attr
						lseek(fd, byteindex + byteoffset + 11, SEEK_SET);
						write(fd, firstbyte, 1);

						//write parent cluster
						if (FileAccessData.byteOffset == getRootCluster())
						{
							writeClus2Dir(byteindex + byteoffset, 0x0000);
						}
						else
						{
							writeClus2Dir(byteindex + byteoffset, FileAccessData.byteOffset);
						}
						writesize2dir(byteindex + byteoffset, 0x0000);
						//write entry into fat table
						write2FAT(firstfree, 0x0FFFFFF8);

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
						firstfree = FSInfo.FSI_Nxt_Free;
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


unsigned char uc_sizef(char **tokens)
	{
	int fd = open("fat32.img",O_RDONLY);

	int MoreDataLeft = 1;
	int DataSectorOffset = 0;


	char buff2[3];   // DirAtt
	char buff4[12];
	unsigned char size[4];


	unsigned int Firstoffset;
	int clusterindex=0;
	int firsttime = 0;
	int Clustercount;
	unsigned char buff6[5];
	unsigned char buff7[5];



	while (MoreDataLeft!=0)
	{
		if (DataSectorOffset == 512)
		{
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
					 FileAccessData.offset=Firstoffset;

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
			printf("found\n");
			
			if (buff2[1]!=0x10)
			{
				printf("its a file");
				lseek(fd,FileAccessData.byteOffset + DataSectorOffset+28,SEEK_SET);
				read(fd,size,4);
				unsigned int x = L2Bint(size,4);
				printf("\nSIZE IS %d\n",x);
			}

			MoreDataLeft=0;
		}
		else
		{
		//	printf("not found\n");
		}

	
		DataSectorOffset += 32;
	}
}



char * string2FAT(char * input)
{
	char temp[10]=" ";
	int i;
	int length = strlen(input);
	for (i=0;i<length && i < 10; i++)
	{
		temp[i]=toupper(input[i]);
	}
	for (i=length; i<10; i++)
	{
		temp[i]=' ';
	}
	temp[10]='\0';
	strcpy(input,temp);
	return input;
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
unsigned int findFirstSector(unsigned int cluster)
{
	unsigned int firstDataSector, firstSectorofClusters;

	firstDataSector = (unsigned int)(us_RsvdSecCnt()) + ((unsigned int)(getNumFATs()) * getFATSz32());
	firstSectorofClusters=firstDataSector + ((cluster-2)*(unsigned int)(getSecperClus()));

	return ( firstSectorofClusters *(unsigned int)(getBytsperSector())  );
}

unsigned int findFirstFreeCluster()
{
	int fd = open("fat32.img",O_RDONLY);

	unsigned int tempcluster=0,clustervalue=0;
	unsigned char temp[4];

	unsigned int FATsectorNum=(unsigned int)(us_RsvdSecCnt()) + ((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

        unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());

	unsigned int currentbyte=fatstart;

	lseek(fd,fatstart,SEEK_SET);
	read(fd,temp,4);

	clustervalue=L2Bint(temp,4);

	while(clustervalue!=0X00000000 && currentbyte < (fatstart + getFATSz32() * (unsigned int)(getBytsperSector())))
	{
		tempcluster++;
		currentbyte+=4;
		lseek(fd,currentbyte,SEEK_SET);
		read(fd,temp,4);
		clustervalue=L2Bint(temp,4);
	}
	if (clustervalue!=0x00000000)
	{
		printf("\nThere are no free cluster\n");
	}
	return tempcluster;
}

unsigned int findNextcluster(unsigned int cluster)
{
	int fd = open("fat32.img",O_RDONLY);
	unsigned char temp[4];
	unsigned int FATsectorNum=(unsigned int)(us_RsvdSecCnt()) + ((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

	unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());
	unsigned int byteindex = fatstart + (cluster * 4);

	lseek(fd,byteindex,SEEK_SET);
	read(fd,temp,4);
	//printf("\n%u\n",temp);
	return L2Bint(temp,4);
}



//big endian int to little endian char
unsigned char * BigToLil (unsigned int value, unsigned int size)
{
	unsigned char * temp = (char*)malloc(4 * sizeof(char));
	unsigned int temp_val = value, m=0;
	int i;
	for (i=0;i<size;i++)
	{
		m = 0x000000FF << (i*8);
		temp[i]=(temp_val & m)>>(i*8);
	}
	return temp;
}


void write2FAT(unsigned int cluster, unsigned int value)
{
	int fd = open("fat32.img",O_RDWR);
	unsigned int clusterValue =0;
	unsigned char temp[4];

	strcpy(temp,BigToLil(value,4));

	 unsigned FATsectorNum=(unsigned int)(us_RsvdSecCnt()) + ((getRootCluster() * 4) / (unsigned int)(getBytsperSector()));

        unsigned int fatstart = FATsectorNum * (unsigned int)(getBytsperSector());
        unsigned int byteindex = fatstart + (cluster * 4);

	lseek(fd,byteindex,SEEK_SET);
	write(fd,temp,4);
}
//little endian char to big endian int
unsigned int L2Bint(unsigned char * value, unsigned int size)
{
	unsigned int total =0;
	int i;
	for (i=0;i<size;i++)
	{
		total += (unsigned int)(value[i])<<(i*8);
	}
	return total;
}
//write cluster number to dir entry
void writesize2dir(unsigned int address, unsigned int value)
{
	//printf("\ndoing size now\n");
	int fd = open("fat32.img",O_RDWR);
	unsigned char * temp = (char*)malloc(4* sizeof(char));
	strcpy(temp,BigToLil(value,4));
	lseek(fd,address+28,SEEK_SET);
	write(fd,temp,4);
}

//write size to dir entry
void writeClus2Dir(unsigned int address, unsigned int value)
{
	int fd = open("fat32.img",O_RDWR);
	unsigned char * temp = (char*)malloc(4 * sizeof(char));
	unsigned char * low = (char*)malloc(2 * sizeof(char));
	unsigned char * high = (char*)malloc(2 * sizeof(char));

	strcpy(temp,BigToLil(value,4));
  
	int i;
	for(i=0;i<2;i++)
	{
		low[i]=temp[i];
		//printf("\nlow %u\n",low[i]);
		high[i]=temp[i+2];
		//printf("\nhigh %u\n",high[i]);
	}

	lseek(fd,address+20,SEEK_SET);
	write(fd,high,2);
	lseek(fd,address+26,SEEK_SET);
	write(fd,low,2);
}
