#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SEEK_SET	       0 /* �ļ���ͷ�ı�� */
#define SEEK_CUR	1 /* ��ǰλ�õı�� */
#define SEEK_END	2 /* �ļ�ĩβ�ı�� */

#define BUFSIZE                 (24*1024)
#define IMG_SIZE                (24*1024)
#define SPL_HEADER_SIZE         16
#define SPL_HEADER              "S5PC110 HEADER  "

int main (int argc, char *argv[])
{
	FILE		*fp;    /* ����һ���ļ�ָ�� */
	char		*Buf, *a;
	int		BufLen;
	int		nbytes, fileLen;
	unsigned int	checksum, count;
	int		i;

	if (argc != 3) /* ��������������󣬴�ӡ������Ϣ */
	{
		/* Ӧ�ù���ʱ,��ʽ������ ./mktiny210spl.exe old.bin new.bin */
		printf("Usage: mkbl1 <source file> <destination file>\n");
		return -1;
	}

	BufLen = BUFSIZE;
	Buf = (char *)malloc(BufLen); /* ��̬����һ��24k���ڴ�ռ� */
	if (!Buf) /* ����ʧ�ܣ�������0 */
	{
		printf("Alloc buffer failed!\n");
		return -1;
	}

	memset(Buf, 0x00, BufLen); /* ���������Ŀռ����� */

	fp = fopen(argv[1], "rb"); /* �Զ������Ƶķ�ʽ��û��ͷ���źŵ�old.bin�ļ� */
	if( fp == NULL)
	{
		printf("source file open error\n");
		free(Buf); /* �����ʧ�ܣ��ͷŵ�ԭ��������ڴ棬���������ڴ�й© */
		return -1;
	}

	fseek(fp, 0L, SEEK_END); /* ���ļ�λ��ָ��ָ���ļ�ĩβ���������е�ͳ�ƴ�С�Ĳ��� */
	fileLen = ftell(fp); /* ���ڵõ��ļ�λ��ָ�뵱ǰλ��������ļ��׵�ƫ���ֽ���,���ļ���С*/
	fseek(fp, 0L, SEEK_SET); /* ���ļ�λ��ָ��ָ���ļ���ʼ */

	/* ���old.bin�ļ��Ĵ�СС�ڹ涨������С����count���ڸ��ļ��Ĵ�С�������������С */
	count = (fileLen < (IMG_SIZE - SPL_HEADER_SIZE))
		? fileLen : (IMG_SIZE - SPL_HEADER_SIZE);

	memcpy(&Buf[0], SPL_HEADER, SPL_HEADER_SIZE); /* ����16�ֽڵ����ݵ�Buf�У�����ʼ��ͷ����Ϣ��λ�� */

	nbytes = fread(Buf + SPL_HEADER_SIZE, 1, count, fp); /* ���������ɵ�old.bin�ļ�������buf�У�������ͷ����Ϣ��ʼ���� */

	if ( nbytes != count ) /* ����ֵ���ڿ�����Ԫ�صĸ��� */
	{
		printf("source file read error\n"); /* ���������ʵ�ʵĲ���ȣ���ʧ�� */
		free(Buf); /* �ͷ��ڴ� */
		fclose(fp); /* �ر��ļ� */
		return -1;
	}

	fclose(fp); /* �ر��ļ� */

	/* �������У����ڶ�̬����checksum����ʽ������ĳ���һ */
	a = Buf + SPL_HEADER_SIZE;
	for(i = 0, checksum = 0; i < IMG_SIZE - SPL_HEADER_SIZE; i++)
		checksum += (0x000000FF) & *a++;

	/* ��checksumд��buf�ĵ������ֽڴ���������checksum��λ�ô� */
	a = Buf + 8;
	*( (unsigned int *)a ) = checksum;

	fp = fopen(argv[2], "wb"); /* �Զ�����д�ķ�ʽ����һ���µĶ������ļ� */
	if (fp == NULL)
	{
		printf("destination file open error\n");
		free(Buf); /* �ͷ��ڴ� */
		return -1;
	}

	a = Buf; /* ָ���ڴ���׵�ַ */
	nbytes	= fwrite( a, 1, BufLen, fp); /* ��buf�е�����д���´�����bin�ļ� */

	if ( nbytes != BufLen ) /* ����ֵ����д���Ԫ�صĸ��� */
	{
		printf("destination file write error\n");
		free(Buf); /* �ͷ��ڴ� */
		fclose(fp);/* �ر��ļ� */
		return -1;
	}

	free(Buf); /* �ͷ��ڴ� */
	fclose(fp);/* �ر��ļ� */

	return 0;
}
