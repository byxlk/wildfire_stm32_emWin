
#include "WFGUI_Message.h"
#include "WFGUI_Keypad.h"

#include "DIALOG.h"
#include "ff.h"


#define MAX_PATH 256

static WM_CALLBACK*     _pcbOldMessageWin = NULL;


/*********************************************************************
*
*       _aBitmapItem
*/
static const BITMAP_ITEM _MessageIcon[] = {
  {&bmWF_Floder, "In box" , "Received messages box"},
  {&bmWF_Floder, "Out box" , "Sended messages box"},
  {&bmWF_Note,   "Draft box" , "Draft messages box"},

};


/**
  * @brief  _cbMessageWin framewin框架的回调函数
	*					
  * @param  none
  * @retval none
  */
static void _cbMessageWin(WM_MESSAGE * pMsg)
{
	int  Id;
	
		switch (pMsg->MsgId) {			//消息类型

#if 1			//奇怪，必须要有这个项
		case WM_NOTIFY_PARENT:		//通知		
			break;//通知代码break
#endif	
			
			case WM_DELETE:
				/* 删除app句柄链表里的记录 */	
				App_Delete(pMsg->hWin);
			
				/* 发送消息通知ctrl窗口*/		
				WM_SendMessageNoPara(WinPara.hWinCtrl,MY_MESSAGE_CTRLCHANGE);	
			
			
				break;
		
			default:		
      WM_DefaultProc(pMsg);
			
		}//MsgId
	
	if (_pcbOldMessageWin) {
			(*_pcbOldMessageWin)(pMsg);	//调用旧的回调函数处理部分未处理过程
		}	
	
}


static void Mesg_GetData(char *path)
{
	

}


static void _cbMesgNew(WM_MESSAGE * pMsg)
{
	
	int NCode;
  int Id;
	int xSize,ySize;
	
	char num[50];
	char mesg[200];
	
	WM_MESSAGE Close_Msg;
	
  switch (pMsg->MsgId) {
		
		case WM_NOTIFY_PARENT:							//通知父窗口
			
		Id    = WM_GetId(pMsg->hWinSrc);      // 控件的ID
		NCode = pMsg->Data.v;                 // 通知代码
		
		switch(NCode){
				 
				 case WM_NOTIFICATION_RELEASED:
					if(Id == GUI_ID_BUTTON0)//发送按钮
					{
						MULTIEDIT_GetText(WM_GetDialogItem(pMsg->hWin,GUI_ID_MULTIEDIT0),num,sizeof(num));//电话号码
						
						MULTIEDIT_GetText(WM_GetDialogItem(pMsg->hWin,GUI_ID_MULTIEDIT1),mesg,sizeof(mesg));//短信内容
						
						

						
					}
					else if(Id == GUI_ID_BUTTON1)
					{
						FIL hFile;
						FRESULT res;
						UINT rwb;
						
						MULTIEDIT_GetText(WM_GetDialogItem(pMsg->hWin,GUI_ID_MULTIEDIT0),num,sizeof(num));//电话号码
						
						MULTIEDIT_GetText(WM_GetDialogItem(pMsg->hWin,GUI_ID_MULTIEDIT1),mesg,sizeof(mesg));//短信内容
						
						res = f_open(&hFile,"0:WF_OS/Mesg/draftbox/newdraft.txt",FA_WRITE|FA_CREATE_ALWAYS);
						
						f_write(&hFile,num,sizeof(num),&rwb);
						
						f_write(&hFile,mesg,sizeof(mesg),&rwb);
						
						f_close(&hFile);

					}
					else if(Id == GUI_ID_BUTTON2)
					{				

						/* 关闭窗口 */
						App_Delete(WM_GetParent(pMsg->hWin));
						
						WM_DeleteWindow(WM_GetParent(pMsg->hWin));
						}						
				 
					 break;
				 
				 default:
					 break;
				 
			 }
		
		break;

	
  case WM_PAINT:
    xSize = WM_GetWindowSizeX(pMsg->hWin);
    ySize = WM_GetWindowSizeY(pMsg->hWin);
		

		GUI_SetColor(GUI_LIGHTGRAY);
		GUI_FillRect(0,0,xSize,ySize);

    break;
	
  default:
    WM_DefaultProc(pMsg);
	break;
	
	}

}

/**
  * @brief  Mesg_New,新建消息函数
  * @param  none
  * @retval none
  */
static void Mesg_New(char *path)
{
	WM_HWIN hFrame;
	WM_HWIN hFrameC;
	

	WM_HWIN hMulti;
	WM_HWIN hText;
	WM_HWIN hEdit;
	WM_HWIN hButton;

	hFrame = FRAMEWIN_CreateEx(0,0,WinPara.xSizeWin,WinPara.ySizeWin,WinPara.hWinMain,WM_CF_SHOW,FRAMEWIN_CF_ACTIVE|FRAMEWIN_SF_MOVEABLE,GUI_ID_FRAMEWIN10,"Write a Message",0);

	App_Insert(hFrame);
	
	/* 创建窗口按钮 */
	FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);
	
	/* 获取框架窗口用户区的句柄 */
	hFrameC = WM_GetClientWindow(hFrame);

	/* 设置信息api的回调函数 */
	WM_SetCallback(hFrameC,_cbMesgNew);	
	
	/* 电话号码窗口 */
	hText = TEXT_CreateEx(5,0,65,20,hFrameC,WM_CF_SHOW,TEXT_CF_LEFT|TEXT_CF_VCENTER,GUI_ID_TEXT0,"number:");
	TEXT_SetFont(hText,GUI_FONT_8X13_1);
	hMulti = MULTIEDIT_CreateEx(65,0,120,20,hFrameC,WM_CF_SHOW,0,GUI_ID_MULTIEDIT0,50,"");
	MULTIEDIT_SetFont(hMulti,&SDFont.XFont);
	
	/* 短信内容窗口 */
	hMulti = MULTIEDIT_CreateEx(0,25,WM_GetWindowSizeX(hFrameC), WM_GetWindowSizeY(hFrameC)-50,hFrameC,WM_CF_SHOW,MULTIEDIT_CF_AUTOSCROLLBAR_V,GUI_ID_MULTIEDIT1,50,"");
	MULTIEDIT_EnableBlink(hMulti,500,1);	
	MULTIEDIT_SetFont(hMulti,&SDFont.XFont);
	
	/* 发送按钮 */
	hButton = BUTTON_CreateEx(10,WM_GetWindowSizeY(hFrameC)-20,40,20,hFrameC,WM_CF_SHOW,0,GUI_ID_BUTTON0);
	BUTTON_SetText(hButton,"Send");
	
	/* 保存按钮 */
	hButton = BUTTON_CreateEx(55,WM_GetWindowSizeY(hFrameC)-20,40,20,hFrameC,WM_CF_SHOW,0,GUI_ID_BUTTON1);
	BUTTON_SetText(hButton,"Save");
	
	/* 取消按钮 */
	hButton = BUTTON_CreateEx(105,WM_GetWindowSizeY(hFrameC)-20,40,20,hFrameC,WM_CF_SHOW,0,GUI_ID_BUTTON2);
	BUTTON_SetText(hButton,"Cancle");
	
	/* 添加键盘 */
	KeypadInit(hFrame); 


}

/**
  * @brief  Mesg_Read,读取消息函数
  * @param  none
  * @retval none
  */
static void Mesg_Read(char *path)
{


}



/**
  * @brief  _cbOutBox,草稿箱回调函数
  * @param  none
  * @retval none
  */
static void _cbOutBox(WM_MESSAGE * pMsg)
{
	WM_HWIN    hWin;
	WM_HWIN    hIcon;
	
	int        NCode;
  int        Id;
	int        Sel;
	
	hWin = pMsg->hWin;	

	switch (pMsg->MsgId) 
	{
		case WM_NOTIFY_PARENT:
		 
			Id    = WM_GetId(pMsg->hWinSrc);      // 控件的ID
			NCode = pMsg->Data.v;                 // 通知代码
				 
			switch (Id) {
				case GUI_ID_LISTVIEW0:								// ListView
					switch (NCode) {
						case WM_NOTIFICATION_RELEASED:
						Sel = LISTVIEW_GetSel(pMsg->hWinSrc);
							if(Sel == 0)
							{								
								Mesg_New((char*)Sel);
							}
							else
							{
								Mesg_New("0");
							}

							break;								
						}		//switch (NCode)					
						
				break;
			}//	switch (Id) 
			
		break;
			
		default:
			break;
		
		case WM_PAINT:
			
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(0,0,WM_GetWindowSizeX(pMsg->hWin),WM_GetWindowSizeY(pMsg->hWin));
		
			break;
		
	}
	

}


/**
  * @brief  _cbDraftBox,草稿箱回调函数
  * @param  none
  * @retval none
  */
static void _cbDraftBox(WM_MESSAGE * pMsg)
{
	WM_HWIN    hWin;
	WM_HWIN    hIcon;
	
	int        NCode;
  int        Id;
	int        Sel;
	
	hWin = pMsg->hWin;	

	switch (pMsg->MsgId) 
	{
		case WM_NOTIFY_PARENT:
		 
			Id    = WM_GetId(pMsg->hWinSrc);      // 控件的ID
			NCode = pMsg->Data.v;                 // 通知代码
				 
			switch (Id) {
				case GUI_ID_LISTVIEW0:								// ListView
					switch (NCode) {
						case WM_NOTIFICATION_RELEASED:
						Sel = LISTVIEW_GetSel(pMsg->hWinSrc);
							if(Sel == 0)
							{
								Mesg_New("0");
							}
							else
							{
								Mesg_Read("0");
							}

							break;								
						}		//switch (NCode)					
						
				break;
			}//	switch (Id) 
			
		break;
			
		default:
			break;
		
		case WM_PAINT:
			
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(0,0,WM_GetWindowSizeX(pMsg->hWin),WM_GetWindowSizeY(pMsg->hWin));
		
			break;
		
	}
	

}

/**
  * @brief  Mesg_InBox,收件箱处理函数
  * @param  none
  * @retval none
  */
static void Mesg_InBox(void)
{
	WM_HWIN hFrame;
	WM_HWIN hFrameC;
	WM_HWIN hListView;	
	WM_HWIN hHeader;

	hFrame = FRAMEWIN_CreateEx(0,0,WinPara.xSizeWin,WinPara.ySizeWin,WinPara.hWinMain,WM_CF_SHOW,FRAMEWIN_CF_ACTIVE,GUI_ID_FRAMEWIN10,"In box",0);

	/* 创建窗口按钮 */
	FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);

	/* 获取框架窗口用户区的句柄 */
	hFrameC = WM_GetClientWindow(hFrame);

	FRAMEWIN_SetClientColor(hFrame,GUI_WHITE);
	
	hListView = LISTVIEW_CreateEx(0,0, WM_GetWindowSizeX(hFrameC), WM_GetWindowSizeY(hFrameC),hFrameC,WM_CF_SHOW,NULL,GUI_ID_LISTVIEW0);

	hHeader = LISTVIEW_GetHeader(hListView);
	
	HEADER_AddItem(hHeader,WM_GetWindowSizeX(hFrameC)/4,"time",GUI_TA_HCENTER);
	
	HEADER_AddItem(hHeader,WM_GetWindowSizeX(hFrameC)/4,"number",GUI_TA_HCENTER);
	
	HEADER_AddItem(hHeader,WM_GetWindowSizeX(hFrameC)/2,"shortcut",GUI_TA_HCENTER);

}

/**
  * @brief  Mesg_OutBox,发件箱处理函数
  * @param  none
  * @retval none
  */
static void Mesg_OutBox(void)
{
	WM_HWIN hFrame;
	WM_HWIN hFrameC;
	WM_HWIN hListView;	
	WM_HWIN hHeader;
	
	const GUI_ConstString NewMesg[]={"NewMesg","-","-"};

	hFrame = FRAMEWIN_CreateEx(0,0,WinPara.xSizeWin,WinPara.ySizeWin,WinPara.hWinMain,WM_CF_SHOW,FRAMEWIN_CF_ACTIVE,GUI_ID_FRAMEWIN10,"Out box",0);

	/* 创建窗口按钮 */
	FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);

	/* 获取框架窗口用户区的句柄 */
	hFrameC = WM_GetClientWindow(hFrame);

	FRAMEWIN_SetClientColor(hFrame,GUI_WHITE);	

	/* 设置信息api的回调函数 */
	WM_SetCallback(hFrameC,_cbOutBox);

	FRAMEWIN_SetClientColor(hFrame,GUI_WHITE);
	
	hListView = LISTVIEW_CreateEx(0,0, WM_GetWindowSizeX(hFrameC), WM_GetWindowSizeY(hFrameC),hFrameC,WM_CF_SHOW,NULL,GUI_ID_LISTVIEW0);


  LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/4, "Time",    GUI_TA_CENTER);
	LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/4, "Number",  GUI_TA_CENTER);
	LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/2, "Shortcut",GUI_TA_CENTER);
	
	LISTVIEW_AddRow(hListView,NewMesg);
	
	




}

/**
  * @brief  Mesg_DraftBox,草稿箱处理函数
  * @param  none
  * @retval none
  */
static void Mesg_DraftBox(void)
{
	WM_HWIN hFrame;
	WM_HWIN hFrameC;
	WM_HWIN hListView;	
	WM_HWIN hHeader;
	
	const GUI_ConstString NewDraft[]={"NewDraft","-","-"};

	hFrame = FRAMEWIN_CreateEx(0,0,WinPara.xSizeWin,WinPara.ySizeWin,WinPara.hWinMain,WM_CF_SHOW,FRAMEWIN_CF_ACTIVE,GUI_ID_FRAMEWIN10,"Draft box",0);

	/* 创建窗口按钮 */
	FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);

	/* 获取框架窗口用户区的句柄 */
	hFrameC = WM_GetClientWindow(hFrame);

	/* 设置信息api的回调函数 */
	WM_SetCallback(hFrameC,_cbDraftBox);

	FRAMEWIN_SetClientColor(hFrame,GUI_WHITE);
	
	hListView = LISTVIEW_CreateEx(0,0, WM_GetWindowSizeX(hFrameC), WM_GetWindowSizeY(hFrameC),hFrameC,WM_CF_SHOW,NULL,GUI_ID_LISTVIEW0);

	hHeader = LISTVIEW_GetHeader(hListView);
	

  LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/4, "Time",    GUI_TA_CENTER);
	LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/4, "Number",  GUI_TA_CENTER);
	LISTVIEW_AddColumn(hListView, WM_GetWindowSizeX(hFrameC)/2, "Shortcut",GUI_TA_CENTER);
	
	LISTVIEW_AddRow(hListView,NewDraft);		

}


/**
  * @brief  _cbMesgWin,短信框回调窗口
  * @param  none
  * @retval none
  */
static void _cbMesgWin(WM_MESSAGE * pMsg)
{
	WM_HWIN    hWin;
	WM_HWIN    hIcon;
	
	int        NCode;
  int        Id;
	int        Sel;
	
	hWin = pMsg->hWin;	

	switch (pMsg->MsgId) 
	{
		case WM_NOTIFY_PARENT:
		 
			Id    = WM_GetId(pMsg->hWinSrc);      // 控件的ID
			NCode = pMsg->Data.v;                 // 通知代码
				 
			switch (Id) {
				case GUI_ID_ICONVIEW0:								// 图标控件
					switch (NCode) {
						case WM_NOTIFICATION_RELEASED:
							Sel   = ICONVIEW_GetSel(pMsg->hWinSrc);
							switch(Sel)
							{
								case 0:
									GUI_Delay(500);								//短暂延时以显示触摸效果
									Mesg_InBox();	
								
									break;
								
								case 1:
									GUI_Delay(500);								//短暂延时以显示触摸效果
									Mesg_OutBox();
								
									break;
								
								case 2:
									GUI_Delay(500);								//短暂延时以显示触摸效果
									Mesg_DraftBox();
								
									break;
								
								default:
									break;	
								
							}//	switch(Sel)
							
							/* 获取ICON的句柄 */
							hIcon = WM_GetDialogItem(hWin, GUI_ID_ICONVIEW0);
						
							/* 设置初始选择的图标为 -1 (表示尚未选择)*/
							ICONVIEW_SetSel(hIcon,-1);	

							break;								
						}		//switch (NCode)					
						
				break;
			}//	switch (Id) 
			
		break;
			
		default:
			break;
		
		case WM_PAINT:
			
			GUI_SetColor(GUI_BLACK);
			GUI_FillRect(0,0,WM_GetWindowSizeX(pMsg->hWin),WM_GetWindowSizeY(pMsg->hWin));
		
			break;
		
	}
	
	
}




/**
  * @brief  WFGUI_Message,短信
  * @param  none
  * @retval none
  */
void WFGUI_Message(void)
{
	WM_HWIN hFrame;
	WM_HWIN hFrameC;
	WM_HWIN hIcon;
	
	uint8_t i;
	
	hFrame = FRAMEWIN_CreateEx(0,0,WinPara.xSizeWin,WinPara.ySizeWin,WinPara.hWinMain,WM_CF_SHOW,FRAMEWIN_CF_ACTIVE,GUI_ID_FRAMEWIN9,"Message",0);
	
	/* 创建窗口按钮 */
  FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);
	
		/* 把app句柄插入链表 */
	App_Insert(hFrame);
	WM_SendMessageNoPara(WinPara.hWinCtrl,MY_MESSAGE_CTRLCHANGE);
	
	//WM_SetCallback(hFrame,_cbSDViewWin);

	//	TBD 使用回调函数会出现无法移动框架窗口的情况
	_pcbOldMessageWin = WM_SetCallback(hFrame, _cbMessageWin);	//获取旧的回调函数指针

	
	/* 获取框架窗口用户区的句柄 */
	hFrameC = WM_GetClientWindow(hFrame);
	
	FRAMEWIN_SetClientColor(hFrame,GUI_BLACK);
	/*
  * 创建图标控件
  */
  hIcon = ICONVIEW_CreateEx(20, 0, WM_GetWindowSizeX(hFrameC)-20,  WM_GetWindowSizeY(hFrameC), 
                           hFrameC, WM_CF_SHOW | WM_CF_HASTRANS, 
                           0, GUI_ID_ICONVIEW0, 55, 70);
	
	/*
	* 添加图标到图标控件
	*/
	for (i = 0; i < GUI_COUNTOF(_MessageIcon); i++) {    

  ICONVIEW_AddBitmapItem(hIcon, _MessageIcon[i].pBitmap, _MessageIcon[i].pText);
  }		

	/* 设置选定图标的颜色 */
  ICONVIEW_SetBkColor(hIcon, ICONVIEW_CI_SEL, GUI_BLUE | 0xC0000000);
	
	/* 设置图标的字体 */
  ICONVIEW_SetFont(hIcon, &GUI_Font13B_ASCII);
	
	/* 设置初始选择的图标为 -1 (表示尚未选择)*/
	ICONVIEW_SetSel(hIcon,-1);		
	
	/* 设置信息api的回调函数 */
	WM_SetCallback(hFrameC,_cbMesgWin);
	
}


