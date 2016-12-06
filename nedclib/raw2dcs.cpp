/* Credits to Tim Schurewegen for his original raw2bmp tool, which I reverse engineered this tool
from.  The main difference between his original tool and mine, is file dependency.  this one does
not need the file dcslong.bmp, since the file is 100% generated by the tool, right down to the
address bars.  Like his tool though, this one cannot generate short dcs files, since the short
dcs address bar info is missing. The other thing reversed from his tool, is 8/10 data modulation.
*/

/* I decided to Rewrite this program, so that it can also be used to convert dcs files, back into raw
files. While there is not much use for the raw files at this point,  if a GBA emulator in the future
is released with some e-reader support, then the raw files will be useful then.
*/

#include <stdio.h>
#include "dcs.h"


#define TYPE_LONG 0
#define TYPE_SHORT 1

int OptV=0;

#define VERBOSE(a) if(OptV) \
	{ \
	printf(a); \
	}

void usage (void)
{
	printf("Usage :\n  DcsTool [options]\n");
	printf("Options :\n");
	printf("  -i <file>\t\tInput File\t\t\t(Required)\n");
	printf("  -o <file>\t\tOutput File\t\t\t(Required)\n");
	printf("  -v\t\t\tVebose\t\t\t(Optional)\n");
	printf("  -d\t\t\tConvert DCS to Raw\t\t\t(Required *)\n");
	printf("  -r\t\t\tConvert Raw to DCS\t\t\t(Required *)\n");
	printf("\n");
	printf("  * Options \"-d\" and \"-r\" cannont be used at the same time\n");
}

int count_raw(FILE *f)
{
	unsigned short i=0,j=0;
	int count=0;
	int result;
	result=fread(&i,1,2,f);
	while(((i==0x0200) || (i==0x0300)) && (result == 2))
	{
		fseek(f,0x66, SEEK_CUR);
		result=fread(&j,1,2,f);
		if(j==0x0100)
		{
			fseek(f,0x6E6,SEEK_CUR);
			result=fread(&i,1,2,f);
			count++;
		}
		else if(j==0x1900)
		{
			fseek(f,0xAF6, SEEK_CUR);
			result=fread(&i,1,2,f);
			count++;
		}
		
	}
	fseek(f,0,SEEK_SET);
	return count;
}

void read_next_raw(FILE *f)
{
	int result;
	/*
	if (filelen == 0xB60)
		{
			fread(raw,1,0xB60,f);
			dotcodelen = 28;
			bmplen = 0x7C;
		}
		else if (filelen == 0x750)
		{
			fread(raw,1,0x750,f);
			dotcodelen = 18;
			bmplen = 0x50;
		}
		*/

	result = fread(raw,1,2,f);
	if(result)
	{
		if(raw[0][1]==3)
		{
			fread(&raw[0][2],1,0xB60-2,f);
			dotcodelen = 28;
			bmplen = 0x7C;
		}
		else if(raw[0][1] == 2)
		{
			fread(&raw[0][2],1,0x750-2,f);
			dotcodelen = 18;
			bmplen = 0x50;
		}
		else
		{
			fclose(f);
		}
	}
	else
	{
		fclose(f);
	}
	
	
}

void write_bmp(FILE *f)
{
	int i,j;
	VERBOSE("Writing BMP file\n")
		/*if(dotcodelen == 28)
		{*/
			if(dotcodelen == 28)
				i=989*dpi_multiplier;
			else
				i=639*dpi_multiplier;
			bmpheader[0x12] = i & 0xFF;
			bmpheader[0x13] = i >> 8;
			i/=32;
			i++;
			i*=4;
			i*=(44*dpi_multiplier);
			bmpheader[2] = (i + 0x3E) & 0xFF;
			bmpheader[3] = ((i + 0x3E) >> 8) & 0xFF;
			bmpheader[4] = ((i + 0x3E) >> 16) & 0xFF;
			bmpheader[0x22] = i & 0xFF;
			bmpheader[0x23] = (i >> 8) & 0xFF;
			bmpheader[0x24] = (i >> 16) & 0xFF;
			bmpheader[0x16] = (44 * dpi_multiplier);

			fwrite(bmpheader,1,62,f);
			for(j=0;j<(44*dpi_multiplier);j++)
				for(i=0;i<(bmplen*dpi_multiplier);i++)
					fputc(bmpdata[j][i],f);

			//fwrite(bmpdata1,bmplen,44,f);
		/*}
		else
		{
			fwrite(bmpheader2,1,62,f);
			fwrite(bmpdata2,bmplen,44,f);
		}*/
}


int main(int argc, char **argv)
{
	//int i, j;
	long filelen;
	FILE *f,*g;

	char filename[256];

	int i,j;
	int OptI=0;
	int InFile;
	int OptO=0;
	int OutFile;
	int OptD=0;
	int OptR=0;

	printf("DcsTool Nintendo dotcode strip tool.\n");
	printf("Copyrighted by CaitSith2\n\n");

	for (i=1;i<argc;i++)
	{
		if(argv[i][0] == '-')
		{
			switch (argv[i][1]) {
			case 'i':
				OptI++;
				InFile = i+1;
				i++;
				break;
			case 'o':
				OptO++;
				OutFile = i+1;
				i++;
				break;
			case 'd':
				OptD=1;
				break;
			case 'r':
				OptR=1;
				break;
			case 'v':
				OptV=1;
			}
		}
	}
	if ((OptI != 1) || (OptO != 1) || (OptD + OptR != 1))
	{
		usage();
		return 1;
	}


	f=fopen(argv[InFile],"rb");
	if (f==NULL)
	{
		printf("Unable to open input file %s\n",argv[InFile]);
		return 1;
	}
	fseek(f,0,SEEK_END);
	filelen=ftell(f);
	fseek(f,0,SEEK_SET);

	VERBOSE("Reading Input File\n")
	if (OptD)
	{
		
		if (filelen == 0x158E)
		{
			fseek(f,62,SEEK_SET);
			fread(bmpdata1,0x7C,44,f);
			dotcodelen = 28;
			bmplen = 0x7C;
		}
		else
		{
			fseek(f,62,SEEK_SET);
			fread(bmpdata2,0x50,44,f);
			dotcodelen = 18;
			bmplen = 0x50;
		}
		fclose(f);
		clear_dcs();
		reversebmp();
		VERBOSE("Storing BMP to DCS array\n")
		reverse_dcs();
		VERBOSE("Extracting 8/10 Modulated Data\n")
		eight_ten_demodulate();
		VERBOSE("Demodulating 8/10 Modulated Data\n")

		f=fopen(argv[OutFile],"wb");
		if (f==NULL)
		{
			printf("unable to open output file %s\n",argv[OutFile]);
			return 1;
		}
		VERBOSE("Writing Demodulated RAW data file\n")
		if(dotcodelen == 28)
		{
			fwrite(raw,1,0xB60,f);
		}
		else
		{
			fwrite(raw,1,0x750,f);
		}
		fclose(f);

	}
	if (OptR)
	{
		int num_raw=count_raw(f);
		for(i=0;i<num_raw;i++)
		{
			read_next_raw(f);
			clear_dcs();
			init_dcs();
			VERBOSE("Initializing DCS Strip\n")
			eight_ten_modulate();
			VERBOSE("Modulating Raw Data\n")
			make_dcs();
			//VERBOSE("Rotating BMP 180 Degrees\n")
			//flipbmp();
			VERBOSE("Inserting Modulated Data into BMP\n")
			makebmp();
			if(num_raw==1)
				sprintf(filename,"%s.bmp",argv[OutFile]);
			else
				sprintf(filename,"%s-%.2d.bmp",argv[OutFile],i+1);
			g=fopen(filename,"wb");
			if(g!=NULL)
			{
				write_bmp(g);
				fclose(g);
			}
		}
	}
	
	// - Debugging text,  piped to a text file, in the form of "raw2dcs <infile> <outfile> > <textfile>
	/*for (i=0;i<(989*dpi_multiplier);i++)
	{
		for (j=0;j<(44*dpi_multiplier);j++)
		{
			if(j>1000)
				break;
			if (dcsbmp[i][j]) 
			{
				printf("X");
			}
			else
				printf(" ");
		}
		printf("\n");
	}
*/
	VERBOSE("Done\n")

	return 0;

}

		



void clear_dcs(void)
{
	int i, j;
	for (i=0;i<3956;i++)
		for (j=0;j<176;j++)
			dcsbmp[i][j] = 0;
	for (i=0;i<28;i++)
		for(j=0;j<130;j++)
			_810mod[i][j]=0;
}
