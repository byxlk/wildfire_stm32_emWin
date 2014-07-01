#if 1

/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : WIDGET_Treeview.c
Purpose     : Demonstrates using the TREEVIEW widget
----------------------------------------------------------------------
*/

#include "DIALOG.h"
#include <stdio.h>
#include "bsp_usart1.h"


/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NUM_CHILD_NODES 1
#define NUM_CHILD_ITEMS 10
#define TREEVIEW_DEPTH  10

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static int _NumNodes;
static int _NumLeaves;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _MakeNumberString
*/
static void _MakeNumberString(char * p, U32 Number) {
  int v, NumDecs = 0;
  
  *p = '0';
  v = Number;
  while (v) {
    NumDecs++;
    v /= 10;
  }
  p += NumDecs - 1;
  while (NumDecs--) {
    *p = '0' + Number % 10;
    Number /= 10;
    p--;
  }
}

/*********************************************************************
*
*       _GetLen
*/
static int _GetLen(char * p) {
  int Len;
  
  for (Len = 0; *p++; Len++);
  return Len;
}

/*********************************************************************
*
*       _FillNode
*
* Purpose:
*   Recursive filling of node
*
* Parameters:
*   hTree     - obvious
*   NumNodes  - Number of child nodes to be created at each node
*   NumLeaves - Number of leaves to be created at each node
*   MaxDepth  - Maximum depth (1.1.1.1.1.....)
*   CurDepth  - Current depth
*   acBuffer  - String to be used for TREEVIEW items
*   p         - Pointer into to string to be used for numbering
*
* Return value:
*  0 on success, 1 on error
*/
static int _FillNode(WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode, int NumNodes, int NumLeaves, int MaxDepth, int CurDepth, char * acBuffer, char * p) {
  TREEVIEW_ITEM_Handle hItem = 0;
  int i, Position;

  *(p + 1) = 0;
  *p = '0' - 1;
  if (--CurDepth) {
    //
    // Create nodes
    //
    for (i = 0; i < NumNodes; i++) {
      (*p)++;
      Position = hItem ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD;
      hItem = TREEVIEW_ITEM_Create(1, acBuffer, 0);
      if (hItem == 0) {
        return 1; // Error
      }
      _NumNodes++;
      TREEVIEW_AttachItem(hTree, hItem, hNode, Position);
      hNode = hItem;
      *(p + 1) = '.';
      p += 2;
      //
      // Recursive call of 'this' function for each node
      //
      _FillNode(hTree, hNode, NumNodes, NumLeaves, MaxDepth, CurDepth, acBuffer, p);
      p -= 2;
      *(p + 1) = 0;
    }
  }
  //
  // Create leaves
  //
  for (i = 0; i < NumLeaves; i++) {
    (*p)++;
    Position = hItem ? TREEVIEW_INSERT_BELOW : TREEVIEW_INSERT_FIRST_CHILD;
    hItem = TREEVIEW_ITEM_Create(0, acBuffer, 0);
    if (hItem == 0) {
      return 1; // Error
    }
    _NumLeaves++;
    TREEVIEW_AttachItem(hTree, hItem, hNode, Position);
    hNode = hItem;
  }
  return 0;
}

/*********************************************************************
*
*       _MakeNumberText
*
* Purpose:
*   Create text widgets for showing the result
*/
static void _MakeNumberText(WM_HWIN hParent, int xPos, int * pyPos, int xSize, int ySize, char * pText, U32 Value) {
  WM_HWIN hText;
  int Len;

  Len = _GetLen(pText);
  _MakeNumberString(pText + Len, Value);
  hText = TEXT_CreateEx(xPos, *pyPos, xSize, ySize, hParent, WM_CF_SHOW, 0, GUI_ID_TEXT0, pText);
  *pyPos += WM_GetWindowSizeY(hText);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MainTask
*/
void ListView_MainTask(void) {
  WM_HWIN hTree;
  TREEVIEW_ITEM_Handle hNode;	
	TREEVIEW_ITEM_Handle hItem;
	
  int xSize, ySize, yPos, r, TimeStart, TimeUsed, ySizeText;
  U32 BytesFree, BytesUsed;
  char acBuffer[(TREEVIEW_DEPTH << 1) + 1];
  char acNumNodes[30]  = "Nodes:  ";
  char acNumLeaves[30] = "Leaves: ";
  char acNumTotal[30]  = "Total:  ";
  char acTimeUsed[30]  = "Time:   ";
  char acBytesUsed[30] = "Memory: ";

	
  //
  // Initialize emWin
  //
  WM_SetCreateFlags(WM_CF_MEMDEV);
  GUI_Init();
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  //
  // Set defaults for background and widgets
  //
  WM_SetDesktopColor(GUI_BLACK);
  SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
  SCROLLBAR_SetDefaultWidth(25);
  SCROLLBAR_SetThumbSizeMin(25);
  TEXT_SetDefaultFont(GUI_FONT_6X8);


  //
  // Draw info message before creating the widgets
  //
  GUI_DrawGradientV(0, 0, xSize - 1, ySize - 1, GUI_BLUE, GUI_BLACK);
  GUI_SetFont(GUI_FONT_20F_ASCII);
  GUI_DispStringHCenterAt("Filling TREEVIEW widget...", xSize >> 1, ySize / 3);
  GUI_X_Delay(1000);

  //
  // Create TREEVIEW
  //
  hTree = TREEVIEW_CreateEx(0, 0, xSize, ySize, WM_HBKWIN, WM_CF_SHOW, TREEVIEW_CF_AUTOSCROLLBAR_H, GUI_ID_TREEVIEW0);
  TREEVIEW_SetAutoScrollV(hTree, 1);
  TREEVIEW_SetSelMode(hTree, TREEVIEW_SELMODE_ROW);
//
  // Fill TREEVIEW
  //
  hNode = TREEVIEW_InsertItem(hTree, TREEVIEW_ITEM_TYPE_NODE, 0, 0, "Tree");
 
 	 hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,"crea",0);
  TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);
	
	 hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,"test",0);
  TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_BELOW);
	
	hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,"test",0);
  TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);
 
 #if 0  
  BytesFree = GUI_ALLOC_GetNumFreeBytes();
  TimeStart = GUI_GetTime();
  r = _FillNode(hTree, hNode, NUM_CHILD_NODES, NUM_CHILD_ITEMS, TREEVIEW_DEPTH, TREEVIEW_DEPTH, acBuffer, acBuffer);
  TimeUsed = GUI_GetTime() - TimeStart;
  BytesUsed = BytesFree - GUI_ALLOC_GetNumFreeBytes();

  if (r) {
    //
    // Error message
    //
    WM_DeleteWindow(hTree);
    GUI_MessageBox("Error", "Not enough memory available!", 0);
  } else {
    //
    // Show result
    //
    yPos = 20;
    ySizeText = GUI_GetYDistOfFont( TEXT_GetDefaultFont()) + 5;
    _MakeNumberText(hTree, xSize >> 1, &yPos, xSize >> 1, ySizeText, acNumNodes, _NumNodes);
    _MakeNumberText(hTree, xSize >> 1, &yPos, xSize >> 1, ySizeText, acNumLeaves, _NumLeaves);
    _MakeNumberText(hTree, xSize >> 1, &yPos, xSize >> 1, ySizeText, acNumTotal, _NumNodes + _NumLeaves);
    _MakeNumberText(hTree, xSize >> 1, &yPos, xSize >> 1, ySizeText, acTimeUsed, TimeUsed);
    _MakeNumberText(hTree, xSize >> 1, &yPos, xSize >> 1, ySizeText, acBytesUsed, BytesUsed);
    WM_SetFocus(hTree);
  }

#endif
  while (1) 
		{
    GUI_Delay(100);
  }
}

#endif 

/*************************** End of file ****************************/