/////////////////////////////////////////////////////////////////////////////
// SystemTray.cpp : source file

/*************************************** add no used for linux  ************************************

#include "stdafx.h"
#include <sys_daemon_package/SystemTray.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSystemTray, CWnd)

UINT_PTR CSystemTray::m_nIDEvent = 4567;
BOOL CSystemTray::g_bLogIn = FALSE;

/////////////////////////////////////////////////////////////////////////////
// CSystemTray construction/creation/destruction

CSystemTray::CSystemTray() {
  Initialise();
}

CSystemTray::CSystemTray(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID) {
  Initialise();
  Create(pParent, uCallbackMessage, szToolTip, icon, uID);
}

void CSystemTray::Initialise() {
  memset(&m_tnd, 0, sizeof(m_tnd));

  m_bEnabled   = FALSE;
  m_bHidden    = FALSE;
  
  m_uIDTimer   = 0;
  m_hSavedIcon = NULL;
  
  m_DefaultMenuItemID = 0;
  m_DefaultMenuItemByPos = TRUE;
  
  m_loginstatus = FALSE;
}

BOOL CSystemTray::Create(CWnd* pParent, UINT uCallbackMessage, LPCTSTR szToolTip, HICON icon, UINT uID) {
  // this is only for Windows 95 (or higher)
  VERIFY(m_bEnabled = (GetVersion() & 0xff) >= 4);
  if (!m_bEnabled) return FALSE;

  // Make sure Notification window is valid (not needed - CJM)
  // VERIFY(m_bEnabled = (pParent && ::IsWindow(pParent->GetSafeHwnd())));
  // if (!m_bEnabled) return FALSE;

  // Make sure we avoid conflict with other messages
  ASSERT(uCallbackMessage >= WM_USER);

  // Tray only supports tooltip text up to 64 characters
  ASSERT(_tcslen(szToolTip) <= 64);

  // Create an invisible window
  CWnd::CreateEx(0, AfxRegisterWndClass(0), _T(""), WS_POPUP, 0,0,10,10, NULL, 0);

  // load up the NOTIFYICONDATA structure
  m_tnd.cbSize = sizeof(NOTIFYICONDATA);
  m_tnd.hWnd   = pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
  m_tnd.uID    = uID;
  m_tnd.hIcon  = icon;
  m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  m_tnd.uCallbackMessage = uCallbackMessage;
  _tcscpy(m_tnd.szTip, szToolTip);

  // Set the tray icon
  VERIFY(m_bEnabled = Shell_NotifyIcon(NIM_ADD, &m_tnd));
  return m_bEnabled;
}

CSystemTray::~CSystemTray() {
  RemoveIcon();
  m_IconList.RemoveAll();
  DestroyWindow();
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray icon manipulation

void CSystemTray::MoveToRight() {
  HideIcon();
  ShowIcon();
}

void CSystemTray::RemoveIcon() {
  if (!m_bEnabled) return;

  m_tnd.uFlags = 0;
  Shell_NotifyIcon(NIM_DELETE, &m_tnd);
  m_bEnabled = FALSE;
}

void CSystemTray::HideIcon() {
  if (m_bEnabled && !m_bHidden) {
    m_tnd.uFlags = NIF_ICON;
    Shell_NotifyIcon(NIM_DELETE, &m_tnd);
    m_bHidden = TRUE;
  }
}

void CSystemTray::ShowIcon() {
  if (m_bEnabled && m_bHidden) {
    m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    Shell_NotifyIcon(NIM_ADD, &m_tnd);
    m_bHidden = FALSE;
  }
}

void CSystemTray::ShowIconEx() {
  RemoveIcon();

  m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;

  VERIFY(m_bEnabled = Shell_NotifyIcon(NIM_ADD, &m_tnd));
}

void CSystemTray::Duringlogin() {
  g_bLogIn = FALSE;
}


BOOL CSystemTray::SetIcon(HICON hIcon) {
  if (!m_bEnabled) return FALSE;

  m_tnd.uFlags = NIF_ICON;
  m_tnd.hIcon = hIcon;
  g_bLogIn = FALSE;
  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetIcon(LPCTSTR lpszIconName) {


  HICON hIcon = (HICON) ::LoadImage(AfxGetResourceHandle(),
                                    lpszIconName,
                                    IMAGE_ICON,
                                    0, 0,
                                    LR_LOADFROMFILE);
  if (!hIcon)
    return FALSE;

  BOOL bSuccess = SetIcon(hIcon);

  return bSuccess;
}

BOOL CSystemTray::SetIcon(UINT nIDResource) {
  HICON hIcon = AfxGetApp()->LoadIcon(nIDResource);
  if (nIDResource == IDI_ICON_ON) { // login
    m_loginstatus = TRUE;
  } else if (nIDResource == IDI_ICON_OFF) { // logout
    m_loginstatus = FALSE;
  }
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(LPCTSTR lpIconName) {
  HICON hIcon = LoadIcon(NULL, lpIconName);
  return SetIcon(hIcon);
}

BOOL CSystemTray::SetStandardIcon(UINT nIDResource) {
  HICON hIcon = LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));

  return SetIcon(hIcon);
}

HICON CSystemTray::GetIcon() const {
  return (m_bEnabled)? m_tnd.hIcon : NULL;
}

BOOL CSystemTray::SetIconList(UINT uFirstIconID, UINT uLastIconID) {
  if (uFirstIconID > uLastIconID)
    return FALSE;

  UINT uIconArraySize = uLastIconID - uFirstIconID + 1;
  const CWinApp * pApp = AfxGetApp();
  ASSERT(pApp != 0);

  m_IconList.RemoveAll();
  try {
    for (UINT i = uFirstIconID; i <= uLastIconID; i++)
      m_IconList.Add(pApp->LoadIcon(i));
  } catch (CMemoryException *e) {
    e->ReportError();
    e->Delete();
    m_IconList.RemoveAll();
    return FALSE;
  }

  return TRUE;
}

BOOL CSystemTray::SetIconList(HICON* pHIconList, UINT nNumIcons) {
  m_IconList.RemoveAll();

  try {
    for (UINT i = 0; i <= nNumIcons; i++)
      m_IconList.Add(pHIconList[i]);
  } catch (CMemoryException *e) {
    e->ReportError();
    e->Delete();
    m_IconList.RemoveAll();
    return FALSE;
  }

  return TRUE;
}

BOOL CSystemTray::Animate(UINT nDelayMilliSeconds, int nNumSeconds) {
  StopAnimation();

  m_nCurrentIcon = 0;
  m_StartTime = COleDateTime::GetCurrentTime();
  m_nAnimationPeriod = nNumSeconds;
  m_hSavedIcon = GetIcon();

  // Setup a timer for the animation
  m_uIDTimer = SetTimer(m_nIDEvent, nDelayMilliSeconds, NULL);

  return (m_uIDTimer != 0);
}

BOOL CSystemTray::StepAnimation() {
  if (!m_IconList.GetSize())
    return FALSE;

  m_nCurrentIcon++;
  if (m_nCurrentIcon >= m_IconList.GetSize())
    m_nCurrentIcon = 0;

  return SetIcon(m_IconList[m_nCurrentIcon]);
}

BOOL CSystemTray::StopAnimation() {
  BOOL bResult = FALSE;

  if (m_uIDTimer)
    bResult = KillTimer(m_uIDTimer);
  m_uIDTimer = 0;

  if (m_hSavedIcon)
    SetIcon(m_hSavedIcon);
  m_hSavedIcon = NULL;

  return bResult;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray tooltip text manipulation

BOOL CSystemTray::SetTooltipText(LPCTSTR pszTip) {
  if (!m_bEnabled) return FALSE;

  m_tnd.uFlags = NIF_TIP;
  _tcscpy(m_tnd.szTip, pszTip);

  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

BOOL CSystemTray::SetTooltipText(UINT nID) {
  CString strText;
  VERIFY(strText.LoadString(nID));

  return SetTooltipText(strText);
}

CString CSystemTray::GetTooltipText() const {
  CString strText;
  if (m_bEnabled)
    strText = m_tnd.szTip;

  return strText;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray notification window stuff

BOOL CSystemTray::SetNotificationWnd(CWnd* pWnd) {
  if (!m_bEnabled) return FALSE;

  // Make sure Notification window is valid
  ASSERT(pWnd && ::IsWindow(pWnd->GetSafeHwnd()));

  m_tnd.hWnd = pWnd->GetSafeHwnd();
  m_tnd.uFlags = 0;

  return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
}

CWnd* CSystemTray::GetNotificationWnd() const {
  return CWnd::FromHandle(m_tnd.hWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray menu manipulation

BOOL CSystemTray::SetMenuDefaultItem(UINT uItem, BOOL bByPos) {
  if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos))
    return TRUE;

  m_DefaultMenuItemID = uItem;
  m_DefaultMenuItemByPos = bByPos;

  CMenu menu, *pSubMenu;

  if (!menu.LoadMenu(m_tnd.uID)) return FALSE;
  if (!(pSubMenu = menu.GetSubMenu(0))) return FALSE;

  ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

  return TRUE;
}

void CSystemTray::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos) {
  uItem = m_DefaultMenuItemID;
  bByPos = m_DefaultMenuItemByPos;
}

/////////////////////////////////////////////////////////////////////////////
// CSystemTray message handlers

BEGIN_MESSAGE_MAP(CSystemTray, CWnd)
  //{{AFX_MSG_MAP(CSystemTray)
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CSystemTray::OnTimer(UINT_PTR nIDEvent) {
  ASSERT(nIDEvent == m_nIDEvent);

  COleDateTime CurrentTime = COleDateTime::GetCurrentTime();
  COleDateTimeSpan period = CurrentTime - m_StartTime;
  if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period.GetTotalSeconds()) {
    StopAnimation();
    return;
  }

  StepAnimation();
}

LRESULT CSystemTray::OnTrayNotification(WPARAM wParam, LPARAM lParam)
//LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent)
{
  //Return quickly if its not for this tray icon
  if (wParam != m_tnd.uID)
    return 0L;

  CMenu menu, *pSubMenu;
  CWnd* pTarget = AfxGetMainWnd();

  // Clicking with right button brings up a context menu
  if (LOWORD(lParam) == WM_RBUTTONUP) {
    if (!menu.LoadMenu(m_tnd.uID)) return 0;
    if (!(pSubMenu = menu.GetSubMenu(0))) return 0;

    // Make chosen menu item the default (bold font)
    ::SetMenuDefaultItem(pSubMenu->m_hMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

    // Display and track the popup menu
    CPoint pos;
    GetCursorPos(&pos);

    pTarget->SetForegroundWindow();
    ::TrackPopupMenu(pSubMenu->m_hMenu, 0, pos.x, pos.y, 0,
                     pTarget->GetSafeHwnd(), NULL);

    // BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
    pTarget->PostMessage(WM_NULL, 0, 0);

    menu.DestroyMenu();
  } else if (LOWORD(lParam) == WM_LBUTTONDBLCLK) {
    // double click received, the default action is to execute default menu item
    if (pTarget) pTarget->SetForegroundWindow();

    UINT uItem;
    if (m_DefaultMenuItemByPos) {
      if (!menu.LoadMenu(m_tnd.uID)) {
        return 0;
      }
      if (!(pSubMenu = menu.GetSubMenu(0))) {
        return 0;
      }
      uItem = pSubMenu->GetMenuItemID(m_DefaultMenuItemID);
    } else
      uItem = m_DefaultMenuItemID;

    if (LOWORD(lParam) == 515) {
      if (!m_loginstatus && !g_bLogIn) {
        g_bLogIn = TRUE;
        pTarget->PostMessage(WM_MENU_LOGIN_ST, 0, 0); //doble click -> LoginDlg
      }
    } else {
      if (pTarget) {
        pTarget->SendMessage(WM_COMMAND, uItem, 0);
      }
    }

    menu.DestroyMenu();
  }

  return 1;
}

LRESULT CSystemTray::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) {
  if (message == m_tnd.uCallbackMessage)
    return OnTrayNotification(wParam, lParam);

  return CWnd::WindowProc(message, wParam, lParam);
}

/*-----------------------------------------------------------------------------
Function : CSysTrayIcon::ShowBalloon()

Abstract : Shows a ballon tip over the tray icon

Parameters :
1. szBallonTitle -> Message title
2. szBalloonMsg -> Balloon message
3. nTimeOut -> Time to show the balloon, min 10 secs.
4. dwIcon -> ICON_WARNING or ICON_ERROR or ICON_INFO

Return Value : BOOL
----------------------------------------------------------------------------*

BOOL CSystemTray::ShowBalloon(PTSTR szBalloonTitle, PTSTR szBalloonMsg, DWORD dwIcon, UINT nTimeOut) {

  // First check if Icon exits
  ASSERT(szBalloonTitle != NULL); // Are you sure ballon message title == NULL??
  ASSERT(szBalloonMsg != NULL);   // Are you sure ballon message == NULL??

  VERIFY((nTimeOut *= 1000) >= 10000); // in Windows 2000 nTimeout is 10 seconds,
  // If even set less no point, so a check
  // just in case.

  ASSERT(::strcmp(szBalloonTitle, _T("")) != 0); // Sure that balloon title == "" ??
  ASSERT(::strcmp(szBalloonMsg, _T("")) != 0); // Sure that balloon message == "" ?

  ASSERT(dwIcon == NIIF_WARNING || dwIcon == NIIF_ERROR || dwIcon == NIIF_INFO || dwIcon == NIIF_NONE);

  m_tnd.dwInfoFlags = dwIcon;
  BOOL bBalloonShown = FALSE;
  m_tnd.uFlags = 0;

  // Set the flags for showing balloon, espcially NIF_INFO
  m_tnd.uFlags |= NIF_INFO;

  // Set the time out for the balloon
  m_tnd.uTimeout = nTimeOut;

  // Set the balloon title
  _tcscpy(m_tnd.szInfoTitle, szBalloonTitle);

  // Set balloon message
  _tcscpy(m_tnd.szInfo, szBalloonMsg);

  // Show balloon....
  bBalloonShown = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

  return bBalloonShown;
}

*************************************** add no used for linux  ************************************/