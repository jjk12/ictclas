//////////////////////////////////////////////////////////////////////
//ICTCLAS��飺����������ʷ�����ϵͳICTCLAS(Institute of Computing Technology, Chinese Lexical Analysis System)��
//             �����У����ķִʣ����Ա�ע��δ��¼��ʶ��
//             �ִ���ȷ�ʸߴ�97.58%(973ר��������)��
//             δ��¼��ʶ���ٻ��ʾ�����90%�������й�������ʶ���ٻ��ʽӽ�98%;
//             �����ٶ�Ϊ31.5Kbytes/s��
//����Ȩ��  Copyright?2002-2005�п�Ժ������ ְ������Ȩ�ˣ��Ż�ƽ ��Ⱥ
//��ѭЭ�飺��Ȼ���Դ�������Դ���֤1.0
//Email: zhanghp@software.ict.ac.cn
//Homepage:www.nlp.org.cn;mtgroup.ict.ac.cn
// Result.cpp: implementation of the CResult class.
//
//////////////////////////////////////////////////////////////////////
#include "Result.h"
#include <iostream>

#define DICT_FILE        "data/coreDict.dct"
#define LEXICAL_FILE     "data/lexical.ctx"
#define BIGRAM_FILE      "data/BigramDict.dct"
#define PERSON_FILE      "data/nr"
#define PLACE_FILE       "data/ns"
#define TRANSPERSON_FILE "data/tr"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static char * concatenate(const char * str1, const char * str2)
{
	static char str0[500];
	//char * p=str0;
	////while (*p++ = *str1++);
	//while ((*p++) = (*str1++));
	//p--;
	////while (*p++ = *str2++);
	//while ((*p++) = (*str2++));
	strcpy(str0,str1);
	strcat(str0,str2);
	return str0;
}

CResult::CResult(const char * strDataPath)
{
	//malloc buffer
	m_pResult=new PWORD_RESULT[MAX_SEGMENT_NUM];
	for(int i=0;i<MAX_SEGMENT_NUM;i++)
	{
		m_pResult[i]=new WORD_RESULT[MAX_WORDS];
	}

	m_dictCore.Load(concatenate(strDataPath,DICT_FILE));
	m_POSTagger.LoadContext(concatenate(strDataPath,LEXICAL_FILE));
/*
	m_dictCore.Load("data\\Dict.dct");
	m_POSTagger.LoadContext("data\\trainTest.ctx");
*/
/*
	
	m_dictCore.AddItem("ʮ��",'d'*256,+500);
	m_dictCore.AddItem("ʮ��",'m'*256,-500);
	m_dictCore.AddItem("�ҹ�",'n'*256,-2000);
	m_dictCore.AddItem("�ҹ�",'r'*256,+2000);
	m_dictCore.AddItem("ǧ��",'t'*256,200);

    m_dictCore.Optimum();
	m_dictCore.Save("data\\coreDictOptimum.dct");
*/


	m_POSTagger.SetTagType();

	m_uPerson.Configure(concatenate(strDataPath,PERSON_FILE),TT_PERSON);
	//Set the person recognition configure
	m_uPlace.Configure(concatenate(strDataPath,PLACE_FILE),TT_PLACE);
	//Set the place recognition configure
	m_uTransPerson.Configure(concatenate(strDataPath,TRANSPERSON_FILE),TT_TRANS_PERSON);
	//Set the transliteration person recognition configure
	
	//0: ֻ�зֲ���ע;
	//1: ��λ��ע;
	//2: Ϊ��λ��ע������ϸ�ı�ע���ͣ�
	m_nOperateType=2;//0:Only Segment;1: First Tag; 2:Second Type
	//���������ı�ʾ�������Լ����Ա�ע����ת���������������׼��973��׼��XML��ʾ��ʽ��
	m_nOutputFormat=0;//0:PKU criterion;1:973 criterion; 2: XML criterion

	m_dSmoothingPara=0.1;//Smoothing parameter
	m_dictBigram.Load(concatenate(strDataPath,BIGRAM_FILE));

	//fixed @ 2015/09/12
	m_nResultCount = 1;
	for(int i = 0; i < MAX_SEGMENT_NUM; ++i){
		m_dResultPossibility[i] = .0;
	}
}

CResult::~CResult()
{
	//free buffer
	for(int i=0;i<MAX_SEGMENT_NUM;i++)
	{
		delete [] m_pResult[i];
	}
	delete m_pResult;

}

bool CResult::Output(PWORD_RESULT pItem, char *sResult,bool bFirstWordIgnore)
{
	int i=0;
	char sTempBuffer[WORD_MAXLENGTH],sPOS[3];
	sPOS[2]=0;
	sResult[0]=0;
	if(bFirstWordIgnore)//Ignore first valid
		i=1;
	//���ζ��д�������������ÿ��������������Ա��ת��Ϊ�ַ�����ʾ��ʽ
	while(pItem[i].sWord[0]!=0&&pItem[i].nHandle!=CT_SENTENCE_END)//Not sentence ending flag
	{
		//Get the POS string
		if(m_nOutputFormat!=0)//Not PKU format
			PKU2973POS(pItem[i].nHandle,sPOS);
		else//PKU format
		{
			sPOS[0]=pItem[i].nHandle/256;
			sPOS[1]=pItem[i].nHandle%256;
		}
		sPOS[m_nOperateType]=0;//Set the sPOS with operate type
		
		if(m_nOutputFormat==0)//PKU format
		{
			sprintf(sTempBuffer,"%s",pItem[i].sWord);
		    strcat(sResult,sTempBuffer);
			if(sPOS[0]!=0)//need POS 
			{
				sprintf(sTempBuffer,"/%s",sPOS);
				strcat(sResult,sTempBuffer);
			}
		    strcat(sResult,"  ");
		}
		else if(m_nOutputFormat==1)//973 format
		{
			sprintf(sTempBuffer,"%s\\",pItem[i].sWord);
		    strcat(sResult,sTempBuffer);
			if(sPOS[0]!=0)//need POS 
			{
				sprintf(sTempBuffer,"[%s]",sPOS);
				strcat(sResult,sTempBuffer);
			}
		}
		else if(m_nOutputFormat==2)//XML format
		{
			if(sPOS[0]!=0)//POS
			{
				sprintf(sTempBuffer,"<any type=\042%s\042>",sPOS);
				strcat(sResult,sTempBuffer);
			}
			sprintf(sTempBuffer,"<src>%s</src>",pItem[i].sWord);
			strcat(sResult,sTempBuffer);
			if(sPOS[0]!=0)
			{
				strcat(sResult,"</any>");
			}
		}
		i++;
	}
	return true;
}

bool CResult::OutputA(PWORD_RESULT pItem, int &curr_pos, int &result_num, 
		result_t *result_arr, bool bFirstWordIgnore)
{
	int i=0;
	//char sTempBuffer[WORD_MAXLENGTH],sPOS[3];
	//char sPOS[3];
	//sPOS[2]=0;
	//sResult[0]=0;
	if(bFirstWordIgnore)//Ignore first valid
		i=1;
	//���ζ��д�������������ÿ��������������Ա��ת��Ϊ�ַ�����ʾ��ʽ
	while(pItem[i].sWord[0]!=0&&pItem[i].nHandle!=CT_SENTENCE_END)//Not sentence ending flag
	{
		////Get the POS string
		//if(m_nOutputFormat!=0)//Not PKU format
		//	PKU2973POS(pItem[i].nHandle,sPOS);
		//else//PKU format
		//{
		//	sPOS[0]=pItem[i].nHandle/256;
		//	sPOS[1]=pItem[i].nHandle%256;
		//}
		//sPOS[m_nOperateType]=0;//Set the sPOS with operate type
		
		//if(m_nOutputFormat==0)//PKU format
		//{
			//sprintf(sTempBuffer,"%s",pItem[i].sWord);
		    //strcat(sResult,sTempBuffer);
			//if(sPOS[0]!=0)//need POS 
			//{
			//	sprintf(sTempBuffer,"/%s",sPOS);
			//	strcat(sResult,sTempBuffer);
			//}
		    //strcat(sResult,"  ");
			result_arr[result_num].start               = curr_pos;
			curr_pos += (result_arr[result_num].length = strlen(pItem[i].sWord));
			result_arr[result_num].part_of_speech      = pItem[i].nHandle;
			result_arr[result_num++].value             = pItem[i].dValue;
		//}
		//else if(m_nOutputFormat==1)//973 format
		//{
		//	sprintf(sTempBuffer,"%s\\",pItem[i].sWord);
		//    strcat(sResult,sTempBuffer);
		//	if(sPOS[0]!=0)//need POS 
		//	{
		//		sprintf(sTempBuffer,"[%s]",sPOS);
		//		strcat(sResult,sTempBuffer);
		//	}
		//}
		//else if(m_nOutputFormat==2)//XML format
		//{
		//	if(sPOS[0]!=0)//POS
		//	{
		//		sprintf(sTempBuffer,"<any type=\042%s\042>",sPOS);
		//		strcat(sResult,sTempBuffer);
		//	}
		//	sprintf(sTempBuffer,"<src>%s</src>",pItem[i].sWord);
		//	strcat(sResult,sTempBuffer);
		//	if(sPOS[0]!=0)
		//	{
		//		strcat(sResult,"</any>");
		//	}
		//}
		i++;
	}
	return true;
}

//bool CResult::Processing(char *sSentence,unsigned int nCount)
//{
//	int nIndex;
//#if _ICT_DEBUG
//	char *sSegment;
//	sSegment=new char[MAX_SENTENCE_LEN*2];
//#endif
//	//Unigram segment
//	//m_Seg.Segment(sSentence,m_dictCore,nCount);
//	//Bigram segment
//	m_Seg.BiSegment(sSentence, m_dSmoothingPara,m_dictCore,m_dictBigram,nCount);
//	
//	m_nResultCount=m_Seg.m_nSegmentCount;
//	//Record the number of result
//	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
//	{
//#if _ICT_DEBUG
//			m_POSTagger.POSTagging(m_Seg.m_pWordSeg[nIndex],m_dictCore,m_dictCore);
//			Output(m_Seg.m_pWordSeg[nIndex],sSegment);
//			printf("POS Tag%d:%s\n",nIndex+1,sSegment);
//#endif
//		m_uPerson.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
//		m_uTransPerson.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
//		m_uPlace.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
//	}		
//	//m_uPerson.Recognition(m_Seg.m_WordSeg[0],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
//	//Person Recognition
//#if _ICT_DEBUG
//	printf("After person recognition.\n");
//#endif
//	//Unigram
//	//m_Seg.OptimumSegmet(nCount);
//	//Bigram
//	m_Seg.BiOptimumSegment(nCount,m_dSmoothingPara,m_dictBigram,m_dictCore);
//
//	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
//	{
//		m_POSTagger.POSTagging(m_Seg.m_pWordSeg[nIndex],m_dictCore,m_dictCore);
//#if _ICT_DEBUG
//		Output(m_Seg.m_pWordSeg[nIndex],sSegment);
//		printf("POS Tag%d:%s\n",nIndex+1,sSegment);
//#endif
//	}
//#if _ICT_DEBUG
//	printf("After Sorting.\n");
//#endif
//	Sort();//Sort the ending 
//#if _ICT_DEBUG
//	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
//	{
//		Output(m_pResult[nIndex],sSegment);
//		printf("POS Tag%d(P=Exp(%f)):%s\n",nIndex+1,m_dResultPossibility[nIndex],sSegment);
//	}
//	delete [] sSegment;
//#endif
//	return true;
//}

bool CResult::Processing(char *sSentence,unsigned int nCount)
{
	int nIndex;
#if _ICT_DEBUG
	char *sSegment;
	sSegment=new char[MAX_SENTENCE_LEN*2];
#endif
	//Unigram segment
	//m_Seg.Segment(sSentence,m_dictCore,nCount);
	//Bigram segment
	//���ж���ִ�
	//������ľ���ʹ���з���m_Seg���ж�Ԫ�з�
	//���ɵĽ����洢��m_Seg.m_pWordSeg��
	//�����������      m_Seg.m_nSegmentCount��
	m_Seg.BiSegment(sSentence, m_dSmoothingPara,m_dictCore,m_dictBigram,nCount);
	
	m_nResultCount=m_Seg.m_nSegmentCount;
	//Record the number of result
	//Ȼ�����ÿ�����ɵĽ����һ��Ϊ1�����10�������ֱ�ʹ������δ��½��ʶ��������ʶ��
	//�������m_Seg.m_graphOptimum��
	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
	{
#if _ICT_DEBUG
			m_POSTagger.POSTagging(m_Seg.m_pWordSeg[nIndex],m_dictCore,m_dictCore);
			Output(m_Seg.m_pWordSeg[nIndex],sSegment);
			printf("POS Tag%d:%s\n",nIndex+1,sSegment);
#endif
		m_uPerson.Recognition(     m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
		m_uTransPerson.Recognition(m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
		m_uPlace.Recognition(      m_Seg.m_pWordSeg[nIndex],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
	}		
	//m_uPerson.Recognition(m_Seg.m_WordSeg[0],m_Seg.m_graphOptimum,m_Seg.m_graphSeg,m_dictCore);
	//Person Recognition
#if _ICT_DEBUG
	printf("After person recognition.\n");
#endif
	//Unigram
	//m_Seg.OptimumSegmet(nCount);
	//Bigram
	//����m_Seg�Ķ�Ԫ�Ż��д�
	m_Seg.BiOptimumSegment(nCount,m_dSmoothingPara,m_dictBigram,m_dictCore);

	//�������ɵ�ÿ�����������m_Seg.m_pWordSeg�У������ô��Ա�ע��m_POSTagger���д��Ա�ע��
	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
	{
		//�ڴ˴����д��Ա�ע
		m_POSTagger.POSTagging(m_Seg.m_pWordSeg[nIndex],m_dictCore,m_dictCore);
#if _ICT_DEBUG
		Output(m_Seg.m_pWordSeg[nIndex],sSegment);
		printf("POS Tag%d:%s\n",nIndex+1,sSegment);
#endif
	}
#if _ICT_DEBUG
	printf("After Sorting.\n");
#endif
	//������ɵĽ�������������յĽ���ᰴ���ʸߵʹ洢��m_pResult��
	Sort();//Sort the ending 
#if _ICT_DEBUG
	for(nIndex=0;nIndex<m_Seg.m_nSegmentCount;nIndex++)
	{
		Output(m_pResult[nIndex],sSegment);
		printf("POS Tag%d(P=Exp(%f)):%s\n",nIndex+1,m_dResultPossibility[nIndex],sSegment);
	}
	delete [] sSegment;
#endif
	return true;
}

//Sort the segmentation and POS result according its possibility
bool CResult::Sort()
{
	ELEMENT_TYPE dPossibility[MAX_SEGMENT_NUM],dTemp;
	int nIndex[MAX_SEGMENT_NUM],nTemp;//Index

	memset(dPossibility,0,sizeof(dPossibility));
	//Init the possibility
	
    int i;
	for(i=0;i<m_Seg.m_nSegmentCount;i++)
	{//Computing the possibility
		dPossibility[i]=ComputePossibility(m_Seg.m_pWordSeg[i]);
		nIndex[i]=i;//Record the index
	}
	
	//Sort with Bubble sort algorithm
	for(i=0;i<m_Seg.m_nSegmentCount;i++)
		for(int j=i+1;j<m_Seg.m_nSegmentCount;j++)
		{
			if(dPossibility[i]<dPossibility[j])
			{//Swap the possition and value
				nTemp=nIndex[i];
				dTemp=dPossibility[i];
				nIndex[i]=nIndex[j];
				dPossibility[i]=dPossibility[j];
				nIndex[j]=nTemp;
				dPossibility[j]=dTemp;
			}
		}
	
	for(i=0;i<m_Seg.m_nSegmentCount;i++)
	{//Adjust the segmentation and POS result and store them in the final result array
	 //Store them according their possibility ascendly
		Adjust(m_Seg.m_pWordSeg[nIndex[i]],m_pResult[i]);	
		m_dResultPossibility[i]=dPossibility[i];	
	}
	return true;
}
//Compute the possibility of current segmentation and POS result
ELEMENT_TYPE CResult::ComputePossibility(PWORD_RESULT pItem)
{
	int i=0;
	ELEMENT_TYPE dResultPossibility=0;
	while(pItem[i].sWord[0]!=0)
	{
		dResultPossibility+=pItem[i].dValue;
		//Compute the possibility of logP(Wi|Ti)
		if(pItem[i+1].sWord[0]!=0)//Not the last one
		{//Compute the possibility of logP(Ti|Ti-1)
			dResultPossibility+=log((double)(m_POSTagger.m_context.GetContextPossibility(0,pItem[i].nHandle,pItem[i+1].nHandle)+1));
			dResultPossibility-=log((double)(m_POSTagger.m_context.GetFrequency(0,pItem[i].nHandle)+1));
		}
		i++;
	}
	return dResultPossibility;
}

//Adjust the result with some rules
bool CResult::Adjust(PWORD_RESULT pItem,PWORD_RESULT pItemRet)
{
	int i=0,j=0;
	unsigned int nLen;
	char sSurName[10],sSurName2[10],sGivenName[10];
	bool bProcessed=false;//Have been processed
	while(pItem[i].sWord[0]!=0)
	{
		//fixed @ 2015/09/12
		pItemRet[j].dValue = pItem[i].dValue;

		nLen=strlen(pItem[i].sWord);
		bProcessed=false;
		
		//Rule1: adjust person name
		if(pItem[i].nHandle==28274&&ChineseNameSplit(pItem[i].sWord,sSurName,sSurName2,sGivenName,m_uPerson.m_dict)&&strcmp(pItem[i].sWord,"Ҷ����")!=0)//'nr'
		{//Divide name into surname and given name
			
			if(sSurName[0])
			{
				strcpy(pItemRet[j].sWord,sSurName);
				pItemRet[j++].nHandle=28274;
			}
			if(sSurName2[0])
			{
				strcpy(pItemRet[j].sWord,sSurName2);
				pItemRet[j++].nHandle=28274;
			}
			if(sGivenName[0])
			{
				strcpy(pItemRet[j].sWord,sGivenName);
				pItemRet[j++].nHandle=28274;
			}
			bProcessed=true;
		}
		//Rule2 for overlap words ABB һ�ζΡ�һƬƬ
		else if(pItem[i].nHandle==27904&&strlen(pItem[i+1].sWord)==2&&strcmp(pItem[i+1].sWord,pItem[i+2].sWord)==0)
		{//(pItem[i+1].nHandle/256=='q'||pItem[i+1].nHandle/256=='a')&&
			strcpy(pItemRet[j].sWord,pItem[i].sWord);
			strcat(pItemRet[j].sWord,pItem[i+1].sWord);
			strcat(pItemRet[j].sWord,pItem[i+2].sWord);
			pItemRet[j].nHandle=27904;
			j+=1;
			i+=2;
			bProcessed=true;
		}
		//Rule3 for overlap words AA
		else if(nLen==2&&strcmp(pItem[i].sWord,pItem[i+1].sWord)==0)
		{
			strcpy(pItemRet[j].sWord,pItem[i].sWord);
			strcat(pItemRet[j].sWord,pItem[i+1].sWord);
		     //24832=='a'*256
			pItemRet[j].nHandle=24832;//a
			if(pItem[i].nHandle/256=='v'||pItem[i+1].nHandle/256=='v')//30208='v'8256
			{
				pItemRet[j].nHandle=30208;
			}
			if(pItem[i].nHandle/256=='n'||pItem[i+1].nHandle/256=='n')//30208='v'8256
			{
				pItemRet[j].nHandle='n'*256;
			}			
			i+=1;
			if(strlen(pItem[i+1].sWord)==2)
			{//AAB:ϴ/ϴ/����������
				if((pItemRet[j].nHandle==30208&&pItem[i+1].nHandle/256=='n')||
				   (pItemRet[j].nHandle==24832&&pItem[i+1].nHandle/256=='a')
				   )
				{
					strcat(pItemRet[j].sWord,pItem[i+1].sWord);
					i+=1;
				}
			}
			j+=1;
			bProcessed=true;
		}

		//Rule 4: AAB ϴ/ϴ��
		else if(nLen==2&&strncmp(pItem[i].sWord,pItem[i+1].sWord,2)==0&&strlen(pItem[i+1].sWord)==4&&(pItem[i].nHandle/256=='v'||pItem[i].nHandle==24832))//v,a
		{
			strcpy(pItemRet[j].sWord,pItem[i].sWord);
			strcat(pItemRet[j].sWord,pItem[i+1].sWord);
		     //24832=='a'*256
			pItemRet[j].nHandle=24832;//'a'
			if(pItem[i].nHandle/256=='v'||pItem[i+1].nHandle/256=='v')//30208='v'8256
			{
				pItemRet[j].nHandle=30208;
			}

			i+=1;
			j+=1;
			bProcessed=true;
		}
		else if(pItem[i].nHandle/256=='u'&&pItem[i].nHandle%256)//uj,ud,uv,uz,ul,ug->u
			pItem[i].nHandle='u'*256;
		else if(nLen==2&&strncmp(pItem[i].sWord,pItem[i+1].sWord,2)==0&&strlen(pItem[i+1].sWord)==4&&strncmp(pItem[i+1].sWord+2,pItem[i+2].sWord,2)==0)
		{//AABB �������� ֦֦ҶҶ
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				strcat(pItemRet[j].sWord,pItem[i+2].sWord);
				pItemRet[j].nHandle=pItem[i+1].nHandle;
				i+=2;
				j+=1;
				bProcessed=true;
		}
		else if(pItem[i].nHandle==28275)//PostFix
		{
			if(m_uPlace.m_dict.IsExist(pItem[i+1].sWord,4))
			{
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				pItemRet[j].nHandle=28275;
				i+=1;
				j+=1;
				bProcessed=true;
			}
			else if(strlen(pItem[i+1].sWord)==2&&CC_Find("��",pItem[i+1].sWord))
			{
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				pItemRet[j].nHandle=28276;
				i+=1;
				j+=1;
				bProcessed=true;
			}
			else if(strlen(pItem[i+1].sWord)==2&&CC_Find("�����ֱ�",pItem[i+1].sWord))
			{
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				pItemRet[j].nHandle=28282;
				i+=1;
				j+=1;
				bProcessed=true;
			}
			else if(strlen(pItem[i+1].sWord)==2&&CC_Find("��",pItem[i+1].sWord))
			{
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				pItemRet[j].nHandle=28160;
				i+=1;
				j+=1;
				bProcessed=true;
			}
		}
		else if(pItem[i].nHandle==30208||pItem[i].nHandle==28160)//v
		{
			if(strlen(pItem[i+1].sWord)==2&&CC_Find("Ա",pItem[i+1].sWord))
			{
				strcpy(pItemRet[j].sWord,pItem[i].sWord);
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				pItemRet[j].nHandle=28160;
				i+=1;
				j+=1;
				bProcessed=true;
			}
		}
		else if(pItem[i].nHandle==28280)
		{//www/nx ./w sina/nx; �ţɣ�/nx  -������/m
			strcpy(pItemRet[j].sWord,pItem[i].sWord);
			pItemRet[j].nHandle=28280;
			while(pItem[i+1].nHandle==28280||strstr(".��",pItem[i+1].sWord)||(pItem[i+1].nHandle==27904&&IsAllNum((unsigned char *)pItem[i+1].sWord)))
			{
				strcat(pItemRet[j].sWord,pItem[i+1].sWord);
				i+=1;
			}
			j+=1;
			bProcessed=true;
		}

		if(!bProcessed)
		{//If not processed,that's mean: not need to adjust;
		 //just copy to the final result
			strcpy(pItemRet[j].sWord,pItem[i].sWord);
			pItemRet[j++].nHandle=pItem[i].nHandle;
		}
		i++;
	}
	pItemRet[j].sWord[0]=0;//Set ending
	return true;
}

////Paragraph Segment and POS Tagging
//bool CResult::ParagraphProcessing(char *sParagraph,char *sResult)
//{
//	char *sSentence,sChar[3];
//	char *sSentenceResult;
//	unsigned int nLen=strlen(sParagraph)+13;
//	sSentence=new char[nLen];//malloc buffer
//	sSentenceResult=new char[nLen*3];//malloc buffer
//	sSentence[0]=0;
//	//unsigned int nPosIndex=0,nParagraphLen=strlen(sParagraph),nSentenceIndex=0;
//	unsigned int nPosIndex=0,nParagraphLen=strlen(sParagraph);
//	sChar[2]=0;
//	sResult[0]=0;//Init the result
//	bool bFirstIgnore=true;
//	strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
//	while(nPosIndex<nParagraphLen)
//	{//Find a whole sentence which separated by ! . \n \r
//		sChar[0]=sParagraph[nPosIndex];//Get a char
//		sChar[1]=0;
//		if(sParagraph[nPosIndex]<0)
//		{//double byte char
//			nPosIndex+=1;
//			sChar[1]=sParagraph[nPosIndex];
//		}
//		nPosIndex+=1;
///*
//#define  SEPERATOR_C_SENTENCE "������������"
//#define  SEPERATOR_C_SUB_SENTENCE "����������������"
//#define  SEPERATOR_E_SENTENCE "!?:;"
//#define  SEPERATOR_E_SUB_SENTENCE ",()\042'"
//#define  SEPERATOR_LINK "\n\r ��"
//*/
//		if(CC_Find(SEPERATOR_C_SENTENCE,sChar)||CC_Find(SEPERATOR_C_SUB_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar)||strstr(SEPERATOR_E_SUB_SENTENCE,sChar)||strstr(SEPERATOR_LINK,sChar))
//		{//Reach end of a sentence.Get a whole sentence
//			if(!strstr(SEPERATOR_LINK,sChar))//Not link seperator
//			{
//				strcat(sSentence,sChar);
//			}
//			if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
//			{
//				if(!strstr(SEPERATOR_C_SUB_SENTENCE,sChar)&&!strstr(SEPERATOR_E_SUB_SENTENCE,sChar))
//					strcat(sSentence,SENTENCE_END);//Add sentence ending flag
//
//				Processing(sSentence,1);//Processing and output the result of current sentence.
//				Output(m_pResult[0],sSentenceResult,bFirstIgnore);//Output to the imediate result
//				//bFirstIgnore=true;
//				strcat(sResult,sSentenceResult);//Store in the result buffer
//			}
//			if(strstr(SEPERATOR_LINK,sChar))//Link the result with the SEPERATOR_LINK
//			{
//				strcat(sResult,sChar);
//				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
//
//				//sSentence[0]=0;//New sentence, and begin new segmentation
//				//bFirstIgnore=false;
//			}
//			else if(strstr(SEPERATOR_C_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar))
//			{
//				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
//				//sSentence[0]=0;//New sentence, and begin new segmentation
//				//bFirstIgnore=false;
//			}
//			else
//			{
//				strcpy(sSentence,sChar);//reset current sentence, and add the previous end at begin position
//			}
//		}
//		else //Other chars and store in the sentence buffer
//			strcat(sSentence,sChar);
//	}
//	if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
//	{
//		strcat(sSentence,SENTENCE_END);//Add sentence ending flag
//		Processing(sSentence,1);//Processing and output the result of current sentence.
//		Output(m_pResult[0],sSentenceResult,bFirstIgnore);//Output to the imediate result
//		strcat(sResult,sSentenceResult);//Store in the result buffer
//	}
//	delete []  sSentence;//FREE sentence buffer 	
//	delete []  sSentenceResult;//free buffer
//	return true;
//}

//Paragraph Segment and POS Tagging
//����һ������
//����Ҫ��һ�����ָ��ݱ������г������Ӿ�
//�Ӿ���β�������Ҫ����Ͼ��ӿ�ʼ������Ǻ�������Processing���д���
//ʹ����һЩ�����������ظ��������ӿ��ٶȡ�
bool CResult::ParagraphProcessing(const char *sParagraph,char *sResult)
{
	char *sSentence,sChar[3];
	char *sSentenceResult;
	unsigned int nLen=strlen(sParagraph)+13;
	sSentence=new char[nLen];//malloc buffer
	sSentenceResult=new char[nLen*3];//malloc buffer
	sSentence[0]=0;
	unsigned int nPosIndex=0,nParagraphLen=strlen(sParagraph);
	sChar[2]=0;
	sResult[0]=0;//Init the result
	bool bFirstIgnore=true;
	strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
	//���ζ����ַ���������ȫ�ǿ����ǰ�ǣ�������ִ���ò��ã�Ӧ��дһ����������������������úܳ�����

	std::cout<<"-------------------------"<<sSentence<<std::endl;
	while(nPosIndex<nParagraphLen)
	{//Find a whole sentence which separated by ! . \n \r
		sChar[0]=sParagraph[nPosIndex];//Get a char
		sChar[1]=0;
		if(sParagraph[nPosIndex]<0)
		{//double byte char
			nPosIndex+=1;
			sChar[1]=sParagraph[nPosIndex];
		}
		nPosIndex+=1;

/*
//ȫ�ǵľ��ӷָ��
#define  SEPERATOR_C_SENTENCE "������������"
//ȫ�ǵ��Ӿ�ָ��
#define  SEPERATOR_C_SUB_SENTENCE "����������������"
//��ǵľ��ӷָ��
#define  SEPERATOR_E_SENTENCE "!?:;"
//��ǵ��Ӿ�ָ��
#define  SEPERATOR_E_SUB_SENTENCE ",()\042'"
//���ӷ�
#define  SEPERATOR_LINK "\n\r ��"
*/
		//���ڽ���ԭ���з�֮ǰ������Ҫ���жϾ䴦��
		//��ν�Ͼ䣬���Ǹ��ݷָ������س����з������ķָ���־����Դ�ַ����ָ��ɶ����΢��һ��Ķ̾䣬
		//�ٽ��зִʴ�������ٰѸ����ִʽ�����������γ����յķִʽ����
		if(CC_Find(SEPERATOR_C_SENTENCE,sChar)||CC_Find(SEPERATOR_C_SUB_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar)||strstr(SEPERATOR_E_SUB_SENTENCE,sChar)||strstr(SEPERATOR_LINK,sChar))
		{//Reach end of a sentence.Get a whole sentence
			//�ֳɶ̾�֮�󣬼��ɽ���ԭ�ӷִʣ���νԭ��,��ָ�ö̾��в��ɷָ����С���ص�λ��
			//һ�����֡��̾�ǰ��Ŀ�ʼ������ʶ�ֶΡ�ȫ�Ǳ����š�����һ���������ĸ���ֽ��ַ��ȡ�
			if(!strstr(SEPERATOR_LINK,sChar))//Not link seperator
			{
				strcat(sSentence,sChar);
			}
//			std::cout<<sSentence<<std::endl;
			//�����Ѿ����ɵĴ�������Ӵ�������ǿ�Ҳ����ֻ�п�ʼ���: ����Processing���д���
			if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
			{
				if(!strstr(SEPERATOR_C_SUB_SENTENCE,sChar)&&!strstr(SEPERATOR_E_SUB_SENTENCE,sChar))
					strcat(sSentence,SENTENCE_END);//Add sentence ending flag

				//ֻ����һ��������������ɵ��зֽ�������׷�ӵ��������
				//ע�������������Ӵ������һ���ַ������Ӿ��������Ҫ׷��һ�����ӽ�����
				Processing(sSentence,1);//Processing and output the result of current sentence.
				//��PWORD_RESULT���������е��дʽṹת���ַ�����ʾ��ʽ
				Output(m_pResult[0],sSentenceResult,bFirstIgnore);//Output to the imediate result
				//bFirstIgnore=true;
				strcat(sResult,sSentenceResult);//Store in the result buffer
			}
			if(strstr(SEPERATOR_LINK,sChar))//Link the result with the SEPERATOR_LINK
			{
#if _ICT_DEBUG
				printf("SEPERATOR_LINK: ->%s<-\n", sChar);
#endif
				strcat(sResult,sChar);
				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag

				//sSentence[0]=0;//New sentence, and begin new segmentation
				//bFirstIgnore=false;
			}
			else if(strstr(SEPERATOR_C_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar))
			{
				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
				//sSentence[0]=0;//New sentence, and begin new segmentation
				//bFirstIgnore=false;
			}
			else
			{
				//�������֮��Ҫ���ô�����ľ���
				strcpy(sSentence,sChar);//reset current sentence, and add the previous end at begin position
			}
		}
		//�������ȫ�ǵľ��ӣ��Ӿ䣩�����������ǰ�ǵľ��ӣ��Ӿ䣩������Ҳ�����Ƕ����������
		//��׷�ӵ�������ľ��Ӵ�����ȡ��һ���ַ��������һ������
		else //Other chars and store in the sentence buffer
			strcat(sSentence,sChar);
	}//while 
	if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
	{
		//������һ���ַ��ǻ��з������Ǿ��ӽ������������¾���Ϊ��ʼ���
		//����򵥵İ�ԭ�������һ���ַ������¾���
		strcat(sSentence,SENTENCE_END);//Add sentence ending flag
		//���������ַ��������������䳤��֮�󣬲�Ҫ���ǰ�δ������ľ�������Processing�����з�
		Processing(sSentence,1);//Processing and output the result of current sentence.
		Output(m_pResult[0],sSentenceResult,bFirstIgnore);//Output to the imediate result
		strcat(sResult,sSentenceResult);//Store in the result buffer
	}
	delete []  sSentence;//FREE sentence buffer 	
	delete []  sSentenceResult;//free buffer
	return true;
}

bool CResult::ParagraphProcessingA(const char *sParagraph, result_t *result_arr, int &result_num)
{
	char *sSentence,sChar[3];
	//char *sSentenceResult;
	unsigned int nLen=strlen(sParagraph)+13;
	sSentence=new char[nLen];//malloc buffer
	//sSentenceResult=new char[nLen*3];//malloc buffer
	sSentence[0]=0;
	unsigned int nPosIndex=0,nParagraphLen=strlen(sParagraph);
	sChar[2]=0;
	//sResult[0]=0;//Init the result
	bool bFirstIgnore=true;
	strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag

	int curr_pos = 0; //��ǰ��λ��
	result_num   = 0; //�������

	while(nPosIndex<nParagraphLen)
	{//Find a whole sentence which separated by ! . \n \r
		sChar[0]=sParagraph[nPosIndex];//Get a char
		sChar[1]=0;
		if(sParagraph[nPosIndex]<0)
		{//double byte char
			nPosIndex+=1;
			sChar[1]=sParagraph[nPosIndex];
		}
		nPosIndex+=1;

		if(CC_Find(SEPERATOR_C_SENTENCE,sChar)||CC_Find(SEPERATOR_C_SUB_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar)||strstr(SEPERATOR_E_SUB_SENTENCE,sChar)||strstr(SEPERATOR_LINK,sChar))
		{//Reach end of a sentence.Get a whole sentence
			if(!strstr(SEPERATOR_LINK,sChar))//Not link seperator
			{
				strcat(sSentence,sChar);
			}
			//�����Ѿ����ɵĴ�������Ӵ�������ǿ�Ҳ����ֻ�п�ʼ���: ����Processing���д���
			if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
			{
				if(!strstr(SEPERATOR_C_SUB_SENTENCE,sChar)&&!strstr(SEPERATOR_E_SUB_SENTENCE,sChar))
					strcat(sSentence,SENTENCE_END);//Add sentence ending flag

				Processing(sSentence,1);//Processing and output the result of current sentence.
				OutputA(m_pResult[0], curr_pos, result_num, result_arr, bFirstIgnore);
			}
			if(strstr(SEPERATOR_LINK,sChar))//Link the result with the SEPERATOR_LINK
			{
				curr_pos += strlen(sChar);
				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
			}
			else if(strstr(SEPERATOR_C_SENTENCE,sChar)||strstr(SEPERATOR_E_SENTENCE,sChar))
			{
				strcpy(sSentence,SENTENCE_BEGIN);//Add a sentence begin flag
			}
			else
			{
				//�������֮��Ҫ���ô�����ľ���
				strcpy(sSentence,sChar);//reset current sentence, and add the previous end at begin position
			}
		}
		//�������ȫ�ǵľ��ӣ��Ӿ䣩�����������ǰ�ǵľ��ӣ��Ӿ䣩������Ҳ�����Ƕ����������
		//��׷�ӵ�������ľ��Ӵ�����ȡ��һ���ַ��������һ������
		else //Other chars and store in the sentence buffer
			strcat(sSentence,sChar);
	}//while 
	if(sSentence[0]!=0&&strcmp(sSentence,SENTENCE_BEGIN)!=0)
	{
		//������һ���ַ��ǻ��з������Ǿ��ӽ������������¾���Ϊ��ʼ���
		//����򵥵İ�ԭ�������һ���ַ������¾���
		strcat(sSentence,SENTENCE_END);//Add sentence ending flag
		//���������ַ��������������䳤��֮�󣬲�Ҫ���ǰ�δ������ľ�������Processing�����з�
		Processing(sSentence,1);//Processing and output the result of current sentence.
		OutputA(m_pResult[0], curr_pos, result_num, result_arr, bFirstIgnore);
	}
	delete []  sSentence;//FREE sentence buffer 	
	//delete []  sSentenceResult;//free buffer
	return true;
}

bool CResult::FileProcessing(char *sSourceFile,char *sResultFile)
{
	FILE *fpSource,*fpResult;//The file pointer of read and write
	char *sParagraph,*sParagraphResult;
	//int nLineIndex=1;
	sParagraph=new char[4*1024];
	sParagraphResult=new char[8*1024];
    if((fpSource=fopen(sSourceFile,"rt"))==NULL)
		return false;//Cannot open the source file to read
    if((fpResult=fopen(sResultFile,"wt"))==NULL) 
		return false;//Cannot open the result  file to write
	if(m_nOutputFormat==2)//XML format
		fprintf(fpResult,"<?xml version=\042 1.0\042 encoding=\042gb2312\042?><result>");
	while(!feof(fpSource))
	{
		if(fgets(sParagraph,4*1024,fpSource)==0)//Get a paragrah
			continue;
#ifndef unix
		TRACE("%d\n",nLineIndex++);
#endif
		ParagraphProcessing(sParagraph,sParagraphResult);
		fprintf(fpResult,"%s",sParagraphResult);
	}
	delete [] sParagraph;
	delete [] sParagraphResult;
	fclose(fpSource);	
	if(m_nOutputFormat==2)//XML format
		fprintf(fpResult,"</result>");
	fclose(fpResult);
	return true;
}

bool CResult::PKU2973POS(int nHandle, char *sPOS973)
{
	int nHandleSet[46]={24832,24932,24935,24942,25088,25344,25600,25703,25856,26112,26368,26624,26880,27136,27392,27648,27904,28160,28263,28274,28275,28276,28280,28282,28416,28672,28928,29184,29440,29696,29799,29952,30052,30055,30058,30060,30070,30074,30208,30308,30311,30318,30464,30720,30976,31232};
						//   "a", "ad","ag","an","b", "c", "d", "dg","e", "f","g", "h", "i", "j", "k", "l", "m", "n", "ng","nr","ns","nt","nx","nz","o", "p", "q", "r", "s", "t", "tg","u", "ud","ug","uj","ul","uv","uz","v", "vd","vg","vn","w", "x", "y", "z"
	char sPOSRelated[46][3]={"a", "ad","ga","an","f", "c", "d", "d", "e","nd","g", "h", "i", "j", "k", "l", "m", "n", "gn","nh","ns","ni","ws", "nz","o", "p", "q", "r", "nl","nt","gt","u", "ud","ug","uj","ul","uv","uz","v", "vd","gv","vn","w", "x", "u", "a"};
/* 
 "Bg","gf",
 "Rg","gr",
 "Mg","gm",
 "Yg","u",
 "Ug","u",
 "Qg","q",
*/

	int nIndex=BinarySearch(nHandle,nHandleSet,46);
	if(nIndex==-1)
		strcpy(sPOS973,"@");
	else
		strcpy(sPOS973,sPOSRelated[nIndex]);
	return true;
}

bool CResult::ChineseNameSplit(char *sPersonName, char *sSurname, char *sSurname2, char *sGivenName, CDictionary &personDict)
{
	int nSurNameLen=4,nLen=strlen(sPersonName),nFreq,i=0,nCharType,nFreqGiven;
	char sTemp[3];
	if(nLen<3||nLen>8)//Not a traditional Chinese person name
		return false;
	while(i<nLen)//No Including non-CHinese char
	{
		nCharType=charType((unsigned char*)sPersonName+i);
		if(nCharType!=CT_CHINESE&&nCharType!=CT_OTHER)
			return false;
		i+=2;
	}
	sSurname2[0]=0;//init 
	strncpy(sSurname,sPersonName,nSurNameLen);	
	sSurname[nSurNameLen]=0;
	if(!personDict.IsExist(sSurname,1))
	{
		nSurNameLen=2;
		sSurname[nSurNameLen]=0;
		if(!personDict.IsExist(sSurname,1))
		{
			nSurNameLen=0;
			sSurname[nSurNameLen]=0;
		}
	}
	strcpy(sGivenName,sPersonName+nSurNameLen);
	if(nLen>6)
	{
		strncpy(sTemp,sPersonName+nSurNameLen,2);
		sTemp[2]=0;//Get the second possible surname
		if(personDict.IsExist(sTemp,1))
		{//Hongkong women's name: Surname+surname+given name
			strcpy(sSurname2,sTemp);
			strcpy(sGivenName,sPersonName+nSurNameLen+2);
		}
	}
	nFreq=personDict.GetFrequency(sSurname,1);
	strncpy(sTemp,sGivenName,2);
	sTemp[2]=0;
	nFreqGiven=personDict.GetFrequency(sTemp,2);
	char sz_zhang[3] = "��";
	char sz_hua[3]   = "��";
	//if(nSurNameLen!=4&&((nSurNameLen==0&&nLen>4)||strlen(sGivenName)>4||(GetForeignCharCount(sPersonName)>=3&&nFreq<personDict.GetFrequency("��",1)/40&&nFreqGiven<personDict.GetFrequency("��",2)/20)||(nFreq<10&&GetForeignCharCount(sGivenName)==(nLen-nSurNameLen)/2)))
	if(nSurNameLen!=4&&((nSurNameLen==0&&nLen>4)||strlen(sGivenName)>4||(GetForeignCharCount(sPersonName)>=3&&nFreq<personDict.GetFrequency(sz_zhang,1)/40&&nFreqGiven<personDict.GetFrequency(sz_hua,2)/20)||(nFreq<10&&GetForeignCharCount(sGivenName)==(nLen-nSurNameLen)/2)))
		return false;
	if(nLen==4&&m_uPerson.IsGivenName(sPersonName))
	{//Single Surname+given name
		return false;
	}
	return true;
}

