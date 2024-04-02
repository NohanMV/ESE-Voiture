/*********************************************************************
*                                                                    *
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
*                                                                    *
**********************************************************************
*                                                                    *
* C-file generated by:                                               *
*                                                                    *
*        GUI_Builder for emWin version 5.30                          *
*        Compiled Jul  1 2015, 10:50:32                              *
*        (c) 2015 Segger Microcontroller GmbH & Co. KG               *
*                                                                    *
**********************************************************************
*                                                                    *
*        Internet: www.segger.com  Support: support@segger.com       *
*                                                                    *
**********************************************************************
*/

// USER START (Optionally insert additional includes)
#include "stdio.h"
// USER END

#include "DIALOG.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_WINDOW_0 (GUI_ID_USER + 0x00)
#define ID_TEXT_0 (GUI_ID_USER + 0x01)
#define ID_TEXT_1 (GUI_ID_USER + 0x02)
#define ID_BUTTON_0 (GUI_ID_USER + 0x03)
#define ID_TEXT_2 (GUI_ID_USER + 0x04)
#define ID_TEXT_3 (GUI_ID_USER + 0x05)
#define ID_TEXT_4 (GUI_ID_USER + 0x06)
#define ID_TEXT_5 (GUI_ID_USER + 0x07)
#define ID_TEXT_6 (GUI_ID_USER + 0x08)


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
extern int variable; 

extern int identifiant;
extern int retour;
extern char data_buf[8];
extern int   taille,i;
extern char tab[20];
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect, "Window", ID_WINDOW_0, 19, 2, 480, 272, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Supervision IT2R", ID_TEXT_0, 30, 20, 160, 23, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Id :", ID_TEXT_1, 25, 146, 157, 20, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "Son", ID_BUTTON_0, 348, 128, 100, 40, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Valeur", ID_TEXT_2, 26, 175, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "rfid", ID_TEXT_3, 30, 60, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "valeur_rfid", ID_TEXT_4, 80, 60, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "IA", ID_TEXT_5, 30, 80, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "vision IA", ID_TEXT_6, 80, 80, 80, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  WM_HWIN hItem;
  int     NCode;
  int     Id;
  // USER START (Optionally insert additional variables)
  // USER END

  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    //
    // Initialization of 'Supervision IT2R'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, GUI_FONT_20B_1);
    //
    // Initialization of 'Id :'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    //
    // Initialization of 'Valeur'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    //
    // Initialization of 'rfid'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetFont(hItem, GUI_FONT_16B_1);
    //
    // Initialization of 'valeur_rfid'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    //
    // Initialization of 'IA'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
    TEXT_SetFont(hItem, GUI_FONT_16B_1);
    //
    // Initialization of 'vision IA'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_6);
    TEXT_SetFont(hItem, GUI_FONT_16_1);
    // USER START (Optionally insert additional code for further widget initialization)
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Son'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
				variable=1;
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
		if (identifiant==0x404)
	{
		if(retour==0xff) sprintf(tab,"ouverture") ;
		else sprintf(tab,"alarme");
		hItem = WM_GetDialogItem(pMsg->hWin,ID_TEXT_4 ); //direction 
		TEXT_SetText(hItem, tab );
	}
		else if (identifiant ==0x169)
	{
		if (retour==0x0f) sprintf(tab,"Panneau 130") ;
		else if (retour==0x0e) sprintf(tab,"Panneau 50") ;
		else if (retour==0x10) sprintf(tab,"Panneau interdiction") ;
		else if (retour==0x07) sprintf(tab,"Feu rouge") ;
		else if (retour==0x11) sprintf(tab,"Panneau STOP") ;
		hItem = WM_GetDialogItem(pMsg->hWin,ID_TEXT_6 ); //direction 
		TEXT_SetText(hItem, tab );
	}
	else
	{
	sprintf(tab,"id %x ",(short)identifiant) ;
	hItem = WM_GetDialogItem(pMsg->hWin,ID_TEXT_1 ); //diirection 
  TEXT_SetText(hItem, tab ); 
	
	sprintf(tab,"valeur %x ",(short)retour) ;
	hItem = WM_GetDialogItem(pMsg->hWin,ID_TEXT_2 ); //direction 
	TEXT_SetText(hItem, tab );
	}
	
	
}


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateWindow
*/
WM_HWIN CreateWindow(void);
WM_HWIN CreateWindow(void) {
  WM_HWIN hWin;

  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
