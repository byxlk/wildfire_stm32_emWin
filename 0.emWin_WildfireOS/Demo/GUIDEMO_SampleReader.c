
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "DIALOG.h"


static FRESULT scan_files (char* path,char*file_name,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode) ;
static void OpenFileProcess(int sel_num);

#define FILE_NAME_LEN 	150			//�ļ������ȣ������⵽�ļ�������50 ��������ļ� 
#define PATH_LEN				100
#define MUSIC_NAME_LEN 	24			//LCD��ʾ���ļ�������󳤶�
#define _DF1S	0x81

#define FILE_LIST_PATH "0:WF_OS/filelist.txt"

	
static		FIL	file;								//�ļ����	
static  	FRESULT fres;							//���ؽ��
static		unsigned int rw_num;			//�Ѷ�����д���ֽ���
static  int xSize  , ySize;	

/**
  * @brief  �Ի����õ���С������Դ��
	*					
  * @param  none
  * @retval none
  */
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Message",    0,      20,  90, 200, 100, FRAMEWIN_CF_MOVEABLE },
  { TEXT_CreateIndirect,     "Do you sure to open the file?",     	GUI_ID_TEXT0,    10,  10, 180,  80 },
  { BUTTON_CreateIndirect,   "Yes",   	GUI_ID_YES,     10, 50,  60,  18 },
  { BUTTON_CreateIndirect,   "No", 			GUI_ID_NO,  120, 50,  60,  18 },
};

/*********************************************************************
*
*       �Ի���ص�����
*/
static void _cbCallback(WM_MESSAGE * pMsg) {
  int i;
  int NCode, Id;
  WM_HWIN hDlg, hItem;
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
    case WM_INIT_DIALOG:									//��ʼ���Ի���ʱ
			hItem = WM_GetDialogItem(hDlg, GUI_ID_TEXT0);
			TEXT_SetFont(hItem,GUI_FONT_16_ASCII);	//����text������
      break;
    case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);    /* Id of widget */
      NCode = pMsg->Data.v;               /* Notification code */
      switch (NCode) {

      case WM_NOTIFICATION_RELEASED:      /* React only if released */
        if (Id == GUI_ID_YES) {           /* Yes Button */     
					GUI_EndDialog(hDlg, 0);					//�����Ի��򣬷���0
        }
        if (Id == GUI_ID_NO) {        		/* No Button */
          GUI_EndDialog(hDlg, 1);					//�����Ի��򣬷���1
        }
        break;
      }
      break;
    default:
      WM_DefaultProc(pMsg);
  }
}

/*******************************************************************
*
*       �������ڻص�����
*/
static void _cbBkWindow(WM_MESSAGE* pMsg) {
		TREEVIEW_ITEM_INFO ItemInfo;
		TREEVIEW_ITEM_Handle hNode;
	
  switch (pMsg->MsgId) {
		
		case WM_NOTIFY_PARENT:	 //֪ͨ����
			
			switch(pMsg->Data.v)	//֪ͨ��������
			{
				case WM_NOTIFICATION_RELEASED:	//�ͷ���
					printf("\r\n release");
				
				/* �鿴ѡ�����ĸ���Ŀ */
				hNode = TREEVIEW_GetSel(pMsg->hWinSrc);		
				/* ��ȡ����Ŀ����Ϣ */
				TREEVIEW_ITEM_GetInfo(hNode,&ItemInfo);
				
				if(ItemInfo.IsNode == 0)//Ҷ��
				{
					printf("\r\n leaf num =%ld",hNode);
					

					/* �ٷ��ֲ�˵�����ڻص������е��������Ի��򣬵����������Ҳû���󣬾��������ð� */
					/* ���ڻص��������õĻ��Ϳ������������ */
					/* ѡyes ����0 ѡno����1*/
					if(	GUI_ExecDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, WM_HBKWIN, 0, 0) == 0)
					{
									//open file
							printf("\r\n open file");
							OpenFileProcess(hNode);
						}
						
					//����ѡ��Ϊ��Ҷ�ӵĸ���㣬��ֹ�󴥷�
					TREEVIEW_SetSel(pMsg->hWinSrc,TREEVIEW_GetItem(pMsg->hWinSrc,hNode,TREEVIEW_GET_PARENT));
	

				}
				else										//���
				{					
					printf("\r\n node num =%ld",hNode);
				}
				break;				
				
			}	
		
    default:		
      WM_DefaultProc(pMsg);
  }
}

/**
  * @brief  SetDefaultSkin����gui��Ĭ��Ƥ��
	*					
  * @param  none
  * @retval none
  */
static void SetDefaultSkin(void)
{

  BUTTON_SetDefaultSkin   (BUTTON_SKIN_FLEX);
  CHECKBOX_SetDefaultSkin (CHECKBOX_SKIN_FLEX);
  DROPDOWN_SetDefaultSkin (DROPDOWN_SKIN_FLEX);
  FRAMEWIN_SetDefaultSkin (FRAMEWIN_SKIN_FLEX);
  HEADER_SetDefaultSkin   (HEADER_SKIN_FLEX);
  PROGBAR_SetDefaultSkin  (PROGBAR_SKIN_FLEX);
  RADIO_SetDefaultSkin    (RADIO_SKIN_FLEX);
  SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
  SLIDER_SetDefaultSkin   (SLIDER_SKIN_FLEX);
	
	FRAMEWIN_SetDefaultTitleHeight(20);
	FRAMEWIN_SetDefaultFont(GUI_FONT_8X16);
	FRAMEWIN_SetDefaultTextColor(1,GUI_BLACK); //���ÿ�ܴ��ڼ���״̬ʱ�ı���������ɫ
}

/**
  * @brief  OpenFileProcess���ļ�
	*					
  * @param  none
  * @retval none
  */
static void OpenFileProcess(int sel_num)
{
	char* file_name;
	char* read_buffer; 
	
	FRAMEWIN_Handle hFrame;
	MULTIEDIT_HANDLE hMultiEdit;
	
	/* ������ܴ��� */
	hFrame = FRAMEWIN_Create("eBook reader", 0, WM_CF_SHOW, 0, 0, xSize, ySize);
  
	/* ���ô��ڿ��ƶ� */
  FRAMEWIN_SetMoveable(hFrame, 1);
  /* �������ڰ�ť */
  FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);
  FRAMEWIN_AddMaxButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 1);
  FRAMEWIN_AddMinButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 2);
	
	
	
	
	file_name 	= (char * ) malloc(FILE_NAME_LEN* sizeof(char));  //Ϊ�洢Ŀ¼����ָ�����ռ�
	read_buffer = (char * ) malloc(300* sizeof(char));  //Ϊ�洢Ŀ¼����ָ�����ռ�
	*read_buffer ='\0';
	
	fres = f_open (&file, FILE_LIST_PATH, FA_READ ); 		//�򿪴��������ļ�
	fres = f_lseek (&file, sel_num*FILE_NAME_LEN);
	fres = f_read(&file, file_name, FILE_NAME_LEN, &rw_num);
	fres = f_close (&file);
	
	
	printf("file_name =%s",file_name);
	fres = f_open (&file, file_name, FA_READ ); 				//��Ҫ�Ķ����ļ�
	
	hMultiEdit=MULTIEDIT_CreateEx(5,22,xSize-5,ySize-22,hFrame,WM_CF_SHOW,MULTIEDIT_CF_READONLY|MULTIEDIT_CF_AUTOSCROLLBAR_V,GUI_ID_MULTIEDIT0,0,0);
	MULTIEDIT_SetFont(hMultiEdit,GUI_FONT_13B_ASCII);
	MULTIEDIT_SetWrapWord(hMultiEdit);

	while(file.fptr!=file.fsize) 		//���ָ��ָ�����ļ�β����ʾ����ȫ������
	{
		fres = f_read(&file, read_buffer, 300, &rw_num);
		if(rw_num<300)
			read_buffer[rw_num]='\0';		//�����ļ�β���Ͻ�����
		
		MULTIEDIT_AddText(hMultiEdit,read_buffer);		
	}
	fres = f_close (&file);
	printf("%s",read_buffer);
	
	
	
	free(read_buffer);								//�ͷ�malloc�ռ�		
	read_buffer = NULL;
	
	free(file_name);								//�ͷ�malloc�ռ�		
	file_name = NULL;

}


/**
  * @brief  scan_files �ݹ�ɨ��sd���ڵĸ����ļ�
  * @param  path:��ʼɨ��·��  hTree Ŀ¼�� hNode Ŀ¼���
  * @retval result:�ļ�ϵͳ�ķ���ֵ
  */
static FRESULT scan_files (char* path,char* file_name,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode) 
{ 
		
    FRESULT res; 		//�����ڵݹ���̱��޸ĵı���������ȫ�ֱ���	
    FILINFO fno; 
    DIR dir; 
    int i; 
    char *fn; 
	
		TREEVIEW_ITEM_Handle hItem;

	
#if _USE_LFN 
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//���ļ���֧��
    fno.lfname = lfn; 
    fno.lfsize = sizeof(lfn); 
#endif 

    res = f_opendir(&dir, path); //��Ŀ¼
    if (res == FR_OK) 
			{ 
        i = strlen(path); 
        for (;;) 
				{ 
            res = f_readdir(&dir, &fno); 										//��ȡĿ¼�µ�����
            if (res != FR_OK || fno.fname[0] == 0) break; 	//Ϊ��ʱ��ʾ������Ŀ��ȡ��ϣ�����
#if _USE_LFN 
            fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
            fn = fno.fname; 
#endif 
            if (*fn == '.') continue; 											//���ʾ��ǰĿ¼������			
            if (fno.fattrib & AM_DIR) 
						{ 																							//Ŀ¼���ݹ��ȡ
								hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,fn,0);						//Ŀ¼���������
								TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//�ѽ����뵽Ŀ¼����
							
							  sprintf(&path[i], "/%s", fn); 							//�ϳ�����Ŀ¼��
                res = scan_files(path,file_name,hTree,hItem);					//�ݹ���� 
                if (res != FR_OK) 
									break; 																		//��ʧ�ܣ�����ѭ��
                path[i] = 0; 
            } 
						else 
						{ 
							printf("%s/%s hItem = %d  \r\n", path, fn,(int)hItem);								//����ļ���	
							hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,fn,0);						//�ļ���������Ҷ
							TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//����Ҷ���ӵ�Ŀ¼��
							
							if (strlen(path)+strlen(fn)<FILE_NAME_LEN)
							{
								sprintf(file_name, "%s/%s", path,fn); 									
								//�洢�ļ�����filelist(��·��)										
								res = f_lseek (&file, hItem*FILE_NAME_LEN);  
								res = f_write (&file, file_name, FILE_NAME_LEN, &rw_num);						
							}								
           }//else
        } //for
    } 

    return res; 
} 



/**
  * @brief  sdCardView_MainTask�����ǵݹ���̣�Ȼ����õݹ麯��scan_filesɨ��Ŀ¼
	*					
  * @param  path:��ʼɨ��·��
  * @retval none
  */
 void SampleReader_MainTask(char* path)
{ 
	FATFS fsys;
	char * p_path;									//Ŀ¼�� ָ��
	char * file_name;								//���ڴ洢��Ŀ¼�ļ�����
	FRESULT fres;										//���ؽ��
	
	WM_HWIN hTree;	  							//Ŀ¼�����
  TREEVIEW_ITEM_Handle hNode;			//�����
	TREEVIEW_ITEM_INFO ItemInfo;
 // int xSize  , ySize;	
	
	SetDefaultSkin();								//����Ĭ��Ƥ��
  //
  // ��ȡ��Ļ��С
  //
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // ���ñ�����ɫ����������С������
  //
  WM_SetDesktopColor(GUI_BLACK);
  SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
  SCROLLBAR_SetDefaultWidth(25);
  SCROLLBAR_SetThumbSizeMin(25);
  TEXT_SetDefaultFont(GUI_FONT_6X8_ASCII);
  //
  // ���ϵͳ��Ϣ
  //
  GUI_DrawGradientV(0, 0, xSize - 1, ySize - 1, GUI_BLUE, GUI_BLACK);
  GUI_SetFont(GUI_FONT_20F_ASCII);
  GUI_DispStringHCenterAt("scanning sd Card...", xSize >> 1, ySize / 3);
  GUI_X_Delay(500);
  //
  // ����Ŀ¼��
  //
  hTree = TREEVIEW_CreateEx(0, 0, xSize, ySize, WM_HBKWIN, WM_CF_SHOW, TREEVIEW_CF_AUTOSCROLLBAR_H, GUI_ID_TREEVIEW0);
  TREEVIEW_SetAutoScrollV(hTree, 1);
  TREEVIEW_SetSelMode(hTree, TREEVIEW_SELMODE_ROW);

	WM_SetCallback(WM_HBKWIN,_cbBkWindow); //����Ŀ¼���ĸ�����(��������)�Ļص�����
  
	// ���Ŀ¼��,������һ��Ŀ¼���
  //
  hNode = TREEVIEW_InsertItem(hTree, TREEVIEW_ITEM_TYPE_NODE, 0, 0, "sd Card");
	
  
	f_mount(0,&fsys);								//ע���ļ�ϵͳ������	
	
	fres = f_unlink(FILE_LIST_PATH);//ɾ���ɵ�filelist

	p_path = (char * ) malloc(500* sizeof(char));  //Ϊ�洢Ŀ¼����ָ�����ռ�
	file_name = (char * ) malloc(FILE_NAME_LEN* sizeof(char));  //Ϊ�洢Ŀ¼����ָ�����ռ�
	
	fres = f_open (&file, FILE_LIST_PATH, FA_READ|FA_WRITE|FA_CREATE_ALWAYS ); //�򿪴��������ļ�

	strcpy(p_path,path);						//����Ŀ¼����ָ��
	
	fres = scan_files(p_path,file_name,hTree,hNode);			//�ݹ�ɨ������ļ�		
	
	fres = f_close (&file);					//�ر������ļ�		

	free(p_path);										//�ͷ�malloc�ռ�		
	p_path = NULL;
	
	free(file_name);								//�ͷ�malloc�ռ�		
	file_name = NULL;

	
 // f_mount(0, NULL);								//�ͷ��ļ�ϵͳ�����ռ�		

while(1)
	{
		GUI_Delay (1000);
	}
}
	

